#include "StateMachine.h"
#include "configs.h"
#include "SingleTimer.h"
#include "MultiTimer.h"
#include "AlarmClock.h"
#include "TimeManager.h"

StateMachine::StateMachine(Adafruit_SSD1306* displayInstance) 
    : display(displayInstance), currentState(STATE_MAIN_MENU), previousState(STATE_MAIN_MENU),
      selectedMenuItem(0), menuItemCount(0), menuItems(nullptr), stateEnterTime(0), lastUpdateTime(0) {
}

void StateMachine::initialize() {
    currentState = STATE_MAIN_MENU;
    previousState = STATE_MAIN_MENU;
    selectedMenuItem = 0;
    stateEnterTime = millis();
    lastUpdateTime = millis();
    
    // Initialize controls
    init_controls();
}

void StateMachine::update() {
    unsigned long currentTime = millis();
    lastUpdateTime = currentTime;
    
    // Handle state-specific logic
    switch (currentState) {
        case STATE_MAIN_MENU:
            handleMainMenu();
            break;
        case STATE_SINGLE_TIMER_SETUP:
            handleSingleTimerSetup();
            break;
        case STATE_SINGLE_TIMER_RUNNING:
            handleSingleTimerRunning();
            break;
        case STATE_SINGLE_TIMER_FINISHED:
            handleSingleTimerFinished();
            break;
        case STATE_MULTI_TIMER_SELECT:
            handleMultiTimerSelect();
            break;
        case STATE_MULTI_TIMER_RUNNING:
            handleMultiTimerRunning();
            break;
        case STATE_MULTI_TIMER_FINISHED:
            handleMultiTimerFinished();
            break;
        case STATE_ALARM_SETUP:
            handleAlarmSetup();
            break;
        case STATE_ALARM_RUNNING:
            handleAlarmRunning();
            break;
        case STATE_SETTINGS:
            handleSettings();
            break;
        case STATE_TIME_DISPLAY:
            handleTimeDisplay();
            break;
    }
}

void StateMachine::attachModules(SingleTimer* st, MultiTimer* mt, AlarmClock* ac, TimeManager* tm) {
    singleTimerModule = st;
    multiTimerModule = mt;
    alarmClockModule = ac;
    timeManagerModule = tm;
}

void StateMachine::handleMainMenu() {
    // Handle Y-axis navigation
    if (can_move()) {
        int y_move = get_y_movement();
        if (y_move != 0 && menuItems != nullptr) {
            if (y_move == -1) {
                // Move up
                selectedMenuItem = (selectedMenuItem > 0) ? selectedMenuItem - 1 : menuItemCount - 1;
            } else if (y_move == 1) {
                // Move down
                selectedMenuItem = (selectedMenuItem < menuItemCount - 1) ? selectedMenuItem + 1 : 0;
            }
            drawMainMenu();
        }
    }
    
    // Handle selection
    if (select_button_pressed() && menuItems != nullptr) {
        if (selectedMenuItem < menuItemCount && menuItems[selectedMenuItem].enabled) {
            transitionTo(menuItems[selectedMenuItem].targetState);
        }
    }
    
    // Draw menu if not already drawn
    static bool menuDrawn = false;
    if (!menuDrawn) {
        drawMainMenu();
        menuDrawn = true;
    }
}

void StateMachine::drawMainMenu() {
    if (!display || !menuItems) return;
    
    display->clearDisplay();
    display->setTextSize(1);
    display->setTextColor(SSD1306_WHITE);
    
    // Draw title
    display->setCursor(0, 0);
    display->println("Chrono-Cubo");
    display->println("============");
    
    // Draw menu items
    for (int i = 0; i < menuItemCount && i < 6; i++) {
        int y = 16 + (i * 8);
        drawMenuItem(i, (i == selectedMenuItem));
    }
    
    // Draw instructions
    display->setCursor(0, 56);
    display->print("Move:Y-Pot Sel:Btn");
    
    display->display();
}

void StateMachine::drawMenuItem(int index, bool selected) {
    if (!display || !menuItems || index >= menuItemCount) return;
    
    int y = 16 + (index * 8);
    
    if (selected) {
        // Highlight selected item
        display->fillRect(0, y - 1, SCREEN_WIDTH, 10, SSD1306_WHITE);
        display->setTextColor(SSD1306_BLACK);
    } else {
        display->setTextColor(SSD1306_WHITE);
    }
    
    display->setCursor(2, y);
    
    if (menuItems[index].enabled) {
        display->print("> ");
        display->print(menuItems[index].name);
    } else {
        display->print("  ");
        display->print(menuItems[index].name);
        display->print(" (Disabled)");
    }
}

