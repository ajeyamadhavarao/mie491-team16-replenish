#include <AccelStepper.h>
// https://hackaday.io/project/183279-accelstepper-the-missing-manual/details
// documentation for the AccelStepper library can be found above^
// Written by Rishabh Garikiparithi. Feel free to modify and share

#define xStep 2  // y Axis Step Pin
#define yStep 3  // y Axis Step Pin
#define zStep 4  // z Axis Step Pin
// #define aStep 12  // A Axis Step Pin

#define xDir 5  // x Axis Direction Pin
#define yDir 6  // y Axis Direction Pin
#define zDir 7  // z Axis Direction Pin

#define enPin 12

const int RELAY_PIN_1 = 9;   // the Arduino pin D4, which connects to the IN pin of relay
const int RELAY_PIN_2 = 10;  // the Arduino pin D4, which connects to the IN pin of relay
const int RELAY_PIN_3 = 11;  // the Arduino pin D4, which connects to the IN pin of relay

//Objects for controlling stepper motor with accelStepper library
AccelStepper PumpLeft(1, xStep, xDir);  //right
AccelStepper PumpMiddle(1, yStep, yDir);
AccelStepper PumpRight(1, zStep, zDir);

float stepsPerInch = 27;
float radius = 3.59;
int MAX_SPEED = 3000;
String cmdStr;
float moves[3];
long startPositions[3];
// bool ready;

void setup() {
  Serial.begin(9600);
  while (!Serial.available()) {
    ;
  }
  Serial.readStringUntil('\n');
  Serial.print("Ard ready!\n");
  pinMode(enPin, OUTPUT);
  pinMode(RELAY_PIN_1, OUTPUT);
  pinMode(RELAY_PIN_2, OUTPUT);
  pinMode(RELAY_PIN_3, OUTPUT);
  updateStartPositions();
}

void loop() {
  // only look for new commands if the motors aren't moving
  if (!moving()) {

    digitalWrite(RELAY_PIN_1, HIGH);  // open valve 5 seconds
    digitalWrite(RELAY_PIN_2, HIGH);  // open valve 5 seconds
    digitalWrite(RELAY_PIN_3, HIGH);  // open valve 5 seconds

    // disable motors
    digitalWrite(enPin, HIGH);

    //Sends odometry data to ESP32 to relay to python
    sendMotion();

    // reset odometry (ensures correct behaviour of stop command)
    for (int i = 0; i < 3; i++) {
      moves[i] = 0.0;
    }

    // Waits for ESP32 to send the next move.
    while (!Serial.available()) {
      ;
    }

    // enable motors
    digitalWrite(enPin, LOW);
    // enable valves
    digitalWrite(RELAY_PIN_1, LOW);  // close valve 5 seconds
    digitalWrite(RELAY_PIN_2, LOW);  // close valve 5 seconds
    digitalWrite(RELAY_PIN_3, LOW);  // close valve 5 seconds
  }
  // Check every loop to see if ESP32 has sent a stop command or the next move command
  read();  // sets next target position



  PumpMiddle.run(); // middle stepper
  PumpLeft.run(); // left stepper
  PumpRight.run();  // right stepper

  // stepper.run() determins whether a signal is due based on target position, acceleration, and speed.
}

void read() {
  // exits and makes no changes if serial buffer is empty
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
  PumpMiddle.setAcceleration(3 * MAX_SPEED);  // idefka
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
}void updateStartPositionsRight() {
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
