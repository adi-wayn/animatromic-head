#include <Arduino.h>
#include "controllers/AnimatronicHead.h"
#include "hardware/PCA9685_Driver.h"
#include "controllers/NetworkManager.h"
#include <ArduinoJson.h>

AnimatronicHead head;
NetworkManager network;
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

// --- Service 1: Network Transport (Core 0, High Priority) ---
void networkTask(void *pvParameters) {
  (void) pvParameters;
  network.begin();
  while (true) {
    network.update();
    vTaskDelay(pdMS_TO_TICKS(5));
  }
}

// --- Service 2: Protocol Dispatcher (Core 1, Medium Priority) ---
void jsonParserTask(void *pvParameters) {
  (void) pvParameters;
  String incomingJson;
  while (true) {
    if (isFullyBooted && network.getNextMessage(incomingJson)) {
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, incomingJson);
      
      if (!error) {
        if (doc.containsKey("command") && strcmp(doc["command"], "EMERGENCY_STOP") == 0) {
          head.setState(SystemState::INTERRUPTED);
          Serial.println("[Dispatcher] State Changed: INTERRUPTED");
        }
        else if (doc.containsKey("cognitive_state")) {
          head.setState(SystemState::SPEAKING_SYNCING);
          Serial.println("[Dispatcher] State Changed: SPEAKING_SYNCING");
        }
      } else {
        Serial.printf("[Dispatcher] JSON Parse Error: %s\n", error.c_str());
      }
    }
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); }

  Serial.println("\n\n-----------------------------------");
  Serial.println("SYSTEM BOOTING... (115200 Baud)");
  Serial.println("-----------------------------------\n");

  // Initialize Isolated Services
  // Kinematic Service (Core 1)
  xTaskCreatePinnedToCore(kinematicsTask, "Kinematics", 4096, NULL, 5, NULL, 1);
  
  // Protocol Dispatcher Service (Core 1)
  xTaskCreatePinnedToCore(jsonParserTask, "JSON_Parser", 4096, NULL, 10, NULL, 1);
  
  // Network Transport Service (Core 0)
  xTaskCreatePinnedToCore(networkTask, "Network", 4096, NULL, 20, NULL, 0);

  // Create Staggered Boot task (runs once on Core 1, then deletes itself)
  xTaskCreatePinnedToCore(staggeredBootTask, "Boot", 4096, NULL, 2, NULL, 1);
}

void loop() {
  // Core 0 loop is intentionally empty right now. Will handle Wi-Fi/UDP in Phase 2/4.
  vTaskDelay(pdMS_TO_TICKS(100));
}
