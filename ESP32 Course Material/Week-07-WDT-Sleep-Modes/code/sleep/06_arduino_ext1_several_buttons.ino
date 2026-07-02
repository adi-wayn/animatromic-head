#include <Arduino.h>

#define BUTTON_1 GPIO_NUM_32
#define BUTTON_2 GPIO_NUM_33

#define BUTTON_PIN_BITMASK ((1ULL << BUTTON_1) | (1ULL << BUTTON_2))

RTC_DATA_ATTR int bootCount = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  bootCount++;

  Serial.println();
  Serial.printf("Boot count: %d\n", bootCount);

  esp_sleep_wakeup_cause_t reason = esp_sleep_get_wakeup_cause();

  if (reason == ESP_SLEEP_WAKEUP_EXT1) {
    Serial.println("Wakeup caused by EXT1");

    uint64_t wakeupPinMask = esp_sleep_get_ext1_wakeup_status();

    if (wakeupPinMask & (1ULL << BUTTON_1)) {
      Serial.println("GPIO32 caused wakeup");
    }

    if (wakeupPinMask & (1ULL << BUTTON_2)) {
      Serial.println("GPIO33 caused wakeup");
    }
  } else {
    Serial.printf("Wakeup reason: %d\n", reason);
  }

  pinMode(BUTTON_1, INPUT_PULLDOWN);
  pinMode(BUTTON_2, INPUT_PULLDOWN);

  esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH);

  Serial.println("Going to deep sleep. Press either button to wake.");
  Serial.flush();

  esp_deep_sleep_start();
}

void loop() {
}
