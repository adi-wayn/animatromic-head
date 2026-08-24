import sys
import os
import io
import wave
import torch
sys.path.append(os.path.dirname(os.path.abspath(__file__)))
from audio.tts.xtts_tts import XTTSStrategy

def test_offline():
    print("Loading XTTS...")
    tts = XTTSStrategy(reference_audio="assets/short_reference.wav")
    text = "Of course I hear you! Do you think I'm some kind of robot that can't process sound waves?"
    
    print(f"Synthesizing: {text}")
    audio_bytes = tts.synthesize(text)
    
    if not audio_bytes:
        print("Failed to generate audio bytes.")
        return
        
    out_path = "test_offline.wav"
    with open(out_path, "wb") as f:
        f.write(audio_bytes)
        
    print(f"Saved to {out_path}!")
    
if __name__ == "__main__":
    test_offline()
