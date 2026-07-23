#include <Arduino.h>

#define uS_TO_S_FACTOR 1000000ULL
#define TIME_TO_SLEEP 5

RTC_DATA_ATTR int bootCount = 0;

void printWakeupReason() {
  esp_sleep_wakeup_cause_t wakeupReason = esp_sleep_get_wakeup_cause();

  switch (wakeupReason) {
    case ESP_SLEEP_WAKEUP_TIMER:
      Serial.println("Wakeup caused by timer");
      break;

    case ESP_SLEEP_WAKEUP_EXT0:
      Serial.println("Wakeup caused by EXT0");
      break;

    case ESP_SLEEP_WAKEUP_EXT1:
      Serial.println("Wakeup caused by EXT1");
      break;

    case ESP_SLEEP_WAKEUP_TOUCHPAD:
      Serial.println("Wakeup caused by touchpad");
      break;

    default:
      Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeupReason);
      break;
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  bootCount++;
  Serial.println();
  Serial.println("ESP32 woke up");
  Serial.printf("Boot count: %d\n", bootCount);

  printWakeupReason();

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);

  Serial.printf("Going to deep sleep for %d seconds\n", TIME_TO_SLEEP);
  Serial.flush();

  esp_deep_sleep_start();

  Serial.println("This line will never run");
}

void loop() {
  // Never reached after deep sleep starts
}
