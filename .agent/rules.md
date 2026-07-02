# Project Development Rules & Workflows

This document establishes the programming guidelines, architecture requirements, and hardware safety constraints for the ESP32 Animatronic Head controller.

## 1. Development & Build System
* **Build System:** PlatformIO (VS Code).
* **Target Framework:** Arduino Framework for ESP32.
* **Libraries:**
  * Use the official `Adafruit PWM Servo Driver Library` for PCA9685 control.
  * Ensure `Wire.h` and `Adafruit_PWMServoDriver.h` are correctly referenced in the code and added to `lib_deps` in `platformio.ini`.
* **I2C Setup:** Explicitly initialize the I2C bus with `Wire.begin(SDA_PIN, SCL_PIN)` where default SDA = 21, SCL = 22.

## 2. Code Architecture & FreeRTOS
* **Task Allocation:**
  * Place servo motion and coordination logic inside a dedicated FreeRTOS task running on **Core 1** to leave Core 0 open for communication or system routines.
  * Avoid any blocking delay functions (`delay()`) inside tasks; use FreeRTOS-friendly delays: `vTaskDelay(pdMS_TO_TICKS(ms))`.
* **Resource Sharing:**
  * If multiple tasks access the PCA9685 driver instance, protect the instance using a FreeRTOS Mutex (`SemaphoreHandle_t`).

## 3. Critical Safety & Movement Constraints
To protect the 3D-printed gears and the SG90 micro-servo motor from mechanical clashing and overheating:
* **Bottom Jaw Left/Right Servo (Channel 0):**
  * **Center Position:** 90°
  * **Safe Range:** **60° to 120°** (Strictly limit motion to ±30° from center).
  * **Teeth Clashing Warning:** Going beyond 60° (left) or 120° (right) will cause the teeth of the bottom jaw to clash with the top jaw.
* **Angle Enforcer Function:** All writes to the servo must pass through an angle-constraining function. Direct raw PWM pulses should never be sent without range clamping:
  ```cpp
  double clampAngle(double angle) {
      if (angle < 60.0) return 60.0;
      if (angle > 120.0) return 120.0;
      return angle;
  }
  ```

## 4. Current & Power Management
* **Velocity Limiting:** Move the servo incrementally (e.g. step sizes of 1.0° or 2.0°) with small delay slices (e.g. 15ms) rather than jumping instantly to target positions. This prevents inductive current spikes that could exceed the 2.1A supply rating.
* **Frequency:** The PCA9685 PWM frequency must be configured to `50Hz` (standard for SG90 and HX5010 analog servos).
