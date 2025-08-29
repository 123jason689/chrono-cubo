#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include "KeyInput.h"

// Forward declarations to avoid heavy includes in header
class SingleTimer;
class MultiTimer;
class AlarmClock;
class TimeManager;

// Application states
enum AppState {
    STATE_MAIN_MENU,
    STATE_SINGLE_TIMER_SETUP,
    STATE_SINGLE_TIMER_RUNNING,
    STATE_SINGLE_TIMER_FINISHED,
    STATE_MULTI_TIMER_SELECT,
    STATE_MULTI_TIMER_RUNNING,
    STATE_MULTI_TIMER_FINISHED,
    STATE_ALARM_SETUP,
    STATE_ALARM_LIST_MENU,
    STATE_ALARM_REMOVE_MENU,
    STATE_ALARM_TRIGGERED,
    STATE_ALARM_RUNNING,
    STATE_SETTINGS,
    STATE_TIME_DISPLAY,
    STATE_SETTINGS_MENU,
    STATE_SETTINGS_VOLUME,
    STATE_SETTINGS_TIMEZONE,
    STATE_ALERTZY_KEY_SETUP,
    STATE_SETTINGS_TIMERS_MENU,
    STATE_SETTINGS_ALERTS_MENU,
    STATE_TIMER_CREATE_EDIT,
    STATE_PHASE_LIST_EDIT,
    STATE_PHASE_EDIT,
    STATE_ALERTZY_KEY_LIST,
    STATE_ALERTZY_KEY_CREATE,
    STATE_CUSTOM_TIMER_START,
    STATE_WIFI_SETUP
};

// Menu item structure
struct MenuItem {
    String name;
    AppState targetState;
    bool enabled;
};

class StateMachine {
private:
    AppState currentState;
    AppState previousState;
    Adafruit_SSD1306* display;
    
    // Connected modules used for drawing state UIs
    SingleTimer* singleTimerModule = nullptr;
    MultiTimer* multiTimerModule = nullptr;
    AlarmClock* alarmClockModule = nullptr;
    TimeManager* timeManagerModule = nullptr;
    
    // Menu navigation
    int selectedMenuItem;
    int menuItemCount;
    MenuItem* menuItems;
    
    // State transition tracking
    unsigned long stateEnterTime;
    unsigned long lastUpdateTime;
    
    // Internal methods
    void handleMainMenu();
    void handleSingleTimerSetup();
    void handleSingleTimerRunning();
    void handleSingleTimerFinished();
    void handleMultiTimerSelect();
    void handleMultiTimerRunning();
    void handleMultiTimerFinished();
    void handleAlarmSetup();
    void handleAlarmRunning();
    void handleSettings();
    void handleSettingsTimeZone();
    void handleTimeDisplay();
    
    void drawMainMenu();
    void drawMenuItem(int index, bool selected);
    void transitionTo(AppState newState);
    
public:
    // Constructor
    StateMachine(Adafruit_SSD1306* displayInstance);
    
    // Wire up modules so the state machine can render their screens
    void attachModules(SingleTimer* st, MultiTimer* mt, AlarmClock* ac, TimeManager* tm);
    
    // State management
    void initialize();
    void update();
    void setState(AppState newState);
    AppState getCurrentState() const;
    AppState getPreviousState() const;
    
    // Menu management
    void setMenuItems(MenuItem* items, int count);
    int getSelectedMenuItem() const;
    void setSelectedMenuItem(int index);
    
    // Utility methods
    unsigned long getStateDuration() const;
    void resetStateTimer();
};

#endif // STATEMACHINE_H
