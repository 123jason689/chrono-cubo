#include "WiFiSelector.h"
#include "KeyInput.h"
#include "configs.h"

WiFiSelector::WiFiSelector(Adafruit_SSD1306* disp, Preferences* pref, const String& namespace_name, int timeout) {
  display = disp;
  preferences = pref;
  pref_namespace = namespace_name;
  connection_timeout = timeout;
  
  // Configure SSID scroller for 18 characters max display, smooth scrolling
  ssid_scroller.setDisplayWidth(18, 108);  // 18 chars * 6 pixels = 108 pixels
  ssid_scroller.enableSmoothScroll(true, 6);  // 6 pixels per character
  ssid_scroller.setScrollDelay(100);  // Fast smooth scrolling
  ssid_scroller.setPauseDelay(1500);  // 1.5 second pause at start/end
}

void WiFiSelector::setConnectionTimeout(int timeout_ms) {
  connection_timeout = timeout_ms;
}

bool WiFiSelector::needsPassword(wifi_auth_mode_t enc_type) {
  return (enc_type != WIFI_AUTH_OPEN);
}

std::vector<NetworkInfo> WiFiSelector::scanNetworks() {
  display->clearDisplay();
  display->setTextSize(1);
  display->setTextColor(SSD1306_WHITE);
  display->setCursor(0, 0);
  display->println("WiFi Selector");
  display->println("Scanning networks...");
  display->display();
  
  int networkCount = WiFi.scanNetworks();
  std::vector<NetworkInfo> networks;
  
  if (networkCount == 0) {
    Serial.println("No networks found");
    
    display->clearDisplay();
    display->setCursor(0, 0);
    display->println("No WiFi networks");
    display->println("found!");
    display->display();
    delay(2000);
    
    return networks;
  }
  
  Serial.printf("Found %d networks\n", networkCount);
  
  for (int i = 0; i < networkCount; i++) {
    NetworkInfo network;
    network.ssid = WiFi.SSID(i);
    network.rssi = WiFi.RSSI(i);
    network.encryption = WiFi.encryptionType(i);
    
    networks.push_back(network);
    
    Serial.printf("%d: %s (%d dBm) %s\n", 
                  i + 1, 
                  network.ssid.c_str(), 
                  network.rssi, 
                  encryptionTypeToString(network.encryption).c_str());
  }
  
  return networks;
}

bool WiFiSelector::connectWithSavedCredentials(const std::vector<NetworkInfo>& networks) {
  if (!preferences->begin(pref_namespace.c_str(), true)) {  // true = read-only
    Serial.println("Failed to open preferences");
    return false;
  }
  
  String saved_ssid = preferences->getString("ssid", "");
  String saved_password = preferences->getString("password", "");
  preferences->end();
  
  if (saved_ssid.length() == 0) {
    Serial.println("No saved credentials found");
    return false;
  }
  
  // Look for saved network in scan results
  for (const NetworkInfo& network : networks) {
    if (network.ssid.equals(saved_ssid)) {
      Serial.println("Found saved network: " + saved_ssid);
      
      display->clearDisplay();
      display->setCursor(0, 0);
      display->println("Connecting to saved:");
      display->println(saved_ssid);
      display->display();
      
      if (needsPassword(network.encryption)) {
        WiFi.begin(network.ssid, saved_password);
      } else {
        WiFi.begin(network.ssid);
      }
      
      if (waitForConnection()) {
        Serial.println("Connected to saved network!");
        showConnectionResult(true, WiFi.localIP().toString());
        return true;
      } else {
        Serial.println("Failed to connect to saved network");
        WiFi.disconnect();
        return false;
      }
    }
  }
  
  Serial.println("Saved network not found in scan");
  return false;
}

