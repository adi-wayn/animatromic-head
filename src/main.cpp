#include <Arduino.h>
#include "controllers/AnimatronicHead.h"
#include "hardware/PCA9685_Driver.h"

AnimatronicHead head;
bool isFullyBooted = false;

void kinematicsTask(void *pvParameters) {
  (void) pvParameters;
  while(true) {
    if (isFullyBooted) {
      head.updateKinematics();
    }
    vTaskDelay(pdMS_TO_TICKS(15)); // ~60Hz update loop
  }
}

void staggeredBootTask(void *pvParameters) {
  (void) pvParameters;
  Serial.println("Starting Staggered Boot Sequence...");
  
  head.begin();

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
  isFullyBooted = true;
  
  // Task is done, delete itself
  vTaskDelete(NULL);
}

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); }

  Serial.println("\n\n-----------------------------------");
  Serial.println("SYSTEM BOOTING... (115200 Baud)");
  Serial.println("-----------------------------------\n");

  // Create Kinematics state machine task (runs forever on Core 1)
  xTaskCreatePinnedToCore(kinematicsTask, "Kinematics", 4096, NULL, 1, NULL, 1);
  
  // Create Staggered Boot task (runs once on Core 1, then deletes itself)
  xTaskCreatePinnedToCore(staggeredBootTask, "Boot", 4096, NULL, 2, NULL, 1);
}

void loop() {
  // Core 0 loop is intentionally empty right now. Will handle Wi-Fi/UDP in Phase 2/4.
  vTaskDelay(pdMS_TO_TICKS(100));
}
