#include <Arduino.h>
#include "core/SystemTasks.h"
#include "controllers/ProtocolDispatcher.h"

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); }

  Serial.println("\n\n-----------------------------------");
  Serial.println("SYSTEM BOOTING... (115200 Baud)");
  Serial.println("-----------------------------------\n");

  // Initialize Isolated Services
  // Singletons will auto-instantiate on their first use inside these tasks.
  xTaskCreatePinnedToCore(kinematicsTask, "Kinematics", 4096, NULL, 5, NULL, 1);
  xTaskCreatePinnedToCore(jsonParserTask, "JSON_Parser", 4096, NULL, 10, NULL, 1);
  xTaskCreatePinnedToCore(networkTask, "Network", 4096, NULL, 20, NULL, 0);
  
  // Create Staggered Boot task (runs once on Core 1, then deletes itself)
  xTaskCreatePinnedToCore(staggeredBootTask, "Boot", 4096, NULL, 2, NULL, 1);
}

void loop() {
  // Main loop is intentionally empty.
  // All logic is handled by dedicated FreeRTOS tasks (Kinematics, Parser, Network).
  vTaskDelay(pdMS_TO_TICKS(100));
}
