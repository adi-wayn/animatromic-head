#ifndef EYELID_LEFT_H
#define EYELID_LEFT_H

#include <Arduino.h>

// Eyelid left channel on PCA9685
#define EYELID_LEFT_CHANNEL 5

// Safety limits for eyelid left
#define EYELID_LEFT_MIN_ANGLE 40.0
#define EYELID_LEFT_MAX_ANGLE 200.0 // Increased from 180 to help the eye close completely
#define EYELID_LEFT_CENTER_ANGLE 90.0

// Eyelid left control task
void eyelidLeftTask(void *pvParameters);

#endif // EYELID_LEFT_H
