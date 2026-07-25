import io
import wave
import time
import json
import socket as _socket
import numpy as np
import logging
import concurrent.futures
from typing import Optional
from scipy.signal import resample as scipy_resample

from .xtts_tts import XTTSStrategy
from .piper_tts import PiperTTS
from tools.esp32_adapter import send_kinematic_intent, _adapter, ESP32_IP
from host.protocol.messages import (
    PORT_AUDIO_DOWNLINK, PORT_CONTROL,
    AUDIO_SAMPLE_RATE_HZ, AUDIO_CHUNK_SIZE_BYTES,
    create_tts_complete_message,
)

logger = logging.getLogger(__name__)

class DualTTSManager:
    """
    Manages the XTTS -> Piper fallback strategy and handles
    Lip-Sync playback via RMS amplitude extraction.
    Audio is streamed to the ESP32 MAX98357A over UDP (port 4212).
    """
    def __init__(self):
        self.xtts = XTTSStrategy()
        self.piper = PiperTTS()
        self.udp_sock = _socket.socket(_socket.AF_INET, _socket.SOCK_DGRAM)
        self.is_interrupted = False
        self.executor = concurrent.futures.ThreadPoolExecutor(max_workers=2)

    def speak(self, text: str):
        """
        Public method to be called by the `speak` LangGraph tool.
        """
        self.is_interrupted = False
        
        # 1. Start XTTS generation with a 1.5s timeout
        logger.info("Attempting XTTS generation...")
        start_time = time.time()
        
        audio_bytes = b""
        future = self.executor.submit(self.xtts.synthesize, text)
        
        try:
            audio_bytes = future.result(timeout=1.5)
        except concurrent.futures.TimeoutError:
            logger.warning("XTTS TTFB exceeded 1.5s limit! Falling back to Piper TTS.")
            self.xtts.stop() # Tell XTTS to abort
            audio_bytes = self.piper.synthesize(text)
        except Exception as e:
            logger.error(f"XTTS Failed: {e}. Falling back to Piper TTS.")
            audio_bytes = self.piper.synthesize(text)
            
        if not audio_bytes:
            # If XTTS returned empty (e.g. not loaded), fallback to Piper
            logger.info("XTTS returned empty. Falling back to Piper TTS.")
            audio_bytes = self.piper.synthesize(text)
            
        if not audio_bytes:
            logger.error("Both XTTS and Piper failed to generate audio.")
            return

        # 2. Stream Audio to ESP32 and calculate Lip-Sync
        self._play_with_lip_sync(audio_bytes)
        
    def interrupt(self):
        """Called by VAD when user speaks."""
        logger.info("DualTTSManager interrupted!")
        self.is_interrupted = True
        self.xtts.stop()
        self.piper.stop()

    def _play_with_lip_sync(self, audio_bytes: bytes):
        """
        Reads WAV bytes, resamples to 16kHz, streams raw PCM to ESP32
        via UDP port 4212, and sends JAW lip-sync intents.
        """
        try:
            wav_io = io.BytesIO(audio_bytes)
            with wave.open(wav_io, 'rb') as wf:
                channels = wf.getnchannels()
                sampwidth = wf.getsampwidth()
                source_rate = wf.getframerate()
                raw_pcm = wf.readframes(wf.getnframes())

            # Decode to numpy
            audio_np = np.frombuffer(raw_pcm, dtype=np.int16).astype(np.float32)

            # Mix to mono if stereo
            if channels > 1:
                audio_np = audio_np.reshape(-1, channels).mean(axis=1)

            # Resample to protocol rate (16kHz) if needed
            if source_rate != AUDIO_SAMPLE_RATE_HZ:
                num_target_samples = int(len(audio_np) * AUDIO_SAMPLE_RATE_HZ / source_rate)
                audio_np = scipy_resample(audio_np, num_target_samples)
                logger.debug(f"Resampled {source_rate}Hz → {AUDIO_SAMPLE_RATE_HZ}Hz")

            # Convert back to int16 PCM
            pcm_data = audio_np.astype(np.int16).tobytes()

            # Stream in 1024-byte chunks with real-time pacing
            chunk_duration_sec = AUDIO_CHUNK_SIZE_BYTES / (AUDIO_SAMPLE_RATE_HZ * 2)  # ~0.032s
            RMS_SCALER = 32768.0
            dest = (ESP32_IP, PORT_AUDIO_DOWNLINK)

            offset = 0
            while offset < len(pcm_data) and not self.is_interrupted:
                chunk = pcm_data[offset:offset + AUDIO_CHUNK_SIZE_BYTES]
                
                # Pad last chunk with silence if needed
                if len(chunk) < AUDIO_CHUNK_SIZE_BYTES:
                    chunk += b'\x00' * (AUDIO_CHUNK_SIZE_BYTES - len(chunk))
                
                # 1. Send chunk to ESP32
                self.udp_sock.sendto(chunk, dest)

                # 2. Calculate RMS for lip-sync (same logic as before)
                chunk_np = np.frombuffer(chunk, dtype=np.int16).astype(np.float32)
                if len(chunk_np) > 0:
                    rms = np.sqrt(np.mean(np.square(chunk_np)))
                    intensity = min(1.0, rms / RMS_SCALER * 3.0)
                    _adapter.send_intent("JAW", intensity=intensity)

                offset += AUDIO_CHUNK_SIZE_BYTES
                
                # 3. Pace to match real-time playback
                time.sleep(chunk_duration_sec)

            # Close jaw at the end
            _adapter.send_intent("JAW", intensity=0.0)
            
            # Send TTS_COMPLETE to signal end of playback
            if not self.is_interrupted:
                msg = create_tts_complete_message()
                tts_data = json.dumps(msg.model_dump()).encode('utf-8')
                ctrl_sock = _socket.socket(_socket.AF_INET, _socket.SOCK_DGRAM)
                ctrl_sock.sendto(tts_data, (ESP32_IP, PORT_CONTROL))
                ctrl_sock.close()
                logger.info("Sent TTS_COMPLETE to ESP32.")

        except Exception as e:
            logger.error(f"Playback error: {e}")

# Singleton instance
dual_tts_manager = DualTTSManager()
