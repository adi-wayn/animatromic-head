import socket
import wave
import struct

UDP_IP = "0.0.0.0"
UDP_PORT = 4211

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

print(f"Listening for ESP32 audio on UDP port {UDP_PORT}...")
print("Make sure you are running the MAIN firmware (not the standalone test) and the ESP32 is connected to Wi-Fi.")
print("Press Ctrl+C to stop recording and save the file.")

frames = []
try:
    while True:
        data, addr = sock.recvfrom(2048)
        # Skip the 6-byte header from AudioManager::sendToHost
        pcm_data = data[6:]
        frames.append(pcm_data)
except KeyboardInterrupt:
    print("\nSaving test_recording.wav...")
    
    with wave.open("test_recording.wav", "wb") as wf:
        wf.setnchannels(1)
        wf.setsampwidth(2) # 16-bit
        wf.setframerate(32000)
        wf.writeframes(b"".join(frames))
        
    print("Done! You can now play test_recording.wav")
