import processing.serial.*;

Serial myPort;

int oldMouseXByte = -1;
int oldMouseYByte = -1;

void setup() {
  size(1200, 800);

  // Print all available serial ports
  printArray(Serial.list());

  // Change the index [0] to the correct port from the printed list
  String portName = Serial.list()[3];

  myPort = new Serial(this, portName, 115200);
}

void draw() {

  int mouseXByte = int(map(mouseX, 0, width - 1, 0, 255));
  int mouseYByte = int(map(mouseY, 0, height - 1, 0, 255));

  mouseXByte = constrain(mouseXByte, 0, 255);
  mouseYByte = constrain(mouseYByte, 0, 255);

  background(mouseXByte, mouseYByte, 0);
  if (mouseXByte != oldMouseXByte || mouseYByte != oldMouseYByte) {
    //myPort.write('S');
    myPort.write(mouseXByte); // first byte
    myPort.write(mouseYByte); // second byte

    oldMouseXByte = mouseXByte;
    oldMouseYByte = mouseYByte;
  }
}
