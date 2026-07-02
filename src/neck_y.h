#ifndef NECK_Y_H
#define NECK_Y_H

#include <Arduino.h>

// Neck Y-direction channel on PCA9685 (formerly Neck X)
#define NECK_Y_CHANNEL 0

// Safety limits for neck Y movement (preventing mechanical stress on MG945/MG995)
#define NECK_Y_MIN_ANGLE 50.0
#define NECK_Y_MAX_ANGLE 90.0
#define NECK_Y_CENTER_ANGLE 70.0

// Neck Y-direction control task
void neckYTask(void *pvParameters);

#endif // NECK_Y_H
