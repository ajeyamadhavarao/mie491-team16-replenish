#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <Wire.h>

MCUFRIEND_kbv tft;

#define LCD_CS A3
#define LCD_RS A2
#define LCD_WR A1
#define LCD_RD A0
#define LCD_RST A4

#define SCREEN_WIDTH  480
#define SCREEN_HEIGHT 320

#define GREEN 0x44be44
#define GREY 0x3c444c

#define SLAVE_ADDRESS 9 // Define the I2C slave address of the Arduino Uno

bool displayed = false;

// Liquid types
const char* liquidTypes[] = {"Engine Oil", "Dish Soap", "Detergent"};
int liquidInput = -1; 

void setup() {
  
  Serial.println("Setup BEFORE I2C");
  Wire.begin(SLAVE_ADDRESS); // Initialize as I2C slave with address 9
  Wire.onReceive(receiveEvent); // Set up callback function for received data
  Serial.begin(9600); // Initialize Serial communication
}

void loop() {
  // No delay here to avoid automatically showing "Engine Oil"
}

void receiveEvent(int bytes) {
  Serial.println("Inside receiveEVENT");
  
  int c = Wire.read(); // Read the incoming byte
  Serial.println("Received: " + String(c));
  liquidInput = c;
  Serial.println(liquidTypes[liquidInput]);
  displayLiquidType(liquidInput);
 
}

void displayLiquidType(int input) {
  // Clear previous liquid type display
  // tft.fillRect(0, SCREEN_HEIGHT / 2 - 20, SCREEN_WIDTH, 40, GREY);

  uint16_t ID = 0x9486; // ILI9486 ID
  tft.reset(); // Initialize display
  tft.begin(ID);
  tft.setRotation(1);

  // Set background color to grey
  tft.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GREY);

  // Set text color and size
  tft.setTextColor(0xFFFF); // White color
  tft.setTextSize(5);
  if (input >= 0 && input < sizeof(liquidTypes) / sizeof(liquidTypes[0])) {
    // Calculate text width to center horizontally
    int16_t x, y;
    uint16_t textWidth, textHeight;
    tft.getTextBounds(liquidTypes[input], 0, 0, &x, &y, &textWidth, &textHeight);
    Serial.println("Inside displayLiquidType: "); 
    Serial.print(liquidTypes[input]);
    // Display liquid type string horizontally
    tft.setCursor((SCREEN_WIDTH - textWidth) / 2, (SCREEN_HEIGHT - textHeight) / 2);
    tft.print(liquidTypes[input]);
  } else {
    // Display error message for invalid input
    tft.setTextColor(GREEN); // Green color
    tft.setCursor(SCREEN_WIDTH / 2 - 60, SCREEN_HEIGHT / 2 - 10);
    tft.println("Invalid Input");
  }
}
