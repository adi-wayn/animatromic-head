# Architecture Improvement Roadmap

*Date: 2026-07-26*

This roadmap outlines the critical improvements needed to harden the Animatronic Head architecture, focusing on the core AI graph, state machines, hardware constraints, OOP/SOLID principles, and testability.

---

## 1. Executive Summary & Flow Analysis

The current end-to-end data flow operates as follows:
**User Voice** -> `INMP441 (I2S)` -> `ESP32 (Core 0)` -> `UDP (Port 4211)` -> `Host (UDPVADBridge)` -> `STT` -> `graph.py (LangGraph)` -> `LLM JSON` -> `TTS` -> `UDP (Port 4212)` -> `ESP32 (Core 0)` -> `MAX98357A` -> **Speaker**.

While the Tri-Layer decoupling (Host/Protocol/Edge) is correct, the "Brain" of the system (`graph.py`) is fundamentally flawed. It masquerades as a State Machine but is actually a thin, stateless wrapper around a basic LLM prompt. Furthermore, the Edge hardware logic lacks dynamic current budgeting and strict watchdog safety, creating massive risks of brownouts and silent lockups. Testing coverage is also severely lacking for the core AI loops and data bridges.

---

## 2. Core Architectural Deficiencies

### A. The "Brain" is not a Behavior Engine
- **Flaw:** `graph.py` only maintains `messages` in its `AgentState`. It has no concept of robot physical state, emotional state, active goals, or system health.
- **Violation:** Separation of Concerns (SoC). The LLM is forced to act as the cognitive engine, the behavior engine, and the kinematics dispatcher simultaneously. 
- **Consequence:** The LLM outputs `{"intent": "JAW"}` directly. The LLM shouldn't know what a "JAW" is. The LLM should output `{"emotion": "SAD", "response": "..."}`, and a dedicated **Behavior Engine** should translate that into physical intents based on current physical telemetry.

### B. Hardware Safety: Voltage, Amps, and Interrupts
- **Flaw:** The 10A power supply is shared across 9 servos. While boot-staggering exists, there is no dynamic current budgeting during runtime. If the LLM commands a full-body `SCARE` macro, 9 servos moving at max velocity will draw >15A, browning out the logic board.
- **Flaw:** FreeRTOS tasks run `while(1)` without Task Watchdog Timers (TWDT). If I2C contention locks Core 1, the I2S DMA interrupts on Core 0 will keep audio running, but the robot will physically freeze, resulting in a zombie state.

### C. Protocol & State Synchronization
- **Flaw:** The UDP stream lacks sequence numbering. If the Wi-Fi drops 3 packets, the STT/VAD receives corrupted PCM frames, leading to hallucinations. 
- **Flaw:** The Host operates open-loop. It fires intents and blindly hopes the Edge executes them.

### D. LLM Parsing & Graph Looping Bugs (From Logs)
- **Flaw:** Analysis of `host/logs/host_2026-07-26_19-44-07_874970.log` reveals the LLM occasionally hallucinates Python `None` instead of JSON `null` (e.g., `{"speak": "", "intent": None, "intensity": 1}`), which crashes the crude `json.loads` parser.
- **Flaw:** The LangGraph gets stuck in a recursive loop (`REASONING` -> `ACTION` -> `REASONING`) when the LLM hallucinates an empty tool call, eventually crashing out and routing back to `LISTENING`.

---

## 3. Implementation To-Do List (The Roadmap)

This is the actionable, step-by-step roadmap to fix the "heart of the system". These tasks must be executed sequentially.

### Phase 1: Edge Hardware Safety & RTOS Hardening (The Body)
*These tasks ensure the physical hardware will not destroy itself or brownout under heavy load.*

- [x] **Task 1.1: FreeRTOS Watchdog Implementation**
  - **Action:** Enable ESP-IDF TWDT (Task Watchdog Timer).
  - **Details:** Add `esp_task_wdt_init()` and subscribe the `kinematicsTask`, `jsonParserTask`, and `networkTask` to the watchdog. Add `esp_task_wdt_reset()` to their loops.
- [x] **Task 1.2: Dynamic Current Budgeting (Kinematics)**
  - **Action:** Implement a velocity-limiter in `updateKinematics()`.
  - **Details:** Calculate the total angular delta across all 9 servos per frame. If the delta implies a current draw >8A (assuming ~1A per servo at high speed), dynamically scale down the `easedT` step size to slow the movement and cap the amperage peak.
