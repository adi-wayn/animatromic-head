"""
Test Script: Record 5 seconds of audio from the ESP32 INMP441 mic.
Run this AFTER flashing the firmware and wiring the microphone.

Usage:
    cd <project_root>
    python -m host.audio.test_mic_recording
"""
import socket
import wave
import time
import sys
sys.path.insert(0, '.')
from host.protocol.messages import PORT_AUDIO_UPLINK, AUDIO_SAMPLE_RATE_HZ, AUDIO_BIT_DEPTH, AUDIO_CHANNELS

DURATION_SEC = 5
OUTPUT_FILE = "test_mic_recording.wav"

def main():
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(("0.0.0.0", PORT_AUDIO_UPLINK))
    sock.settimeout(10.0)
    
    print(f"Listening on UDP port {PORT_AUDIO_UPLINK}...")
    print(f"Recording for {DURATION_SEC} seconds. Speak into the INMP441!")
    
    frames = []
    start = time.time()
    
    try:
        while time.time() - start < DURATION_SEC:
            try:
                data, addr = sock.recvfrom(4096)
                frames.append(data)
            except socket.timeout:
                print("No data received. Is the ESP32 running and connected?")
                break
    except KeyboardInterrupt:
        pass
    
    sock.close()
    
    if not frames:
        print("No audio data received!")
        return
    
    audio_data = b"".join(frames)
    print(f"Captured {len(audio_data)} bytes ({len(audio_data) / (AUDIO_SAMPLE_RATE_HZ * 2):.1f}s)")
    
    with wave.open(OUTPUT_FILE, "wb") as wf:
        wf.setnchannels(AUDIO_CHANNELS)
        wf.setsampwidth(AUDIO_BIT_DEPTH // 8)
        wf.setframerate(AUDIO_SAMPLE_RATE_HZ)
        wf.writeframes(audio_data)
    
    print(f"Saved to {OUTPUT_FILE} — play it to verify audio quality!")

if __name__ == "__main__":
    main()
