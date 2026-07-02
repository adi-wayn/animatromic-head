#ifndef EYES_Y_H
#define EYES_Y_H

#include <Arduino.h>

// Eyes Y-direction (up/down) channel on PCA9685
#define EYES_Y_CHANNEL 0

// Safety limits for eyes Y movement
#define EYES_Y_MIN_ANGLE 80.0
#define EYES_Y_MAX_ANGLE 100.0
#define EYES_Y_CENTER_ANGLE 90.0

// Eyes Y-direction control task
void eyesYTask(void *pvParameters);

#endif // EYES_Y_H
