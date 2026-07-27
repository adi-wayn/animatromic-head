import socket
import threading
import sounddevice as sd
import numpy as np
import time
import queue

HOST_IP = "127.0.0.1"
PORT_AUDIO_UPLINK = 4211
PORT_AUDIO_DOWNLINK = 4212

SAMPLE_RATE = 16000
CHANNELS = 1
CHUNK_SIZE_BYTES = 1024
SAMPLES_PER_CHUNK = CHUNK_SIZE_BYTES // 2

# State variables for Echo Cancellation & Jitter buffering
last_speaker_time = 0.0
audio_buffer = queue.Queue(maxsize=50)

print("Starting Mock ESP32 Hardware (Mic & Speaker) with Anti-Echo & Jitter Buffer...")

def mic_uplink():
    """Captures PC Mic and streams to Host over UDP."""
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    
    def callback(indata, frames, time_info, status):
        # ECHO CANCELLATION: If the speaker played audio in the last 1.0 second, send silence!
        if time.time() - last_speaker_time < 1.0:
            indata = np.zeros_like(indata)
            
        pcm_bytes = (indata * 32767).astype(np.int16).tobytes()
        header = b'\x00' * 6
        
        try:
            sock.sendto(header + pcm_bytes, (HOST_IP, PORT_AUDIO_UPLINK))
        except Exception:
            pass

    try:
        with sd.InputStream(samplerate=SAMPLE_RATE, channels=CHANNELS, dtype='float32', 
                            blocksize=SAMPLES_PER_CHUNK, callback=callback):
            while True:
                time.sleep(1)
    except Exception as e:
        print(f"[Mock Mic] Error starting microphone: {e}")

def speaker_downlink():
    """Listens for Host UDP audio and plays to PC Speaker via a jitter buffer."""
    global last_speaker_time
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(("0.0.0.0", PORT_AUDIO_DOWNLINK))
    
    def audio_callback(outdata, frames, time_info, status):
        try:
            # Pull from jitter buffer
            data = audio_buffer.get_nowait()
            outdata[:] = data.reshape(-1, 1)
        except queue.Empty:
            outdata.fill(0)
            
    try:
        stream = sd.OutputStream(samplerate=SAMPLE_RATE, channels=CHANNELS, dtype='int16', 
                                 blocksize=SAMPLES_PER_CHUNK, callback=audio_callback)
        stream.start()
        print(f"[Mock Speaker] Listening on UDP {PORT_AUDIO_DOWNLINK}...")
        
        while True:
            data, addr = sock.recvfrom(2048)
            pcm_np = np.frombuffer(data, dtype=np.int16)
            last_speaker_time = time.time()
            
            if not audio_buffer.full():
                audio_buffer.put_nowait(pcm_np)
                
    except Exception as e:
        print(f"[Mock Speaker] Error: {e}")
    finally:
        try:
            stream.stop()
            stream.close()
        except:
            pass

if __name__ == "__main__":
    t_mic = threading.Thread(target=mic_uplink, daemon=True)
    t_mic.start()
    speaker_downlink()
