import os
import time
import urllib.request

import soundfile as sf
from kokoro_onnx import Kokoro


def test_kokoro():
    # Download weights
    if not os.path.exists("kokoro-v0_19.onnx"):
        print("Downloading Kokoro...")
        urllib.request.urlretrieve(
            "https://github.com/thewh1teagle/kokoro-onnx/releases/download/model-files-v0.19/kokoro-v0_19.onnx",
            "kokoro-v0_19.onnx",
        )
        urllib.request.urlretrieve(
            "https://github.com/thewh1teagle/kokoro-onnx/releases/download/model-files-v0.19/voices-v0.19.json",
            "voices-v0.19.json",
        )

    kokoro = Kokoro("kokoro-v0_19.onnx", "voices-v0.19.json")

    start = time.time()
    samples, sample_rate = kokoro.create(
        "Hello. This voice is fast and it actually works on Mac.",
        voice="af_sarah",
        speed=1.0,
        lang="en-us",
    )
    end = time.time()

    print(f"Generated {len(samples)} samples in {end - start:.4f} seconds.")
    sf.write("kokoro_out.wav", samples, sample_rate)


test_kokoro()
