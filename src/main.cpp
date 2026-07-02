#include <Arduino.h>
#include "driver.h"
#include "neck_y.h"
#include "jaw_ud.h"
#include "neck_one.h"

// Sequential orchestration task to test Channel 0, 1, and 3 safely
void sequentialServoTestTask(void *pvParameters) {
  (void) pvParameters;

  double yAngle = NECK_Y_CENTER_ANGLE;
  double jawAngle = JAW_UD_CENTER_ANGLE;
  double oneAngle = NECK_ONE_CENTER_ANGLE;

  const TickType_t stepDelay = pdMS_TO_TICKS(35); // Slow and gentle 35ms steps

  while (true) {
    // Reset positions to center constants at the start of each cycle
    yAngle = NECK_Y_CENTER_ANGLE;     // 110.0
    jawAngle = JAW_UD_CENTER_ANGLE;   // 90.0
    oneAngle = NECK_ONE_CENTER_ANGLE; // 70.0

    // 1. Energize Neck Y and Neck One to their centers to hold the head up and aligned
    Serial.println("\n[SEQUENTIAL TEST] Holding Neck Y and Neck One at center...");
    safeSetServoAngle(NECK_Y_CHANNEL, yAngle, NECK_Y_MIN_ANGLE, NECK_Y_MAX_ANGLE);
    safeSetServoAngle(NECK_ONE_CHANNEL, oneAngle, NECK_ONE_MIN_ANGLE, NECK_ONE_MAX_ANGLE);
    vTaskDelay(pdMS_TO_TICKS(1500)); // Give them 1.5 seconds to stabilize and take load

    // === 1. NECK Y SWEEP (Channel 0) ===
    Serial.println("\n[SEQUENTIAL TEST] Step 1: Sweeping Neck Y (CH0)...");
    
    // Sweep Right/Down
    while (yAngle < NECK_Y_MAX_ANGLE) {
      yAngle += 1.0;
      safeSetServoAngle(NECK_Y_CHANNEL, yAngle, NECK_Y_MIN_ANGLE, NECK_Y_MAX_ANGLE);
      vTaskDelay(stepDelay);
    }
    vTaskDelay(pdMS_TO_TICKS(500));

    // Sweep Left/Up
    while (yAngle > NECK_Y_MIN_ANGLE) {
      yAngle -= 1.0;
      safeSetServoAngle(NECK_Y_CHANNEL, yAngle, NECK_Y_MIN_ANGLE, NECK_Y_MAX_ANGLE);
      vTaskDelay(stepDelay);
    }
    vTaskDelay(pdMS_TO_TICKS(500));

    // Return to Center
    while (yAngle < NECK_Y_CENTER_ANGLE) {
      yAngle += 1.0;
      safeSetServoAngle(NECK_Y_CHANNEL, yAngle, NECK_Y_MIN_ANGLE, NECK_Y_MAX_ANGLE);
      vTaskDelay(stepDelay);
    }
    
    // Note: Do NOT detach Neck Y! Keep it active to hold the head up.
    Serial.println("[SEQUENTIAL TEST] Neck Y finished sweep. Holding center position.");
    vTaskDelay(pdMS_TO_TICKS(1000));

    // === 2. NECK ONE SWEEP (Channel 3) ===
    Serial.println("\n[SEQUENTIAL TEST] Step 2: Sweeping Neck One (CH3)...");
    
    // Sweep Right/Down
    while (oneAngle < NECK_ONE_MAX_ANGLE) {
      oneAngle += 1.0;
      safeSetServoAngle(NECK_ONE_CHANNEL, oneAngle, NECK_ONE_MIN_ANGLE, NECK_ONE_MAX_ANGLE);
      vTaskDelay(stepDelay);
    }
    vTaskDelay(pdMS_TO_TICKS(500));

    // Sweep Left/Up
    while (oneAngle > NECK_ONE_MIN_ANGLE) {
      oneAngle -= 1.0;
      safeSetServoAngle(NECK_ONE_CHANNEL, oneAngle, NECK_ONE_MIN_ANGLE, NECK_ONE_MAX_ANGLE);
      vTaskDelay(stepDelay);
    }
    vTaskDelay(pdMS_TO_TICKS(500));

    // Return to Center
    while (oneAngle < NECK_ONE_CENTER_ANGLE) {
      oneAngle += 1.0;
      safeSetServoAngle(NECK_ONE_CHANNEL, oneAngle, NECK_ONE_MIN_ANGLE, NECK_ONE_MAX_ANGLE);
      vTaskDelay(stepDelay);
    }
    
    // Note: Do NOT detach Neck One! Keep it active to hold left/right alignment.
    Serial.println("[SEQUENTIAL TEST] Neck One finished sweep. Holding center position.");
    vTaskDelay(pdMS_TO_TICKS(1000));

    // === 3. JAW UP/DOWN SWEEP (Channel 1) ===
    // Neck Y and Neck One are currently active and holding the head up and aligned!
    Serial.println("\n[SEQUENTIAL TEST] Step 3: Sweeping Jaw Up/Down (CH1)...");
    
    // Open Mouth (Decrease angle to open jaw physically)
    while (jawAngle > JAW_UD_MIN_ANGLE) {
      jawAngle -= 1.5;
      if (jawAngle < JAW_UD_MIN_ANGLE) jawAngle = JAW_UD_MIN_ANGLE;
      safeSetServoAngle(JAW_UD_CHANNEL, jawAngle, JAW_UD_MIN_ANGLE, JAW_UD_MAX_ANGLE);
      vTaskDelay(stepDelay);
    }
    vTaskDelay(pdMS_TO_TICKS(1000)); // Hold open

    // Close Mouth (Increase angle to close jaw physically)
    while (jawAngle < JAW_UD_MAX_ANGLE) {
      jawAngle += 1.5;
      if (jawAngle > JAW_UD_MAX_ANGLE) jawAngle = JAW_UD_MAX_ANGLE;
      safeSetServoAngle(JAW_UD_CHANNEL, jawAngle, JAW_UD_MIN_ANGLE, JAW_UD_MAX_ANGLE);
      vTaskDelay(stepDelay);
    }
    
    // Detach Jaw to relax it
    detachServo(JAW_UD_CHANNEL);
    Serial.println("[SEQUENTIAL TEST] Jaw Up/Down finished and detached.");
    
    // === 4. RELAX NECK SERVOS FOR COOLDOWN ===
    // Detach Neck Y and Neck One now that the active cycle is complete
    detachServo(NECK_Y_CHANNEL);
    detachServo(NECK_ONE_CHANNEL);
    Serial.println("[SEQUENTIAL TEST] Neck Y and Neck One detached for cooldown.");
    
    Serial.println("\n[SEQUENTIAL TEST] Cycle complete. Resting for 5 seconds...");
    vTaskDelay(pdMS_TO_TICKS(5000));
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10); // Wait for serial monitor connection
  }
  Serial.println("==================================================");
  Serial.println("ESP32 Sequential Multi-Servo Safety Test");
  Serial.println("CH0: Neck Y | CH1: Jaw Up/Down | CH3: Neck One");
  Serial.println("==================================================");

  // Initialize PCA9685 driver and setup I2C bus
  initDriver();
  Serial.println("PCA9685 Driver initialized successfully.");

  Serial.println("=== INITIAL START: CENTERING ALL SERVOS ===");
  safeSetServoAngle(NECK_Y_CHANNEL, NECK_Y_CENTER_ANGLE, NECK_Y_MIN_ANGLE, NECK_Y_MAX_ANGLE);
  safeSetServoAngle(JAW_UD_CHANNEL, JAW_UD_CENTER_ANGLE, JAW_UD_MIN_ANGLE, JAW_UD_MAX_ANGLE);
  safeSetServoAngle(NECK_ONE_CHANNEL, NECK_ONE_CENTER_ANGLE, NECK_ONE_MIN_ANGLE, NECK_ONE_MAX_ANGLE);
  
  delay(1500); // Give servos time to reach center
  
  // Detach all servos to rest and save power
  detachServo(NECK_Y_CHANNEL);
  detachServo(JAW_UD_CHANNEL);
  detachServo(NECK_ONE_CHANNEL);
  Serial.println("All servos centered and detached. Starting loop.");

  // Spawn sequential test task on Core 1
  xTaskCreatePinnedToCore(
    sequentialServoTestTask,
    "SequentialServoTestTask",
    4096,
    NULL,
    1,
    NULL,
    1
  );
  Serial.println("SequentialServoTestTask spawned on Core 1.");
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}
