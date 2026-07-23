//Basad

#include <Arduino.h>

const int LED_PIN = 14;
const int LED_Built_In_PIN = 2;
const int BUTTON_PIN = 4;

bool led_state = false;

int stable_button_state = HIGH;
int last_reading = HIGH;

unsigned long last_debounce_time = 0;
const unsigned long debounce_delay = 20;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_Built_In_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void loop() {
  int current_reading = digitalRead(BUTTON_PIN);

  if (current_reading != last_reading) {
    last_debounce_time = millis();
  }

  if ((millis() - last_debounce_time) > debounce_delay) {
    if (current_reading != stable_button_state) {
      stable_button_state = current_reading;

      if (stable_button_state == LOW) {
        led_state = !led_state;
      }
    }
  }

  last_reading = current_reading;

  digitalWrite(LED_PIN, led_state);
  digitalWrite(LED_Built_In_PIN, led_state);
}
