#include <Arduino.h>

// Button pin
const uint8_t buttonPin = 18;

// Shared between ISR and loop
volatile bool interruptHappened = false;

// Normal variables used in loop
uint32_t counter = 0;
unsigned long lastPressTime = 0;

// Debounce time
const unsigned long DEBOUNCE_DELAY = 50; // milliseconds

// Interrupt Service Routine
void ARDUINO_ISR_ATTR buttonISR() {
  interruptHappened = true;
}

void setup() {
  Serial.begin(115200);

  pinMode(buttonPin, INPUT_PULLUP);

  // With INPUT_PULLUP:
  // FALLING = button press
  // RISING = button release
  attachInterrupt(buttonPin, buttonISR, FALLING);

  Serial.println("Press the button on GPIO 18.");
}

void loop() {
  if (interruptHappened)
  {
    interruptHappened = false;

    unsigned long now = millis();

    if (now - lastPressTime > DEBOUNCE_DELAY)
    {
      lastPressTime = now;

      counter++;

      Serial.print("Button pressed ");
      Serial.print(counter);
      Serial.println(" times.");
    }
  }

  delay(10);
}
