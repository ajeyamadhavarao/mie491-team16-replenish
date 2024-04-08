#include <Arduino.h>
#include <Wire.h>

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
#define WIFI_SSID "XPS15"
#define WIFI_PASSWORD "watermelon12"

// Insert Firebase project API Key
#define API_KEY "AIzaSyBqpEv0CFXZ1mjvodzJqX1AsCsJUNAkn9Q"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://replenish-team16-default-rtdb.firebaseio.com/"

// Firebase objects for get and post requests
FirebaseData fbdo;
FirebaseData fbdo_millis;

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

  Serial.println("Serial Setup Complete");
}

void loop() {
  // sendSerialInputAsInteger();
  // delay(100); // Small delay to prevent overwhelming the serial input
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 3000 || sendDataPrevMillis == 0)) {

    sendDataPrevMillis = millis();

    writeMillis();
    getPumpSelection();
    getVolumeSelection();

    delay(3000);

    resetPumpSelection();
  }
}

int getPumpSelection() {
  /*
This function takes return an int value for the pump request
*/
  int leftPump = 0, rightPump = 0, middlePump = 0;

  if (Firebase.RTDB.getInt(&fbdo, "/pump/left")) {
    if (fbdo.dataType() == "int") {
      leftPump = fbdo.intData();
      Serial.print("Left Pump is ");
      Serial.println(leftPump);
      if (leftPump == 1)
        return 1;
    }
  } else if (Firebase.RTDB.getInt(&fbdo, "/pump/middle")) {
    if (fbdo.dataType() == "int") {
      middlePump = fbdo.intData();
      Serial.print("Middle Pump is ");
      Serial.println(middlePump);
      if (middlePump == 1)
        return 2;
    }
  } else if (Firebase.RTDB.getInt(&fbdo, "/pump/right")) {
    if (fbdo.dataType() == "int") {
      rightPump = fbdo.intData();
      Serial.print("Right Pump is ");
      Serial.println(rightPump);
      if (rightPump == 1)
        return 4;
    }
  } else {
    Serial.println(fbdo.errorReason());
  }
}

int getVolumeSelection() {
  /*
This function takes return an int value for the volume request
*/
  int volumeDesired = 0;

  if (Firebase.RTDB.getInt(&fbdo, "/pump/vol")) {
    if (fbdo.dataType() == "int") {
      volumeDesired = fbdo.intData();
      Serial.print("VOlume Desired is ");
      Serial.println(volumeDesired);
      return volumeDesired;
    }
  } else {
    Serial.println(fbdo.errorReason());
  }
}

void resetPumpSelection() {
  if (Firebase.RTDB.setInt(&fbdo, "pump/left", 0) && Firebase.RTDB.setInt(&fbdo, "pump/middle", 0) && Firebase.RTDB.setInt(&fbdo, "pump/right", 0)) {
    Serial.println("PASSED");
    Serial.println("PATH: " + fbdo.dataPath());
    Serial.println("TYPE: " + fbdo.dataType());
  } else {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.errorReason());
  }
}

void writeMillis() {
  // Write an Int number on the database path millis()
  if (Firebase.RTDB.set(&fbdo_millis, "/millis", millis())) {
    Serial.println("PASSED");
    Serial.println("PATH: " + fbdo_millis.dataPath());
    Serial.println("TYPE: " + fbdo_millis.dataType());
  } else {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo_millis.errorReason());
  }
}

void sendSerialInputAsInteger() {
  if (Serial.available() > 0) {
    String inputString = Serial.readStringUntil('\n');  // Read input
    inputString.trim();                                 // Trim any whitespace
    if (inputString.length() > 0) {
      int inputInt = inputString.toInt();  // Convert string to int
      Serial.println("Sending: " + String(inputInt));

      // Begin the transmission to a device with address 8
      Wire.beginTransmission(8);

      // Send the integer as two bytes
      Wire.write((inputInt >> 8) & 0xFF);  // Send the high byte
      Wire.write(inputInt & 0xFF);         // Send the low byte

      // End the transmission
      Wire.endTransmission();
    }
  }
}
