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
#include "TimeManager.h"
#include "StateMachine.h"
#include "SingleTimer.h"
#include "MultiTimer.h"
#include "AlarmClock.h"
#include "NotificationManager.h"
#include "PushNotifier.h"
#include "configs.h"

// Global objects
Preferences pref;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET_PIN);
RTC_DS3231 rtc;
WiFiSelector wifiSelector(&display, &pref);
TimeManager timeManager(&rtc, &display);
StateMachine stateMachine(&display);
SingleTimer singleTimer(&display);
MultiTimer multiTimer(&display);
PushNotifier pushNotifier(&pref);
AlarmClock alarmClock(&display, &rtc, &pushNotifier);
NotificationManager notificationManager;

// Menu items
MenuItem mainMenuItems[] = {
    {"Single Timer", STATE_SINGLE_TIMER_SETUP, true},
    {"Multi-Phase Timer", STATE_MULTI_TIMER_SELECT, true},
    {"Sleep Alarm", STATE_ALARM_SETUP, true},
    {"Settings", STATE_SETTINGS_MENU, true},
    {"Time Display", STATE_TIME_DISPLAY, true}
};

MenuItem settingsMenuItems[] = {
    {"Set Alertzy Key", STATE_ALERTZY_KEY_SETUP, true},
    {"Back to Main", STATE_MAIN_MENU, true}
};

// Global variables
unsigned long globalmilisbuff_start;
unsigned long globalmilisbuff_end;
bool systemInitialized = false;

// function declaration
void entrypoint();
void initializeSystem();
void handleStateMachine();

void loop() {
    // Main loop - handle state machine and timer updates
    if (systemInitialized) {
        handleStateMachine();
        
        // Update alarm clock (runs continuously)
        alarmClock.updateAlarm();
        
        // Handle alarm acknowledgment
        if (alarmClock.isAlarmTriggered() && select_button_pressed()) {
            alarmClock.acknowledgeAlarm();
            notificationManager.stopAlert();
        }
    }
    
    // Keep LED flashing updated
    notificationManager.update();

    delay(10); // Small delay to prevent excessive CPU usage
}

