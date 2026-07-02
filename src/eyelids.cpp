#include "eyelids.h"
#include "driver.h"

void eyelidsTask(void *pvParameters) {
  (void) pvParameters;

  // The eyelids are already initialized to the OPEN state (40.0°) globally during boot
  double currentAngle = EYELIDS_MIN_ANGLE;

  while (true) {
    // 1. Keep eyes open for a random interval between 2 and 5 seconds
    unsigned long openDuration = random(2000, 5000);
    vTaskDelay(pdMS_TO_TICKS(openDuration));

    // 2. Sweep Eyelids Closed (up to 180.0°) - Slower and highly visible
    Serial.println("[EYELIDS] Closing eyelids smoothly to 180.0°...");
    while (currentAngle < EYELIDS_MAX_ANGLE) {
      currentAngle += 2.0; // 2 degree steps
      if (currentAngle > EYELIDS_MAX_ANGLE) {
        currentAngle = EYELIDS_MAX_ANGLE;
      }
      safeSetServoAngle(EYELIDS_CHANNEL, currentAngle, EYELIDS_MIN_ANGLE, EYELIDS_MAX_ANGLE);
      vTaskDelay(pdMS_TO_TICKS(15)); // 15ms delay per step (smooth & visible)
    }

    // 3. Hold eyes closed for a brief moment (150ms)
    vTaskDelay(pdMS_TO_TICKS(150));

    // 4. Open Eyelids back up (down to 40.0°) - Slower return sweep
    Serial.println("[EYELIDS] Opening eyelids smoothly to 40.0°...");
    while (currentAngle > EYELIDS_MIN_ANGLE) {
      currentAngle -= 2.0; // 2 degree steps
      if (currentAngle < EYELIDS_MIN_ANGLE) {
        currentAngle = EYELIDS_MIN_ANGLE;
      }
      safeSetServoAngle(EYELIDS_CHANNEL, currentAngle, EYELIDS_MIN_ANGLE, EYELIDS_MAX_ANGLE);
      vTaskDelay(pdMS_TO_TICKS(25)); // 25ms delay per step
    }
  }
}
