#include "jaw.h"
#include "driver.h"

void jawTask(void *pvParameters) {
  (void) pvParameters;

  // Start at the center angle
  double currentAngle = JAW_CENTER_ANGLE;

  // Velocity limiting speed (1 degree every 30ms)
  const TickType_t stepDelay = pdMS_TO_TICKS(30);

  while (true) {
    // 1. Sweep Right (to JAW_MAX_ANGLE)
    Serial.print("[JAW] Sweeping jaw right to ");
    Serial.print(JAW_MAX_ANGLE);
    Serial.println("°...");
    while (currentAngle < JAW_MAX_ANGLE) {
      currentAngle += 1.0;
      safeSetServoAngle(JAW_CHANNEL, currentAngle, JAW_MIN_ANGLE, JAW_MAX_ANGLE);
      vTaskDelay(stepDelay);
    }
    detachServo(JAW_CHANNEL); // De-energize servo during hold
    vTaskDelay(pdMS_TO_TICKS(1500)); // Hold for 1.5 seconds

    // 2. Sweep Left (to JAW_MIN_ANGLE)
    Serial.print("[JAW] Sweeping jaw left to ");
    Serial.print(JAW_MIN_ANGLE);
    Serial.println("°...");
    while (currentAngle > JAW_MIN_ANGLE) {
      currentAngle -= 1.0;
      safeSetServoAngle(JAW_CHANNEL, currentAngle, JAW_MIN_ANGLE, JAW_MAX_ANGLE);
      vTaskDelay(stepDelay);
    }
    detachServo(JAW_CHANNEL); // De-energize servo during hold
    vTaskDelay(pdMS_TO_TICKS(1500)); // Hold for 1.5 seconds

    // 3. Sweep back to Center (to JAW_CENTER_ANGLE)
    Serial.print("[JAW] Returning jaw to center (");
    Serial.print(JAW_CENTER_ANGLE);
    Serial.println("°)...");
    while (currentAngle < JAW_CENTER_ANGLE) {
      currentAngle += 1.0;
      safeSetServoAngle(JAW_CHANNEL, currentAngle, JAW_MIN_ANGLE, JAW_MAX_ANGLE);
      vTaskDelay(stepDelay);
    }
    detachServo(JAW_CHANNEL); // De-energize servo during hold
    vTaskDelay(pdMS_TO_TICKS(3000)); // Hold at center for 3 seconds
  }
}
