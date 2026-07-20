#ifndef NECK_ROLL_H
#define NECK_ROLL_H

#include <Arduino.h>

// Neck Roll (tilt left/right) channel on PCA9685
#define NECK_ROLL_CHANNEL 9

// Safety limits for neck roll movement
#define NECK_ROLL_MIN_ANGLE 40.0
#define NECK_ROLL_MAX_ANGLE 140.0
#define NECK_ROLL_CENTER_ANGLE 90.0

#endif // NECK_ROLL_H
