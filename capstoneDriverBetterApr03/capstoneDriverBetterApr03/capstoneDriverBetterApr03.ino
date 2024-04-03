#include <AccelStepper.h>
#include <stdint.h>  // For uint8_t
#include <Wire.h>

#define DEVICE_L_BIT (1 << 0)  // Bit 0 for device 1 - 0001
#define DEVICE_M_BIT (1 << 1)  // Bit 1 for device 2 - 0010 - left shift is essentially multiplying 1 by 2^1 = 2
#define DEVICE_R_BIT (1 << 2)  // Bit 2 for device 3 - 0100 - multiplying 1 by 2^2 = 4 - which is 0100 is 4 bit binary

// similarly right shifting these bits will divide the number by 2^n
// (4>>2) will make 0100 into 0001

/*
FOR UNDERSTANDING PURPOSE ONLY

uint8_t deviceStates = 0; // All devices off initially

// Turn on device 1
deviceStates |= DEVICE1_BIT;

// Turn off device 2
deviceStates &= ~DEVICE2_BIT;

// Check if device 3 is on
if (deviceStates & DEVICE3_BIT) {
    // Device 3 is on
}

// Toggle device 1
deviceStates ^= DEVICE1_BIT;
*/

#define xStep 2  // y Axis Step Pin
#define yStep 3  // y Axis Step Pin
#define zStep 4  // z Axis Step Pin

#define xDir 5  // x Axis Direction Pin
#define yDir 6  // y Axis Direction Pin
#define zDir 7  // z Axis Direction Pin

#define enPin 12

const int RELAY_PIN_1 = 9;
const int RELAY_PIN_2 = 10;
const int RELAY_PIN_3 = 11;

//Objects for controlling stepper motor with accelStepper library
AccelStepper PumpLeft(1, xStep, xDir);
AccelStepper PumpMiddle(1, yStep, yDir);
AccelStepper PumpRight(1, zStep, zDir);

uint8_t pumpStates = 0;  // All devices off initially

float stepsPerInch = 27;
float radius = 3.59;
int MAX_SPEED = 3000;
int volumeDesired = 0;
String cmdStr;
float moves[3];
long startPositions[3];
// bool ready = false;

void setup() {
  Wire.begin(8);               // Join I2C bus as a slave with address 8
  Wire.onReceive(receiveI2C);  // Register I2C receive event
  Serial.begin(9600);

  // we dont need the arduino waiting for input from the master

  // while (!Serial.available()) {
  //   ;
  // }
  // Serial.readStringUntil('\n');
  // Serial.print("Ard ready!\n");
  pinMode(enPin, OUTPUT);
  pinMode(RELAY_PIN_1, OUTPUT);
  pinMode(RELAY_PIN_2, OUTPUT);
  pinMode(RELAY_PIN_3, OUTPUT);
  updateStartPositions();
}

void loop() {
  // only look for new commands if the motors aren't moving
  Serial.println("In MAIN loop ... ");

  // Waits for ESP32 I2C to receive full information
  Serial.println("~~~  Waiting for CMD  ~~~");
  while (volumeDesired == 0 || pumpStates == 0) {
    
    static unsigned long lastMillis = 0;  // Remember when we were last called
    unsigned long currentMillis = millis();

    // Check if one second has passed (1000 milliseconds)
    if (currentMillis - lastMillis >= 1000) {
      lastMillis = currentMillis;  // Reset the lastMillis to the current time
      Serial.println(currentMillis);

      // Place the code you want to execute once per second here
      // Serial.println("One second has passed.");
    }
  }
  Serial.println("~~~  Ready to pump!  ~~~");
  // Check every loop to see if ESP32 has sent a stop command or the next move command
  // read();  // sets next target position
  // enable motors
  digitalWrite(enPin, LOW);
  // enable valves
  digitalWrite(RELAY_PIN_1, LOW);  // close valve 5 seconds
  digitalWrite(RELAY_PIN_2, LOW);  // close valve 5 seconds
  digitalWrite(RELAY_PIN_3, LOW);  // close valve 5 seconds


  executeI2C();

  PumpMiddle.run();  // middle stepper
  PumpLeft.run();    // left stepper
  PumpRight.run();   // right stepper

  // stopping pumps
  Serial.println("~~~  Stopping pumps!  ~~~");

  pumpStates = 0;
  volumeDesired = 0;
  digitalWrite(RELAY_PIN_1, HIGH);  // open valve 5 seconds
  digitalWrite(RELAY_PIN_2, HIGH);  // open valve 5 seconds
  digitalWrite(RELAY_PIN_3, HIGH);  // open valve 5 seconds
  // stepper.run() determins whether a signal is due based on target position, acceleration, and speed.
  digitalWrite(enPin, HIGH);

  //Sends odometry data to ESP32 to relay to python
  sendMotion();

  // reset odometry (ensures correct behaviour of stop command)
  for (int i = 0; i < 3; i++) {
    moves[i] = 0.0;
  }
}

