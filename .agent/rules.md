# Project Development Rules & Workflows

**ATTENTION AI AGENT:** These rules are strict boundaries. Do not violate them.

## 1. Dual-Tier Architecture Rules
This project uses a split architecture:
*   **Edge (ESP32):** Handles deterministic kinematics and I2S audio routing. Uses **PlatformIO (Arduino + ESP-IDF)**.
*   **Host (Python):** Handles all heavy AI computation (VAD, STT, LLM, TTS). Uses the **`uv` package manager**.
*   **Constraint:** The ESP32 MUST NOT run any AI models locally due to SRAM limits.

## 2. Hardware Safety & Kinematics (ESP32)
*   **Deterministic Authority:** The Python LLM is non-deterministic and can only output *abstract intents* (e.g., `{"emotion": "SAD"}`). The ESP32 holds the `PoseDictionary` and translates intents into mathematically safe PWM angles.
*   **Power Management:** The 10A power supply can brownout the ESP32 3.3V logic if all 9 servos initialize at once. **You must stage/stagger servo initialization upon boot.**
*   **Jaw Restraints:** The Jaw L/R (Left/Right) servo has a known physical binding defect. Do not rely heavily on lateral jaw movement; clamp its PWM boundaries tightly and blend it into other major expressions.
*   **Right Eyelid Defect:** The right eyelid servo experiences slipping. Mathematically mirror or blend its movements with the left eyelid rather than moving it independently.

## 3. Real-Time Concurrency (FreeRTOS)
*   **Task Pinning:** 
    *   **Core 0 (High Priority):** Reserved exclusively for Wi-Fi, UDP streams, and I2S Audio DMA. Never block this core.
    *   **Core 1 (Medium Priority):** Reserved for JSON parsing, kinematic calculations, and I2C PCA9685 commands.
*   **No Blocking:** Never use `delay()` in FreeRTOS tasks. Use `vTaskDelay(pdMS_TO_TICKS(ms))`.

## 4. Audio Pipeline Protocol
*   **Protocol:** All audio streams between Edge and Host use **Unencrypted UDP** over local Wi-Fi to prioritize latency over packet safety. Do not implement TCP overhead.
*   **Latency SLA:** Total conversational turnaround (STT -> LLM -> TTS) must not exceed 3.0 seconds on average.
*   **Interruption:** If the Host VAD detects user speech, it must immediately send an `EMERGENCY_STOP` JSON command to the ESP32.

## 5. Architectural & Design Pattern Enforcement (ESP32)
*   **Meyers Singletons:** All core subsystems (`KinematicEngine`, `PoseController`, `AnimatronicHead`, `NetworkManager`) MUST be implemented as thread-safe Meyers Singletons (`static ClassName& getInstance()`). **No global variables** are permitted for these systems.
*   **Facade Pattern:** The `AnimatronicHead` class must act as a pure, lightweight Facade. It delegates intent to the `PoseController` and math/hardware-driving to the `KinematicEngine`. It must not contain raw servo logic.
*   **Strict Primitive Composition:** The `PoseController` uses a two-tier abstraction. Macros (like `expressSad`) and mid-level commands (like `lookLeft`) **MUST NEVER** interface directly with the `KinematicEngine`. They must exclusively compose behavior from a strict, private set of atomic Base Primitives (e.g., `moveNeckPan`, `moveEyelidLeft`). This guarantees hardware defect mitigations (like Jaw L/R clamping) applied at the atomic level cannot be bypassed.

## 6. Strict "No Mocks" Policy
*   **ABSOLUTELY NO MOCKS ALLOWED:** Under no circumstances should any code in this project use "mocks" or fake data. All tools, connections, sensors, MCP implementations, and data pipelines must be fully functional and real. If a hardware component is not available, write the real production logic (e.g., real UDP sockets to the ESP32) to ensure the system architecture is robust and immediately ready for hardware integration. Any AI agent that proposes a "mock" violates this foundational rule.
