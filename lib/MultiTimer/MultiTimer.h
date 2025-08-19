#ifndef MULTITIMER_H
#define MULTITIMER_H

#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include "KeyInput.h"

// Timer phase structure
struct TimerPhase {
    String name;
    int duration_seconds;
    bool completed;
};

// Preset routine structure
struct TimerRoutine {
    String name;
    String description;
    TimerPhase phases[10]; // Maximum 10 phases per routine
    int phaseCount;
};

class MultiTimer {
private:
    Adafruit_SSD1306* display;
    
    // Timer state
    bool isRunning;
    bool isFinished;
    unsigned long startTime;
    unsigned long currentPhaseStartTime;
    int currentPhaseIndex;
    unsigned long remainingTime;
    
    // Routine management
    TimerRoutine* currentRoutine;
    int selectedRoutineIndex;
    int routineCount;
    TimerRoutine* availableRoutines;
    
    // Display variables
    unsigned long lastDisplayUpdate;
    const unsigned long displayUpdateInterval = 100; // Update display every 100ms
    
    // Internal methods
    void drawRoutineSelectionScreen();
    void drawRunningScreen();
    void drawFinishedScreen();
    void drawPhaseTransitionScreen();
    String formatTime(unsigned long seconds);
    void updateRemainingTime();
    void advanceToNextPhase();
    void initializePresetRoutines();
    
public:
    // Constructor
    MultiTimer(Adafruit_SSD1306* displayInstance);
    
    // Routine selection methods
    void startRoutineSelection();
    void handleRoutineSelectionInput();
    bool isRoutineSelected() const;
    
    // Running methods
    void startRoutine();
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
    int getSelectedRoutineIndex() const;
    TimerRoutine* getCurrentRoutine() const;
    
    // Preset routines
    void createPomodoroRoutine();
    void createWorkoutRoutine();
    void createStudyRoutine();
};

#endif // MULTITIMER_H
