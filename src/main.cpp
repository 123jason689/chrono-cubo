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
#include "StorageManager.h"
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
StorageManager storageManager(&pref);
AlarmClock alarmClock(&display, &rtc, &pushNotifier);
NotificationManager notificationManager;

// Editor context (for custom timers and phases)
static CustomTimer g_editTimer;
static bool g_isCreateTimer = false;
static int g_editTimerIndex = -1;
static int g_editPhaseIndex = -1;

// Menu items
MenuItem mainMenuItems[] = {
    {"Single Timer", STATE_SINGLE_TIMER_SETUP, true},
    {"Multi-Phase Timer", STATE_MULTI_TIMER_SELECT, true},
    {"Sleep Alarm", STATE_ALARM_SETUP, true},
    {"Settings", STATE_SETTINGS_MENU, true},
    {"Time Display", STATE_TIME_DISPLAY, true}
};

MenuItem settingsMenuItems[] = {
    {"Manage Timers", STATE_SETTINGS_TIMERS_MENU, true},
    {"Manage Alertzy Keys", STATE_ALERTZY_KEY_LIST, true},
    {"WiFi Setup", STATE_WIFI_SETUP, true},
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
                pushNotifier.sendAll("Chrono-Cubo", "Your single timer is complete!");
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
                multiTimer.handleTimerSelectionInput();
                if (select_button_pressed() && multiTimer.isTimerSelected()) {
                    multiTimer.startTimer();
                    stateMachine.setState(STATE_MULTI_TIMER_RUNNING);
                }
            }
            break;
            
        case STATE_MULTI_TIMER_RUNNING:
            multiTimer.updateRoutine();
            if (multiTimer.isRoutineFinished()) {
                // Push notification for multi-phase routine
                pushNotifier.sendAll("Chrono-Cubo", "Your multi-phase routine has finished!");
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

        case STATE_MAIN_MENU:
            // Auto-show time after idle if no timers are running
            if (!singleTimer.isTimerRunning() && !multiTimer.isRoutineRunning()) {
                const unsigned long idleTimeoutMs = 5000; // 5 seconds
                if (stateMachine.getStateDuration() > idleTimeoutMs) {
                    stateMachine.setState(STATE_TIME_DISPLAY);
                }
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

                for (int i = 0; i < 4; i++) {
                    int y = 16 + i * 10;
                    if (i == settingsSelected) {
                        display.fillRect(0, y - 1, SCREEN_WIDTH, 10, SSD1306_WHITE);
                        display.setTextColor(SSD1306_BLACK);
                    } else {
                        display.setTextColor(SSD1306_WHITE);
                    }
                    display.setCursor(2, y);
                    if (i == 0) display.print("Manage Timers");
                    if (i == 1) display.print("Manage Alertzy Keys");
                    if (i == 2) display.print("WiFi Setup");
                    if (i == 3) display.print("Back to Main");
                }

                display.setTextColor(SSD1306_WHITE);
                display.setCursor(0, 56);
                display.print("Move:Y Sel:Btn");
                display.display();
            };

            if (!drawn) { drawSettingsMenu(); drawn = true; }

            if (can_move()) {
                int y_move = get_y_movement();
                if (y_move == -1) { settingsSelected = (settingsSelected > 0) ? settingsSelected - 1 : 3; drawSettingsMenu(); }
                if (y_move == 1)  { settingsSelected = (settingsSelected < 3) ? settingsSelected + 1 : 0; drawSettingsMenu(); }
            }

            if (select_button_pressed()) {
                if (settingsSelected == 0) stateMachine.setState(STATE_SETTINGS_TIMERS_MENU);
                if (settingsSelected == 1) stateMachine.setState(STATE_ALERTZY_KEY_LIST);
                if (settingsSelected == 2) stateMachine.setState(STATE_WIFI_SETUP);
                if (settingsSelected == 3) stateMachine.setState(STATE_MAIN_MENU);
            }
            break;
        }
        case STATE_WIFI_SETUP: {
            // Reuse WiFiSelector to scan/select/connect new WiFi
            display.clearDisplay();
            display.setTextSize(1);
            display.setTextColor(SSD1306_WHITE);
            display.setCursor(0, 0);
            display.println("WiFi Setup");
            display.println("Scanning networks...");
            display.display();

            auto networks = wifiSelector.scanNetworks();
            if (networks.empty()) {
                display.clearDisplay();
                display.setCursor(0, 20);
                display.println("No networks found");
                display.display();
                delay(1000);
                stateMachine.setState(STATE_SETTINGS_MENU);
                break;
            }

            if (!wifiSelector.selectAndConnectNetwork(networks)) {
                display.clearDisplay();
                display.setCursor(0, 20);
                display.println("WiFi not changed");
                display.display();
                delay(800);
            } else {
                display.clearDisplay();
                display.setCursor(0, 20);
                display.println("WiFi connected!");
                display.print("IP: ");
                display.println(WiFi.localIP());
                display.display();
                delay(1000);
            }
            stateMachine.setState(STATE_SETTINGS_MENU);
            break;
        }


        case STATE_ALERTZY_KEY_LIST: {
            static int sel = 0;
            static bool drawn = false;
            if (previousState != STATE_ALERTZY_KEY_LIST) { sel = 0; drawn = false; }

            auto drawList = [&]() {
                const auto& accounts = pushNotifier.getAccounts();
                display.clearDisplay();
                display.setTextSize(1);
                display.setTextColor(SSD1306_WHITE);
                display.setCursor(0, 0);
                display.println("Alertzy Accounts");
                display.println("================");

                // Row 0: Add New
                for (int i = 0; i < (int)accounts.size() + 2; ++i) {
                    int y = 16 + i * 10;
                    if (y > 54) break;
                    bool isSelected = (i == sel);
                    if (isSelected) { display.fillRect(0, y - 1, SCREEN_WIDTH, 10, SSD1306_WHITE); display.setTextColor(SSD1306_BLACK); }
                    else { display.setTextColor(SSD1306_WHITE); }
                    display.setCursor(2, y);
                    if (i == 0) display.print("+ Add New");
                    else if (i == (int)accounts.size() + 1) display.print("< Back");
                    else display.print(accounts[i - 1].name);
                }
                display.setTextColor(SSD1306_WHITE);
                display.setCursor(0, 56);
                display.print("Y:Move Btn:Select");
                display.display();
            };

            if (!drawn) { drawList(); drawn = true; }

            if (can_move()) {
                int y_move = get_y_movement();
                int maxIndex = (int)pushNotifier.getAccounts().size() + 1;
                if (y_move == -1) { sel = (sel > 0) ? sel - 1 : maxIndex; drawList(); }
                if (y_move == 1)  { sel = (sel < maxIndex) ? sel + 1 : 0; drawList(); }
            }

            if (select_button_pressed()) {
                const auto& accounts = pushNotifier.getAccounts();
                if (sel == 0) {
                    stateMachine.setState(STATE_ALERTZY_KEY_CREATE);
                } else if (sel == (int)accounts.size() + 1) {
                    stateMachine.setState(STATE_SETTINGS_MENU);
                } else {
                    // Simple delete confirmation for selected account
                    int target = sel - 1;
                    display.clearDisplay();
                    display.setTextSize(1);
                    display.setTextColor(SSD1306_WHITE);
                    display.setCursor(0, 20);
                    display.print("Delete: ");
                    display.println(accounts[target].name);
                    display.println("");
                    display.println("Press button to confirm");
                    display.display();
                    // wait for button
                    while (!select_button_pressed()) { delay(50); }
                    // delete
                    auto vec = accounts; // copy
                    vec.erase(vec.begin() + target);
                    storageManager.saveAlertzyAccounts(vec);
                    pushNotifier.setAccounts(vec);
                    sel = 0;
                    drawn = false;
                }
            }
            break;
        }

        case STATE_ALERTZY_KEY_CREATE: {
            display.clearDisplay();
            display.setTextSize(1);
            display.setTextColor(SSD1306_WHITE);
            display.setCursor(0, 0);
            display.println("New Alertzy Account");
            display.println("Enter Name:");
            display.display();
            const char* name = prompt_keyboard();

            display.clearDisplay();
            display.setCursor(0, 0);
            display.println("Enter Key:");
            display.display();
            const char* key = prompt_keyboard();

            if (name && key && strlen(name) > 0 && strlen(key) > 0) {
                auto accounts = pushNotifier.getAccounts();
                accounts.push_back(AlertzyAccount{String(name), String(key)});
                storageManager.saveAlertzyAccounts(accounts);
                pushNotifier.setAccounts(accounts);
                display.clearDisplay();
                display.setCursor(0, 20);
                display.println("Saved!");
                display.display();
                delay(800);
            }
            stateMachine.setState(STATE_ALERTZY_KEY_LIST);
            break;
        }

        case STATE_SETTINGS_TIMERS_MENU: {
            static int sel = 0;
            static bool drawn = false;
            if (previousState != STATE_SETTINGS_TIMERS_MENU) { sel = 0; drawn = false; }

            auto draw = [&]() {
                const auto& timers = multiTimer.getTimers();
                display.clearDisplay();
                display.setTextSize(1);
                display.setTextColor(SSD1306_WHITE);
                display.setCursor(0, 0);
                display.println("Timers");
                display.println("======");

                int total = (int)timers.size() + 2; // +Create, <Back
                for (int i = 0; i < total && (16 + i * 10) <= 54; ++i) {
                    int y = 16 + i * 10;
                    bool isSel = (i == sel);
                    if (isSel) { display.fillRect(0, y - 1, SCREEN_WIDTH, 10, SSD1306_WHITE); display.setTextColor(SSD1306_BLACK); }
                    else { display.setTextColor(SSD1306_WHITE); }
                    display.setCursor(2, y);
                    if (i == 0) display.print("+ Create New");
                    else if (i == total - 1) display.print("< Back");
                    else display.print(timers[i - 1].name);
                }
                display.setTextColor(SSD1306_WHITE);
                display.setCursor(0, 56);
                display.print("Y:Move Btn:Select");
                display.display();
            };

            if (!drawn) { draw(); drawn = true; }

            if (can_move()) {
                int y_move = get_y_movement();
                int maxIndex = (int)multiTimer.getTimers().size() + 1;
                if (y_move == -1) { sel = (sel > 0) ? sel - 1 : maxIndex; draw(); }
                if (y_move == 1)  { sel = (sel < maxIndex) ? sel + 1 : 0; draw(); }
            }

            if (select_button_pressed()) {
                auto timers = multiTimer.getTimers();
                int total = (int)timers.size() + 2;
                if (sel == 0) {
                    // Create new timer -> prompt for name then phase list
                    g_editTimerIndex = -1; g_isCreateTimer = true; g_editTimer = CustomTimer{};
                    display.clearDisplay();
                    display.setCursor(0, 0);
                    display.println("Timer Name:");
                    display.display();
                    const char* name = prompt_keyboard();
                    if (name && strlen(name) > 0) g_editTimer.name = String(name); else g_editTimer.name = "New Timer";
                    stateMachine.setState(STATE_PHASE_LIST_EDIT);
                } else if (sel == total - 1) {
                    stateMachine.setState(STATE_SETTINGS_MENU);
                } else {
                    // Edit existing
                    g_editTimerIndex = sel - 1; g_isCreateTimer = false; g_editTimer = timers[g_editTimerIndex];
                    stateMachine.setState(STATE_PHASE_LIST_EDIT);
                }
            }
            break;
        }

        case STATE_TIMER_CREATE_EDIT: {
            // Not used; creation handled inline in timers menu
            stateMachine.setState(STATE_PHASE_LIST_EDIT);
            break;
        }

        case STATE_PHASE_LIST_EDIT: {
            // Editor: list phases, add/edit/delete, save & exit
            static int sel = 0; static bool drawn = false;
            if (previousState != STATE_PHASE_LIST_EDIT) { sel = 0; drawn = false; }

            auto draw = [&]() {
                display.clearDisplay();
                display.setTextSize(1);
                display.setTextColor(SSD1306_WHITE);
                display.setCursor(0, 0);
                display.print("Edit: "); display.println(g_editTimer.name);
                display.println("----------------");
                int total = (int)g_editTimer.phases.size() + 3; // +Add, +Save, <Back
                for (int i = 0; i < total && (16 + i * 10) <= 54; ++i) {
                    int y = 16 + i * 10;
                    bool isSel = (i == sel);
                    if (isSel) { display.fillRect(0, y - 1, SCREEN_WIDTH, 10, SSD1306_WHITE); display.setTextColor(SSD1306_BLACK); }
                    else { display.setTextColor(SSD1306_WHITE); }
                    display.setCursor(2, y);
                    if (i == 0) display.print("+ Add Phase");
                    else if (i == total - 2) display.print("Save & Exit");
                    else if (i == total - 1) display.print("< Back");
                    else {
                        int idx = i - 1;
                        display.print(idx + 1); display.print(". ");
                        display.print(g_editTimer.phases[idx].name);
                    }
                }
                display.setTextColor(SSD1306_WHITE);
                display.setCursor(0, 56);
                display.print("Y:Move X:Del Btn:Select");
                display.display();
            };

            if (!drawn) { draw(); drawn = true; }

            if (can_move()) {
                int y_move = get_y_movement();
                int maxIndex = (int)g_editTimer.phases.size() + 2;
                if (y_move == -1) { sel = (sel > 0) ? sel - 1 : maxIndex; draw(); }
                if (y_move == 1)  { sel = (sel < maxIndex) ? sel + 1 : 0; draw(); }
            }
            // Quick delete with X-axis right-left as example
            if (can_move()) {
                int x_move = get_x_movement();
                if (x_move != 0) {
                    // Delete selected phase if a phase row selected
                    int total = (int)g_editTimer.phases.size() + 3;
                    if (sel > 0 && sel < total - 2) {
                        int idx = sel - 1;
                        if (!g_editTimer.phases.empty()) {
                            g_editTimer.phases.erase(g_editTimer.phases.begin() + idx);
                            if (sel > 0) sel--;
                            draw();
                        }
                    }
                }
            }

            if (select_button_pressed()) {
                int total = (int)g_editTimer.phases.size() + 3;
                if (sel == 0) {
                    // Add new phase -> go to phase editor
                    g_editPhaseIndex = -1;
                    stateMachine.setState(STATE_PHASE_EDIT);
                } else if (sel == total - 2) {
                    // Save & Exit
                    auto timers = multiTimer.getTimers();
                    if (g_isCreateTimer) timers.push_back(g_editTimer);
                    else if (g_editTimerIndex >= 0 && g_editTimerIndex < (int)timers.size()) timers[g_editTimerIndex] = g_editTimer;
                    storageManager.saveCustomTimers(timers);
                    multiTimer.setTimers(timers);
                    stateMachine.setState(STATE_SETTINGS_TIMERS_MENU);
                } else if (sel == total - 1) {
                    stateMachine.setState(STATE_SETTINGS_TIMERS_MENU);
                } else {
                    // Edit phase
                    g_editPhaseIndex = sel - 1;
                    stateMachine.setState(STATE_PHASE_EDIT);
                }
            }
            break;
        }

        case STATE_PHASE_EDIT: {
            // Phase editor: name, duration (H/M/S), sound, notifications (accounts)
            static int field = 0; // 0=name,1=H,2=M,3=S,4=sound,5=notify
            static uint32_t dur = 60;
            static uint8_t track = 1;
            static String name = "Phase";
            static std::vector<uint8_t> notify;
            static bool drawn = false;
            if (!drawn) {
                // Initialize from existing or defaults
                if (g_editPhaseIndex >= 0 && g_editPhaseIndex < (int)g_editTimer.phases.size()) {
                    auto& p = g_editTimer.phases[g_editPhaseIndex];
                    name = p.name; dur = p.duration_seconds; track = p.sound_track; notify = p.alertzy_key_indices;
                } else { name = "Phase"; dur = 60; track = 1; notify.clear(); }
                drawn = true;
            }

            auto draw = [&]() {
                display.clearDisplay();
                display.setTextSize(1);
                display.setTextColor(SSD1306_WHITE);
                display.setCursor(0, 0);
                display.println("Phase Editor");
                display.println("------------");
                // Name
                display.setCursor(0, 16);
                display.print("Name: "); display.println(name);
                // Duration
                uint32_t h = dur / 3600; uint32_t m = (dur % 3600) / 60; uint32_t s = dur % 60;
                display.setCursor(0, 26);
                display.print("Duration: ");
                char buf[16]; sprintf(buf, "%02lu:%02lu:%02lu", (unsigned long)h, (unsigned long)m, (unsigned long)s);
                display.println(buf);
                // Sound
                display.setCursor(0, 36);
                display.print("Sound: Track "); display.println(track);
                // Notifications
                display.setCursor(0, 46);
                display.print("Notify: ");
                const auto& accounts = pushNotifier.getAccounts();
                for (size_t i = 0; i < accounts.size() && i < 3; ++i) { // show up to 3 for space
                    bool on = std::find(notify.begin(), notify.end(), (uint8_t)i) != notify.end();
                    display.print(on ? "[x]" : "[ ]");
                    display.print(accounts[i].name);
                    if (i < accounts.size() - 1) display.print(" ");
                }
                // Field indicator
                display.setCursor(0, 56);
                if (field == 0) display.print("^Name Btn=edit");
                else if (field == 1) display.print("^Hour Y=field X=+-");
                else if (field == 2) display.print("^Minute Y=field X=+-");
                else if (field == 3) display.print("^Second Y=field X=+-");
                else if (field == 4) display.print("^Sound Y=field X=+-");
                else display.print("^Notify Btn=toggle");
                display.display();
            };

            if (!drawn) draw(); else draw();

            if (can_move()) {
                int y_move = get_y_movement();
                if (y_move == -1) { field = (field == 0) ? 5 : field - 1; draw(); }
                if (y_move == 1)  { field = (field == 5) ? 0 : field + 1; draw(); }
                int x_move = get_x_movement();
                if (x_move != 0) {
                    if (field == 1) { // hour
                        int32_t h = (int32_t)(dur / 3600) + (x_move == 1 ? 1 : -1);
                        if (h < 0) h = 23; if (h > 23) h = 0;
                        uint32_t m = (dur % 3600) / 60; uint32_t s = dur % 60;
                        dur = (uint32_t)h * 3600 + m * 60 + s;
                    } else if (field == 2) { // minute
                        int32_t m = (int32_t)((dur % 3600) / 60) + (x_move == 1 ? 1 : -1);
                        if (m < 0) m = 59; if (m > 59) m = 0;
                        uint32_t h = dur / 3600; uint32_t s = dur % 60;
                        dur = h * 3600 + (uint32_t)m * 60 + s;
                    } else if (field == 3) { // second
                        int32_t s = (int32_t)(dur % 60) + (x_move == 1 ? 1 : -1);
                        if (s < 0) s = 59; if (s > 59) s = 0;
                        uint32_t h = dur / 3600; uint32_t m = (dur % 3600) / 60;
                        dur = h * 3600 + m * 60 + (uint32_t)s;
                    } else if (field == 4) { // sound
                        if (x_move == 1) track = (track < 50) ? track + 1 : 1;
                        else track = (track > 1) ? track - 1 : 50;
                    }
                    draw();
                }
            }

            if (select_button_pressed()) {
                if (field == 0) {
                    display.clearDisplay();
                    display.setCursor(0, 0);
                    display.println("Phase Name:");
                    display.display();
                    const char* n = prompt_keyboard();
                    if (n && strlen(n) > 0) name = String(n);
                    draw();
                } else if (field == 5) {
                    // Toggle first account for brevity; can expand with sub-list later
                    if (!pushNotifier.getAccounts().empty()) {
                        uint8_t idx = 0;
                        auto it = std::find(notify.begin(), notify.end(), idx);
                        if (it == notify.end()) notify.push_back(idx);
                        else notify.erase(it);
                        draw();
                    }
                } else {
                    // Save phase and return
                    TimerPhase p; p.name = name; p.duration_seconds = dur; p.sound_track = track; p.alertzy_key_indices = notify;
                    if (g_editPhaseIndex >= 0 && g_editPhaseIndex < (int)g_editTimer.phases.size()) g_editTimer.phases[g_editPhaseIndex] = p;
                    else g_editTimer.phases.push_back(p);
                    drawn = false; field = 0; g_editPhaseIndex = -1;
                    stateMachine.setState(STATE_PHASE_LIST_EDIT);
                }
            }
            break;
        }

        case STATE_SETTINGS_ALERTS_MENU:
        case STATE_CUSTOM_TIMER_START: {
            display.clearDisplay();
            display.setTextSize(1);
            display.setTextColor(SSD1306_WHITE);
            display.setCursor(0, 20);
            display.println("Not used");
            display.setCursor(0, 56);
            display.println("Press button to go back");
            display.display();
            if (select_button_pressed()) stateMachine.setState(STATE_SETTINGS_MENU);
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
    // Load persisted data
    auto alertzyAccounts = storageManager.loadAlertzyAccounts();
    pushNotifier.setAccounts(alertzyAccounts);
    auto customTimers = storageManager.loadCustomTimers();
    multiTimer.setTimers(customTimers);
    
    // Attempt WiFi connection but do not block functionality if unavailable
    bool wifiConnected = false;
    auto networks = wifiSelector.scanNetworks();
    if (!networks.empty()) {
        if (wifiSelector.connectWithSavedCredentials(networks)) {
            wifiConnected = true;
            Serial.println("Connected using saved credentials");
        } else if (wifiSelector.selectAndConnectNetwork(networks)) {
            wifiConnected = true;
      Serial.println("Successfully connected to selected network");
    } else {
      Serial.println("Failed to connect to any network");
    }
  } else {
        Serial.println("No networks found; starting in offline mode");
    }

    // Initialize time management (RTC always available)
    if (timeManager.initialize()) {
        Serial.println("Time manager initialized");
        if (wifiConnected) {
            if (timeManager.syncTime()) {
                Serial.println("Time synchronized with NTP");
            } else {
                Serial.println("NTP sync failed; using RTC time");
            }
        } else {
            Serial.println("Offline; using RTC time");
        }
    }
    
    // Display final status
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Chrono-Cubo Ready!");
    if (wifiConnected) {
    display.print("IP: ");
    display.println(WiFi.localIP());
    } else {
        display.println("Offline mode (no WiFi)");
    }
    display.println();
    display.println("Time: " + timeManager.getCurrentTimeString());
    display.println();
    display.println("Press button to start");
    display.display();

    // Wait for button press to start
    while (!select_button_pressed()) {
        delay(100);
    }

    // Start by showing the clock; press button to open menu
    stateMachine.setState(STATE_TIME_DISPLAY);
    systemInitialized = true;
}

void initializeSystem() {
    // Initialize controls
    init_controls();
    
    // Initialize state machine with menu items
    stateMachine.initialize();
    stateMachine.attachModules(&singleTimer, &multiTimer, &alarmClock, &timeManager);
    stateMachine.setMenuItems(mainMenuItems, sizeof(mainMenuItems) / sizeof(MenuItem));
    
    // Initialize timer components
    singleTimer.startSetup();
    multiTimer.startTimerSelection();
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