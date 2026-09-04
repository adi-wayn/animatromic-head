#pragma once

/**
 * @file PCA9685_Driver.h
 * @brief Header for PCA9685_Driver.h.
 */

#include <Adafruit_PWMServoDriver.h>
#include <Arduino.h>
#include <Wire.h>

/**
 * @brief Mutex for thread-safe I2C access to the PCA9685
 */
extern SemaphoreHandle_t pcaMutex;

/**
 * @brief Global PCA9685 Driver instance
 */
extern Adafruit_PWMServoDriver pwm;

/**
 * @brief Flag to track initialization
 */
extern bool isDriverInitialized;

// Pin Definitions for I2C
#define I2C_SDA 21
#define I2C_SCL 22

// Servo constants
#define USMIN 500      // Microsecond pulse width at 0 degrees
#define USMAX 2400     // Microsecond pulse width at 180 degrees
#define SERVO_FREQ 50  // PWM frequency in Hz

/**
 * @brief Function declarations
 */
void initDriver();
bool safeSetServoAngle(uint8_t channel, double angle, double minAngle, double maxAngle);
void detachServo(uint8_t channel);
