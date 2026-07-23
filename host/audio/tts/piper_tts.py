import os
import io
import wave
import subprocess
import logging
from .base import TTSStrategy

logger = logging.getLogger(__name__)

class PiperTTS(TTSStrategy):
    """
    Piper TTS Implementation for ultra-low latency fallback.
    Downloads and uses an ONNX model natively via Piper CLI/package.
    """
    def __init__(self, model_name: str = "en_US-bryce-medium"):
        self.model_name = model_name
        self.model_path = f"models/{model_name}.onnx"
        self._is_interrupted = False
        
        # Ensure models directory exists
        os.makedirs("models", exist_ok=True)
        
        # Download model if it doesn't exist
        if not os.path.exists(self.model_path):
            logger.info(f"Downloading Piper model {model_name}...")
            subprocess.run(
                ["curl", "-L", f"https://huggingface.co/rhasspy/piper-voices/resolve/main/en/en_US/bryce/medium/{model_name}.onnx", "-o", self.model_path],
                check=True
            )
            subprocess.run(
                ["curl", "-L", f"https://huggingface.co/rhasspy/piper-voices/resolve/main/en/en_US/bryce/medium/{model_name}.onnx.json", "-o", f"{self.model_path}.json"],
                check=True
            )
            logger.info("Piper model downloaded successfully.")

    def synthesize(self, text: str) -> bytes:
        self._is_interrupted = False
        logger.debug(f"Piper generating audio for: {text}")
        
        # Generate audio using piper-tts via subprocess to capture stdout
        # piper -m models/en_US-bryce-medium.onnx --output_raw
        cmd = ["piper", "-m", self.model_path, "--output_raw"]
        
        process = subprocess.Popen(
            cmd,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )
        
        raw_audio, stderr_data = process.communicate(input=text.encode('utf-8'))
        
        if process.returncode != 0:
            logger.error(f"Piper TTS failed: {stderr_data.decode('utf-8')}")
            return b""
            
        # Piper outputs 16-bit 16kHz raw PCM by default for many models (bryce is 22050Hz usually)
        # We must package it as WAV bytes to keep it standard for the player, or just return raw bytes + sample rate.
        # Since we use --output_raw, it's just raw PCM bytes.
        
        # We need the sample rate to construct a WAV. Let's read from the json config.
        import json
        with open(f"{self.model_path}.json", "r") as f:
            config = json.load(f)
            sample_rate = config["audio"]["sample_rate"]
            
        # Build wav in memory
        wav_io = io.BytesIO()
        with wave.open(wav_io, 'wb') as wav_file:
            wav_file.setnchannels(1)
            wav_file.setsampwidth(2)
            wav_file.setframerate(sample_rate)
            wav_file.writeframes(raw_audio)
            
        return wav_io.getvalue()

    def stop(self):
        self._is_interrupted = True
