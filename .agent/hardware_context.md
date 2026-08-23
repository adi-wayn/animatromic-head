# Hardware Context & Specifications

**ATTENTION AI AGENT:** This file documents the current physical state of the hardware, wiring, and mechanical assembly. It serves as a unified reference for hardware-related constraints and AI coding context.

## 1. System Components
*   **Edge Controller:** ESP32 Development Board.
*   **Host Controller:** Local PC running Python (`uv`).
*   **Servo Driver:** PCA9685 16-Channel 12-Bit PWM Driver (I2C Address `0x40`).
*   **Power Supply:** 5V 10A AC/DC Adapter (Wired to PCA9685 green terminal block). The ESP32 logic is isolated from the 10A servo bus.

## 2. Hardware Connection Mapping - Animatronic Head Project

This document provides the exact physical breadboard wiring, pinout mapping, and signal routing for the Animatronic Head project. It serves as a reference for hardware assembly and provides strict context for AI coding agents.

### 1. Power Distribution Architecture
To ensure stability and isolate noisy components (motors) from sensitive logic (microphones/ESP32), the system uses a dual-power-supply architecture:
*   **Logic Power (3.3V):** Powered by the ESP32's onboard regulator, which draws its power from the MacBook via USB. Distributed on the Rightmost Positive (Red) Rail.
*   **High-Power/Motor Rail (5V):** Powered by an external 5V 10A power supply. Distributed on the Leftmost Positive (Red) Rail and directly into the PCA9685.
*   **Common Ground (GND):** Crucially, all ground rails (Left, Middle, Right) and both power supplies are interconnected to establish a unified 0V reference.

### 2. ESP32 Development Board
*   **Physical Position:**
    *   Rows Occupied: Row 43 to Row 57.
    *   Left Pins Location: Left Board, Column H.
    *   Right Pins Location: Right Board, Column B.
*   **Power Connections & Protection:**
    *   **Main Power Input:** Powered via its onboard Micro-USB port, connected directly to a MacBook (acting as the 5V source and Serial monitor).
    *   **GND:** Jumper from Right Board, Row 56, Col E $\rightarrow$ Rightmost GND Rail.
    *   **3V3 (Output):** Jumper from Right Board, Row 57, Col D $\rightarrow$ Rightmost 3.3V Rail.
    *   **Bulk Decoupling Capacitor:** A 470µF electrolytic capacitor is placed directly on the Rightmost Power Rails (around Row 58/60). Long leg (+) in the Red 3.3V rail, Short leg (-) in the Blue GND rail. This stabilizes the ESP32 against Wi-Fi brownouts.

### 3. MAX98357A (I2S Audio Amplifier)
*   **Physical Position:** Left Board, Column A, Rows 28-34.
*   **Power & Decoupling:**
    *   **VCC (Row 28):** Jumper from Left Board, Row 28, Col B $\rightarrow$ Leftmost 5V Rail (External Power).
    *   **GND (Row 29):** Jumper from Left Board, Row 29, Col C $\rightarrow$ Leftmost GND Rail.
    *   **100nF Ceramic Cap:** Connected between Left Board, Row 28, Col D and Row 29, Col D.
    *   **470µF Electrolytic Cap:** Long leg (+) to Row 28, Short leg (-) to Row 29.
*   **Data Connections (I2S):**
    *   **SD_MODE (Row 30):** Not connected (Floating).
    *   **GAIN (Row 31):** Not connected (Floating).
    *   **DIN $\rightarrow$ GPIO 27 (Row 32):**
        *   Resistor (39Ω) from Left Board, 32B crossing trench to 32J.
        *   Jumper from Left Board, 32F $\rightarrow$ Left Board, 52F (Connected to ESP32 D27 at 52H).
    *   **BCLK $\rightarrow$ GPIO 14 (Row 33):**
        *   Resistor (39Ω) from Left Board, 33B crossing trench to 33J.
        *   Jumper from Left Board, 33G $\rightarrow$ Left Board, 53G (Connected to ESP32 D14 at 53H).
    *   **LRC $\rightarrow$ GPIO 12 (Row 34):**
        *   Resistor (39Ω) from Left Board, 34B crossing trench to 34J.
        *   Jumper from Left Board, 34F $\rightarrow$ Left Board, 54F (Connected to ESP32 D12 at 54H).
