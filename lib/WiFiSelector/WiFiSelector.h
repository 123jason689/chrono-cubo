#ifndef WIFISELECTOR_H
#define WIFISELECTOR_H

#include <Arduino.h>
#include <WiFi.h>
#include <vector>
#include <Adafruit_SSD1306.h>
#include <Preferences.h>
#include "ScrollingText.h"
#include "configs.h"

struct NetworkInfo {
  String ssid;
  int32_t rssi;
  wifi_auth_mode_t encryption;
};

class WiFiSelector {
private:
  Adafruit_SSD1306* display;
  Preferences* preferences;
  String pref_namespace;
  int connection_timeout;
  
  // Scrolling text for SSID display
  ScrollingText ssid_scroller;
  
  // Internal methods
  bool needsPassword(wifi_auth_mode_t enc_type);
  void showConnectingScreen(const String& ssid);
  void showConnectionResult(bool success, const String& ip = "");
  bool waitForConnection();
  void saveCredentials(const String& ssid, const String& password);
  
public:
  // Constructor
  WiFiSelector(Adafruit_SSD1306* disp, Preferences* pref, const String& namespace_name = "wifi-creds", int timeout = 10000);
  
  // Main public methods
  std::vector<NetworkInfo> scanNetworks();
  bool connectWithSavedCredentials(const std::vector<NetworkInfo>& networks);
  bool selectAndConnectNetwork(std::vector<NetworkInfo>& networks);
  
  // Utility methods
  void setConnectionTimeout(int timeout_ms);
  void displayNetworkList(const std::vector<NetworkInfo>& networks);
  
  // Static utility
  static String encryptionTypeToString(wifi_auth_mode_t enc);
  static int getSignalStrength(int32_t rssi);
};

#endif // WIFISELECTOR_H
