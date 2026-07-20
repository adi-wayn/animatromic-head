#ifndef EYELID_RIGHT_H
#define EYELID_RIGHT_H

#include <Arduino.h>

// Eyelid right channel on PCA9685
#define EYELID_RIGHT_CHANNEL 4

// Safety limits for eyelid right
#define EYELID_RIGHT_MIN_ANGLE 60.0  // Increased from 40 to prevent open-stall noise
#define EYELID_RIGHT_MAX_ANGLE 140.0 // Decreased from 180 to prevent close-stall noise
#define EYELID_RIGHT_CENTER_ANGLE 90.0

// Eyelid right control task
void eyelidRightTask(void *pvParameters);

#endif // EYELID_RIGHT_H