bool WiFiSelector::selectAndConnectNetwork(std::vector<NetworkInfo>& networks) {
  if (networks.empty()) {
    display->clearDisplay();
    display->setCursor(0, 0);
    display->println("No networks to");
    display->println("select from!");
    display->display();
    delay(2000);
    return false;
  }
  
  int selected_network = 0;
  int total_networks = networks.size();
  int last_selected = -1;  // Track when selection changes
  
  init_controls();  // Initialize potentiometers and button
  
  while (true) {
    // Update scrolling text when selection changes
    if (selected_network != last_selected) {
      ssid_scroller.setText(networks[selected_network].ssid);
      last_selected = selected_network;
    }
    
    // Update scrolling animation
    ssid_scroller.update();
    
    // Display current network
    display->clearDisplay();
    display->setTextSize(1);
    display->setTextColor(SSD1306_WHITE);
    display->setCursor(0, 0);
    display->println("Select Network:");
    
    // Draw scrolling SSID with background clearing
    display->setCursor(0, 16);
    display->print("SSID: ");
    ssid_scroller.drawWithBackground(display, 36, 16, 1, SSD1306_WHITE, SSD1306_BLACK);
    
    display->setCursor(0, 24);
    display->print("Signal: ");
    display->print(networks[selected_network].rssi);
    display->println(" dBm");
    
    display->setCursor(0, 32);
    display->print("Security: ");
    if (needsPassword(networks[selected_network].encryption)) {
      display->println("Protected");
    } else {
      display->println("Open");
    }
    
    display->setCursor(0, 40);
    display->print("Network ");
    display->print(selected_network + 1);
    display->print(" of ");
    display->println(total_networks);
    
    display->setCursor(0, 48);
    display->println("Y-Pot:nav Button:select");
    
    // Show scroll indicator if needed
    if (ssid_scroller.needsScrolling()) {
      display->setCursor(0, 56);
      display->print("Scrolling...");
    }
    
    display->display();
    
    // Handle navigation with Y potentiometer
    static unsigned long last_nav_time = 0;
    if (millis() - last_nav_time > MOVE_DELAY) {
      int y_move = get_y_movement();
      if (y_move == -1) {
        selected_network = (selected_network - 1 + total_networks) % total_networks;
        last_nav_time = millis();
      } else if (y_move == 1) {
        selected_network = (selected_network + 1) % total_networks;
        last_nav_time = millis();
      }
    }
    
    // Handle selection with button
    if (select_button_pressed()) {
      NetworkInfo& network = networks[selected_network];
      String password = "";
      
      if (needsPassword(network.encryption)) {
        // Show password input screen
        display->clearDisplay();
        display->setCursor(0, 0);
        display->println("Enter password for:");
        
        // Use scrolling text for password prompt too
        ScrollingText password_prompt;
        password_prompt.setText(network.ssid);
        password_prompt.setDisplayWidth(21, 126);  // Almost full width
        password_prompt.drawWithBackground(display, 0, 8, 1, SSD1306_WHITE, SSD1306_BLACK);
        
        display->display();
        delay(1000);
        
        // Get password using keyboard
        const char* entered_password = prompt_keyboard();
        password = String(entered_password);
        
        WiFi.begin(network.ssid, password);
      } else {
        WiFi.begin(network.ssid);
      }
      
      showConnectingScreen(network.ssid);
      
      if (waitForConnection()) {
        // Save credentials on successful connection
        saveCredentials(network.ssid, password);
        showConnectionResult(true, WiFi.localIP().toString());
        return true;
      } else {
        showConnectionResult(false);
        
        // Wait for button press to continue
        while (!select_button_pressed()) {
          delay(100);
        }
        
        WiFi.disconnect();
        // Continue loop to try again
      }
    }
    
    delay(20);  // Reduced delay for smooth scrolling
  }
}

void WiFiSelector::showConnectingScreen(const String& ssid) {
  display->clearDisplay();
  display->setCursor(0, 0);
  display->println("Connecting to:");
  display->println(ssid);
  display->println("Please wait...");
  display->display();
}

void WiFiSelector::showConnectionResult(bool success, const String& ip) {
  display->clearDisplay();
  display->setCursor(0, 0);
  
  if (success) {
    display->println("Connected!");
    display->print("IP: ");
    display->println(ip);
  } else {
    display->println("Connection failed!");
    display->println("Press button");
    display->println("to try again");
  }
  
  display->display();
  delay(2000);
}

bool WiFiSelector::waitForConnection() {
  unsigned long start_time = millis();
  
  while (WiFi.status() != WL_CONNECTED && millis() - start_time < connection_timeout) {
    delay(500);
    Serial.print(".");
  }
  
  return (WiFi.status() == WL_CONNECTED);
}

void WiFiSelector::saveCredentials(const String& ssid, const String& password) {
  if (!preferences->begin(pref_namespace.c_str(), false)) {  // false = read-write
    Serial.println("Failed to open preferences for writing");
    return;
  }
  
  preferences->putString("ssid", ssid);
  preferences->putString("password", password);
  preferences->end();
  
  Serial.println("Credentials saved: " + ssid);
}

void WiFiSelector::displayNetworkList(const std::vector<NetworkInfo>& networks) {
  display->clearDisplay();
  display->setTextSize(1);
  display->setTextColor(SSD1306_WHITE);
  display->setCursor(0, 0);
  
  display->printf("Found %d networks:\n", networks.size());
  
  int displayed = 0;
  for (const auto& network : networks) {
    if (displayed >= 6) break;  // Limit to screen space
    
    display->printf("%s (%d)\n", network.ssid.c_str(), network.rssi);
    displayed++;
  }
  
  if (networks.size() > 6) {
    display->printf("...and %d more", networks.size() - 6);
  }
  
  display->display();
}

String WiFiSelector::encryptionTypeToString(wifi_auth_mode_t enc) {
  switch (enc) {
    case WIFI_AUTH_OPEN:
      return "Open";
    case WIFI_AUTH_WEP:
      return "WEP";
    case WIFI_AUTH_WPA_PSK:
      return "WPA";
    case WIFI_AUTH_WPA2_PSK:
      return "WPA2";
    case WIFI_AUTH_WPA_WPA2_PSK:
      return "WPA/WPA2";
    case WIFI_AUTH_WPA3_PSK:
      return "WPA3";
    case WIFI_AUTH_WPA2_ENTERPRISE:
      return "Enterprise";
    default:
      return "Unknown";
  }
}

int WiFiSelector::getSignalStrength(int32_t rssi) {
  if (rssi >= -50) return 4;      // Excellent
  else if (rssi >= -60) return 3; // Good
  else if (rssi >= -70) return 2; // Fair
  else if (rssi >= -80) return 1; // Weak
  else return 0;                  // Very weak
}
