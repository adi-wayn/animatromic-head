#include <Arduino.h>
#include "esp_task_wdt.h"

#define LED_PIN LED_BUILTIN
#define WDT_TIMEOUT_MS 5000

unsigned long startTime;

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(LED_PIN, OUTPUT);

  Serial.println("Configuring Task Watchdog...");

  // This core uses the classic Task WDT API: timeout in seconds + panic flag.
  esp_err_t err = esp_task_wdt_init(WDT_TIMEOUT_MS / 1000, true);

  if (err == ESP_ERR_INVALID_STATE) {
    // Already initialized by framework startup; keep going.
    err = ESP_OK;
  }

  if (err != ESP_OK) {
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
      delay(1000);
      Serial.println("Second 1...");
      delay(1000);
      Serial.println("Second 2...");
      delay(1000);
      Serial.println("Second 3...");
      delay(1000);
      Serial.println("Second 4...");
      delay(1000);
      Serial.println("Second 5...");
      delay(1000);
      Serial.println("Second 6...");
      delay(1000);
      Serial.println("Second 7...");
      //esp_task_wdt_reset();
      // No delay, no yield, no watchdog reset.
      // The watchdog should eventually trigger.
    }
  }
}
