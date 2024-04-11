#include <Wire.h>

void setup() {
  Serial.begin(115200); // Start the Serial communication
  Wire.begin(); // Start I2C as master
  Serial.println("Enter an integer:");
}

void loop() {
  sendSerialInputAsInteger();
  delay(100); // Small delay to prevent overwhelming the serial input
}

void sendSerialInputAsInteger() {
  if (Serial.available() > 0) {
    String inputString = Serial.readStringUntil('\n'); // Read input
    inputString.trim(); // Trim any whitespace
    if (inputString.length() > 0) {
      int inputInt = inputString.toInt(); // Convert string to int
      Serial.println("Sending: " + String(inputInt));

      // Begin the transmission to a device with address 8
      Wire.beginTransmission(8);

      // Send the integer as two bytes
      Wire.write((inputInt >> 8) & 0xFF); // Send the high byte
      Wire.write(inputInt & 0xFF); // Send the low byte

      // End the transmission
      Wire.endTransmission();
    }
  }
}
