# Hardware Context & Specifications

**ATTENTION AI AGENT:** This file documents the current physical state of the hardware.

## 1. System Components
*   **Edge Controller:** ESP32 Development Board.
*   **Host Controller:** Local PC running Python (`uv`).
*   **Servo Driver:** PCA9685 16-Channel 12-Bit PWM Driver (I2C Address `0x40`).
*   **Power Supply:** 5V 10A AC/DC Adapter (Wired to PCA9685 green terminal block). The ESP32 logic is isolated from the 10A servo bus.

## 2. The 9-Servo Topology
1.  **Neck/Platform:** 3x MG945 High-Torque Metal Gear Servos.
2.  **Head Alignment:** 1x HX5010 Large Standard Servo.
3.  **Jaw (Speaking):** 2x SG90 Micro Servos (Up/Down and Left/Right).
4.  **Eyes & Face:** 3x SG90 Micro Servos (Yaw/Pitch vectors and Eyelids).

## 3. Audio Modules (I2S Peripherals)
> [!WARNING] 
> **Current Status:** The INMP441 and MAX98357A are *currently missing* from the physical build. The software roadmap dictates we mock the Host AI pipeline locally before integrating these hardware pieces.

*   **Microphone (Ears):** INMP441 Omnidirectional Digital I2S Mic.
    *   *To be wired to ESP32 I2S0 peripheral.*
*   **Amplifier (Voice):** MAX98357A I2S Class-D Amplifier.
    *   *To be wired to ESP32 I2S1 peripheral.*
*   **Speaker:** 4Ω 4311 Mid-Range Cone.
    *   *Wired directly to the MAX98357A output terminals.*

## 4. Known Hardware Limitations
*   The ESP32 possesses exactly **two** I2S peripherals. Both are saturated by the Mic and Amplifier. No further I2S hardware can be added.
*   The mechanical 3D-printed gears are fragile. All software must enforce strict `Config.h` boundary limits to prevent stripping.
