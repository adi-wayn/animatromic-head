import socket
import struct
import numpy as np
import time
import sys
import wave

# --- Configuration ---
PORT = 4211
ESP32_CTRL_PORT = 4210
# Now dynamically matching the new 32000Hz sampling rate
SAMPLE_RATE = 32000
DURATION_SEC = 10
BYTES_PER_SAMPLE = 2
CHUNK_SIZE_BYTES = 2048
EXPECTED_BYTES = SAMPLE_RATE * DURATION_SEC * BYTES_PER_SAMPLE
HEADER_SIZE = 6

def collect_and_analyze(duration, esp32_ip):
    # 1. Setup the receiving socket (RX)
    rx_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    rx_sock.bind(("0.0.0.0", PORT))
    rx_sock.settimeout(2.0)
    
    # 2. Setup the control socket (TX)
    ctrl_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    
    wakeup_msg = b'{"type": "TELEMETRY", "payload": {}}'
    
    audio_data = bytearray()
    target_bytes = SAMPLE_RATE * duration * 2 # 16-bit = 2 bytes per sample
    
    print(f"Sending wakeup packet to {esp32_ip}:4210...")
    
    start_time = time.time()
    last_wakeup = 0
    packets_received = 0
    
    print(f"Listening on UDP {PORT} for {duration} seconds of audio. Please SPEAK NOW...")
    
    try:
        while len(audio_data) < target_bytes:
            # Re-ping every 2 seconds
            if time.time() - last_wakeup > 2.0:
                ctrl_sock.sendto(wakeup_msg, (esp32_ip, 4210))
                last_wakeup = time.time()
                
            try:
                data, addr = rx_sock.recvfrom(4096)
                if len(data) > HEADER_SIZE:
                    packets_received += 1
                    payload = data[HEADER_SIZE:]
                    audio_data.extend(payload)
                    sys.stdout.write(f"\rCollected {len(audio_data)} / {target_bytes} bytes...")
                    sys.stdout.flush()
            except socket.timeout:
                pass # Just loop and re-ping
                
    except KeyboardInterrupt:
        print("\nInterrupted.")
        
    rx_sock.close()
    ctrl_sock.close()
    
    print(f"\nFinished collection. Received {packets_received} packets.")
    if len(audio_data) == 0:
        print("Error: No audio data received at all.")
        return
        
    # Analyze the data
    audio_np = np.frombuffer(audio_data, dtype=np.int16)
    
    # Mathematical Analysis
    peak_amplitude = np.max(np.abs(audio_np))
    rms = np.sqrt(np.mean(audio_np.astype(np.float64)**2))
    dc_offset = np.mean(audio_np)
    zero_crossings = np.sum(np.diff(np.sign(audio_np)) != 0)
    
    print("\n=== MATHEMATICAL ANALYSIS ===")
    print(f"Total Samples:  {len(audio_np)}")
    print(f"Peak Amplitude: {peak_amplitude} (Max is 32767)")
    print(f"RMS Amplitude:  {rms:.2f}")
    print(f"DC Offset:      {dc_offset:.2f}")
    print(f"Zero Crossings: {zero_crossings}")
    print("=============================")
    
    if peak_amplitude == 0:
        print("WARNING: The audio is COMPLETELY SILENT (all zeros).")
    elif rms < 10:
        print("WARNING: The audio is basically pure silence/dead.")
    elif peak_amplitude > 32000:
        print("WARNING: The audio is CLIPPING (too loud/distorted).")
    else:
        print("SUCCESS: The audio has a healthy dynamic range!")
        
    # Plotting
    try:
        import matplotlib.pyplot as plt
        from scipy.fft import fft, fftfreq
        
        print("Plotting data...")
        t = np.linspace(0, len(audio_np) / SAMPLE_RATE, num=len(audio_np))
        
        fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 8))
        
        # Time Domain
        ax1.plot(t, audio_np, color='blue', alpha=0.7)
        ax1.set_title("Time-Domain Waveform (Look for distinct speech envelopes vs static noise)")
        ax1.set_xlabel("Time (s)")
        ax1.set_ylabel("Amplitude (16-bit PCM)")
        ax1.set_ylim([-32768, 32767])
        ax1.grid(True)
        
        # Frequency Domain (FFT)
        N = len(audio_np)
        yf = fft(audio_np)
        xf = fftfreq(N, 1 / SAMPLE_RATE)[:N//2]
        
        ax2.plot(xf, 2.0/N * np.abs(yf[0:N//2]), color='red', alpha=0.7)
        ax2.set_title("Frequency-Domain Spectrum (Human Speech is 300Hz - 3400Hz)")
        ax2.set_xlabel("Frequency (Hz)")
        ax2.set_ylabel("Magnitude")
        ax2.grid(True)
        
        output_file = "audio_analysis.png"
        plt.tight_layout()
        plt.savefig(output_file)
        print(f"Plot saved to {output_file}")
    except ImportError:
        print("matplotlib or scipy not installed, skipping plot. (Run: pip install matplotlib scipy)")
        
    # Save to WAV
    wav_file = "analysis_output.wav"
    with wave.open(wav_file, 'wb') as wf:
        wf.setnchannels(1)
        wf.setsampwidth(2)
        wf.setframerate(SAMPLE_RATE)
        wf.writeframes(audio_data)
    print(f"Audio saved to {wav_file} so you can play it to verify.")

if __name__ == "__main__":
    import sys
    esp_ip = sys.argv[1] if len(sys.argv) > 1 else "animatronic-head.local"
    try:
        esp_ip = socket.gethostbyname(esp_ip)
    except Exception:
        pass
    print("Ensure the ESP32 is powered on and connected to Wi-Fi.")
    collect_and_analyze(DURATION_SEC, esp_ip)
