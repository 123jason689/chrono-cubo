#include "MultiTimer.h"
#include "configs.h"
#include "NotificationManager.h"

extern NotificationManager notificationManager;

MultiTimer::MultiTimer(Adafruit_SSD1306* displayInstance) 
    : display(displayInstance), isRunning(false), isFinished(false), startTime(0), currentPhaseStartTime(0),
      currentPhaseIndex(0), remainingTime(0), currentRoutine(nullptr), selectedRoutineIndex(0), routineCount(0), availableRoutines(nullptr), lastDisplayUpdate(0) {
    initializePresetRoutines();
}

void MultiTimer::initializePresetRoutines() {
    // Allocate space for preset routines
    routineCount = 3;
    availableRoutines = new TimerRoutine[routineCount];
    
    // Create Pomodoro routine
    createPomodoroRoutine();
    
    // Create Workout routine
    createWorkoutRoutine();
    
    // Create Study routine
    createStudyRoutine();
}

void MultiTimer::createPomodoroRoutine() {
    TimerRoutine& pomodoro = availableRoutines[0];
    pomodoro.name = "Pomodoro";
    pomodoro.description = "25min work, 5min break";
    pomodoro.phaseCount = 2;
    
    // Work phase
    pomodoro.phases[0].name = "Work";
    pomodoro.phases[0].duration_seconds = 25 * 60; // 25 minutes
    pomodoro.phases[0].completed = false;
    
    // Break phase
    pomodoro.phases[1].name = "Break";
    pomodoro.phases[1].duration_seconds = 5 * 60; // 5 minutes
    pomodoro.phases[1].completed = false;
}

void MultiTimer::createWorkoutRoutine() {
    TimerRoutine& workout = availableRoutines[1];
    workout.name = "Workout";
    workout.description = "Warmup, Exercise, Cooldown";
    workout.phaseCount = 3;
    
    // Warmup phase
    workout.phases[0].name = "Warmup";
    workout.phases[0].duration_seconds = 5 * 60; // 5 minutes
    workout.phases[0].completed = false;
    
    // Exercise phase
    workout.phases[1].name = "Exercise";
    workout.phases[1].duration_seconds = 20 * 60; // 20 minutes
    workout.phases[1].completed = false;
    
    // Cooldown phase
    workout.phases[2].name = "Cooldown";
    workout.phases[2].duration_seconds = 5 * 60; // 5 minutes
    workout.phases[2].completed = false;
}

void MultiTimer::createStudyRoutine() {
    TimerRoutine& study = availableRoutines[2];
    study.name = "Study";
    study.description = "Focus, Review, Break";
    study.phaseCount = 3;
    
    // Focus phase
    study.phases[0].name = "Focus";
    study.phases[0].duration_seconds = 45 * 60; // 45 minutes
    study.phases[0].completed = false;
    
    // Review phase
    study.phases[1].name = "Review";
    study.phases[1].duration_seconds = 10 * 60; // 10 minutes
    study.phases[1].completed = false;
    
    // Break phase
    study.phases[2].name = "Break";
    study.phases[2].duration_seconds = 15 * 60; // 15 minutes
    study.phases[2].completed = false;
}

void MultiTimer::startRoutineSelection() {
    selectedRoutineIndex = 0;
    isRunning = false;
    isFinished = false;
    currentRoutine = nullptr;
    drawRoutineSelectionScreen();
}

void MultiTimer::handleRoutineSelectionInput() {
    // Handle Y-axis movement to navigate routines
    if (can_move()) {
        int y_move = get_y_movement();
        if (y_move != 0) {
            if (y_move == -1) {
                selectedRoutineIndex = (selectedRoutineIndex > 0) ? selectedRoutineIndex - 1 : routineCount - 1;
            } else if (y_move == 1) {
                selectedRoutineIndex = (selectedRoutineIndex < routineCount - 1) ? selectedRoutineIndex + 1 : 0;
            }
            drawRoutineSelectionScreen();
        }
    }
}

bool MultiTimer::isRoutineSelected() const {
    return selectedRoutineIndex >= 0 && selectedRoutineIndex < routineCount;
}

