# Animatronic Head Platform - Implementation Roadmap

**Version:** 1.0.0
**Status:** Active Document
**Purpose:** This document serves as the master checklist and project lifecycle tracker for the Animatronic Head Platform. It outlines the step-by-step implementation strategy, prioritizing available hardware (ESP32 and Servos) while mocking pending audio hardware.

---

## 1. The Golden Path (Current Strategy)
Due to pending physical audio hardware (INMP441 and MAX98357A), the development priority is shifted to establish the "Brain" and the "Muscles" first.
1. **Foundation:** Initialize ESP32 (PlatformIO) and Python (`uv`) environments.
2. **Servo Mechanics (Ears/Eyes/Neck):** Build the JSON parsing and servo controllers on the ESP32. We will test by sending mock JSON commands over UDP.
3. **Host Cognitive Brain:** Build the Python AI pipeline on the PC using the PC's built-in microphone and speakers. Implement the LLM multi-model fallback.
4. **Hardware Integration:** When the I2S modules arrive, install them and reroute the audio pipeline from the PC to the ESP32.

---

## 2. Phase Breakdown & Task Tracker

### Phase 1: Environment & Foundation
**Goal:** Establish safe, reliable development environments that protect the physical hardware from power surges.

- [x] **Task 1.1: Edge Environment Setup [Software]**
  - Initialize the PlatformIO project (`espressif32`, `arduino` framework).
  - Setup `<freertos/FreeRTOS.h>` task pinning structures.
  - Setup basic Serial logging (115200 baud).

- [x] **Task 1.2: Host Environment Setup [Software]**
  - Initialize the Python environment using `uv`.
  - Set up the `asyncio` event loop structure.
  - Setup `logging` architecture for debug visibility.

- [x] **Task 1.3: Servo Safety & Boot Staggering [Both]**
  - Create `Config.h` defining mechanical PWM boundaries for all 9 servos.
  - Write ESP32 initialization sequence to power up PCA9685 servos in a staggered manner (e.g., 500ms delay between each).
  - **CRITICAL:** Ensure 10A power supply does not brownout the 3.3V logic during boot.

### Phase 2: Kinematics & Reactivity (Available Hardware)
**Goal:** Enable the physical head to react to abstract JSON commands sent over Wi-Fi.

- [x] **Task 2.1: UDP Command Bridge & Edge JSON Parser [Both]**
  - Establish unencrypted UDP Wi-Fi connection on ESP32.
  - Implement FreeRTOS Core 1 task to parse incoming JSON payloads.
  - Define state machine: `IDLE_LISTENING`, `SPEAKING_SYNCING`, `INTERRUPTED`.

- [x] **Task 2.2: Pose Dictionary & Easing [Both]**
  - Create a local `PoseDictionary` mapping JSON intents (e.g., `SAD`, `HAPPY`) to precise servo PWM limits.
  - Implement cubic easing functions (`ease-in-out`) so servos move fluidly rather than snapping instantly.

- [x] **Task 2.3: Defect Mitigation & Saccades [Software]**
  - Implement a background task injecting random eye saccades (micro-movements) during the `IDLE_LISTENING` state.
  - Implement software fix for Right Eyelid binding: Mathematically blend its movement into the Left Eyelid macro.
  - Clamp lateral (L/R) jaw movement to prevent gear stripping.

- [x] **Task 2.4: Strict Primitive Composition Pattern [Software]**
  - Create atomic primitives in `PoseController` for hardware abstraction.
  - Prevent macros from directly interacting with `KinematicEngine`.
  - Embed hardware limits (e.g. jaw clamping) directly into atomic layer.

### Phase 3: Host Cognitive Pipeline (Local PC Mocking)
**Goal:** Build the conversational AI loop locally, using PC mic/speakers to substitute for the missing I2S modules.

- [x] **Task 3.1: Host VAD & STT Ingestion [Software]**
  - Implement WebRTC VAD.
  - *Temporary:* Ingest audio via PC microphone (`pyaudio`).
  - Route segmented audio to Whisper STT for text transcription.

- [x] **Task 3.2: Multi-Model LLM Intent Generation [Software]**
  - Feed STT output and conversation history into Ollama.
  - Implement **Fallback Pattern**: Try `Llama 3` -> if timeout/fail -> try `Mistral` -> if fail -> try `Phi-3`.
  - Constrain LLM output to structured JSON containing conversational text and `cognitive_state`.

- [x] **Task 3.3: Dual-TTS & Audio Dispatch [Software]**
  - Route LLM text to Coqui XTTS.
  - Implement 1500ms TTFB (Time-to-First-Byte) limit -> fallback to Piper TTS if exceeded.
  - *Temporary:* Play TTS audio directly out of PC speakers. Save amplitude envelope data to send to ESP32 for Lip-Sync testing.

- [x] **Task 3.4: Hybrid Architecture & SOLID Compliance [Software]**
  - Decouple memory management into `MemoryManager`.
  - Replace deprecated `create_react_agent` with an explicit LangGraph `StateGraph` (Listen -> Think -> Act).
  - Enforce strict Pydantic JSON schemas over UDP (`send_kinematic_intent`).
  - Wire Voice Activity Detection (VAD) to trigger immediate `EMERGENCY_STOP` UDP commands to the ESP32.

- [x] **Task 3.5: Unified Communication Protocol Layer [Software]**
  - Design a decoupled, unified Protocol Envelope for Host <-> Edge UDP communication.
  - Implement Pydantic schema validation for standard `INTENT`, `PHASE_UPDATE`, and `EMERGENCY_STOP` payloads.
  - Formally document the exact JSON structures in the SDD for Edge firmware parsing.

### Phase 4: Network Audio Pipeline (Hardware Dependent)
**Goal:** Install the I2S modules and transition the audio topology from the PC to the ESP32.

- [ ] **Task 4.1: Edge Audio Uplink (Microphone) [Both]**
  - Install INMP441 on the breadboard.
  - Configure I2S0 DMA in ESP-IDF.
  - Stream chunked raw audio (PCM, 16-bit, 16kHz) over UDP to Python Host. (Remove `pyaudio` dependency from Phase 3).

- [ ] **Task 4.2: Edge Audio Downlink (Speaker) [Both]**
  - Install MAX98357A on the breadboard.
  - Configure I2S1 DMA in ESP-IDF.
  - Receive UDP audio chunks from Python Host and push to 4Ω speaker. (Remove PC speaker playback from Phase 3).

- [ ] **Task 4.3: Real-Time Lip Sync & Interruption [Both]**
  - Route incoming TTS audio amplitude envelope to Core 1 kinematics to dynamically modulate `JAW_UD` and `JAW_LR`.
  - Finalize VAD `EMERGENCY_STOP` JSON command. If the user speaks, Host sends interrupt -> ESP32 flushes I2S DMA buffer and snaps to `IDLE_LISTENING`.

---

## 3. Maintenance & Expansion Notes
- **Testing Methodology:** Always mock JSON payloads from Python before relying on the LLM to generate them. 
- **Configuration Management:** All physical limits and Wi-Fi credentials should reside exclusively in `include/Config.h`.
- **Future Integration:** Once Phase 4 is stable, consider adding ultrasonic proximity sensors (HC-SR04) to allow the head to physically orient toward approaching users.
