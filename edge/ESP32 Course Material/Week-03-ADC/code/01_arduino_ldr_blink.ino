#include <Arduino.h>
const int LDR_PIN = 4;   // ADC pin on ESP32-S3
const int LED_PIN = 21;  // LED pin
int mapLdrToDelay(int adcValue) {
  // Limit the ADC value to expected 12-bit range
  if (adcValue < 0) adcValue = 0;
  if (adcValue > 4095) adcValue = 4095;
  // Convert ADC value to blink delay:
  // brighter -> faster blink
  // darker -> slower blink
  return 100 + ((4095 - adcValue) * 900) / 4095; // 100..1000 ms
}
void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  analogReadResolution(12);                       // 0..4095
  analogSetPinAttenuation(LDR_PIN, ADC_11db);     // for wider voltage range
}
void loop() {
  int adcValue = analogRead(LDR_PIN);
  int delayMs = mapLdrToDelay(adcValue);
  Serial.print("ADC = ");
  Serial.print(adcValue);
  Serial.print(" , Delay = ");
  Serial.print(delayMs);
  Serial.println(" ms");
  digitalWrite(LED_PIN, HIGH);
  delay(delayMs);
  digitalWrite(LED_PIN, LOW);
  delay(delayMs);
}
