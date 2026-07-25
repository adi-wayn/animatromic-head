"""
Test Script: Stream a WAV file to the ESP32 MAX98357A speaker.
Run this AFTER flashing the firmware and wiring the MAX98357A.

Usage:
    cd <project_root>
    python -m host.audio.test_speaker_stream <path_to_wav>
"""
import socket
import wave
import time
import sys
import json
import numpy as np
from scipy.signal import resample as scipy_resample

sys.path.insert(0, '.')
from host.protocol.messages import (
    PORT_AUDIO_DOWNLINK, PORT_CONTROL,
    AUDIO_SAMPLE_RATE_HZ, AUDIO_CHUNK_SIZE_BYTES,
    create_tts_complete_message,
)
from host.tools.esp32_adapter import ESP32_IP

def main():
    if len(sys.argv) < 2:
        print("Usage: python -m host.audio.test_speaker_stream <path_to_wav>")
        sys.exit(1)
    
    wav_path = sys.argv[1]
    
    with wave.open(wav_path, 'rb') as wf:
        source_rate = wf.getframerate()
        channels = wf.getnchannels()
        raw = wf.readframes(wf.getnframes())
    
    audio = np.frombuffer(raw, dtype=np.int16).astype(np.float32)
    
    # Mix to mono if stereo
    if channels > 1:
        audio = audio.reshape(-1, channels).mean(axis=1)
    
    # Resample to protocol rate if needed
    if source_rate != AUDIO_SAMPLE_RATE_HZ:
        n = int(len(audio) * AUDIO_SAMPLE_RATE_HZ / source_rate)
        audio = scipy_resample(audio, n)
        print(f"Resampled {source_rate}Hz → {AUDIO_SAMPLE_RATE_HZ}Hz")
    
    pcm = audio.astype(np.int16).tobytes()
    
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    dest = (ESP32_IP, PORT_AUDIO_DOWNLINK)
    chunk_dur = AUDIO_CHUNK_SIZE_BYTES / (AUDIO_SAMPLE_RATE_HZ * 2)
    
    print(f"Streaming {len(pcm)} bytes to {ESP32_IP}:{PORT_AUDIO_DOWNLINK}")
    print(f"Duration: {len(pcm) / (AUDIO_SAMPLE_RATE_HZ * 2):.1f}s")
    
    offset = 0
    chunks_sent = 0
    while offset < len(pcm):
        chunk = pcm[offset:offset + AUDIO_CHUNK_SIZE_BYTES]
        if len(chunk) < AUDIO_CHUNK_SIZE_BYTES:
            chunk += b'\x00' * (AUDIO_CHUNK_SIZE_BYTES - len(chunk))
        sock.sendto(chunk, dest)
        offset += AUDIO_CHUNK_SIZE_BYTES
        chunks_sent += 1
        time.sleep(chunk_dur)
    
    print(f"Sent {chunks_sent} chunks.")
    
    # Send TTS_COMPLETE
    msg = create_tts_complete_message()
    ctrl = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    ctrl.sendto(json.dumps(msg.model_dump()).encode(), (ESP32_IP, PORT_CONTROL))
    ctrl.close()
    sock.close()
    print("Done! TTS_COMPLETE sent.")

if __name__ == "__main__":
    main()
