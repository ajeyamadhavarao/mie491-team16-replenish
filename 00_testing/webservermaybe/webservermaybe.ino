#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// Replace with your WiFi credentials
const char* ssid = "XPS15";
const char* password = "watermelon12";

WebServer server(80);

// Declare global variables to store the input values
int dishSoapVolume = 0;
int engineOilVolume = 0;
int handSoapVolume = 0;

// HTML content including the CSS styles and JavaScript
const char webpage[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Replenish</title>
<style>
* {
    margin: 0;
    padding: 0;
    box-sizing: border-box;
  }
  
  html, body {
    height: 100%; /* Set the minimum height to the full viewport */
    font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif, sans-serif;
    background-color: #285c33; /* Green background for the body */
    color: #F8F8FF; /* Off white text color */
  }
  h2 {
    margin-bottom: 16px; /* Adjust the space below the h2 heading */
    /* Other styling for h2 */
  }
  
  h3 {
    margin-bottom: 24px; /* Add space below the h3 element */
    color: #F8F8FF; /* Use the off-white color for text */
    font-weight: normal; /* If you want a lighter font-weight than bold */
  }
  
  /* Flexbox centering for the container */
  body {
    display: flex;
    justify-content: center; /* Horizontal centering */
    align-items: center; /* Vertical centering */
    text-align: center; /* Center text for any direct children */
  }
  
  .container {
    max-width: 600px;
    width: 100%; /* Makes the container take up to 100% of the body width */
    padding: 20px;
    background-color: #4b4c4b; /* Dark gray background for the container */
    box-shadow: 0 0 20px rgba(0, 0, 0, 0.2); /* A subtle shadow to lift the container */
    border-radius: 10px; /* Rounded corners for the container */
    margin: 20px; /* Provides a little space from viewport edges */
  }
  
  .form-group {
    margin-bottom: 20px;
  }
  
  input[type="text"] {
    width: 70%;
    padding: 10px;
    margin-bottom: 10px;
    border: 1px solid #ccc;
    border-radius: 5px;
    background: rgba(255, 255, 255, 0.8); /* Semi-transparent white for inputs */
    color: #4b4c4b; /* Text color to match the container */
  }
  
  button {
    padding: 10px 20px;
    background-color: #2fa72f; /* Sample green color */
    color: white;
    border: none;
    border-radius: 5px;
    cursor: pointer;
  }
  
  button:hover {
    background-color: #006400; /* A darker shade for hover state */
  }
  
  .title {
    font-size: 18px;
    margin-bottom: 5px;
    font-weight: bold;
    text-align: center; /* Ensure titles are centered */
    color: #F8F8FF; /* Off white text color for titles */
  }
  
  /* Responsive adjustments */
  @media (max-width: 600px) {
    input[type="text"], button {
      width: 100%;
      margin-top: 10px;
    }
  }
  
  /* Center the logo */
  .logo {
    display: block;
    margin-bottom: 20px;
    max-width: 100%; /* Ensures the logo is responsive and fits the container */
    height: auto; /* Maintains the aspect ratio of the image */
  }
</style>
</head>
<body>

div class="container">
  <img src="src/logo-png.png" alt="Replenish Logo" class="logo">
  
  <h2>Replenish - One bottle, Several Lives</h2>
  <h3>MIE491 Capstone Design - Team 16 - Replenish</h3>
  
  <div class="form-group">
    <div class="title">Dish Soap</div>
    <input type="text" id="dataInput1" placeholder="Enter Volume for Dish Soap">
    <button onclick="writeData1()">Dispense 1</button>
  </div>
  <div class="form-group">
    <div class="title">Engine Oil</div>
    <input type="text" id="dataInput2" placeholder="Enter Volume for Engine Oil">
    <button onclick="writeData2()">Dispense 2</button>
  </div>
  <div class="form-group">
    <div class="title">Hand Soap</div>
    <input type="text" id="dataInput3" placeholder="Enter Volume for Hand Soap">
    <button onclick="writeData3()">Dispense 3</button>
  </div>
</div>

<script>
function writeData(product, volume) {
  var xhr = new XMLHttpRequest();
  xhr.open("POST", "/", true);
  xhr.setRequestHeader('Content-Type', 'application/json');
  xhr.send(JSON.stringify({product: product, volume: volume}));
}
</script>
</body>
</html>
)=====";

void handleRoot() {
  server.send_P(200, "text/html", webpage);
}

void handlePost() {
  if (server.hasArg("plain") == false) {
    server.send(200, "text/plain", "No message received");
    return;
  }
  
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, server.arg("plain"));
  
  String product = doc["product"];
  int volume = doc["volume"];

  if (product == "dishSoap") {
    dishSoapVolume = volume;
  } else if (product == "engineOil") {
    engineOilVolume = volume;
  } else if (product == "handSoap") {
    handSoapVolume = volume;
  }

  Serial.print("Received volume for ");
  Serial.print(product);
  Serial.print(": ");
  Serial.println(volume);

  server.send(200, "text/plain", "Data received");
}


void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  server.on("/", HTTP_GET, handleRoot);
  server.on("/", HTTP_POST, handlePost);

  server.begin();
  Serial.println("HTTP server started");
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.println(WiFi.localIP());
}

void loop() {
  server.handleClient();
}
