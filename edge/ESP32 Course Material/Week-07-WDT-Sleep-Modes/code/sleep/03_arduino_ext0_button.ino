#include <Arduino.h>

#define BUTTON_PIN GPIO_NUM_33

RTC_DATA_ATTR int bootCount = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  bootCount++;
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Serial.println();
  Serial.println("ESP32 woke up");
  Serial.printf("Boot count: %d\n", bootCount);

  esp_sleep_wakeup_cause_t reason = esp_sleep_get_wakeup_cause();

  if (reason == ESP_SLEEP_WAKEUP_EXT0) {
    Serial.println("Wakeup caused by button on EXT0");
  } else {
    Serial.printf("Wakeup reason: %d\n", reason);
  }

  esp_sleep_enable_ext0_wakeup(BUTTON_PIN, 0);

  Serial.println("Going to deep sleep. Press button to wake.");
  Serial.flush();

  esp_deep_sleep_start();
}

void loop() {
}
