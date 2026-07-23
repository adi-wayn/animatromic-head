#include <Arduino.h>
#include "esp_task_wdt.h"

#define LED_PIN 21
#define WDT_TIMEOUT_MS 5000

unsigned long startTime;

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(LED_PIN, OUTPUT);

  Serial.println("Configuring Task Watchdog...");

  esp_task_wdt_config_t wdt_config = {
    .timeout_ms = WDT_TIMEOUT_MS,
    .idle_core_mask = 0,
    .trigger_panic = true
  };

  // If TWDT is already initialized by the framework, reconfigure it.
  esp_err_t err = esp_task_wdt_reconfigure(&wdt_config);

  if (err == ESP_ERR_INVALID_STATE) {
    err = esp_task_wdt_init(&wdt_config);
  }

  if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
    Serial.printf("WDT config failed: %s\n", esp_err_to_name(err));
  }

  // Subscribe the current Arduino loop task to the watchdog.
  esp_task_wdt_add(NULL);

  startTime = millis();
}

void loop() {
  digitalWrite(LED_PIN, !digitalRead(LED_PIN));
  Serial.println("Loop is alive. Feeding watchdog.");

  esp_task_wdt_reset();

  delay(1000);

  // After 10 seconds, deliberately stop feeding the watchdog.
  if (millis() - startTime > 10000) {
    Serial.println("Now simulating a stuck program...");
    while (true) {
      // No delay, no yield, no watchdog reset.
      // The watchdog should eventually trigger.
    }
  }
}
