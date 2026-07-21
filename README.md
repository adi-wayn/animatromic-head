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
   - Runs the heavy AI pipeline: WebRTC VAD -> Whisper STT -> Ollama LLM -> Dual-TTS (Coqui XTTS / Piper).
   - Translates spoken words into text, infers emotional intent, and generates voice and JSON control commands for the Edge.

## 🛠️ Hardware Requirements
- **Microcontroller:** ESP32 Development Board
- **Servo Controller:** PCA9685 16-Channel 12-bit PWM Driver
- **Audio Input:** INMP441 I2S Omnidirectional Microphone
- **Audio Output:** MAX98357A I2S Class-D Amplifier + 4Ω 4311 Mid-Range Speaker
- **Actuators:** 9 Servos total (3x MG945, 1x HX5010, 5x SG90)
- **Power:** 5V 10A AC/DC Power Supply

## 🚀 Getting Started

### 1. Edge Firmware (ESP32)
The embedded firmware is built using **PlatformIO**.
1. Open the project folder in VSCode with the PlatformIO extension installed.
2. PlatformIO will automatically resolve dependencies via `platformio.ini`.
3. Compile and upload to the ESP32.

### 2. Host AI Pipeline (Python)
The Host environment uses the **`uv`** package manager for fast resolution.
1. Ensure `uv` is installed on your local PC.
2. Initialize the environment and install dependencies (instructions pending pipeline completion).
3. Ensure Ollama is running locally with your model of choice.

## 📄 Documentation
For future contributors and AI Agents, please review the following foundational documents in the `docs/` folder:
- `docs/requirements/AnimatronicHead_SRS.md` - Software Requirements Specification (The "What")
- `docs/design/AnimatronicHead_SDD.md` - Software Design Document (The "How")
- `docs/Implementation_Roadmap.md` - The current lifecycle tracker

## 📜 License
This project is licensed under the **MIT License**. See the [LICENSE](LICENSE) file for details.
