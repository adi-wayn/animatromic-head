"""Piper TTS generation strategy."""

import os
import subprocess

from loguru import logger

from .base import TTSStrategy


class PiperTTSStrategy(TTSStrategy):
    """
    Piper TTS integration using the 'piper' CLI binary.
    This avoids the espeak-ng-data path issues on macOS while retaining Piper's zero-latency speed.
    """

    def __init__(self, model_path: str = "models/en_GB-alan-medium.onnx"):
        self._is_interrupted = False
        self.model_path = os.path.abspath(model_path)

        logger.info(f"Loading Piper TTS (CLI Strategy) model: {self.model_path}")
        if not os.path.exists(self.model_path):
            logger.error(f"Piper model not found at {self.model_path}")

    def synthesize(self, text: str) -> bytes:
        self._is_interrupted = False
        if not os.path.exists(self.model_path):
            return b""

        try:
            logger.debug(f"Piper CLI synthesizing: {text}")

            # Use standalone piper CLI to write WAV bytes to stdout
            piper_bin = os.path.join(
                os.path.dirname(os.path.dirname(os.path.dirname(__file__))),
                "bin",
                "piper",
                "piper",
            )

            cmd = [
                piper_bin,
                "--model",
                self.model_path,
                "--output_file",
                "-",  # Write to stdout
            ]

            process = subprocess.Popen(
                cmd,
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
            )

            stdout_data, stderr_data = process.communicate(input=text.encode("utf-8"))

            if process.returncode != 0:
                logger.error(f"Piper CLI error: {stderr_data.decode('utf-8')}")
                return b""

            return stdout_data
        except Exception as e:
            logger.error(f"Piper TTS synthesis failed: {e}")
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
