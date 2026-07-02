#include "neck_one.h"
#include "driver.h"

void neckOneTask(void *pvParameters) {
  (void) pvParameters;

  double currentAngle = NECK_ONE_CENTER_ANGLE; // Starts at 110.0
  const TickType_t stepDelay = pdMS_TO_TICKS(35); // Slow and gentle

  while (true) {
    // 1. Sweep Right/Down (to NECK_ONE_MAX_ANGLE)
    Serial.print("[NECK_ONE] Sweeping to ");
    Serial.print(NECK_ONE_MAX_ANGLE);
    Serial.println("°...");
    while (currentAngle < NECK_ONE_MAX_ANGLE) {
      currentAngle += 1.0;
      safeSetServoAngle(NECK_ONE_CHANNEL, currentAngle, NECK_ONE_MIN_ANGLE, NECK_ONE_MAX_ANGLE);
      vTaskDelay(stepDelay);
    }
    detachServo(NECK_ONE_CHANNEL); // Relax to save power
    vTaskDelay(pdMS_TO_TICKS(2000)); // Hold for 2 seconds

    // 2. Sweep Left/Up (to NECK_ONE_MIN_ANGLE)
    Serial.print("[NECK_ONE] Sweeping to ");
    Serial.print(NECK_ONE_MIN_ANGLE);
    Serial.println("°...");
    while (currentAngle > NECK_ONE_MIN_ANGLE) {
      currentAngle -= 1.0;
      safeSetServoAngle(NECK_ONE_CHANNEL, currentAngle, NECK_ONE_MIN_ANGLE, NECK_ONE_MAX_ANGLE);
      vTaskDelay(stepDelay);
    }
    detachServo(NECK_ONE_CHANNEL); // Relax to save power
    vTaskDelay(pdMS_TO_TICKS(2000)); // Hold for 2 seconds

    // 3. Sweep back to Center (to NECK_ONE_CENTER_ANGLE)
    Serial.print("[NECK_ONE] Returning to center (");
    Serial.print(NECK_ONE_CENTER_ANGLE);
    Serial.println("°)...");
    while (currentAngle < NECK_ONE_CENTER_ANGLE) {
      currentAngle += 1.0;
      safeSetServoAngle(NECK_ONE_CHANNEL, currentAngle, NECK_ONE_MIN_ANGLE, NECK_ONE_MAX_ANGLE);
      vTaskDelay(stepDelay);
    }
    detachServo(NECK_ONE_CHANNEL); // Relax to save power
    vTaskDelay(pdMS_TO_TICKS(4000)); // Hold at center for 4 seconds
  }
}
