#include "eyelid_left.h"
#include "driver.h"

void eyelidLeftTask(void *pvParameters) {
  (void) pvParameters;

  // The eyelid left is initialized to the OPEN state (40.0°) globally during boot
  double currentAngle = EYELID_LEFT_MIN_ANGLE;

  while (true) {
    unsigned long openDuration = random(2000, 5000);
    vTaskDelay(pdMS_TO_TICKS(openDuration));

    Serial.println("[EYELID LEFT] Closing eyelid smoothly to 180.0°...");
    while (currentAngle < EYELID_LEFT_MAX_ANGLE) {
      currentAngle += 2.0;
      if (currentAngle > EYELID_LEFT_MAX_ANGLE) currentAngle = EYELID_LEFT_MAX_ANGLE;
      safeSetServoAngle(EYELID_LEFT_CHANNEL, currentAngle, EYELID_LEFT_MIN_ANGLE, EYELID_LEFT_MAX_ANGLE);
      vTaskDelay(pdMS_TO_TICKS(15));
    }

    vTaskDelay(pdMS_TO_TICKS(150));

    Serial.println("[EYELID LEFT] Opening eyelid smoothly to 40.0°...");
    while (currentAngle > EYELID_LEFT_MIN_ANGLE) {
      currentAngle -= 2.0;
      if (currentAngle < EYELID_LEFT_MIN_ANGLE) currentAngle = EYELID_LEFT_MIN_ANGLE;
      safeSetServoAngle(EYELID_LEFT_CHANNEL, currentAngle, EYELID_LEFT_MIN_ANGLE, EYELID_LEFT_MAX_ANGLE);
      vTaskDelay(pdMS_TO_TICKS(25));
    }
  }
}