void StateMachine::handleSingleTimerSetup() {
    if (!singleTimerModule || !display) return;
    // Delegate drawing and input handling to SingleTimer module
    singleTimerModule->drawCurrentScreen();
    // Input handling is expected to be called from here by design
    singleTimerModule->handleSetupInput();
}

void StateMachine::handleSingleTimerRunning() {
    if (!singleTimerModule || !display) return;
    singleTimerModule->drawCurrentScreen();
    singleTimerModule->updateTimer();
    if (singleTimerModule->isTimerFinished()) {
        // Transition to finished state
        transitionTo(STATE_SINGLE_TIMER_FINISHED);
    }
}

void StateMachine::handleSingleTimerFinished() {
    if (!singleTimerModule || !display) return;
    singleTimerModule->drawCurrentScreen();
}

void StateMachine::handleMultiTimerSelect() {
    if (!multiTimerModule || !display) return;
    multiTimerModule->drawCurrentScreen();
    multiTimerModule->handleTimerSelectionInput();
}

void StateMachine::handleMultiTimerRunning() {
    if (!multiTimerModule || !display) return;
    // Non-blocking update
    multiTimerModule->updateRoutine();
    multiTimerModule->drawCurrentScreen();
    if (multiTimerModule->isRoutineFinished()) {
        transitionTo(STATE_MULTI_TIMER_FINISHED);
    }

}

void StateMachine::handleMultiTimerFinished() {
    if (!multiTimerModule || !display) return;
    multiTimerModule->drawCurrentScreen();
}

void StateMachine::handleAlarmSetup() {
    if (!alarmClockModule || !display) return;
    alarmClockModule->drawCurrentScreen();
}

void StateMachine::handleAlarmRunning() {
    if (!alarmClockModule || !display) return;
    alarmClockModule->drawCurrentScreen();
}

void StateMachine::handleSettings() {
    if (!display) return;
    display->clearDisplay();
    display->setTextSize(1);
    display->setTextColor(SSD1306_WHITE);
    display->setCursor(0, 0);
    display->println("Settings");
    display->println("========");
    display->setCursor(0, 16);
    display->println("Use main menu to manage.");
    display->setCursor(0, 56);
    display->println("Press button to return");
    display->display();
}

void StateMachine::handleTimeDisplay() {
    if (!timeManagerModule || !display) return;
    timeManagerModule->displayCurrentTime();
}

void StateMachine::handleSettingsTimeZone() {
    if (!timeManagerModule || !display) return;
    static bool drawn = false;
    static int tz = 0;
    if (previousState != STATE_SETTINGS_TIMEZONE) { drawn = false; }

    auto draw = [&]() {
        display->clearDisplay();
        display->setTextSize(1);
        display->setTextColor(SSD1306_WHITE);
        display->setCursor(0, 0);
        display->println("Set Timezone");
        display->println("============");
        display->setCursor(0, 24);
        display->printf("GMT%+d\n", tz);
        display->setCursor(0, 56);
        display->print("X:Adj Btn:Save");
        display->display();
    };

    if (!drawn) { drawn = true; /* read current from timeManager if available (not exposed) */ }

    if (can_move()) {
        int x = get_x_movement();
        if (x != 0) {
            tz += (x == 1) ? 1 : -1;
            if (tz < -12) tz = -12;
            if (tz > 14) tz = 14;
            timeManagerModule->setTimezone(tz);
            draw();
        }
    }

    if (select_button_pressed()) {
        // Save to preferences
        Preferences p;
        if (p.begin("storage", false, "nvs")) {
            p.putInt("gmt_offset", tz);
            p.end();
        }
        transitionTo(STATE_SETTINGS_MENU);
    }
}

void StateMachine::setState(AppState newState) {
    transitionTo(newState);
}

void StateMachine::transitionTo(AppState newState) {
    previousState = currentState;
    currentState = newState;
    stateEnterTime = millis();
    
    Serial.print("State transition: ");
    Serial.print(previousState);
    Serial.print(" -> ");
    Serial.println(newState);
}

AppState StateMachine::getCurrentState() const {
    return currentState;
}

AppState StateMachine::getPreviousState() const {
    return previousState;
}

void StateMachine::setMenuItems(MenuItem* items, int count) {
    menuItems = items;
    menuItemCount = count;
    selectedMenuItem = 0;
}

int StateMachine::getSelectedMenuItem() const {
    return selectedMenuItem;
}

void StateMachine::setSelectedMenuItem(int index) {
    if (index >= 0 && index < menuItemCount) {
        selectedMenuItem = index;
    }
}

unsigned long StateMachine::getStateDuration() const {
    return millis() - stateEnterTime;
}

void StateMachine::resetStateTimer() {
    stateEnterTime = millis();
}