void handleStateMachine() {
    // Update state machine
    stateMachine.update();
    
    // Handle specific state logic
    AppState currentState = stateMachine.getCurrentState();
    AppState previousState = stateMachine.getPreviousState();
    
    switch (currentState) {
        case STATE_SINGLE_TIMER_SETUP:
            if (!singleTimer.isTimerRunning() && !singleTimer.isTimerFinished()) {
                singleTimer.handleSetupInput();
                if (select_button_pressed() && singleTimer.isSetupComplete()) {
                    singleTimer.startTimer();
                    stateMachine.setState(STATE_SINGLE_TIMER_RUNNING);
                }
            }
            break;
            
        case STATE_SINGLE_TIMER_RUNNING:
            singleTimer.updateTimer();
            if (singleTimer.isTimerFinished()) {
                // Push notification for single timer
                pushNotifier.sendNotification("Chrono-Cubo", "Your single timer is complete!");
                stateMachine.setState(STATE_SINGLE_TIMER_FINISHED);
            } else if (select_button_pressed()) {
                singleTimer.reset();
                stateMachine.setState(STATE_MAIN_MENU);
            }
            break;
            
        case STATE_SINGLE_TIMER_FINISHED:
            if (select_button_pressed()) {
                notificationManager.stopAlert();
                singleTimer.reset();
                stateMachine.setState(STATE_MAIN_MENU);
            }
            break;
            
        case STATE_MULTI_TIMER_SELECT:
            if (!multiTimer.isRoutineRunning() && !multiTimer.isRoutineFinished()) {
                multiTimer.handleRoutineSelectionInput();
                if (select_button_pressed() && multiTimer.isRoutineSelected()) {
                    multiTimer.startRoutine();
                    stateMachine.setState(STATE_MULTI_TIMER_RUNNING);
                }
            }
            break;
            
        case STATE_MULTI_TIMER_RUNNING:
            multiTimer.updateRoutine();
            if (multiTimer.isRoutineFinished()) {
                // Push notification for multi-phase routine
                pushNotifier.sendNotification("Chrono-Cubo", "Your multi-phase routine has finished!");
                stateMachine.setState(STATE_MULTI_TIMER_FINISHED);
            } else if (select_button_pressed()) {
                multiTimer.reset();
                stateMachine.setState(STATE_MAIN_MENU);
            }
            break;
            
        case STATE_MULTI_TIMER_FINISHED:
            if (select_button_pressed()) {
                notificationManager.stopAlert();
                multiTimer.reset();
                stateMachine.setState(STATE_MAIN_MENU);
            }
            break;
            
        case STATE_ALARM_SETUP:
            if (!alarmClock.isAlarmSet()) {
                alarmClock.handleSetupInput();
                if (select_button_pressed() && alarmClock.isSetupComplete()) {
                    alarmClock.setAlarm(alarmClock.getAlarmHour(), alarmClock.getAlarmMinute());
                    stateMachine.setState(STATE_MAIN_MENU);
                }
            }
            break;
            
        case STATE_TIME_DISPLAY:
            timeManager.displayCurrentTime();
            if (select_button_pressed()) {
                stateMachine.setState(STATE_MAIN_MENU);
            }
            break;

        case STATE_SETTINGS_MENU: {
            // Simple settings menu UI and navigation
            static int settingsSelected = 0;
            static bool drawn = false;
            if (previousState != STATE_SETTINGS_MENU) {
                settingsSelected = 0;
                drawn = false;
            }

            auto drawSettingsMenu = [&]() {
                display.clearDisplay();
                display.setTextSize(1);
                display.setTextColor(SSD1306_WHITE);
                display.setCursor(0, 0);
                display.println("Settings");
                display.println("========");

                for (int i = 0; i < 2; i++) {
                    int y = 16 + i * 10;
                    if (i == settingsSelected) {
                        display.fillRect(0, y - 1, SCREEN_WIDTH, 10, SSD1306_WHITE);
                        display.setTextColor(SSD1306_BLACK);
                    } else {
                        display.setTextColor(SSD1306_WHITE);
                    }
                    display.setCursor(2, y);
                    if (i == 0) {
                        display.print("Set Alertzy Key");
                    } else {
                        display.print("Back to Main");
                    }
                }

                display.setTextColor(SSD1306_WHITE);
                display.setCursor(0, 56);
                display.print("Move:Y Sel:Btn");
                display.display();
            };

            if (!drawn) { drawSettingsMenu(); drawn = true; }

            if (can_move()) {
                int y_move = get_y_movement();
                if (y_move == -1) { settingsSelected = (settingsSelected > 0) ? settingsSelected - 1 : 1; drawSettingsMenu(); }
                if (y_move == 1)  { settingsSelected = (settingsSelected < 1) ? settingsSelected + 1 : 0; drawSettingsMenu(); }
            }

            if (select_button_pressed()) {
                if (settingsSelected == 0) {
                    stateMachine.setState(STATE_ALERTZY_KEY_SETUP);
                } else {
                    stateMachine.setState(STATE_MAIN_MENU);
                }
            }
            break;
        }

        case STATE_ALERTZY_KEY_SETUP: {
            // Prompt for Alertzy key via on-screen keyboard
            display.clearDisplay();
            display.setTextSize(1);
            display.setTextColor(SSD1306_WHITE);
            display.setCursor(0, 0);
            display.println("Enter Alertzy Key");
            display.println("Press button to finish");
            display.display();

            const char* entered = prompt_keyboard();
            if (entered && strlen(entered) > 0) {
                pushNotifier.setAccountKey(String(entered));
                display.clearDisplay();
                display.setTextSize(1);
                display.setTextColor(SSD1306_WHITE);
                display.setCursor(0, 20);
                display.println("Key Saved!");
                display.display();
                delay(1000);
            }
            stateMachine.setState(STATE_SETTINGS_MENU);
            break;
        }
            
        default:
            break;
    }
}

void setup() {
    entrypoint();
    
    // Initialize system components
    initializeSystem();
    
    // Initialize notification manager
    notificationManager.begin();
    // Initialize push notifier
    pushNotifier.begin();

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
        
        // Initialize time management
        if (timeManager.initialize()) {
            Serial.println("Time manager initialized");
            
            // Sync time with NTP
            if (timeManager.syncTime()) {
                Serial.println("Time synchronized with NTP");
            } else {
                Serial.println("Failed to sync time, using RTC time");
            }
        }
        
        // Display final status
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 0);
        display.println("Chrono-Cubo Ready!");
        display.print("IP: ");
        display.println(WiFi.localIP());
        display.println();
        display.println("Time: " + timeManager.getCurrentTimeString());
        display.println();
        display.println("Press button to start");
        display.display();
        
        // Wait for button press to start
        while (!select_button_pressed()) {
            delay(100);
        }
        
        systemInitialized = true;
    }
}

void initializeSystem() {
    // Initialize controls
    init_controls();
    
    // Initialize state machine with menu items
    stateMachine.initialize();
    stateMachine.setMenuItems(mainMenuItems, sizeof(mainMenuItems) / sizeof(MenuItem));
    
    // Initialize timer components
    singleTimer.startSetup();
    multiTimer.startRoutineSelection();
    alarmClock.startSetup();
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
    display.println("Chrono-Cubo");
    display.println("Initializing...");
    display.display();
}