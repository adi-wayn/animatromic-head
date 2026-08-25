# Animatronic Head Platform - Implementation Roadmap

**Version:** 1.0.0
**Status:** Active Document
**Purpose:** This document serves as the master checklist and project lifecycle tracker for the Animatronic Head Platform. It outlines the step-by-step implementation strategy, prioritizing available hardware (ESP32 and Servos) while mocking pending audio hardware.

---

## 1. The Golden Path (Current Strategy)
*Update:* The physical audio hardware (INMP441 and MAX98357A) has been acquired. Phase 4 is now unblocked.
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

- [x] **Task 3.5: Unified Communication Protocol Layer [Both]**
  - Design `protocol/schema.json` as a decoupled Single Source of Truth for Host <-> Edge UDP communication.
  - Implement `generate_protocol.py` to auto-generate Pydantic Python models and C++ parsing logic.
  - Integrate pre-build hooks in PlatformIO to automatically compile against the latest JSON schema.

- [x] **Task 3.6: Edge Code Encapsulation & Repository Reorganization [Both]**
  - Encapsulate all PlatformIO ESP32 code (`src/`, `include/`, `lib/`, `platformio.ini`, etc.) into a dedicated `edge/` directory.
  - Enforce the tri-layer physical repository structure: `host/`, `protocol/`, `edge/`.
  - Update build automation scripts to maintain correct relative paths.

- [x] **Task 3.7: LLM Context & TTS Voice Refinement [Software]**
  - Disable sentence-by-sentence streaming chunk logic; LLM must generate full JSON message first.
  - Relax "scary skull" system prompt so it acts more like a normal, highly intelligent LLM with animatronic persona.
  - Ensure LLM successfully translates direct user commands (e.g., "look left", "be sad") into correct JSON cognitive_state and physical intents.
  - Replace the bad XTTS reference audio sample in `host/audio/tts/dual_tts_manager.py` with a high-quality reference audio file.


### Phase 4: Network Audio Pipeline (Hardware Dependent)
**Goal:** Install the I2S modules and transition the audio topology from the PC to the ESP32.

- [x] **Task 4.0: Protocol Expansion [Both]**
  - Establish the 3-port UDP topology (Control, Audio Uplink, Audio Downlink).
  - Update `schema.json` and `generate_protocol.py` to act as the single source of truth for port constants and audio formats.
  - Implement the `TTS_COMPLETE` control message.

- [x] **Task 4.1: Edge Audio Uplink (Microphone) [Both]**
  - *STATUS: ROLLED BACK DUE TO HARDWARE FAILURE.* INMP441 module is defective/deaf (likely fried MEMS or bad soldering). ESP32 code maintains the robust I2S 32-bit DSP implementation for future use, but the Host Orchestrator (`USE_MAC_MIC = True`) has reverted to using the Mac's built-in PyAudio microphone.
  - Install INMP441 on the breadboard.
  - Configure I2S0 DMA in ESP-IDF.
  - Stream chunked raw audio (PCM, 16-bit, 16kHz) over UDP to Python Host.

- [x] **Task 4.2: Edge Audio Downlink (Speaker) [Both]**
  - Install MAX98357A on the breadboard.
  - Configure I2S1 DMA in ESP-IDF.
  - Receive UDP audio chunks from Python Host and push to 4Ω speaker. (Remove PC speaker playback from Phase 3).

- [x] **Task 4.3: Real-Time Lip Sync & Interruption [Both]**
  - Route incoming TTS audio amplitude envelope to Core 1 kinematics to dynamically modulate `JAW_UD` and `JAW_LR`.
  - Finalize VAD `EMERGENCY_STOP` JSON command. If the user speaks, Host sends interrupt -> ESP32 flushes I2S DMA buffer and snaps to `IDLE_LISTENING`.

### Phase 5: Simulation & Virtualization
**Goal:** Establish a pristine testing environment that mimics the physical hardware on the Host side without hardware dependencies.

- [x] **Task 5.1: Live Audio Proxy Script (`test/live_esp32_proxy.py`)**
  - Uses `pyaudio` to mimic ESP32 INMP441 uplink and MAX98357A downlink.
  - Receives kinematic intents for fast console validation.

