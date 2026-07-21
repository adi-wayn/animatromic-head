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
