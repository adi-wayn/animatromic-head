// button event -> ISR runs -> counter changes -> loop prints result
#include <Arduino.h>

// Global variables for the button
const uint8_t buttonPin = 18;
volatile int32_t counter = 0;
volatile bool pressed = false;

// Interrupt Service Routine (ISR)
void ARDUINO_ISR_ATTR buttonISR() {
  counter++;
  pressed = true;
}

void setup() {
  Serial.begin(115200);

  pinMode(buttonPin, INPUT_PULLUP);

  attachInterrupt(buttonPin, buttonISR, RISING);

  Serial.println("Press the button on GPIO 18.");
}

void loop() {
  if (pressed) {
    Serial.print("Button pressed ");
    Serial.print(counter);
    Serial.println(" times.");

    pressed = false;
  }

  delay(10);
}

/*
Why Volatile?
The `counter` variable is marked as volatile because it is changed inside the ISR and read inside
loop(). Without volatile, the compiler may assume that counter only changes inside normal code.
But the ISR can change it "behind the compiler's back," like a tiny hardware goblin.
*/
