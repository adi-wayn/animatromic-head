#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

struct ServoConfig {
    uint8_t channel;
    double minAngle;
    double maxAngle;
    double centerAngle;
};

constexpr ServoConfig NECK_Y      = { 0, 40.0, 95.0, 60.0 };
constexpr ServoConfig JAW_UD      = { 1, 50.0, 120.0, 80.0 }; // minAngle=Open, maxAngle=Closed
constexpr ServoConfig NECK_ONE    = { 3, 40.0, 140.0, 90.0 };
constexpr ServoConfig EYELID_RIGHT= { 4, 60.0, 140.0, 90.0 };
constexpr ServoConfig EYELID_LEFT = { 5, 40.0, 220.0, 90.0 };
constexpr ServoConfig EYES_X      = { 6, 30.0, 150.0, 90.0 };
constexpr ServoConfig EYES_Y      = { 7, 30.0, 150.0, 90.0 };
constexpr ServoConfig JAW_LR      = { 8, 85.0, 135.0, 110.0 };
constexpr ServoConfig NECK_ROLL   = { 9, 30.0, 140.0, 60.0 };

// ---------------------------------------------------------
// HC-SR05 Ultrasonic Radar Configuration
// ---------------------------------------------------------
#define RADAR_TRIG_PIN 13
#define RADAR_ECHO_PIN 5
#define RADAR_SERVO_CHANNEL 12
#define RADAR_THRESHOLD_CM 100.0
#define RADAR_OFFSET_X_CM -30.0  // 30cm to the left
#define RADAR_OFFSET_Y_CM 30.0   // 30cm forward (up in 2D plane)

#endif // CONFIG_H
