#include <Arduino.h>
#include <Wire.h>
// #include <FirebaseESP32.h> // do not uncomment

#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "2201"
#define WIFI_PASSWORD "Gupta4512#"

// Insert Firebase project API Key
#define API_KEY "AIzaSyBqpEv0CFXZ1mjvodzJqX1AsCsJUNAkn9Q"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://replenish-team16-default-rtdb.firebaseio.com/"

// Firebase objects for get and post requests
FirebaseData fbdo;
FirebaseData fbdo_millis;
// FirebaseData firebaseData;

// Firebase Authentication and Configuration Requests
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int intValue;
int floatValue;
int count = 0;
bool signupOK = false;


void setup() {
  Serial.begin(115200);  // Start the Serial communication

  // Wifi Connection
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ok");
    signupOK = true;
  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback;  //see addons/TokenHelper.h

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // I2C Communication with Devices
  Wire.begin();  // Start I2C as master

  // reset numbers on the firebase every ESP32 reset

  resetPumpSelection();
  resetVolSelection();

  Serial.println("Serial Setup Complete");
}

void loop() {

  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)) {

    sendDataPrevMillis = millis();
    Serial.println("~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

    // writeMillis();
    while (getPumpSelection() == 0) {
      ;
    }
    int pumpSelected = getPumpSelection();
    sendCommandtoArduino(pumpSelected);

    while (getVolumeSelection() == 0) {
      ;
    }
    int volumeInput = getVolumeSelection();
    sendCommandtoArduino(volumeInput);

    resetPumpSelection();
    resetVolSelection();

    Serial.println("~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
  }
}



int getPumpSelection() {
  /*
This function takes return an int value for the pump request
*/
  int leftPump = 0, rightPump = 0, middlePump = 0;

  if (Firebase.RTDB.getInt(&fbdo, "/pump/left/value")) {
    if (fbdo.dataType() == "int") {
      leftPump = fbdo.intData();
      Serial.print("Left Pump is ");
      Serial.println(leftPump);
    }
  }
  delay(500); // delays to ensure smooth loop through the RTDB
  if (Firebase.RTDB.getInt(&fbdo, "/pump/middle/value")) {
    if (fbdo.dataType() == "int") {
      middlePump = fbdo.intData();
      Serial.print("Middle Pump is ");
      Serial.println(middlePump);
    }
  }
  delay(500); // delays to ensure smooth loop through the RTDB
  if (Firebase.RTDB.getInt(&fbdo, "/pump/right/value")) {
    if (fbdo.dataType() == "int") {
      rightPump = fbdo.intData();
      Serial.print("Right Pump is ");
      Serial.println(rightPump);
    }
  }
  delay(500); // delays to ensure smooth loop through the RTDB
  int safety_counter = leftPump + middlePump + rightPump;
  if (safety_counter > 1) {
    return 0;
  } else {
    if (leftPump == 1) {
      return 1;
    } else if (middlePump == 1) {
      return 2;
    } else if (rightPump == 1) {
      return 4;
    } else {
      return 0;
    }
  }
}

int getVolumeSelection() {
  /*
This function takes return an int value for the volume request
*/
  int volumeDesired = 0;
  delay(1500); // delays to ensure smooth loop through the RTDB
  if (Firebase.RTDB.getInt(&fbdo, "/pump/vol/value")) {
    if (fbdo.dataType() == "int") {
      volumeDesired = fbdo.intData();
      Serial.print("Volume Desired is ");
      Serial.println(volumeDesired);
      return volumeDesired;
    }
  } else {
    Serial.println(fbdo.errorReason());
  }
  
}

void resetPumpSelection() {
  if (Firebase.RTDB.setInt(&fbdo, "pump/left/value", 0) && Firebase.RTDB.setInt(&fbdo, "pump/middle/value", 0) && Firebase.RTDB.setInt(&fbdo, "pump/right/value", 0)) {
    Serial.println("Pumps Reset!");
  } else {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.errorReason());
  }
}
void resetVolSelection() {
  if (Firebase.RTDB.setInt(&fbdo, "pump/vol/value", 0)) {
    Serial.println("Volume Reset!");
  } else {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.errorReason());
  }
}

void writeMillis() {
  // Write an Int number on the database path millis()
  if (Firebase.RTDB.set(&fbdo_millis, "/millis", millis())) {
    Serial.println("Wrote millis()");
  } else {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo_millis.errorReason());
  }
}

void sendCommandtoArduino(int inputInt) {
  if (inputInt > 0) {
    Serial.println("Sending: " + String(inputInt));

    // Begin the transmission to a device with address 8
    Wire.beginTransmission(8);

    // Send the integer as two bytes
    Wire.write((inputInt >> 8) & 0xFF);  // Send the high byte
    Wire.write(inputInt & 0xFF);         // Send the low byte

    // End the transmission
    Wire.endTransmission();
  } else {
    Serial.println("Sending a zero!");

    Wire.beginTransmission(8);

    // Send the integer as two bytes
    Wire.write((inputInt >> 8) & 0xFF);  // Send the high byte
    Wire.write(inputInt & 0xFF);         // Send the low byte

    // End the transmission
    Wire.endTransmission();
  }
}
