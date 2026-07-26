#include "controllers/ProtocolDispatcher.h"
#include <Arduino.h>
#include "controllers/ProtocolParser.h"
#include "controllers/AnimatronicHead.h"
#include "controllers/NetworkManager.h"
#include "controllers/AudioManager.h"
#include "esp_task_wdt.h"

void jsonParserTask(void *pvParameters) {
  (void) pvParameters;
  String incomingJson;
  ESP_ERROR_CHECK(esp_task_wdt_add(NULL));
  while (true) {
    ESP_ERROR_CHECK(esp_task_wdt_reset());
    AnimatronicHead& head = AnimatronicHead::getInstance();
    NetworkManager& network = NetworkManager::getInstance();

    if (head.isBooted() && network.getNextMessage(incomingJson)) {
      ParsedMessage msg = ProtocolParser::parse(incomingJson);
      
      if (msg.isValid) {
        switch (msg.type) {
            case MessageType::EMERGENCY_STOP:
                head.setState(SystemState::INTERRUPTED);
                AudioManager::getInstance().flushSpeaker();
                PoseController::getInstance().executePose("JAW_CLOSE");
                Serial.println("[Dispatcher] State Changed: INTERRUPTED");
                break;
                
            case MessageType::INTENT: {
                const char* emotion = msg.payload["emotion_primary"];
                if (emotion) {
                    head.executePose(emotion);
                    Serial.printf("[Dispatcher] Executing Pose: %s\n", emotion);
                }
                break;
            }
                
            case MessageType::PHASE_UPDATE: {
                const char* phase = msg.payload["conversational_phase"];
                if (phase) {
                    if (strcmp(phase, "LISTENING") == 0) head.setState(SystemState::IDLE_LISTENING);
                    else if (strcmp(phase, "SPEAKING") == 0) head.setState(SystemState::SPEAKING_SYNCING);
                    else if (strcmp(phase, "MOVING") == 0) head.setState(SystemState::IDLE_LISTENING);
                    Serial.printf("[Dispatcher] Phase Update: %s\n", phase);
                }
                break;
            }
                
            case MessageType::TTS_COMPLETE:
                head.setState(SystemState::IDLE_LISTENING);
                Serial.println("[Dispatcher] TTS_COMPLETE received. State → IDLE_LISTENING.");
                break;
                
            case MessageType::UNKNOWN:
            default:
                Serial.println("[Dispatcher] Unknown message type received.");
                break;
        }
      }
    }
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}
