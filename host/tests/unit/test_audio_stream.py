import socket
import time
import math
import struct

ESP32_IP = "animatronic-head.local"  # Or replace with explicit IP like "192.168.1.100"
PORT_AUDIO_DOWNLINK = 4212
AUDIO_SAMPLE_RATE_HZ = 16000
AUDIO_CHUNK_SIZE_BYTES = 1024

def generate_tone_chunk(frequency=440.0, volume=0.5, phase=0.0):
    """Generates one chunk (512 samples) of a sine wave tone."""
    num_samples = AUDIO_CHUNK_SIZE_BYTES // 2
    chunk = bytearray()
    
    for i in range(num_samples):
        # Calculate sine wave value [-1.0, 1.0]
        sample = math.sin(phase)
        
        # Advance phase
        phase += 2 * math.pi * frequency / AUDIO_SAMPLE_RATE_HZ
        if phase > 2 * math.pi:
            phase -= 2 * math.pi
            
        # Scale to 16-bit integer range (-32768 to 32767) and apply volume
        pcm_value = int(sample * 32767 * volume)
        
        # Pack as 16-bit little-endian
        chunk.extend(struct.pack('<h', pcm_value))
        
    return bytes(chunk), phase

def main():
    print(f"Resolving {ESP32_IP}...")
    try:
        ip = socket.gethostbyname(ESP32_IP)
        print(f"Resolved to {ip}")
    except socket.gaierror:
        print(f"Could not resolve {ESP32_IP}. Make sure the ESP32 is on and connected to WiFi.")
        print("You can manually edit ESP32_IP in this script if you know its IP address.")
        return

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    
    print("\n=======================================================")
    print(f"Streaming continuous 440Hz test tone to {ip}:{PORT_AUDIO_DOWNLINK}")
    print("This will simulate the AI speaking continuously.")
    print("Check the ESP32 Serial Monitor for '[AudioDownlink] ACTIVE' logs.")
    print("Probe the MAX98357A amplifier outputs with your LED now!")
    print("Press Ctrl+C to stop.")
    print("=======================================================\n")
    
    phase = 0.0
    chunk_duration_sec = AUDIO_CHUNK_SIZE_BYTES / (AUDIO_SAMPLE_RATE_HZ * 2)
    
    try:
        packet_count = 0
        while True:
            # Generate the next chunk of the sine wave
            chunk, phase = generate_tone_chunk(frequency=440.0, volume=1.0, phase=phase)
            
            # Send to ESP32
            sock.sendto(chunk, (ip, PORT_AUDIO_DOWNLINK))
            packet_count += 1
            
            if packet_count % 30 == 0:  # Roughly once a second
                print(f"Sent {packet_count} packets... (Streaming)")
                
            # Sleep to match the exact hardware playback rate
            time.sleep(chunk_duration_sec)
            
    except KeyboardInterrupt:
        print("\nTest stopped by user.")
    finally:
        sock.close()

if __name__ == "__main__":
    main()
