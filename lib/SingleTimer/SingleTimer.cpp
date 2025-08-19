#include "SingleTimer.h"
#include "configs.h"
#include "NotificationManager.h"

// Use the global notification manager defined in main.cpp
extern NotificationManager notificationManager;

SingleTimer::SingleTimer(Adafruit_SSD1306* displayInstance) 
    : display(displayInstance), isRunning(false), isFinished(false), startTime(0), duration(0), remainingTime(0),
      setupMinutes(0), setupSeconds(0), setupSoundTrack(1), setupState(0), lastDisplayUpdate(0) {
}

void SingleTimer::startSetup() {
    setupMinutes = 0;
    setupSeconds = 0;
    setupSoundTrack = 1;
    setupState = 0;
    isRunning = false;
    isFinished = false;
    drawSetupScreen();
}

void SingleTimer::handleSetupInput() {
    // Handle Y-axis movement to cycle through minutes, seconds, sound
    if (can_move()) {
        int y_move = get_y_movement();
        if (y_move != 0) {
            if (y_move == -1) {
                // up
                setupState = (setupState == 0) ? 2 : setupState - 1;
            } else if (y_move == 1) {
                // down
                setupState = (setupState == 2) ? 0 : setupState + 1;
            }
            drawSetupScreen();
        }
    }
    
    // Handle X-axis movement to adjust values
    if (can_move()) {
        int x_move = get_x_movement();
        if (x_move != 0) {
            switch (setupState) {
                case 0: // minutes
                    if (x_move == 1) setupMinutes = (setupMinutes < 99) ? setupMinutes + 1 : 0;
                    else setupMinutes = (setupMinutes > 0) ? setupMinutes - 1 : 99;
                    break;
                case 1: // seconds
                    if (x_move == 1) setupSeconds = (setupSeconds < 59) ? setupSeconds + 1 : 0;
                    else setupSeconds = (setupSeconds > 0) ? setupSeconds - 1 : 59;
                    break;
                case 2: // sound
                    if (x_move == 1) setupSoundTrack = (setupSoundTrack < 50) ? setupSoundTrack + 1 : 1;
                    else setupSoundTrack = (setupSoundTrack > 1) ? setupSoundTrack - 1 : 50;
                    break;
            }
            drawSetupScreen();
        }
    }
}

bool SingleTimer::isSetupComplete() const {
    return (setupMinutes > 0 || setupSeconds > 0);
}

void SingleTimer::startTimer() {
    if (isSetupComplete()) {
        duration = (setupMinutes * 60) + setupSeconds;
        remainingTime = duration;
        startTime = millis();
        isRunning = true;
        isFinished = false;
        lastDisplayUpdate = millis();
        drawRunningScreen();
    }
}

void SingleTimer::updateTimer() {
    if (!isRunning) return;
    
    updateRemainingTime();
    
    // Check if timer finished
    if (remainingTime <= 0) {
        isRunning = false;
        isFinished = true;
        remainingTime = 0;
        // Trigger audio/LED alert
        notificationManager.playAlert(setupSoundTrack, 25);
        drawFinishedScreen();
        return;
    }
    
    // Update display periodically
    unsigned long currentTime = millis();
    if (currentTime - lastDisplayUpdate >= displayUpdateInterval) {
        drawRunningScreen();
        lastDisplayUpdate = currentTime;
    }
}

bool SingleTimer::isTimerRunning() const {
    return isRunning;
}

bool SingleTimer::isTimerFinished() const {
    return isFinished;
}

void SingleTimer::drawCurrentScreen() {
    if (isFinished) {
        drawFinishedScreen();
    } else if (isRunning) {
        drawRunningScreen();
    } else {
        drawSetupScreen();
    }
}

