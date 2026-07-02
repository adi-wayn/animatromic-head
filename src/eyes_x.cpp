#include "eyes_x.h"
#include "driver.h"

void eyesXTask(void *pvParameters) {
  (void) pvParameters;

  // Eyes X starts at center (90.0°), initialized globally during boot
  double currentAngle = EYES_X_CENTER_ANGLE;

  // Velocity limiting speed (1 degree every 35ms)
  const TickType_t stepDelay = pdMS_TO_TICKS(35);

  while (true) {
    // Sweep Left (to 50°)
    Serial.println("[EYES X] Sweeping eyes LEFT to 50.0°...");
    while (currentAngle > EYES_X_MIN_ANGLE) {
      currentAngle -= 1.0;
      safeSetServoAngle(EYES_X_CHANNEL, currentAngle, EYES_X_MIN_ANGLE, EYES_X_MAX_ANGLE);
      vTaskDelay(stepDelay);
    }
    vTaskDelay(pdMS_TO_TICKS(1500)); // Hold for 1.5 seconds

    // Sweep Right (to 130°)
    Serial.println("[EYES X] Sweeping eyes RIGHT to 130.0°...");
    while (currentAngle < EYES_X_MAX_ANGLE) {
      currentAngle += 1.0;
      safeSetServoAngle(EYES_X_CHANNEL, currentAngle, EYES_X_MIN_ANGLE, EYES_X_MAX_ANGLE);
      vTaskDelay(stepDelay);
    }
    vTaskDelay(pdMS_TO_TICKS(1500)); // Hold for 1.5 seconds

    // Sweep back to Center (to 90°)
    Serial.println("[EYES X] Returning eyes X-axis to center (90.0°)...");
    while (currentAngle > EYES_X_CENTER_ANGLE) {
      currentAngle -= 1.0;
      safeSetServoAngle(EYES_X_CHANNEL, currentAngle, EYES_X_MIN_ANGLE, EYES_X_MAX_ANGLE);
      vTaskDelay(stepDelay);
    }
    vTaskDelay(pdMS_TO_TICKS(3000)); // Hold for 3 seconds
  }
}
