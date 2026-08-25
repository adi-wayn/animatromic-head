import sys
import os
import wave
import numpy as np

# Add host to path to import host modules
sys.path.append(os.path.dirname(os.path.abspath(__file__)))

from audio.tts.dual_tts_manager import dual_tts_manager

phrases = [
    ("I see you there.", "voice1.wav"),
    ("Don't move.", "voice2.wav"),
    ("Are you looking at me?", "voice3.wav")
]

for text, filename in phrases:
    print(f"Synthesizing: {text}")
    audio_bytes = dual_tts_manager.tts.synthesize(text)
    
    # Save to wav
    with wave.open(filename, 'wb') as wf:
        wf.setnchannels(1)
        wf.setsampwidth(2)
        wf.setframerate(24000) # XTTS default
        wf.writeframes(audio_bytes)
        
    print(f"Saved {filename}")

print("Done.")
