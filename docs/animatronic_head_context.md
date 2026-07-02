# Project Context: ESP32 Animatronic Head Control System

This document serves as a complete system specification and structural context file for the AI coding assistant ("Anti-Gravity" / Cursor / Copilot) inside VS Code. It provides all architectural layout, hardware specifications, wiring connections, and power optimization strategies needed to write the control firmware.

---

## 1. Project Overview
The objective is to control an advanced **3D-printed Animatronic Head** using an **ESP32 microcontroller** interacting with a **PCA9685 16-Channel PWM I2C Driver**. The system translates algorithmic behavior or sensor triggers into smooth, organic facial expressions, jaw movements (speech sync), and multi-axis neck rotations.

---

## 2. System Architecture & Component Breakdown

The hardware layout consists of the following components identified from physical inspection:

1. **Microcontroller (The Brain):**
   * **Model:** ESP32 Development Board (mounted on a standard white breadboard).
   * **Power Source:** Powered directly via a USB cable connected to the development PC/Mac (used for flashing code and serial debugging).

2. **PWM/Servo Driver (The Muscle Controller):**
   * **Model:** PCA9685 16-Channel 12-bit PWM Driver board.
   * **Communication Interface:** I2C protocol (requires only 2 data pins from the ESP32).
   * **Physical Setup:** Sitting standalone (insulated from static/conductive surfaces), connected via jumper wires to the breadboard.

3. **Actuators / Servos (Total: 9 Servos):**
    * **Neck & Multi-Axis Base Platform:** 3× **MG945** High-Torque Metal Gear Servos. These operate in a coordinated arrangement (ball-joint / Stewart-like platform) to allow pitch, roll, and yaw (nodding, turning, tilting).
    * **Head Alignment / Stabilization:** 1× **HX5010** Large Standard Servo positioned at the top of the head. It utilizes a wired iron mechanism to keep the head aligned and stabilize it from the top.
    * **Jaw / Speech Mechanism:** 2× **SG90** Micro Servos powering the top jaw and the bottom jaw. Note: The goal of this test is to move only one servo. The only servo currently connected to the PCA9685 driver is the SG90 micro-servo that moves the bottom jaw left and right.
    * **Eyes & Face Expressions:** 3× **SG90** Micro Servos controlling eye horizontal/vertical look vectors and upper/lower eyelid blinking/winking mechanics via thin pushrods.

4. **Power Infrastructure:**
   * **Power Adapter:** *Comtive Cube USB Adapter*.
   * **Rated Output:** **DC 5V, 2.1A (Maximum Total Output)**.
   * **Wiring Modification:** A custom USB-A cable has been modified—one end plugs into the Comtive wall adapter, and the other end has its outer insulation stripped to expose the bare **Red (Positive/V+)** and **Black (Negative/GND)** power rails. These are screwed directly into the green terminal block of the PCA9685 board to feed the servo power bus.

---

## 3. Precise Wiring & Pinout Guide

To ensure proper communication and electrical safety, wire the components according to the exact mappings below:

### A. ESP32 to PCA9685 (Logic & Communication)
Connect the side header pins of the PCA9685 to the ESP32 development board using female-to-male or male-to-male jumper wires:

| PCA9685 Pin | ESP32 Pin | Wire Function / Notes |
| :--- | :--- | :--- |
| **GND** | **GND** | **Common Ground Reference** (Mandatory for I2C signal integrity) |
| **VCC** | **3V3** (or 3.3V) | Logic Power for the PCA9685 onboard chip |
| **SDA** | **GPIO 21 (D21)** | I2C Serial Data Rail |
| **SCL** | **GPIO 22 (D22)** | I2C Serial Clock Rail |
| **V+** | *Leave Unconnected* | Not needed on this header (Power is fed via the green terminal block) |
| **OE** | *Leave Unconnected* | Output Enable (Active Low). Internal pull-down keeps it enabled by default |

### B. PCA9685 Green Terminal Block (Servo Power Bus)
| Terminal Port | Source Wire (From Stripped USB Cable) | Electrical Parameter |
| :--- | :--- | :--- |
| **V+ / Plus** | **Red Wire** | +5V DC from Comtive Wall Adapter |
| **GND / Minus** | **Black Wire** | Return Ground to Comtive Wall Adapter |

### C. Servos to PCA9685 Output Channels
Servos plug into the 3-pin columns distributed across the bottom of the PCA9685 board. 
* **Initial Test Channel:** **Channel 0** (The leftmost vertical 3-pin column, clearly labeled `0` on the PCB silk screen).
* **Pin Orientation & Wire Colors:**
  * **Top Row (PWM):** Connects to the **Orange or Yellow** wire (Control Signal).
  * **Middle Row (V+):** Connects to the **Red** wire (Positive Power).
  * **Bottom Row (GND):** Connects to the **Brown or Black** wire (Ground).

---

## 4. Electrical Constraints & Power Management Strategy

An advanced software logic pattern must be used to prevent hardware overcurrent conditions due to power supply limitations:

### The Problem (Physics Limits):
* **MG945 & HX5010 Large Servos:** Each can draw **1.0A to 1.5A** under mechanical strain or during initialization spikes.
* **SG90 Micro Servos:** Each can draw up to **0.5A** under sudden movement.
* **Total Peak Potential:** If all 9 servos actuate simultaneously, the instantaneous current draw can spike to **8.0A – 10.0A**.
* **Available Supply:** The *Comtive Cube Adapter* safely cuts off or drops voltage if current exceeds **2.1A**. Attempting to move all servos at once will cause a critical voltage sag, triggering an ESP32 hardware **Brownout Reset**, or causing severe servo jittering.

### The Software Solution (To be implemented in Code):
The AI assistant must leverage the ESP32's native **FreeRTOS** architecture (priority scheduling and task management) to implement a **Smart Current Smoothing Logic**:
1. **Actuation Staggering / Time-Slicing:** Avoid triggering multiple large servo movements in the exact same execution cycle. Use small, unnoticeable deterministic delays (e.g., 20–50ms) between task commands to flatten the peak current curve (`vTaskDelay`).
2. **Sequential Motion Queues:** Implement FreeRTOS queues (`QueueHandle_t`) to handle dense motion frames sequentially instead of executing broad parallel sweeps.
3. **Holding Current Management:** Be aware that the 3 base MG945 servos draw constant "holding current" to resist gravity and keep the heavy skull balanced upright. Prioritize neck stability tasks over non-essential facial expressions if power drops.
4. **Velocity Limiting:** Instead of instantly setting a servo to a target angle (which creates a massive inductive current spike), increment the angles gradually over multiple cycles to create soft-start and soft-stop acceleration curves.

---

## 5. Instructions for the AI Coding Assistant

When generating code inside VS Code based on this document, adhere to these guidelines:
* **Target Framework:** Arduino Framework for ESP32 in VS Code (PlatformIO or Arduino IDE setup).
* **Primary Library:** Use the official `Adafruit_PWMServoDriver.h` library for I2C handling.
* **I2C Address:** Default address `0x40`.
* **PWM Frequency:** Set servo frequency to `50Hz` (`pwm.setPWMFreq(50);`).
* **Pulse Lengths:** Standard SG90/MG945 pulse widths range between a `USMIN` of ~500–600 microseconds (0 degrees) and a `USMAX` of ~2400–2500 microseconds (180 degrees). Map degrees to 12-bit tick values (0-4095).
* **Architecture Style:** Write highly modular, thread-safe, FreeRTOS-task-driven C++ code utilizing safe resource sharing techniques.
