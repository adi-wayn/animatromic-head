import os
import io
import wave
import torch
from loguru import logger
from typing import Optional
from .base import TTSStrategy
from TTS.api import TTS
from TTS.tts.configs.xtts_config import XttsConfig
from TTS.tts.models.xtts import XttsAudioConfig, XttsArgs




class XTTSStrategy(TTSStrategy):
    """
    Coqui XTTS v2 Implementation.
    Highly expressive, zero-shot cloning, but slower TTFB.
    """
    def __init__(self, reference_audio: str = "assets/scary_voice.wav"):
        self.reference_audio = reference_audio
        self._is_interrupted = False
        self.tts = None
        
        # We lazy-load the model to prevent massive memory usage if not used
        # and to handle cases where the TTS package is missing/failing.
        try:
            from TTS.api import TTS
            import torch
            
            # Use MPS (Metal Performance Shaders) on Apple Silicon if available
            device = "mps" if torch.backends.mps.is_available() else "cpu"
            logger.info(f"Loading XTTS v2 on device: {device}...")
            
            # Patch torch.load temporarily to bypass weights_only=True limitation in PyTorch 2.6
            original_load = torch.load
            
            def safe_load(*args, **kwargs):
                kwargs['weights_only'] = False
                return original_load(*args, **kwargs)
                
            torch.load = safe_load
            try:
                # Load the model
                self.tts = TTS("tts_models/multilingual/multi-dataset/xtts_v2").to(device)
            finally:
                torch.load = original_load
                
            logger.info("XTTS v2 loaded successfully.")
            
        except Exception as e:
            logger.error(f"Failed to load XTTS v2. Check dependencies. Error: {e}")
            self.tts = None

    def synthesize(self, text: str) -> bytes:
        self._is_interrupted = False
        
        if not self.tts:
            logger.warning("XTTS v2 is not loaded. Returning empty bytes to force fallback.")
            return b""
            
        if not os.path.exists(self.reference_audio):
            logger.warning(f"Reference audio {self.reference_audio} not found. Returning empty bytes.")
            return b""

        logger.debug(f"XTTS generating audio for: {text}")
        
        try:
            # Generate audio as a python list of floats (normalized PCM)
            wav_list = self.tts.tts(
                text=text,
                speaker_wav=self.reference_audio,
                language="en"
            )
            
            # Convert float list to 16-bit PCM bytes
            import numpy as np
            audio_np = np.array(wav_list, dtype=np.float32)
            audio_int16 = (audio_np * 32767.0).astype(np.int16)
            
            # XTTS v2 outputs at 24000 Hz
            sample_rate = 24000
            
            wav_io = io.BytesIO()
            with wave.open(wav_io, 'wb') as wav_file:
                wav_file.setnchannels(1)
                wav_file.setsampwidth(2)
                wav_file.setframerate(sample_rate)
                wav_file.writeframes(audio_int16.tobytes())
                
            return wav_io.getvalue()
            
        except Exception as e:
            logger.error(f"XTTS generation failed: {e}")
            return b""

    def stop(self):
        self._is_interrupted = True
