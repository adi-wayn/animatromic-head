import os
import sys
import time

from loguru import logger

# Setup minimal path
sys.path.append(os.path.join(os.path.dirname(__file__)))

from audio.tts.kokoro_tts import KokoroTTSStrategy


def test():
    tts = KokoroTTSStrategy(voice="am_echo")  # deep, authoritative voice
    start = time.time()
    audio = tts.synthesize("I am Kokoro, the new standard for open source text to speech.")
    end = time.time()

    logger.info(f"Generated {len(audio)} bytes in {end - start:.4f} seconds.")
    if len(audio) > 1000:
        logger.info("Success! The TTFB is insanely fast!")


if __name__ == "__main__":
    test()
