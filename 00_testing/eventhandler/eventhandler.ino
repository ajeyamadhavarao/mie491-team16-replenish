#include <WiFi.h>
#include <Wire.h>
#include <FirebaseESP32.h>
#include "addons/TokenHelper.h"  // For token generation process info
#include "addons/RTDBHelper.h"   // For RTDB payload printing info and helper functions

#define WIFI_SSID "XPS15"
#define WIFI_PASSWORD "watermelon12"
#define API_KEY "AIzaSyBqpEv0CFXZ1mjvodzJqX1AsCsJUNAkn9Q"
#define DATABASE_URL "https://replenish-team16-default-rtdb.firebaseio.com/"
// #define USER_EMAIL "your_user_email"
// #define USER_PASSWORD "your_user_password"

FirebaseData firebaseDataPump;
FirebaseData firebaseDataVolume;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int intValue;
int floatValue;
int count = 0;
bool signUpOK = false;

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

void pumpSelectionCallback(StreamData data) {
  Serial.println("Pump Selection Change Detected:");

  if (data.dataType() == "int") {
    int pumpSelection = data.intData();  // Extract the integer data
    Serial.print("New Pump Selection: ");
    Serial.println(pumpSelection);
    // Here, you can store 'pumpSelection' in a global variable or act upon the value as needed
    sendCommandtoArduino(pumpSelection);

  } else {
    Serial.println("Received non-integer data type.");
  }
}

void volumeInputCallback(StreamData data) {
  Serial.println("Volume Input Change Detected:");

  if (data.dataType() == "int") {
    int volumeInput = data.intData();  // Extract the integer data
    Serial.print("New Volume Input: ");
    Serial.println(volumeInput);
    // Similar to 'pumpSelection', 'volumeInput' can be stored or used as required
    sendCommandtoArduino(volumeInput);

  } else {
    Serial.println("Received non-integer data type.");
  }
}

void streamTimeoutCallback(bool timeout) {
  if (timeout) {
    Serial.println("Stream timed out, resuming...");
  }
}
void checkWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected, attempting to reconnect...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("WiFi reconnected successfully.");
  }
}
void checkFirebaseStream(FirebaseData &firebaseData, const String &path) {
  if (!firebaseData.httpConnected()) {
    Serial.println("Firebase stream disconnected, attempting to reconnect...");
    if (!Firebase.beginStream(firebaseData, path)) {
      Serial.println("Failed to reconnect Firebase stream.");
      Serial.println(firebaseData.errorReason());
    } else {
      Serial.println("Firebase stream reconnected successfully.");
    }
  }
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);  // Start the Serial communication
  Wire.begin();          // Start I2C as master

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println(" connected.");

  config.api_key = API_KEY;
  // auth.user.email = USER_EMAIL;
  // auth.user.password = USER_PASSWORD;
  config.database_url = DATABASE_URL;
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


  // if (!Firebase.beginStream(firebaseDataPump, "/pump/left/intValue1")) {
  //   Serial.println("Could not begin pump selection stream");
  // }
  // Firebase.setStreamCallback(firebaseDataPump, pumpSelectionCallback, streamTimeoutCallback);

  // if (!Firebase.beginStream(firebaseDataVolume, "/pump/vol")) {
  //   Serial.println("Could not begin volume input stream");
  // }
  // Firebase.setStreamCallback(firebaseDataVolume, volumeInputCallback, streamTimeoutCallback);

  if (!Firebase.beginStream(firebaseDataPump, "/pump/left/intValue1")) {
    Serial.printf("stream 1 begin error, %s\n\n", firebaseDataPump.errorReason().c_str());
  }
  if (!Firebase.beginStream(firebaseDataVolume, "/pump/vol")) {
    Serial.printf("stream 1 begin error, %s\n\n", firebaseDataVolume.errorReason().c_str());
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  // checkWiFi();
  // checkFirebaseStream(firebaseDataPump, "pump/left/intValue1");
  // checkFirebaseStream(firebaseDataVolume, "pump/vol");

  // delay(1000);
  // ---------- READ data from a RTDB onDataChange
  if (Firebase.ready() && signUpOK) {

    if (!Firebase.readStream(&firebaseDataPump)) {
      Serial.printf("Stream Pump read error, %s\n\n", firebaseDataPump.errorReason().c_str());
    }
    if (firebaseDataPump.streamAvailable()) {
      if (firebaseDataPump.dataType() == "int") {
        int intValue = firebaseDataPump.intData();
        Serial.println("Successful READ from " + firebaseDataPump.dataPath() + ": " + intValue + " (" + firebaseDataPump.dataType() + ")");
        sendCommandtoArduino(intValue);
      }
    } else {
      Serial.printf("Stream Pump read error, %s\n\n", firebaseDataPump.errorReason().c_str());
    }

    if (!Firebase.readStream(&firebaseDataVolume)) {
      Serial.printf("Stream Volume read error, %s\n\n", firebaseDataVolume.errorReason().c_str());
    }
    if (firebaseDataPump.streamAvailable()) {
      if (firebaseDataVolume.dataType() == "int") {
        int volValue = firebaseDataVolume.intData();
        Serial.println("Successful READ from " + firebaseDataVolume.dataPath() + ": " + volValue + " (" + firebaseDataVolume.dataType() + ")");
        sendCommandtoArduino(volValue);
      }
    } else {
      Serial.printf("Stream Volume read error, %s\n\n", firebaseDataVolume.errorReason().c_str());
    }
  }
}
