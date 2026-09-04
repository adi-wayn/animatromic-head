import io
import os
import sys
import time
import wave

from loguru import logger

# Setup minimal path
sys.path.append(os.path.join(os.path.dirname(__file__)))

from audio.tts.piper_tts import PiperTTSStrategy


def test_piper():
    tts = PiperTTSStrategy(model_path="models/en_GB-alan-medium.onnx")

    start = time.time()
    logger.info("Generating audio...")
    audio = tts.synthesize(
        "Hello! This is a test of the Piper TTS system. Do you find my voice intimidating?"
    )
    end = time.time()

    logger.info(f"Generated {len(audio)} bytes in {end - start:.4f} seconds.")

    if len(audio) > 0:
        wav = wave.open(io.BytesIO(audio), "rb")
        logger.info(f"Sample Rate: {wav.getframerate()} Hz")
        logger.info("Success!")
    else:
        logger.error("Failed to generate audio.")


if __name__ == "__main__":
    test_piper()
