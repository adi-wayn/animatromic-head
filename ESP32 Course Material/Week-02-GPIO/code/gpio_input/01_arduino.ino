//Basa"d
#include <Arduino.h>

const int LED_PIN = 14;
const int LED_Built_In_PIN = 2;
const int BUTTON_PIN = 4;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_Built_In_PIN, OUTPUT);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  //pinMode(BUTTON_PIN, INPUT);
}

void loop() {
  int button_state = digitalRead(BUTTON_PIN);

  if (button_state == LOW) {
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(LED_Built_In_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
    digitalWrite(LED_Built_In_PIN, LOW);
  }
}
