#ifndef NECK_ONE_H
#define NECK_ONE_H

#include <Arduino.h>

// Neck One channel on PCA9685
#define NECK_ONE_CHANNEL 3

// Safety limits for Neck One movement (subtle ±30° range around center)
#define NECK_ONE_MIN_ANGLE 80.0
#define NECK_ONE_MAX_ANGLE 140.0
#define NECK_ONE_CENTER_ANGLE 110.0

// Neck One control task
void neckOneTask(void *pvParameters);

#endif // NECK_ONE_H
