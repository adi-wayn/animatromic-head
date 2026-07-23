#include <Arduino.h>

#define uS_TO_S_FACTOR 1000000ULL
#define SLEEP_TIME_SEC 5

int counter = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("Light sleep demo");
}

void loop() {
  counter++;
  // Serial.flush(); Here will not help
  Serial.printf("Before light sleep. Counter = %d\n", counter);
  Serial.flush();

  esp_sleep_enable_timer_wakeup(SLEEP_TIME_SEC * uS_TO_S_FACTOR);

  esp_light_sleep_start();

  Serial.println("Woke up from light sleep");
}

/////////////*. Desired Output

// Before light sleep. Counter = 1
// Woke up from light sleep
// Before light sleep. Counter = 2
// Woke up from light sleep

// */
