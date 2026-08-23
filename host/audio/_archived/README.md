# Archived: PyAudio-Based VAD (Phase 3 Legacy)

This directory contains the original `VADManager` class that was built during
Phase 3 when the system used the **local PC microphone** (`pyaudio`) for
audio input, before the INMP441 I2S microphone was installed on the ESP32.

## Files

- `vad_pyaudio_legacy.py` — The original `VADManager` class. Uses `pyaudio`
  callbacks to read audio from the PC microphone, runs Silero VAD, and emits
  speech segments. **Replaced by `audio/udp_vad_bridge.py`** which reads
  audio from the ESP32 over UDP instead.

## Why Archived (Not Deleted)

The Silero VAD logic, ring buffer design, and interrupt callback pattern in
this file are well-tested and may be useful as reference if we ever need to
run the host pipeline standalone (without an ESP32) for development/debugging.

## Current Architecture

The active VAD pipeline is:

```
ESP32 INMP441 → I2S DMA → UDP (port 4211) → Host audio_rx_queue
  → UDPVADBridge (Silero VAD) → segment_queue → Whisper STT
```

The edge-side RMS silence detection (in `SystemTasks.cpp`) serves a
DIFFERENT purpose: it triggers power wakeup from LOW_POWER_IDLE mode.
It does NOT replace Silero VAD for speech segmentation.
