#ifndef SINGLETIMER_H
#define SINGLETIMER_H

#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include "KeyInput.h"

class SingleTimer {
private:
    Adafruit_SSD1306* display;
    
    // Timer state
    bool isRunning;
    bool isFinished;
    unsigned long startTime;
    unsigned long duration;
    unsigned long remainingTime;
    
    // Setup state
    int setupMinutes;
    int setupSeconds;
    bool editingMinutes; // true = editing minutes, false = editing seconds
    
    // Display variables
    unsigned long lastDisplayUpdate;
    const unsigned long displayUpdateInterval = 100; // Update display every 100ms
    
    // Internal methods
    void drawSetupScreen();
    void drawRunningScreen();
    void drawFinishedScreen();
    String formatTime(unsigned long seconds);
    void updateRemainingTime();
    
public:
    // Constructor
    SingleTimer(Adafruit_SSD1306* displayInstance);
    
    // Setup methods
    void startSetup();
    void handleSetupInput();
    bool isSetupComplete() const;
    
    // Running methods
    void startTimer();
    void updateTimer();
    bool isTimerRunning() const;
    bool isTimerFinished() const;
    
    // Display methods
    void drawCurrentScreen();
    
    // Control methods
    void reset();
    void pause();
    void resume();
    
    // Getters
    unsigned long getRemainingTime() const;
    unsigned long getDuration() const;
    int getSetupMinutes() const;
    int getSetupSeconds() const;
};

#endif // SINGLETIMER_H