void MultiTimer::startRoutine() {
    if (isRoutineSelected()) {
        currentRoutine = &availableRoutines[selectedRoutineIndex];
        currentPhaseIndex = 0;
        startTime = millis();
        currentPhaseStartTime = millis();
        isRunning = true;
        isFinished = false;
        
        // Initialize first phase
        remainingTime = currentRoutine->phases[0].duration_seconds;
        currentRoutine->phases[0].completed = false;
        
        lastDisplayUpdate = millis();
        drawRunningScreen();
    }
}

void MultiTimer::updateRoutine() {
    if (!isRunning || !currentRoutine) return;
    
    updateRemainingTime();
    
    // Check if current phase finished
    if (remainingTime <= 0) {
        // Mark current phase as completed
        currentRoutine->phases[currentPhaseIndex].completed = true;
        
        // Check if all phases completed
        if (currentPhaseIndex >= currentRoutine->phaseCount - 1) {
            isRunning = false;
            isFinished = true;
            // Trigger audio/LED alert for routine completion
            notificationManager.playAlert(2, 25);
            drawFinishedScreen();
            return;
        } else {
            // Show phase transition
            drawPhaseTransitionScreen();
            delay(2000); // Show transition for 2 seconds
            
            // Advance to next phase
            advanceToNextPhase();
        }
    }
    
    // Update display periodically
    unsigned long currentTime = millis();
    if (currentTime - lastDisplayUpdate >= displayUpdateInterval) {
        drawRunningScreen();
        lastDisplayUpdate = currentTime;
    }
}

bool MultiTimer::isRoutineRunning() const {
    return isRunning;
}

bool MultiTimer::isRoutineFinished() const {
    return isFinished;
}

void MultiTimer::drawCurrentScreen() {
    if (isFinished) {
        drawFinishedScreen();
    } else if (isRunning) {
        drawRunningScreen();
    } else {
        drawRoutineSelectionScreen();
    }
}

void MultiTimer::drawRoutineSelectionScreen() {
    if (!display || !availableRoutines) return;
    
    display->clearDisplay();
    display->setTextSize(1);
    display->setTextColor(SSD1306_WHITE);
    
    // Title
    display->setCursor(0, 0);
    display->println("Multi-Phase Timer");
    display->println("=================");
    
    // Display available routines
    for (int i = 0; i < routineCount && i < 4; i++) {
        int y = 16 + (i * 10);
        
        if (i == selectedRoutineIndex) {
            // Highlight selected routine
            display->fillRect(0, y - 1, SCREEN_WIDTH, 10, SSD1306_WHITE);
            display->setTextColor(SSD1306_BLACK);
        } else {
            display->setTextColor(SSD1306_WHITE);
        }
        
        display->setCursor(2, y);
        display->print(availableRoutines[i].name);
        display->print(": ");
        display->print(availableRoutines[i].description);
    }
    
    // Instructions
    display->setTextColor(SSD1306_WHITE);
    display->setCursor(0, 56);
    display->print("Y-Pot: Select Button: Start");
    
    display->display();
}

void MultiTimer::drawRunningScreen() {
    if (!display || !currentRoutine) return;
    
    display->clearDisplay();
    display->setTextSize(1);
    display->setTextColor(SSD1306_WHITE);
    
    // Title and routine name
    display->setCursor(0, 0);
    display->print("Routine: ");
    display->println(currentRoutine->name);
    display->println("================");
    
    // Current phase info
    display->setCursor(0, 16);
    display->print("Phase ");
    display->print(currentPhaseIndex + 1);
    display->print("/");
    display->print(currentRoutine->phaseCount);
    display->print(": ");
    display->println(currentRoutine->phases[currentPhaseIndex].name);
    
    // Progress bar for current phase
    unsigned long phaseElapsed = currentRoutine->phases[currentPhaseIndex].duration_seconds - remainingTime;
    int progress = (currentRoutine->phases[currentPhaseIndex].duration_seconds > 0) ? 
                   (phaseElapsed * 100) / currentRoutine->phases[currentPhaseIndex].duration_seconds : 0;
    progress = constrain(progress, 0, 100);
    
    display->setCursor(0, 28);
    display->print("Progress: ");
    display->print(progress);
    display->print("%");
    
    // Progress bar visualization
    int barWidth = 120;
    int barHeight = 4;
    int barX = 4;
    int barY = 38;
    
    display->drawRect(barX, barY, barWidth, barHeight, SSD1306_WHITE);
    int fillWidth = (progress * barWidth) / 100;
    if (fillWidth > 0) {
        display->fillRect(barX, barY, fillWidth, barHeight, SSD1306_WHITE);
    }
    
    // Time display
    display->setTextSize(2);
    display->setCursor(20, 48);
    display->print(formatTime(remainingTime));
    
    display->display();
}

