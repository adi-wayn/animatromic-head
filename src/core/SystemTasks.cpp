#include "core/SystemTasks.h"
#include <Arduino.h>
#include "controllers/AnimatronicHead.h"
#include "controllers/NetworkManager.h"
#include "hardware/PCA9685_Driver.h"

void kinematicsTask(void *pvParameters) {
  (void) pvParameters;
  while(true) {
    if (AnimatronicHead::getInstance().isBooted()) {
      AnimatronicHead::getInstance().updateKinematics();
    }
    vTaskDelay(pdMS_TO_TICKS(15)); // ~60Hz update loop
  }
}

void staggeredBootTask(void *pvParameters) {
  (void) pvParameters;
  Serial.println("Starting Staggered Boot Sequence...");
  
  AnimatronicHead::getInstance().begin();

  // Array of all servos to initialize safely
  ServoConfig servos[] = {NECK_ONE, NECK_Y, NECK_ROLL, EYES_X, EYES_Y, JAW_UD, JAW_LR, EYELID_LEFT, EYELID_RIGHT};
  int numServos = sizeof(servos) / sizeof(ServoConfig);

  for (int i = 0; i < numServos; i++) {
    Serial.printf("Initializing Servo Channel %d...\n", servos[i].channel);
    // Move to center/safe angle
    double initAngle = servos[i].centerAngle;
    if (servos[i].channel == EYELID_LEFT.channel) initAngle = EYELID_LEFT.minAngle;
    if (servos[i].channel == EYELID_RIGHT.channel) initAngle = EYELID_RIGHT.minAngle;
    
    safeSetServoAngle(servos[i].channel, initAngle, servos[i].minAngle, servos[i].maxAngle);
    
    // Wait 500ms for current spike to subside before turning on the next servo
    vTaskDelay(pdMS_TO_TICKS(500));
  }

  Serial.println("Staggered Boot Complete. System Ready.");
  AnimatronicHead::getInstance().setBooted(true);
  
  // Task is done, delete itself
  vTaskDelete(NULL);
}

// --- Service 1: Network Transport (Core 0, High Priority) ---
void networkTask(void *pvParameters) {
  (void) pvParameters;
  NetworkManager::getInstance().begin();
  while (true) {
    NetworkManager::getInstance().update();
    vTaskDelay(pdMS_TO_TICKS(5));
  }
}
