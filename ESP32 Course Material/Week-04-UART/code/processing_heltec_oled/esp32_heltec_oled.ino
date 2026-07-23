#include <Wire.h>
#include "HT_SSD1306Wire.h"

// Direct display object like in official examples
static SSD1306Wire display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED);

int counter = 0;
int number = 0;
uint8_t mouseXValue = 0;
uint8_t mouseYValue = 0;

// Manually defined Vext control function
void VextON()
{
  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, LOW); // LOW turns ON external power (VEXT)
}

void setup()
{
  Serial.begin(115200);
  delay(100);

  //Serial.println("\n\nStarting display...");

  // Turn on external power
  VextON();
  delay(100);

  // Initialize display
  display.init();
  display.setFont(ArialMT_Plain_24);
  display.setTextAlignment(TEXT_ALIGN_CENTER);

  // Show startup message
  display.clear();
  display.drawString(64, 22, "Starting...");
  display.display();

  //Serial.println("Display initialized!");
  delay(2000);
}

void loop()
{

  if (Serial.available() >= 2) {
    mouseXValue = Serial.read(); // first byte
    mouseYValue = Serial.read(); // second byte
  }

  display.clear();

  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_16);

  display.drawString(0, 5, "MouseX: " + String(mouseXValue));
  display.drawString(0, 32, "MouseY: " + String(mouseYValue));

  display.display();

  delay(30);
}