- [x] **Task 1.3: I2C Fast-Mode Plus Optimization**
  - **Action:** Maximize PCA9685 throughput.
  - **Details:** Set `Wire.setClock(400000)` (400kHz) in `PCA9685_Driver.cpp` to prevent Core 1 Mutex contention when floating-point easing is calculated.

### Phase 2: Protocol Reliability (The Nervous System)
*These tasks fix the silent failures in the communication bridge.*

- [x] **Task 2.1: UDP Sequence Numbering & Synchronization**
  - **Action:** Update `protocol/schema.json` to include `seq_num` and `timestamp`.
  - **Details:** Run `generate_protocol.py`. Modify `UDPVADBridge` on the Host to buffer and re-order audio chunks, dropping packets that are too old to prevent STT corruption.
- [x] **Task 2.2: Edge-to-Host Telemetry (Closed Loop)**
  - **Action:** Implement a 10Hz telemetry uplink.
  - **Details:** The ESP32 must transmit `{"type": "TELEMETRY", "angles": [...], "cpu_load": 45}` to the Host. The Host must save this to the global state.
- [x] **Task 2.3: Dynamic IP Latching Fix**
  - **Action:** Fix `NetworkManager.cpp`.
  - **Details:** Ensure `hostIP` updates on every valid control packet, not just the first one, allowing Host PC restarts without breaking the connection.

### Phase 3: The True Cognitive Architecture (The Brain)
*These tasks rewrite `graph.py` to follow strict SOLID and OOP principles, turning it into a real state machine.*

- [x] **Task 3.1: Robust LLM Structured Output & Parsing**
  - **Action:** Fix the `json.loads` crash seen in the logs.
  - **Details:** Implement `pydantic` models for the LLM output tool calls, utilizing LangChain's `with_structured_output` instead of manual regex and JSON stripping.
- [x] **Task 3.2: Redefine the `AgentState`**
  - **Action:** Expand the LangGraph `AgentState` `TypedDict`.
  - **Details:** It must include `robot_physical_state` (from telemetry), `current_emotion`, `active_goal`, and `conversation_history`.
- [x] **Task 3.3: Implement the Behavior Engine Node**
  - **Action:** Decouple LLM reasoning from hardware mapping.
  - **Details:** Create a new LangGraph node `behavior_node`. The LLM `agent_node` now only outputs `{"emotion": "SAD", "text": "I feel lonely"}`. The `behavior_node` intercepts this, looks at `robot_physical_state`, and translates it into physical intents (e.g., "Look down, slow speed").
- [x] **Task 3.4: Preemptive Interruption Handling**
  - **Action:** Make the LangGraph interruptible.
  - **Details:** When `UDPVADBridge` detects speech, it must inject an `INTERRUPT` event into the graph. The graph must have an `interrupt_node` that stops TTS generation, sends `EMERGENCY_STOP` to the Edge, and clears the active goal.
- [x] **Task 3.5: Fix Graph Looping State Bug**
  - **Action:** Address the `REASONING -> ACTION -> REASONING` infinite loop seen in logs.
  - **Details:** Modify `route_after_agent` to explicitly route to `listen_node` if a tool call was invalid or empty, rather than blindly bouncing back to the agent node without new human input.

### Phase 4: Test Driven Expansion (The Immune System)
*Expand testing coverage beyond the minimal existing tests.*

- [x] **Task 4.1: Unit Tests for UDPVADBridge**
  - **Action:** Create `tests/unit/test_udp_vad_bridge.py`.
  - **Details:** Mock incoming UDP packets with out-of-order sequence numbers to verify packet-dropping logic (Task 2.1).
- [ ] **Task 4.2: Integration Tests for Behavior Node**
  - **Action:** Add `tests/integration/test_behavior_node.py`.
  - **Details:** Feed mock telemetry data and mock LLM emotions into the new Behavior Engine to verify it translates intents correctly without passing invalid angles.
- [ ] **Task 4.3: Graph E2E Testing**
  - **Action:** Expand `test_orchestrator.py` or `test_graph.py`.
  - **Details:** Simulate a full conversation flow, including a VAD interruption mid-TTS, asserting the graph routes correctly to the `interrupt_node`.

---

## Final Verdict

The current implementation is an excellent Proof of Concept (PoC) but violates the Separation of Concerns by forcing the LLM to act as the Behavior Engine, violates hardware safety by ignoring dynamic current draw, and is currently vulnerable to JSON parsing crashes and graph state loops. 

By executing the 4 Phases outlined above, the system will transform into a production-grade, fault-tolerant robotic architecture capable of running continuously without lockups, brownouts, or cognitive drift.
