# LLM-Powered Animatronic Head Platform

An advanced, open-source physical robotic avatar that leverages local Large Language Models to engage in intelligent conversation, derive context-aware emotions, and express those emotions through lifelike physical servo movements.

## 🏗️ System Architecture

This project utilizes a **Dual-Tier Architecture** to separate physical real-time constraints from heavy AI computation.

1. **The Edge (ESP32):** 
   - Written in C++ using FreeRTOS.
   - Manages deterministic kinematics (9 servos via PCA9685) and I2S audio routing.
   - Streams raw audio back and forth via ultra-low-latency UDP.

2. **The Host (Python):** 
   - An asynchronous Python environment managed by `uv`.
   - Runs the heavy AI pipeline: Silero VAD -> Whisper STT -> Ollama LLM -> XTTS v2 TTS.
   - Translates spoken words into text, infers emotional intent, and generates voice and JSON control commands for the Edge.

## 🛠️ Hardware Requirements
- **Microcontroller:** ESP32 Development Board
- **Servo Controller:** PCA9685 16-Channel 12-bit PWM Driver
- **Audio Input:** INMP441 I2S Omnidirectional Microphone
- **Audio Output:** MAX98357A I2S Class-D Amplifier + 4Ω 4311 Mid-Range Speaker
- **Actuators:** 9 Servos total (2x MG945, 1x MG995, 1x HX5010, 5x SG90)
- **Power:** 5V 10A AC/DC Power Supply

## 🔌 Network Topology (UDP over Wi-Fi)

| Port | Direction | Purpose |
|------|-----------|---------|
| 4210 | Host → Edge | Control commands (JSON: intents, phase updates, emergency stop) |
| 4211 | Edge → Host | Audio uplink (raw PCM from INMP441 microphone) |
| 4212 | Host → Edge | Audio downlink (TTS PCM to MAX98357A speaker) |
| 4213 | Edge → Host | Telemetry (heap size, power state) |

The ESP32 advertises itself via mDNS as `animatronic-head.local`. The Host resolves this hostname automatically — **no hardcoded IPs are needed**.

---

## 🚀 Running the System End-to-End

Follow these steps in order to go from a cold start to a fully running conversational animatronic head.

### Prerequisites (Install Once)

#### A. PlatformIO CLI (Edge firmware toolchain)
```bash
# Install PlatformIO Core CLI (if not already installed)
pip install platformio

# Or if using VSCode, install the PlatformIO IDE extension instead
```

#### B. uv Package Manager (Host Python toolchain)
```bash
# Install uv (Astral's fast Python package manager)
curl -LsSf https://astral.sh/uv/install.sh | sh
```

#### C. Ollama (Local LLM runtime)
```bash
# Install Ollama for macOS
brew install ollama
```

#### D. Python 3.11
```bash
# The host requires exactly Python 3.11 (for PyTorch/TTS compatibility)
# uv will manage this automatically, but ensure 3.11 is available:
uv python install 3.11
```

---

### Step 1: Start Ollama and Pull the LLM Model

Open **Terminal 1** — this stays running in the background.

```bash
# Start the Ollama server (keeps running in the background)
ollama serve
```

Open **Terminal 2** (or a new tab) — pull the model once, then this terminal is free.

```bash
# Pull the LLM model (only needed the first time, ~2GB download)
ollama pull llama3.2

# Verify it's available
ollama list
```

You should see `llama3.2` in the output. Keep Ollama running in Terminal 1.

---

### Step 2: Flash the ESP32 Firmware and Power Up Safely

**⚠️ CRITICAL SAFETY SEQUENCE:** 
Never flash the ESP32 while the servos are actively pulling power, and never connect 5V to the PCA9685 while the wires are live. Follow this sequence exactly:

1. **Unplug** the 5V 10A power supply from the wall.
2. **Plug** the ESP32 into your computer via USB (this powers the ESP32 safely at 3.3V).
3. **Flash** the firmware:
   ```bash
   # Navigate to the edge firmware directory
   cd edge
   
   # Compile and upload the firmware to the ESP32
   ~/.platformio/penv/bin/pio run --target upload
   ```
4. **Power up the servos:** Once the upload says `[SUCCESS]`, plug the 5V 10A power supply back into the wall.
5. **Monitor:** Open the serial monitor to verify the boot sequence (keep this open).
   ```bash
   ~/.platformio/penv/bin/pio device monitor --baud 115200
   ```

**Expected serial output on successful boot:**
```
-----------------------------------
SYSTEM BOOTING... (115200 Baud)
-----------------------------------

[PowerManager] Dynamic CPU scaling enabled: 80–240 MHz. Light Sleep: ON.
[Kinematics] 60 Hz hardware timer ISR armed.
[Network] Configuring SoftAP mode...
```

> **Network Setup:** The ESP32 acts as its own Wi-Fi Access Point (AP) called `Edgar_AP`. Simply connect your laptop directly to this network using the password `edgarpassword123`. This ensures a dedicated, low-latency connection without needing an external router, phone hotspot, or internet access.

After the AP starts, the serial monitor should show:
```
[Network] ═══════════════════════════════════════
[Network]  SoftAP ACTIVE
[Network]  SSID     : Edgar_AP
[Network]  Password : edgarpassword123
[Network]  AP IP    : 192.168.4.1
[Network]  Subnet   : 255.255.255.0
[Network] ═══════════════════════════════════════

[Network] mDNS responder started: animatronic-head.local
[Network] Listening on UDP port 4210
Starting Staggered Boot Sequence...
Initializing Servo Channel 3...
Initializing Servo Channel 0...
...
Staggered Boot Complete. System Ready.
```

> **Important:** Keep this serial monitor terminal open. It will show all system events in real time.

---

### Step 3: Install Host Python Dependencies

