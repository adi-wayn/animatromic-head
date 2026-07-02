#ifndef JAW_UD_H
#define JAW_UD_H

#include <Arduino.h>

// Jaw Up/Down (open/close pitch) channel on PCA9685
#define JAW_UD_CHANNEL 1

// Safety limits for jaw up/down movement (preventing mechanical stress on HX5010)
#define JAW_UD_MIN_ANGLE 40.0
#define JAW_UD_MAX_ANGLE 90.0
#define JAW_UD_CENTER_ANGLE 90.0

// Jaw Up/Down control task
void jawUDTask(void *pvParameters);

#endif // JAW_UD_H
