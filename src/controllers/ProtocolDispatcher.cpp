#include "controllers/ProtocolDispatcher.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include "controllers/AnimatronicHead.h"
#include "controllers/NetworkManager.h"

void jsonParserTask(void *pvParameters) {
  (void) pvParameters;
  String incomingJson;
  while (true) {
    AnimatronicHead& head = AnimatronicHead::getInstance();
    NetworkManager& network = NetworkManager::getInstance();

    if (head.isBooted() && network.getNextMessage(incomingJson)) {
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
          const char* emotion = doc["cognitive_state"]["emotion_primary"];
          if (emotion) {
            head.executePose(emotion);
            Serial.printf("[Dispatcher] Executing Pose: %s\n", emotion);
          }
        }
        // Handle Direct Physical Commands
        else if (doc.containsKey("physical_command")) {
          head.setState(SystemState::IDLE_LISTENING);
          const char* command = doc["physical_command"];
          if (command) {
            head.executePose(command);
            Serial.printf("[Dispatcher] Executing Command: %s\n", command);
          }
        }
      } else {
        Serial.printf("[Dispatcher] JSON Parse Error: %s\n", error.c_str());
      }
    }
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}