void receiveI2C(int bytesReceived) {
  if (bytesReceived == 2) {  // Expecting 2 bytes for a four-digit integer
    int receivedValue = Wire.read() << 8 | Wire.read();
    Serial.print("Received I2C: ");
    Serial.println(receivedValue);
    volumeDesired = 0;  // should be zero before new value is received
    switch (receivedValue) {
      case 1:
        pumpStates = 0;
        pumpStates |= DEVICE_L_BIT;  // using left pump's bit mask - turning on device state of left pump
        Serial.print("pumpState is ");
        Serial.println(pumpStates);
        break;
      case 2:
        pumpStates = 0;
        pumpStates |= DEVICE_M_BIT;  // using middle pump's bit mask - turning on device state of middle pump
        Serial.print("pumpState is ");
        Serial.println(pumpStates);
        break;
      case 4:
        pumpStates = 0;
        pumpStates |= DEVICE_R_BIT;  // using right pump's bit mask - turning on device state of right pump
        Serial.print("pumpState is ");
        Serial.println(pumpStates);
        break;
      case 250:
        volumeDesired = 250;
        Serial.print("volumeDesired is ");
        Serial.println(volumeDesired);
        break;
      case 500:
        volumeDesired = 500;
        Serial.print("volumeDesired is ");
        Serial.println(volumeDesired);
        break;
      case 750:
        volumeDesired = 750;
        Serial.print("volumeDesired is ");
        Serial.println(volumeDesired);
        break;
      case 1000:
        volumeDesired = 1000;
        Serial.print("volumeDesired is ");
        Serial.println(volumeDesired);
        break;
      default:
        // ready = false;
        pumpStates = 0;
        volumeDesired = 0;
        Serial.print("defaulted to ");
        Serial.println(volumeDesired);
    }
  }
}
void executeI2C() {
  // read the pumpStates
  Serial.println("In executeI2C loop ... ");
  if (volumeDesired != 0) {
    switch (pumpStates) {
      case 1:
        Serial.print("Pumping LEFT - ");
        Serial.print(volumeDesired);
        Serial.println("ml");
        // add rotate function
        break;
      case 2:
        Serial.print("Pumping MIDDLE - ");
        Serial.print(volumeDesired);
        Serial.println("ml");
        // add rotate function
        break;
      case 4:
        Serial.print("Pumping RIGHT - ");
        Serial.print(volumeDesired);
        Serial.println("ml");
        // add rotate function
        break;
    }
  }
}

void read() {
  // exits and makes no changes if serial buffer is empty
  Serial.println("In read() loop ... ");
  if (Serial.available() > 0) {
    cmdStr = Serial.readString();

    switch (cmdStr.charAt(0)) {
      case 'l':
        // Remove characters to get only the inch distance value
        cmdStr.remove(0, 2);
        rotateCWLeft(cmdStr.toFloat());
        break;
      case 'm':
        // Remove characters to get only the inch distance value
        cmdStr.remove(0, 2);
        rotateCWMiddle(cmdStr.toFloat());
        break;
      case 'r':
        // Remove characters to get only the inch distance value
        cmdStr.remove(0, 2);
        rotateCWRight(cmdStr.toFloat());
        break;
      case 's':
        // Remove characters to get only the inch distance value
        stopAll();
        break;
    }
  }
}


