#include "driver.h"

// Instantiate the global driver and mutex
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver(0x40);
SemaphoreHandle_t pcaMutex = NULL;

void initDriver() {
  // Create the mutex to prevent concurrent I2C writes
  pcaMutex = xSemaphoreCreateMutex();
  if (pcaMutex == NULL) {
    Serial.println("[ERROR] Failed to create I2C PCA9685 Mutex!");
  }

  // Start I2C bus
  Wire.begin(I2C_SDA, I2C_SCL);

  // Initialize PCA9685
  pwm.begin();
  pwm.setOscillatorFrequency(27000000);
  pwm.setPWMFreq(SERVO_FREQ);
}

// Convert angle to PWM ticks and write to PCA9685 under Mutex protection
bool safeSetServoAngle(uint8_t channel, double angle, double minAngle, double maxAngle) {
  // 1. Safety Clamping
  double clampedAngle = angle;
  if (clampedAngle < minAngle) {
    clampedAngle = minAngle;
  } else if (clampedAngle > maxAngle) {
    clampedAngle = maxAngle;
  }

  // 2. Convert angle to 12-bit PWM ticks
  double us = USMIN + (clampedAngle / 180.0) * (USMAX - USMIN);
  uint16_t ticks = (uint16_t)((us / 20000.0) * 4096.0);

  // 3. Thread-safe execution using FreeRTOS Mutex
  if (pcaMutex != NULL) {
    if (xSemaphoreTake(pcaMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
      pwm.setPWM(channel, 0, ticks);
      xSemaphoreGive(pcaMutex);
      return true;
    } else {
      Serial.printf("[WARNING] Mutex timeout. Failed to set channel %d to %.1f°\n", channel, clampedAngle);
      return false;
    }
  } else {
    // Fallback if mutex is not initialized
    pwm.setPWM(channel, 0, ticks);
    return true;
  }
}

void detachServo(uint8_t channel) {
  if (pcaMutex != NULL) {
    if (xSemaphoreTake(pcaMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
      pwm.setPWM(channel, 0, 0); // Stops sending PWM pulses
      xSemaphoreGive(pcaMutex);
    }
  } else {
    pwm.setPWM(channel, 0, 0);
  }
}
