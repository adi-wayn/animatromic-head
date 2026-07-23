import io
import wave
import time
import pyaudio
import numpy as np
import logging
import concurrent.futures
from typing import Optional

from .xtts_tts import XTTSStrategy
from .piper_tts import PiperTTS
from tools.esp32_adapter import send_kinematic_intent, _adapter

logger = logging.getLogger(__name__)

class DualTTSManager:
    """
    Manages the XTTS -> Piper fallback strategy and handles
    Lip-Sync playback via RMS amplitude extraction.
    """
    def __init__(self):
        self.xtts = XTTSStrategy()
        self.piper = PiperTTS()
        self.pyaudio_instance = pyaudio.PyAudio()
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

        # 2. Play Audio and calculate Lip-Sync
        self._play_with_lip_sync(audio_bytes)
        
    def interrupt(self):
        """Called by VAD when user speaks."""
        logger.info("DualTTSManager interrupted!")
        self.is_interrupted = True
        self.xtts.stop()
        self.piper.stop()

    def _play_with_lip_sync(self, audio_bytes: bytes):
        """
        Reads the WAV bytes, plays them, calculates RMS volume,
        and blasts JAW intents to the ESP32.
        """
        try:
            wav_io = io.BytesIO(audio_bytes)
            with wave.open(wav_io, 'rb') as wf:
                format_type = self.pyaudio_instance.get_format_from_width(wf.getsampwidth())
                channels = wf.getnchannels()
                rate = wf.getframerate()
                
                stream = self.pyaudio_instance.open(
                    format=format_type,
                    channels=channels,
                    rate=rate,
                    output=True
                )
                
                chunk_size = 1024
                data = wf.readframes(chunk_size)
                
                # JAW scaling factor (adjust to make mouth open wider)
                # Max 16-bit PCM value is 32768
                RMS_SCALER = 32768.0 
                
                while data and not self.is_interrupted:
                    # 1. Play chunk
                    stream.write(data)
                    
                    # 2. Calculate RMS for lip-sync
                    audio_np = np.frombuffer(data, dtype=np.int16).astype(np.float32)
                    if len(audio_np) > 0:
                        rms = np.sqrt(np.mean(np.square(audio_np)))
                        # Normalize intensity between 0.0 and 1.0
                        intensity = min(1.0, rms / RMS_SCALER * 3.0) # Multiply by 3 for sensitivity
                        
                        # 3. Dispatch JAW intent
                        # We use the raw _adapter to avoid LangGraph tool overhead in a fast loop
                        _adapter.send_intent("JAW", intensity=intensity)
                    
                    # Read next chunk
                    data = wf.readframes(chunk_size)
                    
                stream.stop_stream()
                stream.close()
                
                # Close jaw at the end
                _adapter.send_intent("JAW", intensity=0.0)
                
        except Exception as e:
            logger.error(f"Playback error: {e}")

# Singleton instance
dual_tts_manager = DualTTSManager()
