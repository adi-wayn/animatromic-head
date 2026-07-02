#include "neck_y.h"
#include "driver.h"

void neckYTask(void *pvParameters) {
  (void) pvParameters;

  double currentAngle = NECK_Y_CENTER_ANGLE; // Starts at 90.0
  const TickType_t stepDelay = pdMS_TO_TICKS(35); // Slow and gentle

  while (true) {
    // 1. Sweep Right/Down (to NECK_Y_MAX_ANGLE)
    Serial.print("[NECK_Y] Sweeping to ");
    Serial.print(NECK_Y_MAX_ANGLE);
    Serial.println("°...");
    while (currentAngle < NECK_Y_MAX_ANGLE) {
      currentAngle += 1.0;
      safeSetServoAngle(NECK_Y_CHANNEL, currentAngle, NECK_Y_MIN_ANGLE, NECK_Y_MAX_ANGLE);
      vTaskDelay(stepDelay);
    }
    detachServo(NECK_Y_CHANNEL); // Relax to save power
    vTaskDelay(pdMS_TO_TICKS(2000)); // Hold for 2 seconds

    // 2. Sweep Left/Up (to NECK_Y_MIN_ANGLE)
    Serial.print("[NECK_Y] Sweeping to ");
    Serial.print(NECK_Y_MIN_ANGLE);
    Serial.println("°...");
    while (currentAngle > NECK_Y_MIN_ANGLE) {
      currentAngle -= 1.0;
      safeSetServoAngle(NECK_Y_CHANNEL, currentAngle, NECK_Y_MIN_ANGLE, NECK_Y_MAX_ANGLE);
      vTaskDelay(stepDelay);
    }
    detachServo(NECK_Y_CHANNEL); // Relax to save power
    vTaskDelay(pdMS_TO_TICKS(2000)); // Hold for 2 seconds

    // 3. Sweep back to Center (to NECK_Y_CENTER_ANGLE)
    Serial.print("[NECK_Y] Returning to center (");
    Serial.print(NECK_Y_CENTER_ANGLE);
    Serial.println("°)...");
    while (currentAngle < NECK_Y_CENTER_ANGLE) {
      currentAngle += 1.0;
      safeSetServoAngle(NECK_Y_CHANNEL, currentAngle, NECK_Y_MIN_ANGLE, NECK_Y_MAX_ANGLE);
      vTaskDelay(stepDelay);
    }
    detachServo(NECK_Y_CHANNEL); // Relax to save power
    vTaskDelay(pdMS_TO_TICKS(4000)); // Hold at center for 4 seconds
  }
}
