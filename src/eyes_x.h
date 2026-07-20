#ifndef EYES_X_H
#define EYES_X_H

#include <Arduino.h>

// Eyes X-direction (left/right) channel on PCA9685
#define EYES_X_CHANNEL 6

// Safety limits for eyes X movement
#define EYES_X_MIN_ANGLE 50.0
#define EYES_X_MAX_ANGLE 130.0
#define EYES_X_CENTER_ANGLE 90.0

// Eyes X-direction control task
void eyesXTask(void *pvParameters);

#endif // EYES_X_H