- [x] **Task 5.2: Wokwi ESP32 Setup (`test/wokwi/`)**
  - Basic diagram with ESP32 and PCA9685 driving a single SG90 servo.
  - Strictly enforce: *NO TRASH FILES IN CORE DIRECTORIES (`host/` or `edge/`). All tests go in `test/`.*

---

## 3. Maintenance & Expansion Notes
- **Testing Methodology:** Always mock JSON payloads from Python before relying on the LLM to generate them. 
- **Configuration Management:** All physical limits and Wi-Fi credentials should reside exclusively in `include/Config.h`.
- **Future Integration:** Once Phase 4 is stable, consider adding ultrasonic proximity sensors (HC-SR04) to allow the head to physically orient toward approaching users.

---

### Phase 7: Ultrasonic Hardware Integration (Current)
**Goal:** Integrate ultrasonic proximity sensors (HC-SR05) to allow the head to physically orient toward approaching users.

- [x] **Task 7.1: Standalone Hardware Verification (`edge/test_hardware`)**
  - Created an isolated PlatformIO environment `[env:test_hardware]` to test PCA9685 servos alongside HC-SR05 distance measurement.
  - Test loop uses `millis()` to avoid FreeRTOS blocking, verifying wiring (Trig GPIO 13, Echo GPIO 5) before integration into the main kinematic engine.


### Phase 6: ESP32 Resource & Power Optimization ✅ COMPLETE
**Goal:** Minimize CPU, power, and I2C bus consumption when the system is idle.
**Branch:** `feature/esp32-power-optimization`

- [x] **Task 6.1: 3-State Power Model**
  - Added `LOW_POWER_IDLE` to `SystemState` enum.
  - `AnimatronicHead` tracks `_lastActivityMs` for 60s inactivity detection.

- [x] **Task 6.2: PowerManager Singleton**
  - New `edge/include/core/PowerManager.h` / `edge/src/core/PowerManager.cpp`.
  - `enterLowPowerIdle()`: detaches all 9 servos, sets LOW_POWER_IDLE state.
  - `enterFullPower()`: restores IDLE_LISTENING state, resets activity timer.
  - Enables ESP-IDF dynamic CPU scaling (80–240 MHz) via `esp_pm_configure`.

- [x] **Task 6.3: Hardware Timer ISR for Kinematics (60 Hz)**
  - Replaced `vTaskDelay(15ms)` spin-loop with `timerBegin/timerAlarmWrite` hardware timer.
  - ISR (`IRAM_ATTR onKinematicsTimer`) gives `kinematicsTriggerSem` every 16.6ms.
  - CPU enters FreeRTOS tickless idle (→ Light Sleep) between firings.

- [x] **Task 6.4: Audio Hardware Interrupt Wakeup**
  - `audioUplinkTask` computes RMS per I2S DMA buffer.
  - RMS > `SILENCE_RMS_THRESHOLD` (250.0) → gives `PowerManager::micWakeupSem`.
  - `powerWatchdogTask` blocks on `micWakeupSem` in LOW_POWER_IDLE → acts as hardware audio interrupt wakeup.

- [x] **Task 6.5: JSON Parser Blocking Queue**
  - `NetworkManager::getNextMessage(timeout)` now accepts a `timeoutMs` parameter.
  - `jsonParserTask` uses 25ms blocking wait → CPU yields to tickless idle.

- [x] **Task 6.6: FreeRTOS Tickless Idle + ESP-IDF PM**
  - `platformio.ini` build flags: `CONFIG_PM_ENABLE=1`, `CONFIG_FREERTOS_USE_TICKLESS_IDLE=1`.
  - When ALL tasks are blocked, FreeRTOS scheduler enters ESP32 Light Sleep automatically.

- [x] **Task 6.7: powerWatchdogTask**
  - New lowest-priority task (Priority 1, Core 1).
  - Checks inactivity every 5s. Triggers `enterLowPowerIdle()` after 60s.
  - In LOW_POWER_IDLE: blocks on `micWakeupSem` (portMAX_DELAY) → full tickless idle.