Open **Terminal 3** — this is where the Host AI pipeline will run.

```bash
# Navigate to the host directory
cd host

# Install all Python dependencies (first time takes a few minutes)
uv sync
```

This will create a `.venv/` virtual environment and install all packages listed in `pyproject.toml`, including:
- `openai-whisper` (Speech-to-Text)
- `tts` (Coqui XTTS v2 — Text-to-Speech with voice cloning)
- `torch` / `torchaudio` (PyTorch for ML inference)
- `langchain` / `langgraph` (Cognitive AI pipeline)
- `langchain-ollama` (Ollama LLM integration)

---

### Step 4: Connecting the Network (Anywhere!)

The system is designed for maximum portability and runs without requiring an internet connection or external routers. The ESP32 is configured to act as a **Wi-Fi Access Point (SoftAP)** instead of relying on a phone hotspot or local network.

This setup eliminates the constraint of needing a cellular connection on your phone and bypasses public Wi-Fi client isolation issues entirely. You can connect and run the system from anywhere—whether in a university lab, at a park, or on an airplane.

**How to Connect:**
1. Open your laptop's Wi-Fi menu.
2. Look for the network called **`Edgar_AP`**.
3. Connect using the password **`edgarpassword123`**.
4. That's it! Your laptop is now directly communicating with the ESP32 over a dedicated, low-latency UDP link.

Now, just run the host script! It will seamlessly communicate with the ESP32 without any manual `export ESP32_IP` commands needed, as the ESP32 uses a static IP (`192.168.4.1`) and mDNS.

---

### Step 5: Start the Host AI Pipeline

In Terminal 3 (still in the `host/` directory):

```bash
# Run the full cognitive pipeline
uv run python main.py
```

#### 🧠 Wiping AI Memory (Context Pollution)
The Host AI pipeline uses a local SQLite database (`data/memory.db`) to remember conversation history between runs. If the AI begins acting strangely (e.g., ignoring physical commands or overusing specific words like "mortal" because it's copying its past behavior), its context has become polluted. 
To wipe its memory and force a fresh start with clean instructions, use the `--wipe` flag:
```bash
uv run python main.py --wipe
```

**Expected startup output:**
```
Loading Silero VAD...
Silero VAD loaded.
Loading Whisper model: base.en on device: mps
Whisper model loaded successfully.
Preloading XTTS model to eliminate first-time latency...
Loading XTTS v2 on device: mps...
XTTS v2 loaded successfully.
Starting Cognitive Orchestrator...
UDPVADBridge started. Rate: 16000Hz
STT Worker started. Waiting for speech segments...
LLM Worker started. Beginning autonomous loop...
Graph entered LISTENING state.
```

> **First-time model downloads:** Whisper, Silero VAD, and XTTS v2 models will be downloaded automatically on the first run (~3GB total). Subsequent runs load from cache.

---

### Step 6: Talk to the Head!

The system is now running. The full data flow is:

```
You speak → INMP441 mic (ESP32) → UDP → Host
  → Silero VAD (speech segmentation)
  → Whisper STT (speech-to-text)
  → Ollama llama3.2 (LLM reasoning + emotion)
  → XTTS v2 (text-to-speech with voice cloning)
  → UDP → MAX98357A speaker (ESP32)
  → Lip sync (jaw moves with audio amplitude)
  → Emotion pose (head/eyes express the emotion)
```

Speak clearly into the INMP441 microphone. You should see in the Host terminal:
```
Speech started. Publishing INTERRUPT event.
Speech segment emitted (25600 bytes)
USER SAID: Hello, who are you?
XTTS generating sentence: I am the haunted skull...
Sent TTS_COMPLETE to ESP32.
```

And on the ESP32 serial monitor:
```
[Dispatcher] Phase Update: SPEAKING
[Dispatcher] Executing Pose: HAPPY
[Dispatcher] TTS_COMPLETE received. State → IDLE_LISTENING.
```

---

## 🛑 Stopping the System

1. **Host:** Press `Ctrl+C` in Terminal 3 (the `main.py` process).
2. **Ollama:** Press `Ctrl+C` in Terminal 1 (the `ollama serve` process).
3. **ESP32:** Simply unplug USB power. Or press the RST button on the board.

---

## 🔧 Troubleshooting

| Problem | Solution |
|---------|----------|
| `Could not resolve...` & UDP discovery times out | Ensure your laptop is connected directly to the `Edgar_AP` Wi-Fi network. |
| AI ignores commands or repeats phrases | Run `uv run python main.py --wipe` to clear polluted memory |
| `XTTS generation failed` | Check that `host/assets/scary_voice.wav` exists (reference audio for voice cloning) |
| `Ollama CLI not found` | Run `brew install ollama` and ensure `ollama serve` is running |
| No audio from speaker | Check MAX98357A wiring and that GAIN pin is not grounded |
| Servos jitter on boot | Normal — staggered boot takes ~4.5s to initialize all 9 servos sequentially |
| `[PowerWDT] Entering LOW_POWER_IDLE` | System idle >60s. Speak into the mic or send a UDP packet to wake it |
| `torch.hub.load` fails for Silero | Check internet connection; Silero VAD model downloads from GitHub on first run |

---

## 📄 Documentation
For future contributors and AI Agents, please review the following foundational documents:
- `docs/requirements/AnimatronicHead_SRS.md` — Software Requirements Specification (The "What")
- `docs/design/AnimatronicHead_SDD.md` — Software Design Document (The "How")
- `docs/Implementation_Roadmap.md` — The current lifecycle tracker
- `.agent/hardware_context.md` — Physical wiring and breadboard layout
- `.agent/rules.md` — Strict architectural rules and constraints

## 📜 License
This project is licensed under the **MIT License**. See the [LICENSE](LICENSE) file for details.