*   **Output (Speaker Connection):**
    *   **Component:** 4Ω 4311 Mid-Range Speaker.
    *   **Connection Point:** Green screw terminal block on the MAX98357A module.
    *   **Wiring Method:** Improvised using a repurposed cut USB cable.
    *   **Positive (+):** The Red wire connects the positive (+) terminal of the green block to the positive (+) terminal of the speaker.
    *   **Negative (-):** The Black wire connects the negative (-) terminal of the green block to the negative (-) terminal of the speaker.
    *   **Notes:** The wires are soldered directly to the speaker terminals and physically reinforced with adhesive tape (sellotape) for strain relief against vibrations.

### 4. INMP441 (I2S Microphone)
*   **Physical Position:** Right Board, straddling the central trench (Left pins in Col E, Right pins in Col F), Rows 36-38.
*   **Power & Hardware RC Filter (Audio Upgrade):**
    *   To isolate the microphone from ESP32 Wi-Fi transmission spikes on the 3.3V line, a Hardware RC Low-Pass filter was constructed directly on the VDD line.
    *   **GND (Row 38F):** Jumper to Rightmost GND Rail.
    *   **L/R (Row 38E):** Jumper to Middle GND Rail (Configures to Left Channel).
    *   **VDD RC Filter (Row 37):** 
        *   A 39Ω Resistor connects the Rightmost 3.3V Rail to Right Board, Row 37, Col J.
        *   470µF Electrolytic Cap: Long leg (+) at 37I, Short leg (-) at 38I (GND).
        *   100nF Ceramic Cap: Connected between 37G (VDD) and 38G (GND).
    *   **RC Filter Math & Theory:** With $R = 39\Omega$ and $C \approx 470\mu F$, the cutoff frequency is $f_c = 1 / (2\pi RC) \approx 8.7 \text{ Hz}$. This means the filter allows pure DC power ($0\text{ Hz}$) to pass to the microphone while aggressively blocking any electrical noise or ripple above $8.7\text{ Hz}$ caused by Wi-Fi or servos.
*   **Data Connections (I2S) - Advanced Routing:**
    *   **WS $\rightarrow$ GPIO 25 (Row 37E):**
        *   Resistor (39Ω) from Right Board, 37B crossing trench to 37I.
        *   Jumper from Right Board, 37H $\rightarrow$ Left Board, 46E.
        *   Jumper from Left Board, 46D $\rightarrow$ Left Board, 50D.
        *   Jumper crossing trench from Left Board, 50E $\rightarrow$ 50F (Connected to ESP32 D25).
    *   **SCK $\rightarrow$ GPIO 26 (Row 36E):**
        *   Resistor (39Ω) crossing boards from Right Board, 36C $\rightarrow$ Left Board, 36J.
        *   Jumper from Left Board, 36H/I $\rightarrow$ Left Board, 44C.
        *   Jumper from Left Board, 44B $\rightarrow$ Left Board, 51C.
        *   Jumper crossing trench from Left Board, 51E $\rightarrow$ 51G (Connected to ESP32 D26).
    *   **SD $\rightarrow$ GPIO 33 (Row 36F):**
        *   Jumper from Right Board, 36I $\rightarrow$ Right Board, 32I.
        *   Jumper crossing trench from Right Board, 32H $\rightarrow$ 32B.
        *   Resistor (39Ω) crossing boards from Right Board, 32A $\rightarrow$ Left Board, 35J.
        *   Jumper from Left Board, 35I $\rightarrow$ Left Board, 42C.
        *   Jumper from Left Board, 42A $\rightarrow$ Left Board, 49A.
        *   Jumper crossing trench from Left Board, 49E $\rightarrow$ 49F (Connected to ESP32 D33).

