#include "controllers/ProtocolDispatcher.h"
#include <Arduino.h>
#include "controllers/ProtocolParser.h"
#include "controllers/AnimatronicHead.h"
#include "controllers/NetworkManager.h"
#include "controllers/AudioManager.h"
#include "core/PowerManager.h"
#include "esp_task_wdt.h"

// ============================================================
//  JSON Parser / Protocol Dispatcher Task (Core 1, Priority 10)
//
//  OPTIMIZATION: Uses blocking getNextMessage(timeout=25ms)
//  so xQueueReceive blocks for up to 25ms per iteration.
//  CPU is in FreeRTOS tickless idle / Light Sleep during the wait.
//  This replaces the old polling + vTaskDelay(20) spin loop.
//
//  When a message arrives:
//   1. If in LOW_POWER_IDLE → call enterFullPower() (wakeup)
//   2. Update activity timestamp to reset 60s inactivity timer
//   3. Parse and dispatch the message
// ============================================================
void jsonParserTask(void *pvParameters) {
    (void) pvParameters;
    String incomingJson;
    ESP_ERROR_CHECK(esp_task_wdt_add(NULL));

    while (true) {
        ESP_ERROR_CHECK(esp_task_wdt_reset());

        AnimatronicHead& head    = AnimatronicHead::getInstance();
        NetworkManager&  network = NetworkManager::getInstance();

        if (!head.isBooted()) {
            // Must manually yield while waiting for staggeredBootTask
            vTaskDelay(pdMS_TO_TICKS(25));
            continue;
        }

        // ── BLOCKING WAIT (25ms) — replaces polling + vTaskDelay ──
        // CPU yields to FreeRTOS scheduler during the wait.
        if (network.getNextMessage(incomingJson, 25)) {
            // ── WAKEUP TRIGGER ──
            PowerManager::getInstance().enterFullPower();
            head.updateActivityTimestamp();

            ParsedMessage msg = ProtocolParser::parse(incomingJson);

            if (msg.isValid) {
                switch (msg.type) {
                    case MessageType::EMERGENCY_STOP:
                        head.setState(SystemState::INTERRUPTED);
                        AudioManager::getInstance().cancelLocalPlayback();
                        AudioManager::getInstance().flushSpeaker();
                        PoseController::getInstance().executePose("JAW_CLOSE");
                        Serial.println("[Dispatcher] State Changed: INTERRUPTED");
                        break;

                    case MessageType::INTENT: {
                        const char* emotion = msg.payload["emotion_primary"];
                        if (emotion) {
                            if (strcmp(emotion, "JAW") == 0) {
                                float intensity = msg.payload["intensity_level"];
                                PoseController::getInstance().syncJawToAmplitude(intensity);
                            } else {
                                head.executePose(emotion);
                                Serial.printf("[Dispatcher] Executing Pose: %s\n", emotion);
                            }
                        }
                        break;
                    }

                    case MessageType::PHASE_UPDATE: {
                        const char* phase = msg.payload["conversational_phase"];
                        if (phase) {
                            if      (strcmp(phase, "LISTENING") == 0) head.setState(SystemState::IDLE_LISTENING);
                            else if (strcmp(phase, "SPEAKING")  == 0) head.setState(SystemState::SPEAKING_SYNCING);
                            else if (strcmp(phase, "MOVING")    == 0) head.setState(SystemState::IDLE_LISTENING);
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
    }
}