void MultiTimer::drawPhaseTransitionScreen() {
    if (!display || !currentRoutine) return;
    
    display->clearDisplay();
    display->setTextSize(1);
    display->setTextColor(SSD1306_WHITE);
    
    display->setCursor(0, 20);
    display->println("Phase Complete!");
    display->println();
    
    if (currentPhaseIndex < currentRoutine->phaseCount - 1) {
        display->print("Next: ");
        display->println(currentRoutine->phases[currentPhaseIndex + 1].name);
    } else {
        display->println("All phases complete!");
    }
    
    display->display();
}

void MultiTimer::drawFinishedScreen() {
    if (!display) return;
    
    // Flash effect
    static bool flashState = false;
    static unsigned long lastFlash = 0;
    unsigned long currentTime = millis();
    
    if (currentTime - lastFlash > 500) { // Flash every 500ms
        flashState = !flashState;
        lastFlash = currentTime;
    }
    
    display->clearDisplay();
    
    if (flashState) {
        display->setTextSize(2);
        display->setTextColor(SSD1306_WHITE);
        display->setCursor(10, 15);
        display->println("ROUTINE");
        display->setCursor(20, 30);
        display->println("COMPLETE!");
        
        display->setTextSize(1);
        display->setCursor(20, 50);
        display->println("Press button to return");
    }
    
    display->display();
}

String MultiTimer::formatTime(unsigned long seconds) {
    unsigned long minutes = seconds / 60;
    seconds = seconds % 60;
    
    char timeStr[10];
    sprintf(timeStr, "%02lu:%02lu", minutes, seconds);
    return String(timeStr);
}

void MultiTimer::updateRemainingTime() {
    if (!isRunning || !currentRoutine) return;
    
    unsigned long currentTime = millis();
    unsigned long phaseElapsed = (currentTime - currentPhaseStartTime) / 1000; // Convert to seconds
    
    if (phaseElapsed >= currentRoutine->phases[currentPhaseIndex].duration_seconds) {
        remainingTime = 0;
    } else {
        remainingTime = currentRoutine->phases[currentPhaseIndex].duration_seconds - phaseElapsed;
    }
}

void MultiTimer::advanceToNextPhase() {
    if (!currentRoutine || currentPhaseIndex >= currentRoutine->phaseCount - 1) return;
    
    currentPhaseIndex++;
    currentPhaseStartTime = millis();
    remainingTime = currentRoutine->phases[currentPhaseIndex].duration_seconds;
    currentRoutine->phases[currentPhaseIndex].completed = false;
}

void MultiTimer::reset() {
    isRunning = false;
    isFinished = false;
    startTime = 0;
    currentPhaseStartTime = 0;
    currentPhaseIndex = 0;
    remainingTime = 0;
    currentRoutine = nullptr;
    selectedRoutineIndex = 0;
    
    // Reset all phases
    for (int i = 0; i < routineCount; i++) {
        for (int j = 0; j < availableRoutines[i].phaseCount; j++) {
            availableRoutines[i].phases[j].completed = false;
        }
    }
}

void MultiTimer::pause() {
    if (isRunning) {
        isRunning = false;
    }
}

void MultiTimer::resume() {
    if (!isRunning && !isFinished && currentRoutine) {
        isRunning = true;
        currentPhaseStartTime = millis() - ((currentRoutine->phases[currentPhaseIndex].duration_seconds - remainingTime) * 1000);
    }
}

unsigned long MultiTimer::getRemainingTime() const {
    return remainingTime;
}

int MultiTimer::getCurrentPhaseIndex() const {
    return currentPhaseIndex;
}

String MultiTimer::getCurrentPhaseName() const {
    if (currentRoutine && currentPhaseIndex < currentRoutine->phaseCount) {
        return currentRoutine->phases[currentPhaseIndex].name;
    }
    return "";
}

int MultiTimer::getSelectedRoutineIndex() const {
    return selectedRoutineIndex;
}

TimerRoutine* MultiTimer::getCurrentRoutine() const {
    return currentRoutine;
}