void rotateCW(float angle) {
  // runs the steppers ONE STEP if a step is due

  updateStartPositions();
  moves[0] = 0.0;
  moves[1] = 0.0;
  moves[2] = angle;

  PumpMiddle.setMaxSpeed(MAX_SPEED);
  PumpLeft.setMaxSpeed(MAX_SPEED);
  PumpRight.setMaxSpeed(MAX_SPEED);
  PumpMiddle.setAcceleration(3 * MAX_SPEED);  // idefka
  PumpLeft.setAcceleration(3 * MAX_SPEED);
  PumpRight.setAcceleration(3 * MAX_SPEED);

  PumpMiddle.move(-angle * radius * stepsPerInch / 57.2957795131);  // 180/pi = 57.blablabla
  PumpLeft.move(-angle * radius * stepsPerInch / 57.2957795131);
  PumpRight.move(-angle * radius * stepsPerInch / 57.2957795131);
}
void rotateCWRight(float angle) {
  // runs the steppers ONE STEP if a step is due

  updateStartPositions();
  moves[0] = 0.0;
  moves[1] = 0.0;
  moves[2] = angle;

  // PumpMiddle.setMaxSpeed(MAX_SPEED);
  // PumpLeft.setMaxSpeed(MAX_SPEED);
  PumpRight.setMaxSpeed(MAX_SPEED);
  // PumpMiddle.setAcceleration(3 * MAX_SPEED);  // idefka
  // PumpLeft.setAcceleration(3 * MAX_SPEED);
  PumpRight.setAcceleration(3 * MAX_SPEED);

  // PumpMiddle.move(-angle * radius * stepsPerInch / 57.2957795131);  // 180/pi = 57.blablabla
  // PumpLeft.move(-angle * radius * stepsPerInch / 57.2957795131);
  PumpRight.move(-angle * radius * stepsPerInch / 57.2957795131);
}
void rotateCWLeft(float angle) {
  // runs the steppers ONE STEP if a step is due

  updateStartPositions();
  moves[0] = 0.0;
  moves[1] = 0.0;
  moves[2] = angle;

  // PumpMiddle.setMaxSpeed(MAX_SPEED);
  PumpLeft.setMaxSpeed(MAX_SPEED);
  // PumpRight.setMaxSpeed(MAX_SPEED);
  // PumpMiddle.setAcceleration(3 * MAX_SPEED);  // idefka
  PumpLeft.setAcceleration(3 * MAX_SPEED);
  // PumpRight.setAcceleration(3 * MAX_SPEED);

  // PumpMiddle.move(-angle * radius * stepsPerInch / 57.2957795131);  // 180/pi = 57.blablabla
  PumpLeft.move(-angle * radius * stepsPerInch / 57.2957795131);
  // PumpRight.move(-angle * radius * stepsPerInch / 57.2957795131);
}
void rotateCWMiddle(float angle) {
  // runs the steppers ONE STEP if a step is due

  updateStartPositions();
  moves[0] = 0.0;
  moves[1] = 0.0;
  moves[2] = angle;

  PumpMiddle.setMaxSpeed(MAX_SPEED);
  // PumpLeft.setMaxSpeed(MAX_SPEED);
  // PumpRight.setMaxSpeed(MAX_SPEED);
  PumpMiddle.setAcceleration(3 * MAX_SPEED);
  // PumpLeft.setAcceleration(3 * MAX_SPEED);
  // PumpRight.setAcceleration(3 * MAX_SPEED);

  PumpMiddle.move(-angle * radius * stepsPerInch / 57.2957795131);  // 180/pi = 57.blablabla
  // PumpLeft.move(-angle * radius * stepsPerInch / 57.2957795131);
  // PumpRight.move(-angle * radius * stepsPerInch / 57.2957795131);
}
void stopAll() {
  long den = (PumpMiddle.targetPosition() - startPositions[0]);
  float moveFrac;
  if (den == 0) {
    moveFrac = 0;  // any number should be fine really.
  } else {
    moveFrac = static_cast<double>(PumpMiddle.currentPosition() - startPositions[0]) / static_cast<double>(den);
  }
  // Adjust odometry data before sending
  for (int i = 0; i < 3; i++) {
    moves[i] = moves[i] * moveFrac;
  }
  PumpMiddle.setAcceleration(1000000);  // high acceleration to stop immediately.
  PumpLeft.setAcceleration(1000000);
  PumpRight.setAcceleration(1000000);
  PumpMiddle.stop();
  PumpLeft.stop();
  PumpRight.stop();
}
// updates the stored start positions, called each time a new move is commenced
// helps for sending odometry data back to ESP32 at the end of a move.
void updateStartPositions() {
  startPositions[0] = PumpMiddle.currentPosition();
  startPositions[1] = PumpLeft.currentPosition();
  startPositions[2] = PumpRight.currentPosition();
}
void updateStartPositionsLeft() {
  // startPositions[0] = PumpMiddle.currentPosition();
  startPositions[1] = PumpLeft.currentPosition();
  // startPositions[2] = PumpRight.currentPosition();

  // Serial.println(startPositions[0]);
  // Serial.println(startPositions[1]);
  // Serial.println(startPositions[2]);
}
void updateStartPositionsMiddle() {
  startPositions[0] = PumpMiddle.currentPosition();
  // startPositions[1] = PumpLeft.currentPosition();
  // startPositions[2] = PumpRight.currentPosition();

  // Serial.println(startPositions[0]);
  // Serial.println(startPositions[1]);
  // Serial.println(startPositions[2]);
}
void updateStartPositionsRight() {
  // startPositions[0] = PumpMiddle.currentPosition();
  // startPositions[1] = PumpLeft.currentPosition();
  startPositions[2] = PumpRight.currentPosition();

  // Serial.println(startPositions[0]);
  // Serial.println(startPositions[1]);
  // Serial.println(startPositions[2]);
}

// Sends a string to ESP32 detailing what motions it just completed.
// Starts with $ to let ESP know to relay to python as data, rather than just a comment
// sends [f,n,r], where
// f is the inches moved in the forward direction
// n is the inches moved in the right direction
// r is the degrees rotated in the CW direction

void sendMotion() {
  Serial.print("$[");
  for (int i = 0; i < 3; ++i) {
    if (i) {
      Serial.print(", ");
    }
    Serial.print(moves[i], 2);
  }
  Serial.print("]\n");
}

bool moving() {
  return (PumpMiddle.isRunning() || PumpLeft.isRunning() || PumpRight.isRunning());
}
