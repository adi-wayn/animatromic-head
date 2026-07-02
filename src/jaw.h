#ifndef JAW_H
#define JAW_H

#include <Arduino.h>

// Jaw channel on PCA9685
#define JAW_CHANNEL 5

// Safety limits for jaw left/right movement (shifted center workaround)
#define JAW_MIN_ANGLE 85.0
#define JAW_MAX_ANGLE 135.0
#define JAW_CENTER_ANGLE 110.0

// Jaw control task
void jawTask(void *pvParameters);

#endif // JAW_H
