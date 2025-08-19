#include "StateMachine.h"
#include "configs.h"

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
    // This state is handled in main.cpp
    // Just return to main menu if button is pressed
    if (select_button_pressed()) {
        transitionTo(STATE_MAIN_MENU);
    }
}

void StateMachine::handleSingleTimerRunning() {
    // This state is handled in main.cpp
    // Just return to main menu if button is pressed
    if (select_button_pressed()) {
        transitionTo(STATE_MAIN_MENU);
    }
}

void StateMachine::handleSingleTimerFinished() {
    // This state is handled in main.cpp
    // Just return to main menu if button is pressed
    if (select_button_pressed()) {
        transitionTo(STATE_MAIN_MENU);
    }
}

void StateMachine::handleMultiTimerSelect() {
    // This state is handled in main.cpp
    // Just return to main menu if button is pressed
    if (select_button_pressed()) {
        transitionTo(STATE_MAIN_MENU);
    }
}

void StateMachine::handleMultiTimerRunning() {
    // This state is handled in main.cpp
    // Just return to main menu if button is pressed
    if (select_button_pressed()) {
        transitionTo(STATE_MAIN_MENU);
    }
}

void StateMachine::handleMultiTimerFinished() {
    // This state is handled in main.cpp
    // Just return to main menu if button is pressed
    if (select_button_pressed()) {
        transitionTo(STATE_MAIN_MENU);
    }
}

void StateMachine::handleAlarmSetup() {
    // This state is handled in main.cpp
    // Just return to main menu if button is pressed
    if (select_button_pressed()) {
        transitionTo(STATE_MAIN_MENU);
    }
}

void StateMachine::handleAlarmRunning() {
    // This state is handled in main.cpp
    // Just return to main menu if button is pressed
    if (select_button_pressed()) {
        transitionTo(STATE_MAIN_MENU);
    }
}

void StateMachine::handleSettings() {
    // Placeholder for future implementation
    if (select_button_pressed()) {
        transitionTo(STATE_MAIN_MENU);
    }
}

void StateMachine::handleTimeDisplay() {
    // Simple time display state
    if (select_button_pressed()) {
        transitionTo(STATE_MAIN_MENU);
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
