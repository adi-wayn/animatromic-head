#include <Arduino.h>
#define LED 35   // for esp32-s3.
int i;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  i = 0;
  Serial.println("Hello, ESP32-S3!");
}

void loop() {
  // put your main code here, to run repeatedly:

  Serial.println(String("Hello, shuki ") + i);
  i = i + 1;
  if (i % 10 == 0) {
    digitalWrite(LED, HIGH);
    delay(1500);
    digitalWrite(LED, LOW);
  }
  else
    delay(500);   // this speeds up the simulation
}
