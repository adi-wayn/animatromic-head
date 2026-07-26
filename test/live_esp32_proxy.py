import socket
import pyaudio
import threading
import json
import time

# Host address (where the cognitive pipeline is running)
# By default we assume the host is running locally during simulation
HOST_IP = "127.0.0.1"

# Ports matching host/protocol/messages.py
PORT_CONTROL = 4210
PORT_AUDIO_UPLINK = 4211
PORT_AUDIO_DOWNLINK = 4212

# Audio Constants matching host schema
AUDIO_SAMPLE_RATE_HZ = 16000
AUDIO_CHUNK_SIZE_BYTES = 1024
CHANNELS = 1

def audio_uplink_worker():
    """Captures microphone and sends via UDP to the host."""
    p = pyaudio.PyAudio()
    stream = p.open(format=pyaudio.paInt16,
                    channels=CHANNELS,
                    rate=AUDIO_SAMPLE_RATE_HZ,
                    input=True,
                    frames_per_buffer=AUDIO_CHUNK_SIZE_BYTES // 2)

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    print(f"[Uplink] Sending live microphone data to {HOST_IP}:{PORT_AUDIO_UPLINK}...")

    try:
        while True:
            # Read from PC microphone
            data = stream.read(AUDIO_CHUNK_SIZE_BYTES // 2, exception_on_overflow=False)
            # Send to Host
            sock.sendto(data, (HOST_IP, PORT_AUDIO_UPLINK))
    except KeyboardInterrupt:
        pass
    except Exception as e:
        print(f"[Uplink] Error: {e}")
    finally:
        stream.stop_stream()
        stream.close()
        p.terminate()

def audio_downlink_worker():
    """Listens for TTS audio from host and plays it to speakers."""
    p = pyaudio.PyAudio()
    stream = p.open(format=pyaudio.paInt16,
                    channels=CHANNELS,
                    rate=AUDIO_SAMPLE_RATE_HZ,
                    output=True)

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    # Bind to all interfaces so we receive packets from the Host
    sock.bind(("0.0.0.0", PORT_AUDIO_DOWNLINK))
    print(f"[Downlink] Listening for TTS audio on port {PORT_AUDIO_DOWNLINK}...")

    try:
        while True:
            # Receive TTS from Host
            data, addr = sock.recvfrom(AUDIO_CHUNK_SIZE_BYTES * 4)
            # Play live to PC speakers
            stream.write(data)
    except KeyboardInterrupt:
        pass
    except Exception as e:
        print(f"[Downlink] Error: {e}")
    finally:
        stream.stop_stream()
        stream.close()
        p.terminate()

def control_worker():
    """Listens for kinematic intents and prints them."""
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(("0.0.0.0", PORT_CONTROL))
    print(f"[Control] Listening for kinematics/intents on port {PORT_CONTROL}...")

    try:
        while True:
            data, addr = sock.recvfrom(4096)
            try:
                msg = json.loads(data.decode('utf-8'))
                print(f"\n[KINEMATICS] Received Intent: {msg}")
            except Exception as e:
                print(f"[KINEMATICS] Failed to parse JSON: {e}")
    except KeyboardInterrupt:
        pass
    except Exception as e:
        print(f"[Control] Error: {e}")

if __name__ == "__main__":
    print("Starting Live ESP32 Proxy Simulation...")
    print("This script mocks the physical ESP32 and I2S modules.")
    print("Run your host environment with: ESP32_IP=127.0.0.1 uv run host/main.py")
    print("Press Ctrl+C to stop.\n")
    
    t1 = threading.Thread(target=audio_uplink_worker, daemon=True)
    t2 = threading.Thread(target=audio_downlink_worker, daemon=True)
    t3 = threading.Thread(target=control_worker, daemon=True)
    
    t1.start()
    t2.start()
    t3.start()
    
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("\nShutting down proxy.")
