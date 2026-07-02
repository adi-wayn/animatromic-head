#ifndef EYELIDS_H
#define EYELIDS_H

#include <Arduino.h>

// Eyelids channel on PCA9685
#define EYELIDS_CHANNEL 4

// Safety limits for eyelids
#define EYELIDS_MIN_ANGLE 40.0
#define EYELIDS_MAX_ANGLE 180.0
#define EYELIDS_CENTER_ANGLE 90.0

// Eyelid control task
void eyelidsTask(void *pvParameters);

#endif // EYELIDS_H
