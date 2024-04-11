#include <Arduino.h>
#include <Wire.h>
#include <FirebaseESP32.h>

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
FirebaseData firebaseData;

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

  // // Set listener for pump selection

  // //FOR TESTING USE LEFT FOR ALL PUMPS - CHANGE LATER
  // Firebase.setStreamCallback(firebaseData, "pump/left/intValue1", onPumpSelectionChanged, nullptr);

  // // Set listener for volume input
  // Firebase.setStreamCallback(firebaseData, "pump/vol", onVolumeInputChanged, nullptr);

  if (!Firebase.beginStream(firebaseData, "pump/left/intValue1") && !Firebase.beginStream(firebaseData, "pump/vol")) {
    Serial.println("Could not begin stream");
    Serial.println(firebaseData.errorReason());
  }


  // I2C Communication with Devices
  Wire.begin();  // Start I2C as master

  Serial.println("Serial Setup Complete");
}

void loop() {

  if (Firebase.readStream(firebaseData)) {
    if (firebaseData.streamAvailable()) {
      if(firebaseData.dataType() == "int") {
        int value = firebaseData.intData(); // Use this value as needed
        // Call your handling function directly with the value
        // For example: sendCommandtoArduino(value);
      }
    }
  } else if (firebaseData.streamTimeout()) {
    Serial.println("Stream timeout, reconnecting...");
    // Handle timeout, possibly by restarting the stream
  }
}

void onPumpSelectionChanged(FirebaseData &data) {
  if (data.dataType() == "int") {
    int pumpSelected = data.intData();
    sendCommandtoArduino(pumpSelected);
    // Additional logic as needed
  }
}

void onVolumeInputChanged(FirebaseData &data) {
  if (data.dataType() == "int") {
    int volumeInput = data.intData();
    sendCommandtoArduino(volumeInput);
    // Additional logic as needed
  }
}


// int getPumpSelection() {
//   /*
// This function takes return an int value for the pump request
// */
//   int leftPump = 0, rightPump = 0, middlePump = 0;

//   if (Firebase.RTDB.getInt(&fbdo, "/pump/left/intValue1")) {
//     if (fbdo.dataType() == "int") {
//       leftPump = fbdo.intData();
//       Serial.print("Left Pump is ");
//       Serial.println(leftPump);
//     }
//   }
//   if (Firebase.RTDB.getInt(&fbdo, "/pump/middle/intValue2")) {
//     if (fbdo.dataType() == "int") {
//       middlePump = fbdo.intData();
//       Serial.print("Middle Pump is ");
//       Serial.println(middlePump);
//     }
//   }
//   if (Firebase.RTDB.getInt(&fbdo, "/pump/right/intValue3")) {
//     if (fbdo.dataType() == "int") {
//       rightPump = fbdo.intData();
//       Serial.print("Right Pump is ");
//       Serial.println(rightPump);
//     }
//   }
//   int safety_counter = leftPump + middlePump + rightPump;
//   if (safety_counter > 1) {
//     return 0;
//   } else {
//     if (leftPump == 1) {
//       return 1;
//     } else if (middlePump == 1) {
//       return 2;
//     } else if (rightPump == 1) {
//       return 4;
//     } else {
//       return 0;
//     }
//   }
// }

// int getVolumeSelection() {
//   /*
// This function takes return an int value for the volume request
// */
//   int volumeDesired = 0;

//   if (Firebase.RTDB.getInt(&fbdo, "/pump/vol")) {
//     if (fbdo.dataType() == "int") {
//       volumeDesired = fbdo.intData();
//       Serial.print("Volume Desired is ");
//       Serial.println(volumeDesired);
//       return volumeDesired;
//     }
//   } else {
//     Serial.println(fbdo.errorReason());
//   }
// }

// void resetPumpSelection() {
//   if (Firebase.RTDB.setInt(&fbdo, "pump/left/intValue1", 0) && Firebase.RTDB.setInt(&fbdo, "pump/middle/intValue2", 0) && Firebase.RTDB.setInt(&fbdo, "pump/right/intValue3", 0) && Firebase.RTDB.setInt(&fbdo, "pump/vol", 0)) {
//     Serial.println("Pumps Reset!");
//   } else {
//     Serial.println("FAILED");
//     Serial.println("REASON: " + fbdo.errorReason());
//   }
// }
// void resetVolSelection() {
//   if (Firebase.RTDB.setInt(&fbdo, "pump/vol", 0)) {
//     Serial.println("Volume Reset!");
//   } else {
//     Serial.println("FAILED");
//     Serial.println("REASON: " + fbdo.errorReason());
//   }
// }

// void writeMillis() {
//   // Write an Int number on the database path millis()
//   if (Firebase.RTDB.set(&fbdo_millis, "/millis", millis())) {
//     Serial.println("Wrote millis()");
//   } else {
//     Serial.println("FAILED");
//     Serial.println("REASON: " + fbdo_millis.errorReason());
//   }
// }

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
