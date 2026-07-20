#include "eyelid_right.h"
#include "driver.h"

void eyelidRightTask(void *pvParameters) {
  (void) pvParameters;

  // Start safely at the center angle
  double currentAngle = EYELID_RIGHT_CENTER_ANGLE;
  
  // Safe, narrow sweep range to prevent mechanical jamming
  double safeMin = 70.0;
  double safeMax = 110.0;

  // Initial centering to sync physical position
  safeSetServoAngle(EYELID_RIGHT_CHANNEL, currentAngle, EYELID_RIGHT_MIN_ANGLE, EYELID_RIGHT_MAX_ANGLE);
  vTaskDelay(pdMS_TO_TICKS(1000));

  while (true) {
    Serial.println("[EYELID RIGHT] Sweeping to 110°...");
    while (currentAngle < safeMax) {
      currentAngle += 1.0;
      safeSetServoAngle(EYELID_RIGHT_CHANNEL, currentAngle, EYELID_RIGHT_MIN_ANGLE, EYELID_RIGHT_MAX_ANGLE);
      vTaskDelay(pdMS_TO_TICKS(40));
    }

    vTaskDelay(pdMS_TO_TICKS(500));

    Serial.println("[EYELID RIGHT] Sweeping to 70°...");
    while (currentAngle > safeMin) {
      currentAngle -= 1.0;
      safeSetServoAngle(EYELID_RIGHT_CHANNEL, currentAngle, EYELID_RIGHT_MIN_ANGLE, EYELID_RIGHT_MAX_ANGLE);
      vTaskDelay(pdMS_TO_TICKS(40));
    }
    
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}
