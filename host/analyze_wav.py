import wave
import struct
import numpy as np
import matplotlib.pyplot as plt
from scipy.fft import fft, fftfreq

wav_file = "analysis_output.wav"

with wave.open(wav_file, 'rb') as wf:
    n_frames = wf.getnframes()
    sample_rate = wf.getframerate()
    audio_data = wf.readframes(n_frames)
    
audio_np = np.frombuffer(audio_data, dtype=np.int16)

# Time Domain
t = np.linspace(0, len(audio_np) / sample_rate, num=len(audio_np))

fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 8))

ax1.plot(t, audio_np, color='blue', alpha=0.7)
ax1.set_title("Time-Domain Waveform")
ax1.set_xlabel("Time (s)")
ax1.set_ylabel("Amplitude (16-bit PCM)")
ax1.grid(True)

# Frequency Domain (FFT)
N = len(audio_np)
yf = fft(audio_np)
xf = fftfreq(N, 1 / sample_rate)[:N//2]
magnitude = 2.0/N * np.abs(yf[0:N//2])

ax2.plot(xf, magnitude, color='red', alpha=0.7)
ax2.set_title("Frequency-Domain Spectrum")
ax2.set_xlabel("Frequency (Hz)")
ax2.set_ylabel("Magnitude")
ax2.grid(True)

plt.tight_layout()
out_path = "/Users/testmac/.gemini/antigravity-cli/brain/cc7d5eca-4bd0-41f4-8b66-28ed11475a96/fft_plot.png"
plt.savefig(out_path)
print(f"Plot saved to {out_path}")

# Print dominant frequencies
peaks_indices = np.argsort(magnitude)[-10:][::-1]
print("\nTop 10 Dominant Frequencies (Hz):")
for idx in peaks_indices:
    print(f" - {xf[idx]:.1f} Hz (Magnitude: {magnitude[idx]:.2f})")

