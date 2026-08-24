from .base import TTSStrategy
from piper.voice import PiperVoice
from loguru import logger
import os
import wave
import io

class PiperTTSStrategy(TTSStrategy):
    """
    Piper TTS integration for ultra-fast, local, offline neural TTS.
    Uses ONNX models.
    """
    def __init__(self, model_path: str = "models/en_US-bryce-medium.onnx"):
        self._is_interrupted = False
        self.model_path = os.path.abspath(model_path)
        
        logger.info(f"Loading Piper TTS model: {self.model_path}")
        if not os.path.exists(self.model_path):
            logger.error(f"Piper model not found at {self.model_path}")
            self.voice = None
            return
            
        self.voice = PiperVoice.load(self.model_path)
        logger.info("Piper TTS model loaded successfully.")

    def synthesize(self, text: str) -> bytes:
        self._is_interrupted = False
        if not self.voice:
            return b""
            
        try:
            logger.debug(f"Piper synthesizing: {text}")
            
            wav_io = io.BytesIO()
            with wave.open(wav_io, 'wb') as wav_file:
                # Piper outputs 16-bit mono PCM. 
                # The sample rate depends on the voice (often 16000 or 22050)
                wav_file.setnchannels(1)
                wav_file.setsampwidth(2)
                wav_file.setframerate(self.voice.config.sample_rate)
                
                # Synthesize directly writes to the wave file
                self.voice.synthesize(text, wav_file)
                
            return wav_io.getvalue()
        except Exception as e:
            logger.error(f"Piper TTS synthesis failed: {e}")
            return b""
            
    def synthesize_stream(self, text: str):
        """
        Piper synthesizes extremely fast. We can just yield the full sentence.
        """
        self._is_interrupted = False
        audio = self.synthesize(text)
        if not self._is_interrupted and audio:
            yield audio
            
    def stop(self):
        self._is_interrupted = True
