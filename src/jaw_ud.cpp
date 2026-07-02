#include "jaw_ud.h"
#include "driver.h"

// Helper to open and close the jaw to simulate speaking a single word/syllable
void speakSyllable(double &currentAngle, double openTarget) {
  const TickType_t stepDelay = pdMS_TO_TICKS(15); // Fast movement for natural speech

  // 1. Open jaw (decrease angle from 90° towards openTarget)
  while (currentAngle > openTarget) {
    currentAngle -= 2.0; // Quick step down (opens mouth physically)
    if (currentAngle < openTarget) currentAngle = openTarget;
    safeSetServoAngle(JAW_UD_CHANNEL, currentAngle, JAW_UD_MIN_ANGLE, JAW_UD_MAX_ANGLE);
    vTaskDelay(stepDelay);
  }

  vTaskDelay(pdMS_TO_TICKS(random(50, 150))); // Short hold open

  // 2. Close jaw (increase angle back to JAW_UD_MAX_ANGLE)
  while (currentAngle < JAW_UD_MAX_ANGLE) {
    currentAngle += 2.0; // Quick step up (closes mouth physically)
    if (currentAngle > JAW_UD_MAX_ANGLE) currentAngle = JAW_UD_MAX_ANGLE;
    safeSetServoAngle(JAW_UD_CHANNEL, currentAngle, JAW_UD_MIN_ANGLE, JAW_UD_MAX_ANGLE);
    vTaskDelay(stepDelay);
  }

  vTaskDelay(pdMS_TO_TICKS(random(100, 300))); // Brief pause between syllables
}

void jawUDTask(void *pvParameters) {
  (void) pvParameters;

  double currentAngle = JAW_UD_CENTER_ANGLE; // Starts at 90.0 (closed)

  while (true) {
    Serial.println("[JAW_UD] Starting speech sentence...");

    // Generate a sentence with 4 to 8 syllables
    int syllables = random(4, 9);
    for (int i = 0; i < syllables; i++) {
      // Set a random mouth opening width (between 45° and 75°)
      double openTarget = random(45, 76);
      speakSyllable(currentAngle, openTarget);
    }

    // De-energize the servo between sentences to prevent heat, hum, and power draw
    Serial.println("[JAW_UD] Sentence finished, detaching jaw servo...");
    detachServo(JAW_UD_CHANNEL);

    // Pause for a random interval (1.5 to 4.0 seconds) before speaking again
    vTaskDelay(pdMS_TO_TICKS(random(1500, 4001)));
  }
}
