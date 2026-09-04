"""Module for analyze_filtered.py."""

import wave

import numpy as np

wav_file = "latest_filtered_audio.wav"
with wave.open(wav_file, "rb") as wav:
    frames = wav.readframes(wav.getnframes())
    audio = np.frombuffer(frames, dtype=np.int16)

print(f"Max value: {np.max(audio)}")
print(f"Min value: {np.min(audio)}")
print(f"Mean: {np.mean(audio):.2f}")
print(f"RMS: {np.sqrt(np.mean(audio.astype(np.float64) ** 2)):.2f}")

zero_crossings = np.sum(np.abs(np.diff(np.sign(audio)))) / 2
print(f"ZCR: {zero_crossings / len(audio):.4f}")

fft = np.fft.rfft(audio)
freqs = np.fft.rfftfreq(len(audio), 1.0 / 16000)
peak_idx = np.argmax(np.abs(fft))
print(f"Peak Frequency: {freqs[peak_idx]:.2f} Hz")
