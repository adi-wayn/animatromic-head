#include <Arduino.h>
#include "core/SystemTasks.h"
#include "controllers/ProtocolDispatcher.h"
#include "esp_task_wdt.h"

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); }

  Serial.println("\n\n-----------------------------------");
  Serial.println("SYSTEM BOOTING... (115200 Baud)");
  Serial.println("-----------------------------------\n");

  esp_task_wdt_config_t twdt_config = {
      .timeout_ms = 3000,
      .idle_core_mask = (1 << 0) | (1 << 1),
      .trigger_panic = true
  };
  ESP_ERROR_CHECK(esp_task_wdt_init(&twdt_config));

  // Initialize Isolated Services
  // Singletons will auto-instantiate on their first use inside these tasks.
  xTaskCreatePinnedToCore(kinematicsTask, "Kinematics", 4096, NULL, 5, NULL, 1);
  xTaskCreatePinnedToCore(jsonParserTask, "JSON_Parser", 4096, NULL, 10, NULL, 1);
  xTaskCreatePinnedToCore(idleBehaviorTask, "Idle_Behavior", 4096, NULL, 3, NULL, 1);
  xTaskCreatePinnedToCore(networkTask, "Network", 4096, NULL, 20, NULL, 0);
  xTaskCreatePinnedToCore(audioUplinkTask, "Audio_Uplink", 4096, NULL, 20, NULL, 0);
  xTaskCreatePinnedToCore(audioDownlinkTask, "Audio_Downlink", 4096, NULL, 20, NULL, 0);
  xTaskCreatePinnedToCore(telemetryTask, "Telemetry", 4096, NULL, 5, NULL, 1);
  
  // Create Staggered Boot task (runs once on Core 1, then deletes itself)
  xTaskCreatePinnedToCore(staggeredBootTask, "Boot", 4096, NULL, 2, NULL, 1);
}

void loop() {
  // Main loop is intentionally empty.
  // All logic is handled by dedicated FreeRTOS tasks (Kinematics, Parser, Network).
  vTaskDelay(pdMS_TO_TICKS(100));
}