void SingleTimer::drawSetupScreen() {
    if (!display) return;
    
    display->clearDisplay();
    display->setTextSize(1);
    display->setTextColor(SSD1306_WHITE);
    
    // Title
    display->setCursor(0, 0);
    display->println("Single Timer Setup");
    display->println("=================");
    
    // Instructions
    display->setCursor(0, 16);
    display->println("Y: Select field");
    display->println("X: Adjust value");
    display->println("Btn: Start timer");
    
    // Time display
    display->setTextSize(2);
    display->setCursor(12, 36);
    char timeStr[10];
    sprintf(timeStr, "%02d:%02d", setupMinutes, setupSeconds);
    display->print(timeStr);

    // Sound display
    display->setTextSize(1);
    display->setCursor(0, 54);
    display->print("Sound: < Track ");
    display->print(setupSoundTrack);
    display->print(" >");

    // Highlight active field indicator
    display->setCursor(0, 28);
    if (setupState == 0) {
        display->print("^ Minutes");
    } else if (setupState == 1) {
        display->print("        ^ Seconds");
    } else {
        // underline handled by context near sound line (optional visual hint)
    }
    
    display->display();
}

void SingleTimer::drawRunningScreen() {
    if (!display) return;
    
    display->clearDisplay();
    display->setTextSize(1);
    display->setTextColor(SSD1306_WHITE);
    
    // Title
    display->setCursor(0, 0);
    display->println("Timer Running");
    display->println("=============");
    
    // Progress bar
    unsigned long elapsed = duration - remainingTime;
    int progress = (duration > 0) ? (elapsed * 100) / duration : 0;
    progress = constrain(progress, 0, 100);
    
    display->setCursor(0, 20);
    display->print("Progress: ");
    display->print(progress);
    display->print("%");
    
    // Progress bar visualization
    int barWidth = 120;
    int barHeight = 4;
    int barX = 4;
    int barY = 30;
    
    display->drawRect(barX, barY, barWidth, barHeight, SSD1306_WHITE);
    int fillWidth = (progress * barWidth) / 100;
    if (fillWidth > 0) {
        display->fillRect(barX, barY, fillWidth, barHeight, SSD1306_WHITE);
    }
    
    // Time display
    display->setTextSize(2);
    display->setCursor(20, 40);
    display->print(formatTime(remainingTime));
    
    // Instructions
    display->setTextSize(1);
    display->setCursor(0, 56);
    display->print("Button: Stop timer");
    
    display->display();
}

void SingleTimer::drawFinishedScreen() {
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
        display->setCursor(10, 20);
        display->println("TIME'S UP!");
        
        display->setTextSize(1);
        display->setCursor(20, 45);
        display->println("Press button to return");
    }
    
    display->display();
}

String SingleTimer::formatTime(unsigned long seconds) {
    unsigned long minutes = seconds / 60;
    seconds = seconds % 60;
    
    char timeStr[10];
    sprintf(timeStr, "%02lu:%02lu", minutes, seconds);
    return String(timeStr);
}

void SingleTimer::updateRemainingTime() {
    if (!isRunning) return;
    
    unsigned long currentTime = millis();
    unsigned long elapsed = (currentTime - startTime) / 1000; // Convert to seconds
    
    if (elapsed >= duration) {
        remainingTime = 0;
    } else {
        remainingTime = duration - elapsed;
    }
}

void SingleTimer::reset() {
    isRunning = false;
    isFinished = false;
    startTime = 0;
    duration = 0;
    remainingTime = 0;
    setupMinutes = 0;
    setupSeconds = 0;
    setupSoundTrack = 1;
    setupState = 0;
}

void SingleTimer::pause() {
    if (isRunning) {
        isRunning = false;
        // Store remaining time for resume
    }
}

void SingleTimer::resume() {
    if (!isRunning && !isFinished && remainingTime > 0) {
        isRunning = true;
        startTime = millis() - ((duration - remainingTime) * 1000);
    }
}

unsigned long SingleTimer::getRemainingTime() const {
    return remainingTime;
}

unsigned long SingleTimer::getDuration() const {
    return duration;
}

int SingleTimer::getSetupMinutes() const {
    return setupMinutes;
}

int SingleTimer::getSetupSeconds() const {
    return setupSeconds;
}
