#include <Arduino.h>

const int LED_PIN = 14;
const int LED_Built_In_PIN = 2;

const int BLINK_DELAY_MS = 1000;
const int Blink_led_off = 300;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_Built_In_PIN, OUTPUT);
}

void loop() {
  digitalWrite(LED_PIN, HIGH);
  digitalWrite(LED_Built_In_PIN, HIGH);
  delay(BLINK_DELAY_MS);

  digitalWrite(LED_PIN, LOW);
  digitalWrite(LED_Built_In_PIN, LOW);
  delay(Blink_led_off);
}
