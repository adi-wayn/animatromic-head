#include "eyes_y.h"
#include "driver.h"

void eyesYTask(void *pvParameters) {
  (void) pvParameters;

  // Eyes Y starts at center (90.0°), initialized globally during boot
  double currentAngle = EYES_Y_CENTER_ANGLE;

  // Velocity limiting speed (1 degree every 60ms)
  const TickType_t stepDelay = pdMS_TO_TICKS(60);

  while (true) {
    // Sweep Up (to 100°)
    Serial.println("[EYES Y] Sweeping eyes UP to 100.0°...");
    while (currentAngle < EYES_Y_MAX_ANGLE) {
      currentAngle += 1.0;
      safeSetServoAngle(EYES_Y_CHANNEL, currentAngle, EYES_Y_MIN_ANGLE, EYES_Y_MAX_ANGLE);
      vTaskDelay(stepDelay);
    }
    vTaskDelay(pdMS_TO_TICKS(1200)); // Hold for 1.2 seconds

    // Sweep Down (to 80°)
    Serial.println("[EYES Y] Sweeping eyes DOWN to 80.0°...");
    while (currentAngle > EYES_Y_MIN_ANGLE) {
      currentAngle -= 1.0;
      safeSetServoAngle(EYES_Y_CHANNEL, currentAngle, EYES_Y_MIN_ANGLE, EYES_Y_MAX_ANGLE);
      vTaskDelay(stepDelay);
    }
    vTaskDelay(pdMS_TO_TICKS(1200)); // Hold for 1.2 seconds

    // Sweep back to Center (to 90°)
    Serial.println("[EYES Y] Returning eyes Y-axis to center (90.0°)...");
    while (currentAngle < EYES_Y_CENTER_ANGLE) {
      currentAngle += 1.0;
      safeSetServoAngle(EYES_Y_CHANNEL, currentAngle, EYES_Y_MIN_ANGLE, EYES_Y_MAX_ANGLE);
      vTaskDelay(stepDelay);
    }
    vTaskDelay(pdMS_TO_TICKS(2500)); // Hold for 2.5 seconds
  }
}
