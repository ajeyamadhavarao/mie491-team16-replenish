#include <Wire.h>

void setup() {
  Wire.begin(8);                 // Initialize as I2C slave with address 8
  Wire.onReceive(receiveEvent);  // Register event for receiving data
  Serial.begin(9600);            // Start serial communication
}

void loop() {
  // Nothing to do here
}

void receiveEvent(int howMany) {
  if (howMany >= 2) {  // Ensure we receive at least 4 bytes (size of uint32_t)
    int howMany = Wire.read() << 8 | Wire.read();
    Serial.print("Received I2C: ");
    Serial.println(howMany);
  }
}
