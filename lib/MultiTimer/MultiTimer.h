#ifndef MULTITIMER_H
#define MULTITIMER_H

#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <vector>
#include "KeyInput.h"
#include "DataModels.h"

class MultiTimer {
private:
    Adafruit_SSD1306* display;
    
    // Timer state
    bool isRunning;
    bool isFinished;
    unsigned long currentPhaseStartTime;
    int currentPhaseIndex;
    unsigned long remainingTime;
    
    // Routines (custom timers)
    std::vector<CustomTimer> timers;
    int selectedTimerIndex;
    CustomTimer* currentTimer;
    
    // Display variables
    unsigned long lastDisplayUpdate;
    const unsigned long displayUpdateInterval = 100; // Update display every 100ms
    
    // Internal methods
    void drawTimerSelectionScreen();
    void drawRunningScreen();
    void drawFinishedScreen();
    void drawPhaseTransitionScreen();
    String formatTime(unsigned long seconds);
    void updateRemainingTime();
    void advanceToNextPhase();
    
public:
    // Constructor
    MultiTimer(Adafruit_SSD1306* displayInstance);
    
    // Load/Set timers
    void setTimers(const std::vector<CustomTimer>& list);
    const std::vector<CustomTimer>& getTimers() const;
    
    // Selection UI
    void startTimerSelection();
    void handleTimerSelectionInput();
    bool isTimerSelected() const;
    int getSelectedTimerIndex() const;
    void setSelectedTimerIndex(int index);
    
    // Running methods
    void startTimer();
    void updateRoutine();
    bool isRoutineRunning() const;
    bool isRoutineFinished() const;
    
    // Display methods
    void drawCurrentScreen();
    
    // Control methods
    void reset();
    void pause();
    void resume();
    
    // Getters
    unsigned long getRemainingTime() const;
    int getCurrentPhaseIndex() const;
    String getCurrentPhaseName() const;
};

#endif // MULTITIMER_H
