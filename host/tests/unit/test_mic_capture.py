import socket
import struct
import math
import wave
import json
import time
import sys

ESP32_IP = "animatronic-head.local"  # Replace if needed
PORT_CONTROL = 4210
PORT_AUDIO_UPLINK = 4211
AUDIO_SAMPLE_RATE_HZ = 16000
AUDIO_CHUNK_SIZE_BYTES = 1024

def draw_vu_meter(rms, max_rms=32768.0):
    """Draws a simple text-based VU meter."""
    bars = int((rms / max_rms) * 50)
    bars = min(50, max(0, bars))
    meter = "#" * bars + "-" * (50 - bars)
    sys.stdout.write(f"\r[VU] {rms:7.1f} | {meter} |")
    sys.stdout.flush()

def main():
    print(f"Resolving {ESP32_IP}...")
    try:
        esp_ip = socket.gethostbyname(ESP32_IP)
        print(f"Resolved ESP32 to {esp_ip}")
    except socket.gaierror:
        print(f"Could not resolve {ESP32_IP}. Make sure it is on.")
        return

    # 1. Setup the receiving socket
    rx_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    rx_sock.bind(("0.0.0.0", PORT_AUDIO_UPLINK))
    rx_sock.settimeout(2.0)  # 2 second timeout for silence

    # 2. Setup the control socket to wake up the ESP32
    # The ESP32 only sends audio to the IP that last sent it a control packet!
    ctrl_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    wakeup_msg = {
        "type": "PHASE_UPDATE",
        "payload": {"conversational_phase": "LISTENING"}
    }
    
    print("\n=======================================================")
    print(f"Listening for INMP441 Microphone data on UDP {PORT_AUDIO_UPLINK}")
    print("Talk or snap your fingers near the mic to trigger it!")
    print("Press Ctrl+C to stop and save the recording.")
    print("=======================================================\n")

    # Send wake up packet to ESP32 so it learns our IP
    ctrl_sock.sendto(json.dumps(wakeup_msg).encode('utf-8'), (esp_ip, PORT_CONTROL))

    recorded_audio = bytearray()
    packets_received = 0
    last_wakeup = time.time()

    try:
        while True:
            # Re-ping the ESP32 every 5 seconds so it doesn't go to sleep
            if time.time() - last_wakeup > 5.0:
                ctrl_sock.sendto(json.dumps(wakeup_msg).encode('utf-8'), (esp_ip, PORT_CONTROL))
                last_wakeup = time.time()

            try:
                data, addr = rx_sock.recvfrom(4096)
                if len(data) > 0:
                    packets_received += 1
                    recorded_audio.extend(data)
                    
                    # Calculate RMS for VU meter
                    # Data is 16-bit little-endian Mono
                    num_samples = len(data) // 2
                    if num_samples > 0:
                        samples = struct.unpack(f'<{num_samples}h', data)
                        sum_sq = sum(float(s) * float(s) for s in samples)
                        rms = math.sqrt(sum_sq / num_samples)
                        draw_vu_meter(rms)
                        
            except socket.timeout:
                sys.stdout.write("\r[VU] Silence... (Waiting for noise to trigger mic)           ")
                sys.stdout.flush()

    except KeyboardInterrupt:
        print("\n\nTest stopped by user.")
    
    finally:
        rx_sock.close()
        ctrl_sock.close()
        
        if packets_received > 0:
            filename = "test_mic_recording.wav"
            print(f"Saving {packets_received} packets to {filename}...")
            with wave.open(filename, 'wb') as wf:
                wf.setnchannels(1)
                wf.setsampwidth(2) # 16-bit
                wf.setframerate(AUDIO_SAMPLE_RATE_HZ)
                wf.writeframes(recorded_audio)
            print("Done! You can play this file on your Mac to hear what the INMP441 captured.")
        else:
            print("No audio data was received. Check your wiring and power.")

if __name__ == "__main__":
    main()
