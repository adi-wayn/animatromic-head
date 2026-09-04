/**
 * @file main.cpp
 * @brief Main entry point for the edge system.
 */
#include <Adafruit_PWMServoDriver.h>
#include <Arduino.h>
#include <Wire.h>

// ---------------------------------------------------------
// Hardware Constants & Pin Definitions
// ---------------------------------------------------------

// I2C Pins for PCA9685
#define I2C_SDA 21
#define I2C_SCL 22

// HC-SR05 Ultrasonic Sensor Pins
#define TRIG_PIN 13
#define ECHO_PIN 5

// PCA9685 Channel for the Servo
#define SERVO_CHANNEL 12

// Servo PWM Limits (Adjusted for a 90-degree centered sweep)
// Center is ~375. 90 degree span = +/- 112 from center.
#define SERVO_MIN_PULSE 263  // roughly 45 degrees
#define SERVO_MAX_PULSE 487  // roughly 135 degrees

// ---------------------------------------------------------
// Radar / Detection Settings
// ---------------------------------------------------------

// Max distance to consider someone "close enough to speak" (in cm)
#define DETECTION_THRESHOLD_CM 100.0

// Sweep settings for smoother movement
#define SWEEP_STEP 2  // Very small step for ultra-smooth panning

// ---------------------------------------------------------
// Global Objects
// ---------------------------------------------------------
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        delay(10);
    }
    Serial.println("\n--- Radar Sweep Test (HC-SR05 + Servo) ---");

    // Initialize Ultrasonic Sensor Pins
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    digitalWrite(TRIG_PIN, LOW);  // Ensure Trig is low initially

    // Initialize I2C Bus and PCA9685
    Wire.begin(I2C_SDA, I2C_SCL);
    pwm.begin();
    pwm.setPWMFreq(50);  // Analog servos run at 50Hz

    Serial.println("Initialization Complete. Starting scanning sweep...");
}

// Helper function to check distance so the loop stays clean
void checkRadar(uint16_t currentPulse) {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    // Timeout of 15ms (~2.5 meters)
    long duration = pulseIn(ECHO_PIN, HIGH, 15000);

    if (duration > 0) {
        float distanceCm = duration * 0.034 / 2.0;

        if (distanceCm <= DETECTION_THRESHOLD_CM) {
            // Map the 90-degree pulse range back to 45-135 degrees for logging
            int currentAngle = map(currentPulse, 150, 600, 0, 180);
            Serial.print("[TARGET DETECTED] Angle: ");
            Serial.print(currentAngle);
            Serial.print("° | Distance: ");
            Serial.print(distanceCm);
            Serial.println(" cm");
        }
    }
}

void loop() {
    // 1. Smooth sweep from MIN to MAX
    for (uint16_t pulse = SERVO_MIN_PULSE; pulse <= SERVO_MAX_PULSE; pulse += SWEEP_STEP) {
        pwm.setPWM(SERVO_CHANNEL, 0, pulse);
        checkRadar(pulse);
        delay(15);  // Adjust this delay to control the overall speed of the radar sweep
    }

    // Pause briefly at the end of the sweep to prevent mechanical jerking
    delay(200);

    // 2. Smooth sweep from MAX to MIN
    for (uint16_t pulse = SERVO_MAX_PULSE; pulse >= SERVO_MIN_PULSE; pulse -= SWEEP_STEP) {
        pwm.setPWM(SERVO_CHANNEL, 0, pulse);
        checkRadar(pulse);
        delay(15);
    }

    // Pause briefly at the other end
    delay(200);
}
