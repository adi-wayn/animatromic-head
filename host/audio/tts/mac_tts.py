"""macOS native TTS strategy using the 'say' command."""

import os
import subprocess

from loguru import logger

from .base import TTSStrategy


class MacTTSStrategy(TTSStrategy):
    """
    Ultra-fast TTS using macOS built-in 'say' command.
    Generates audio in <0.1s for zero-latency conversations.
    """

    def __init__(self, voice="Samantha"):
        self.voice = voice
        self._is_interrupted = False
        logger.info(f"MacTTSStrategy initialized with voice: {voice}")

    def synthesize(self, text: str) -> bytes:
        self._is_interrupted = False
        logger.debug(f"Mac TTS generating audio for: {text}")

        temp_file = "/tmp/mac_tts_out.wav"

        try:
            # Generate 16kHz LEI16 WAV directly
            subprocess.run(
                [
                    "say",
                    "-v",
                    self.voice,
                    "-o",
                    temp_file,
                    "--data-format=LEI16@16000",
                    text,
                ],
                check=True,
            )

            with open(temp_file, "rb") as f:
                wav_bytes = f.read()

            # Cleanup
            os.remove(temp_file)
            return wav_bytes

        except Exception as e:
            logger.error(f"Mac TTS generation failed: {e}")
            return b""

    def stop(self):
        self._is_interrupted = True

    def synthesize_stream(self, text: str):
        self._is_interrupted = False
        audio = self.synthesize(text)
        if not self._is_interrupted and audio:
            yield audio
