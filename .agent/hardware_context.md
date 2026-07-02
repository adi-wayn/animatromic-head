# Hardware Specifications & Wiring Guide

This document maintains the hardware configuration and interface mappings for the ESP32 Animatronic Head control system.

## 1. System Components

### Microcontroller (The Brain)
* **Model:** ESP32 Development Board.
* **Power Source:** Powered via micro-USB connected to development PC/Mac.
* **Communication Interface:** I2C protocol.

### PCA9685 16-Channel 12-Bit PWM Driver
* **I2C Address:** `0x40` (Default).
* **Clock Reference:** Standard internal oscillator runs at `27000000` (27 MHz).
* **Power Input (Green Terminal Block):** 5V DC from the *Comtive Cube USB Adapter* via a custom-stripped USB cable.
* **Voltage Smoothing Capacitor:** A **1000 μF** electrolytic capacitor is connected across the PCA9685's green terminal block power rails to absorb peak inductive currents and prevent voltage sags.

### Actuators / Servos (Total: 9 Servos)
* **Neck Base Platform:** 3× **MG945** High-Torque Metal Gear Servos.
* **Head Alignment / Stabilization:** 1× **HX5010** Large Standard Servo positioned at the top of the head (utilizes a wired iron mechanism to keep the head aligned).
* **Jaw / Speech Mechanism:** 2× **SG90** Micro Servos (one for the top jaw, one for the bottom jaw).
  * *Current Test Scope:* Only the SG90 micro-servo moving the bottom jaw left and right is connected.
* **Eyes & Face Expressions:** 3× **SG90** Micro Servos (eyes yaw/pitch and eyelids blink).

---

## 2. Pin and Channel Mapping

### A. ESP32 to PCA9685 I2C Wiring
| PCA9685 Pin | ESP32 Pin | Wire Function | Notes |
| :--- | :--- | :--- | :--- |
| **GND** | **GND** | Ground | Common Ground Reference |
| **VCC** | **3V3** | Logic Power | Logic power for PCA9685 chip |
| **SDA** | **GPIO 21 (D21)**| I2C Data | Serial Data Line |
| **SCL** | **GPIO 22 (D22)**| I2C Clock | Serial Clock Line |

### B. PCA9685 Power Bus
| Terminal Port | Source Wire (USB-A Stripped) | Electrical Details |
| :--- | :--- | :--- |
| **V+ / Plus** | **Red Wire** | +5V DC from Comtive Wall Adapter |
| **GND / Minus** | **Black Wire** | Return Ground to Comtive Wall Adapter |

### C. PCA9685 Output Channels
| Channel | Actuator Description | Servo Model | Operating Range (Degrees) | Pulse Limits (μs) |
| :---: | :--- | :---: | :---: | :---: |
| **0** | **Neck Y-Axis (Pitch)** | MG995 | **50° to 90°** (Center: 70°) | 500 - 2400 |
| **1** | **Jaw Up/Down (Speaking)** | HX5010 | **40° to 90°** (Center: 90°) | 500 - 2400 |
| **3** | **Neck One (Bottom Left/Right)** | MG995 | **80° to 140°** (Center: 110°) | 500 - 2400 |
| **4** | **Eyelids (Open/Close)** | SG90 | **40° to 180°** (Open: 40°) | 500 - 2400 |
| **5** | **Bottom Jaw (Left/Right)** | SG90 | **85° to 135°** (Center: 110°) | 500 - 2400 |

---

## 3. Power Metrics & Constraints
* **Comtive Cube Adapter Output:** 5V DC, 2.1A (max continuous load).
* **SG90 Micro Servo Current Draw:** ~100mA idle, ~500mA peak under movement.
* **1000 μF Capacitor:** Acts as a local reservoir to supply peak startup current, reducing the chance of triggering an ESP32 brownout.
* **Software Staggering:** Despite the capacitor, avoid simultaneous step commands to multiple channels if more servos are connected in future stages.
