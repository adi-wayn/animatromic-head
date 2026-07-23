// Illustrates why pure polling is bad: the button can be missed
// while the CPU is busy with slow work.
void loop() {
  readTemperature();
  sendDataToServer();
  delay(5000);

  if (digitalRead(buttonPin) == LOW) {
    // too late
  }
}
