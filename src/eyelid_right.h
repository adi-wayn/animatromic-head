#ifndef EYELID_RIGHT_H
#define EYELID_RIGHT_H

#include <Arduino.h>

// Eyelid right channel on PCA9685
#define EYELID_RIGHT_CHANNEL 4

// Safety limits for eyelid right
#define EYELID_RIGHT_MIN_ANGLE 40.0
#define EYELID_RIGHT_MAX_ANGLE 180.0
#define EYELID_RIGHT_CENTER_ANGLE 90.0

// Eyelid right control task
void eyelidRightTask(void *pvParameters);

#endif // EYELID_RIGHT_H
