#include <Arduino.h>
#include <Wire.h>
#include <string.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <RTClib.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <freertos/FreeRTOS.h>
#include <Preferences.h>
#include "KeyInput.h"
#include "WiFiSelector.h"
#include "configs.h"

// Global objects
Preferences pref;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET_PIN);
WiFiSelector wifiSelector(&display, &pref);

// Global variables
unsigned long globalmilisbuff_start;
unsigned long globalmilisbuff_end;

// function declaration
void entrypoint();

void loop() {
  // Main loop - can be used for other tasks after WiFi connection
  delay(1000);
}


void setup() {
  entrypoint();
  
  // Scan for available networks
  auto networks = wifiSelector.scanNetworks();
  
  if (networks.empty()) {
    Serial.println("No networks found, cannot proceed");
    return;
  }
  
  // 1. Try to connect with previously saved credentials
  if (!wifiSelector.connectWithSavedCredentials(networks)) {
    // 2. If that fails, prompt user to select and connect to a network
    if (wifiSelector.selectAndConnectNetwork(networks)) {
      Serial.println("Successfully connected to selected network");
    } else {
      Serial.println("Failed to connect to any network");
    }
  } else {
    Serial.println("Connected using saved credentials");
  }
  
  // Continue with your main application logic here
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi setup complete!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    
    // Display final status
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("WiFi Connected!");
    display.print("IP: ");
    display.println(WiFi.localIP());
    display.println();
    display.println("Ready for operation");
    display.display();
  }
}

void entrypoint(){
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  
  // Initialize I2C with custom pins
  Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
  
  // Initialize display
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("WiFi Display Module");
  display.println("Initializing...");
  display.display();
}