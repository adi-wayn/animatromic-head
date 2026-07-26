# Project Development Rules & Workflows

**ATTENTION AI AGENT:** These rules are strict boundaries. Do not violate them.

## 1. Tri-Layer Architecture Rules
This project strictly enforces a tri-layer physical repository structure to ensure decoupling:
*   **Host (`host/`):** Python orchestration handling heavy AI computation (VAD, STT, LLM, TTS, LangGraph). Uses the **`uv` package manager**.
*   **Protocol (`protocol/`):** The absolute Single Source of Truth for communication. Contains language-agnostic JSON schemas and code generators.
*   **Edge (`edge/`):** ESP32 C++ firmware handling deterministic kinematics and I2S audio routing. Uses **PlatformIO (Arduino + ESP-IDF)**.
*   **Constraint:** The ESP32 MUST NOT run any AI models locally due to SRAM limits.

## 2. Hardware Safety & Kinematics (ESP32)
*   **Deterministic Authority:** The Python LLM is non-deterministic and can only output *abstract intents* (e.g., `{"emotion": "SAD"}`). The ESP32 holds the `PoseDictionary` and translates intents into mathematically safe PWM angles.
*   **Power Management:** The 10A power supply can brownout the ESP32 3.3V logic if all 9 servos initialize at once. **You must stage/stagger servo initialization upon boot.**
*   **Jaw Restraints:** The Jaw L/R (Left/Right) servo has a known physical binding defect. Do not rely heavily on lateral jaw movement; clamp its PWM boundaries tightly and blend it into other major expressions.
*   **Right Eyelid Defect:** The right eyelid servo experiences slipping. Mathematically mirror or blend its movements with the left eyelid rather than moving it independently.

## 3. Real-Time Concurrency & Safety (FreeRTOS)
*   **Task Pinning:** 
    *   **Core 0 (High Priority):** Reserved exclusively for Wi-Fi, UDP streams, and I2S Audio DMA. Never block this core.
    *   **Core 1 (Medium Priority):** Reserved for JSON parsing, kinematic calculations, and I2C PCA9685 commands.
*   **No Blocking:** Never use `delay()` in FreeRTOS tasks. Use `vTaskDelay(pdMS_TO_TICKS(ms))`.
*   **Task Watchdog Timer (TWDT):** All long-running tasks (`kinematicsTask`, `jsonParserTask`, `networkTask`) MUST be subscribed to the TWDT (`esp_task_wdt_add`) and regularly fed (`esp_task_wdt_reset`) to trigger kernel panics instead of silent zombie locks if the I2C bus hangs.
*   **I2C Optimizations:** The PCA9685 I2C bus must operate in Fast-Mode Plus (`Wire.setClock(400000)`) to minimize Core 1 Mutex contention during kinematic calculations.

## 3.5. Dynamic Current Budgeting (Kinematics)
*   The `KinematicEngine` MUST actively predict peak current draw per frame based on angular deltas. If the sum of all servo currents exceeds **8.0A**, it must linearly scale back the `easedT` velocities for all servos to prevent logic brownouts on the 10A power supply.

## 4. Audio Pipeline Protocol
*   **Topology:** The system strictly uses a 3-port UDP topology over local Wi-Fi (Control: 4210, Uplink: 4211, Downlink: 4212) to prioritize latency over packet safety. Do not implement TCP overhead.
*   **Dynamic Discovery (No Hardcoded IPs):** The ESP32 and Host must never use hardcoded IP addresses. The Host discovers the ESP32 via mDNS (`animatronic.local`), and the ESP32 latches onto the Host's IP from the first control packet. This ensures network portability.
*   **Latency SLA:** Total conversational turnaround (STT -> LLM -> TTS) must not exceed 3.0 seconds on average.
*   **Interruption:** If the Host VAD detects user speech, it must immediately send an `EMERGENCY_STOP` JSON command to the ESP32.

## 5. Architectural & Design Pattern Enforcement (ESP32)
*   **Meyers Singletons:** All core subsystems (`KinematicEngine`, `PoseController`, `AnimatronicHead`, `NetworkManager`) MUST be implemented as thread-safe Meyers Singletons (`static ClassName& getInstance()`). **No global variables** are permitted for these systems.
*   **Facade Pattern:** The `AnimatronicHead` class must act as a pure, lightweight Facade. It delegates intent to the `PoseController` and math/hardware-driving to the `KinematicEngine`. It must not contain raw servo logic.
*   **Strict Primitive Composition:** The `PoseController` uses a two-tier abstraction. Macros (like `expressSad`) and mid-level commands (like `lookLeft`) **MUST NEVER** interface directly with the `KinematicEngine`. They must exclusively compose behavior from a strict, private set of atomic Base Primitives (e.g., `moveNeckPan`, `moveEyelidLeft`). This guarantees hardware defect mitigations (like Jaw L/R clamping) applied at the atomic level cannot be bypassed.

## 6. Strict "No Mocks" Policy
*   **ABSOLUTELY NO MOCKS ALLOWED:** Under no circumstances should any code in this project use "mocks" or fake data. All tools, connections, sensors, MCP implementations, and data pipelines must be fully functional and real. If a hardware component is not available, write the real production logic (e.g., real UDP sockets to the ESP32) to ensure the system architecture is robust and immediately ready for hardware integration. Any AI agent that proposes a "mock" violates this foundational rule.

## 7. Strict Agentic Task Workflow & `main` Branch Protection
*   **`main` is READ-ONLY:** You must NEVER, NEVER, EVER write or commit code to the `main` branch. 
*   **Workflow Enforcement:** You MUST strictly follow the `agentic-task-workflow` skill (all 16 steps) every single time you work on a task.
*   **Documentation Maintenance:** You MUST continually update `AGENT.md`, `docs/`, and `.agent/` files at the end of your workflow so that future agents do not repeat mistakes. This is a hard-coded mandate.

## 8. Agentic Proactivity & Autonomy
*   **You are a Principal Autonomous Agent, NOT a passive chatbot.** Do not wait for the user to hand-hold you through technical steps. You must operate with maximum autonomy.
*   **Use Your Skills & Tools:** You are equipped with powerful tools (e.g., MCP servers, search tools, bash commands). Proactively `search_web` for documentation (like FreeRTOS watchdog APIs or ESP32 limitations) before asking the user.
*   **Use Sub-Agents Extensively:** If a task requires deep research, a massive code refactor, or exploring a dense documentation page, you MUST invoke sub-agents (via `invoke_subagent`) to handle it in parallel so you don't lose your main context. Do not do all the heavy lifting yourself in a single thread.
*   **Create Skills:** If you encounter a repetitive workflow or solve a complex problem, proactively use the `workflow-skill-creator` or write bash/python scripts to automate it. You do not need permission to create a skill.
*   **Act Decisively:** If you encounter a bug, fix it. If you need a script, write and execute it.
*   **Force Alignment:** If requirements are ambiguous, proactively prompt the user to use the `/grill-me` command to force an interactive alignment session before you write code.
