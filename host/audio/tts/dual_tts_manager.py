import io
import wave
import time
import json
import socket as _socket
import numpy as np
from loguru import logger
import concurrent.futures
from typing import Optional
from scipy.signal import resample_poly

from .mac_tts import MacTTSStrategy
from adapters.esp32_adapter import send_kinematic_intent, _adapter, ESP32_IP
from protocol.messages import (
    PORT_AUDIO_DOWNLINK, PORT_CONTROL,
    AUDIO_SAMPLE_RATE_HZ, AUDIO_CHUNK_SIZE_BYTES,
    create_tts_complete_message,
)

class DualTTSManager:
    """
    Manages the Mac TTS strategy and handles
    Lip-Sync playback via RMS amplitude extraction.
    Audio is streamed to the ESP32 MAX98357A over UDP (port 4212).
    """
    def __init__(self):
        self._tts = None
        self.udp_sock = _socket.socket(_socket.AF_INET, _socket.SOCK_DGRAM)
        self.is_interrupted = False
        self.is_speaking_active = False
        self.last_speaking_end_time = 0.0
        self.executor = concurrent.futures.ThreadPoolExecutor(max_workers=2)

    @property
    def tts(self):
        if self._tts is None:
            import os
            # Fully decoupled TTS selection via Environment Variable (defaults to Piper with Alan voice)
            tts_provider = os.getenv("TTS_PROVIDER", "kokoro").lower()
            
            if tts_provider == "xtts":
                from .xtts_tts import XTTSStrategy
                self._tts = XTTSStrategy()
            elif tts_provider == "kokoro":
                from .kokoro_tts import KokoroTTSStrategy
                voice = os.getenv("KOKORO_VOICE", "am_echo")
                self._tts = KokoroTTSStrategy(voice=voice)
            elif tts_provider == "piper":
                from .piper_tts import PiperTTSStrategy
                model = os.getenv("PIPER_MODEL", "models/en_GB-alan-medium.onnx")
                self._tts = PiperTTSStrategy(model_path=model)
            elif tts_provider == "mac":
                from .mac_tts import MacTTSStrategy
                self._tts = MacTTSStrategy()
            else:
                raise ValueError(f"Unknown TTS provider: {tts_provider}")
                
        return self._tts

    def speak(self, text: str):
        """
        Public method to be called by the `speak` LangGraph tool.
        """
        import re
        import queue
        from core.metrics import turn_metrics
        turn_metrics.mark_llm_end()
        
        self.is_interrupted = False
        self.is_speaking_active = True
        
        try:
            # 1. Split text into sentences for pipelined generation
            sentences = [s.strip() for s in re.split(r'(?<=[.!?]) +', text) if s.strip()]
            if not sentences:
                sentences = [text]
                
            logger.info(f"Attempting TTS generation (pipelined over {len(sentences)} sentences)...")
            
            audio_queue = queue.Queue()
            
            def generator_worker():
                for sentence in sentences:
                    if self.is_interrupted:
                        break
                    try:
                        # Iterate through sub-sentence audio chunks
                        for chunk_bytes in self.tts.synthesize_stream(sentence):
                            if self.is_interrupted:
                                break
                            if chunk_bytes:
                                audio_queue.put(chunk_bytes)
                    except Exception as e:
                        logger.error(f"TTS pipeline failed on sentence: {e}")
                audio_queue.put(None) # EOF marker
                
            gen_thread = self.executor.submit(generator_worker)
            
            # 2. Play Audio chunks as they arrive
            while not self.is_interrupted:
                try:
                    audio_bytes = audio_queue.get(timeout=1.0)
                    if audio_bytes is None:
                        break # EOF
                    self._play_with_lip_sync(audio_bytes)
                except queue.Empty:
                    if gen_thread.done():
                        break
                        
            # 3. Robustly close the jaw to ensure it doesn't get stuck
            for _ in range(3):
                _adapter.send_intent("JAW_CLOSE", intensity=0.0)
                time.sleep(0.05)
                
            # Send TTS_COMPLETE to signal end of playback
            if not self.is_interrupted:
                msg = create_tts_complete_message()
                tts_data = json.dumps(msg.model_dump()).encode('utf-8')
                ctrl_sock = _socket.socket(_socket.AF_INET, _socket.SOCK_DGRAM)
                ctrl_sock.sendto(tts_data, (ESP32_IP, PORT_CONTROL))
                ctrl_sock.close()
                logger.info("Sent TTS_COMPLETE to ESP32.")
                
        finally:
            if hasattr(self, 'full_response_pcm') and len(self.full_response_pcm) > 0:
                try:
                    import wave
                    with wave.open("debug_ai_output.wav", 'wb') as wf:
                        wf.setnchannels(1)
                        wf.setsampwidth(2)
                        wf.setframerate(24000)
                        wf.writeframes(self.full_response_pcm)
                except Exception:
                    pass
                self.full_response_pcm = b""
                
            self.is_speaking_active = False
            self.last_speaking_end_time = time.time()
            
    def speak_stream(self, sentence_queue):
        """
        Processes sentences as they arrive in the queue (streaming from LLM).
        """
        import queue
        from core.metrics import turn_metrics
        # TTFB is measured when the FIRST chunk of audio hits the hardware, not here.
        
        self.is_interrupted = False
        self.is_speaking_active = True
        
        try:
            audio_queue = queue.Queue()
            
            def generator_worker():
                while not self.is_interrupted:
                    sentence = sentence_queue.get()
                    if sentence is None: # EOF
                        break
                    if not sentence.strip():
                        continue
                        
                    logger.info(f"TTS generating sentence: {sentence}")
                    try:
                        for chunk_bytes in self.tts.synthesize_stream(sentence):
                            if self.is_interrupted:
                                break
                            if chunk_bytes:
                                audio_queue.put(chunk_bytes)
                    except Exception as e:
                        logger.error(f"TTS pipeline failed on sentence: {e}")
                        
                audio_queue.put(None) # EOF marker
                
            gen_thread = self.executor.submit(generator_worker)
            
            # Play Audio chunks as they arrive
            while not self.is_interrupted:
                try:
                    audio_bytes = audio_queue.get(timeout=1.0)
                    if audio_bytes is None:
                        break # EOF
                    self._play_with_lip_sync(audio_bytes)
                except queue.Empty:
                    if gen_thread.done():
                        break
                        
            # Robustly close the jaw to ensure it doesn't get stuck
            for _ in range(3):
                _adapter.send_intent("JAW", intensity=0.0)
                time.sleep(0.05)
                
            # Send TTS_COMPLETE to signal end of playback
            if not self.is_interrupted:
                msg = create_tts_complete_message()
                tts_data = json.dumps(msg.model_dump()).encode('utf-8')
                ctrl_sock = _socket.socket(_socket.AF_INET, _socket.SOCK_DGRAM)
                ctrl_sock.sendto(tts_data, (ESP32_IP, PORT_CONTROL))
                ctrl_sock.close()
                logger.info("Sent TTS_COMPLETE to ESP32.")
                
        finally:
            if hasattr(self, 'full_response_pcm') and len(self.full_response_pcm) > 0:
                try:
                    import wave
                    with wave.open("debug_ai_output.wav", 'wb') as wf:
                        wf.setnchannels(1) # Assuming XTTS output is mono 24kHz originally
                        wf.setsampwidth(2)
                        wf.setframerate(24000) # Assuming XTTS default rate for raw_pcm
                        wf.writeframes(self.full_response_pcm)
                    logger.info("Saved complete AI TTS audio to debug_ai_output.wav")
                except Exception as e:
                    logger.error(f"Failed to dump debug_ai_output.wav: {e}")
                self.full_response_pcm = b"" # Reset for next turn
                
            self.is_speaking_active = False
            self.last_speaking_end_time = time.time()
            
    def interrupt(self):
        """Called by VAD when user speaks."""
        logger.info("DualTTSManager interrupted!")
        self.is_interrupted = True
        self.tts.stop()
        
        # Instantly tell the ESP32 to flush its audio buffer and stop lip-syncing
        try:
            from adapters.esp32_adapter import send_emergency_stop
            send_emergency_stop()
            logger.info("Emergency stop intent sent to edge.")
        except Exception as e:
            logger.error(f"Failed to send emergency stop: {e}")

    def _play_with_lip_sync(self, audio_bytes: bytes):
        """
        Reads WAV bytes, resamples to 16kHz, streams raw PCM to ESP32
        via UDP port 4212, and sends JAW lip-sync intents.
        """
        from core.metrics import turn_metrics
        turn_metrics.mark_tts_start()
        
        try:
            wav_io = io.BytesIO(audio_bytes)
            with wave.open(wav_io, 'rb') as wf:
                channels = wf.getnchannels()
                sampwidth = wf.getsampwidth()
                source_rate = wf.getframerate()
                raw_pcm = wf.readframes(wf.getnframes())

            # Accumulate for debugging
            if not hasattr(self, 'full_response_pcm'):
                self.full_response_pcm = b""
            self.full_response_pcm += raw_pcm

            # Decode to numpy
            audio_np = np.frombuffer(raw_pcm, dtype=np.int16).astype(np.float32)

            # Mix to mono if stereo
            if channels > 1:
                audio_np = audio_np.reshape(-1, channels).mean(axis=1)

            # Resample to protocol rate (16kHz) if needed
            if source_rate != AUDIO_SAMPLE_RATE_HZ:
                import math
                gcd = math.gcd(AUDIO_SAMPLE_RATE_HZ, source_rate)
                up = AUDIO_SAMPLE_RATE_HZ // gcd
                down = source_rate // gcd
                audio_np = resample_poly(audio_np, up, down)

            # Boost volume by 2.5x for the MAX98357A hardware
            audio_np = audio_np * 2.5
            np.clip(audio_np, -32768.0, 32767.0, out=audio_np)

            # Convert back to int16 PCM
            pcm_data = audio_np.astype(np.int16).tobytes()

            # Stream in 1024-byte chunks with real-time pacing
            chunk_duration_sec = AUDIO_CHUNK_SIZE_BYTES / (AUDIO_SAMPLE_RATE_HZ * 2)
            RMS_SCALER = 32768.0
            dest = (ESP32_IP, PORT_AUDIO_DOWNLINK)

            offset = 0
            while offset < len(pcm_data) and not self.is_interrupted:
                chunk = pcm_data[offset:offset + AUDIO_CHUNK_SIZE_BYTES]
                
                if len(chunk) < AUDIO_CHUNK_SIZE_BYTES:
                    chunk += b'\x00' * (AUDIO_CHUNK_SIZE_BYTES - len(chunk))
                
                self.udp_sock.sendto(chunk, dest)

                chunk_np = np.frombuffer(chunk, dtype=np.int16).astype(np.float32)
                if len(chunk_np) > 0:
                    rms = np.sqrt(np.mean(np.square(chunk_np)))
                    intensity = min(1.0, rms / RMS_SCALER * 3.0)
                    
                    # Threshold for neural TTS background noise
                    if intensity < 0.05:
                        intensity = 0.0
                        
                    # _adapter.send_intent("JAW", intensity=intensity) // Removed to prevent UDP flood, ESP32 does RMS natively


                offset += AUDIO_CHUNK_SIZE_BYTES
                time.sleep(chunk_duration_sec * 0.8)

        except Exception as e:
            logger.error(f"Playback error: {e}")

# Singleton instance
dual_tts_manager = DualTTSManager()
