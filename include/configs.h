#ifndef CONFIG_H
#define CONFIG_H

// ========================================
// WiFi Display Module Configuration
// ========================================
// This file is auto-generated from .env.local
// Do not edit directly - edit .env.local instead

// OLED Display I2C Configuration
#define OLED_I2C_ADDRESS 0x3C
#define OLED_SDA_PIN 8
#define OLED_SCL_PIN 9
#define OLED_RESET_PIN -1

// Display Settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Input Control Pins
#define POT_X_PIN 0
#define POT_Y_PIN 1
#define BTN_SELECT 2

// Hardware Settings
#define POT_DEADZONE 100
#define POT_CENTER 2048
#define POT_SENSITIVITY 3
#define MOVE_DELAY 150

// Power Management
#define SLEEP_DURATION_SECONDS 300
#define WIFI_TIMEOUT_MS 15000

// Special Characters for KeyInput
#define REMOVE_CHAR 127
#define LEFT_CHAR 128
#define RIGHT_CHAR 129
#define SPACE_CHAR 130

#endif // CONFIG_H
