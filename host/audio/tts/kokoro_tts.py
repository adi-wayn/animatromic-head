"""Kokoro TTS generation strategy."""

import io
import os
import urllib.request

import numpy as np
import soundfile as sf
from loguru import logger

from .base import TTSStrategy


class KokoroTTSStrategy(TTSStrategy):
    """
    Kokoro TTS integration (via kokoro-onnx).
    Blazing fast, state-of-the-art voice cloning-like quality with zero latency.
    """

    def __init__(self, voice: str = "am_echo"):
        self._is_interrupted = False
        self.voice = voice
        self.kokoro = None

        try:
            from kokoro_onnx import Kokoro
        except ImportError:
            logger.error("kokoro-onnx is not installed. Run 'uv pip install kokoro-onnx soundfile'")
            return

        model_dir = os.path.join(
            os.path.dirname(os.path.dirname(os.path.dirname(__file__))), "models"
        )
        os.makedirs(model_dir, exist_ok=True)

        self.model_path = os.path.join(model_dir, "kokoro-v1.0.onnx")
        self.voices_path = os.path.join(model_dir, "voices-v1.0.bin")

        # Auto-download models if missing
        if not os.path.exists(self.model_path):
            logger.info("Downloading Kokoro ONNX model (this happens only once)...")
            urllib.request.urlretrieve(
                "https://github.com/thewh1teagle/kokoro-onnx/releases/download/model-files-v1.1/kokoro-v1.0.onnx",
                self.model_path,
            )
        if not os.path.exists(self.voices_path):
            logger.info("Downloading Kokoro voices metadata...")
            urllib.request.urlretrieve(
                "https://github.com/thewh1teagle/kokoro-onnx/releases/download/model-files-v1.1/voices-v1.0.bin",
                self.voices_path,
            )

        logger.info(f"Loading Kokoro TTS model (Voice: {self.voice})...")

        import platform

        from kokoro_onnx import EspeakConfig

        espeak_config = None
        if platform.system() == "Darwin" and os.path.exists(
            "/opt/homebrew/lib/libespeak-ng.1.dylib"
        ):
            logger.info("macOS Apple Silicon detected: using Homebrew espeak-ng to avoid path bugs")
            espeak_config = EspeakConfig(
                data_path="/opt/homebrew/share/espeak-ng-data",
                lib_path="/opt/homebrew/lib/libespeak-ng.1.dylib",
            )

        self.kokoro = Kokoro(self.model_path, self.voices_path, espeak_config=espeak_config)
        logger.info("Kokoro TTS loaded successfully.")

    def synthesize(self, text: str) -> bytes:
        self._is_interrupted = False
        if not self.kokoro:
            return b""

        try:
            logger.debug(f"Kokoro synthesizing: {text}")

            # Create audio samples
            samples, sample_rate = self.kokoro.create(
                text, voice=self.voice, speed=1.0, lang="en-us"
            )

            # Amplify volume (Kokoro output is typically a bit quiet)
            # Reduced to 1.8x per user feedback (2.5x was too loud)
            samples = samples * 1.8
            samples = np.clip(samples, -1.0, 1.0)

            # Write to WAV bytes
            wav_io = io.BytesIO()
            sf.write(wav_io, samples, sample_rate, format="WAV", subtype="PCM_16")

            return wav_io.getvalue()
        except Exception as e:
            logger.error(f"Kokoro TTS synthesis failed: {e}")
            return b""

    def synthesize_stream(self, text: str):
        """
        Yields the full synthesized WAV to ensure compatibility.
        """
        self._is_interrupted = False
        audio = self.synthesize(text)
        if not self._is_interrupted and audio:
            yield audio

    def stop(self):
        self._is_interrupted = True