### 5. PCA9685 (Servo Controller) & External Power
*   **Physical Position:** External to the breadboard, connected via extension jumper wires.
*   **External Power Supply:**
    *   **Power Source:** 5V 10A external power supply connected to a wall outlet.
    *   **Adapter:** The power supply connects to an adapter (likely a barrel jack to screw terminal adapter).
    *   **Wiring to PCA9685:** From the adapter, thick wires (red for positive, black for negative) with exposed, twisted strands are inserted into the green screw terminal block on the PCA9685 to provide power for the servos.
    *   **Protection:** The PCA9685 module has an onboard 1000µF electrolytic capacitor to protect against servo current spikes.
*   **Data Connections (I2C) & Logic Power:**
    *   **GND:** Connected to Rightmost GND Rail.
    *   **SDA $\rightarrow$ GPIO 21:**
        *   Jumper from PCA9685 $\rightarrow$ Left Board, 17E.
        *   Jumper from Left Board, 17D $\rightarrow$ Right Board, 47D.
        *   Jumper connecting to Right Board, 47B (ESP32 D21).
    *   **SCL $\rightarrow$ GPIO 22:**
        *   Jumper from PCA9685 $\rightarrow$ Right Board, 14B.
        *   Jumper from Right Board, 14C $\rightarrow$ Right Board, 44C.
        *   Jumper connecting to Right Board, 44B (ESP32 D22).

### Servo Mapping
The PCA9685 controls 16 channels (addresses 0 to 15) corresponding to the various servos in the animatronic head.
*   **Channel 0:** `NECK_Y` (MG945 servo) - Controls up/down head movement.
*   **Channel 1:** `JAW_UD` (HX5010 servo) - Controls upper/lower jaw movement for speech sync.
*   **Channel 2:** *Unassigned*
*   **Channel 3:** `NECK_ONE` (MG945 servo) - Multi-axis base platform movement.
*   **Channel 4:** `EYELID_RIGHT` (SG90 servo) - Controls right eyelid blinking mechanics.
*   **Channel 5:** `EYELID_LEFT` (SG90 servo) - Controls left eyelid blinking mechanics.
*   **Channel 6:** `EYES_X` (SG90 servo) - Controls horizontal eye movement vector.
*   **Channel 7:** `EYES_Y` (SG90 servo) - Controls vertical eye movement vector.
*   **Channel 8:** `JAW_LR` (SG90 servo) - Moves the bottom jaw left and right.
*   **Channel 9:** `NECK_ROLL` (MG995 servo) - Controls head tilting/roll.
*   **Channel 10-15:** *Unassigned*

## 3. Mechanical Assembly & 3D Model Notes
*   **Model:** Animatronic Skull by Djfx on Thingiverse (https://www.thingiverse.com/thing:2456550)
*   **Jaw Articulation:** The jaw swings slightly forward when opening for realistic speech (Open/Close pitch and Left/Right yaw).
*   **Eye Linkage:** Linked mechanism to control yaw/pitch coordinates using only 2 servos for the pair.
*   **Teeth Interference:** The teeth (`teeth.stl`) on the bottom jaw clash with the upper teeth if rotated too far left or right. Mechanical clearance permits **at most ±25° of rotation** around the shifted center alignment.
*   **Jaw Servo Physical Issue (2026-06-13):** During testing, the jaw servo was observed making a high-pitched whirring/spinning sound ("moving fast") without translating to physical jaw movement. This indicates either a stripped plastic gear train inside the micro-servo, a loose/broken glue joint between the servo horn and the spline, or a physical linkage blockage.

## 4. Electrical Constraints & Power Management Strategy
An advanced software logic pattern must be used to prevent hardware overcurrent conditions (total peak potential ~8-10A, supply cutoff at 2.1A or significant brownout risk for logic):
1.  **Actuation Staggering / Time-Slicing:** Avoid triggering multiple large servo movements in the exact same execution cycle. Use small deterministic delays (e.g., 20–50ms) between task commands.
2.  **Sequential Motion Queues:** Use FreeRTOS queues for dense motion frames rather than broad parallel sweeps.
3.  **Holding Current Management:** The base MG945 and MG995 servos draw constant "holding current". Prioritize neck stability tasks over non-essential facial expressions if power drops.
4.  **Velocity Limiting:** Increment angles gradually over multiple cycles to create soft-start/soft-stop acceleration curves, reducing inductive current spikes.
