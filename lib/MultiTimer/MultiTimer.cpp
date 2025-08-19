#include "MultiTimer.h"
#include "configs.h"
#include "NotificationManager.h"
#include "PushNotifier.h"

extern NotificationManager notificationManager;
extern PushNotifier pushNotifier;

MultiTimer::MultiTimer(Adafruit_SSD1306* displayInstance) 
    : display(displayInstance), isRunning(false), isFinished(false), currentPhaseStartTime(0),
      currentPhaseIndex(0), remainingTime(0), selectedTimerIndex(0), currentTimer(nullptr), lastDisplayUpdate(0) {}

void MultiTimer::setTimers(const std::vector<CustomTimer>& list) {
    timers = list;
}

const std::vector<CustomTimer>& MultiTimer::getTimers() const {
    return timers;
}

void MultiTimer::startTimerSelection() {
    selectedTimerIndex = 0;
    isRunning = false;
    isFinished = false;
    currentTimer = nullptr;
    drawTimerSelectionScreen();
}

void MultiTimer::handleTimerSelectionInput() {
    if (can_move()) {
        int y_move = get_y_movement();
        if (y_move != 0 && !timers.empty()) {
            if (y_move == -1) {
                selectedTimerIndex = (selectedTimerIndex > 0) ? selectedTimerIndex - 1 : (int)timers.size() - 1;
            } else if (y_move == 1) {
                selectedTimerIndex = (selectedTimerIndex < (int)timers.size() - 1) ? selectedTimerIndex + 1 : 0;
            }
            drawTimerSelectionScreen();
        }
    }
}

bool MultiTimer::isTimerSelected() const {
    return !timers.empty() && selectedTimerIndex >= 0 && selectedTimerIndex < (int)timers.size();
}

int MultiTimer::getSelectedTimerIndex() const { return selectedTimerIndex; }
void MultiTimer::setSelectedTimerIndex(int index) { if (index >= 0 && index < (int)timers.size()) selectedTimerIndex = index; }

void MultiTimer::startTimer() {
    if (isTimerSelected()) {
        currentTimer = &timers[selectedTimerIndex];
        if (currentTimer->phases.empty()) return;
        currentPhaseIndex = 0;
        currentPhaseStartTime = millis();
        isRunning = true;
        isFinished = false;
        remainingTime = currentTimer->phases[0].duration_seconds;
        lastDisplayUpdate = millis();
        drawRunningScreen();
    }
}

void MultiTimer::updateRoutine() {
    if (!isRunning || !currentTimer) return;

    updateRemainingTime();

    if (remainingTime <= 0) {
        // End-of-phase alerts
        const TimerPhase& phase = currentTimer->phases[currentPhaseIndex];
        if (phase.sound_track > 0) {
            notificationManager.playAlert(phase.sound_track, 25);
        }
        if (!phase.alertzy_key_indices.empty()) {
            pushNotifier.sendNotification("Chrono-Cubo", String("Phase complete: ") + phase.name, phase.alertzy_key_indices);
        }

        // Proceed to next phase or finish
        if (currentPhaseIndex >= (int)currentTimer->phases.size() - 1) {
            isRunning = false;
            isFinished = true;
            drawFinishedScreen();
            return;
        } else {
            drawPhaseTransitionScreen();
            delay(2000);
            advanceToNextPhase();
        }
    }

    unsigned long currentTime = millis();
    if (currentTime - lastDisplayUpdate >= displayUpdateInterval) {
        drawRunningScreen();
        lastDisplayUpdate = currentTime;
    }
}

bool MultiTimer::isRoutineRunning() const { return isRunning; }
bool MultiTimer::isRoutineFinished() const { return isFinished; }

void MultiTimer::drawCurrentScreen() {
    if (isFinished) {
        drawFinishedScreen();
    } else if (isRunning) {
        drawRunningScreen();
    } else {
        drawTimerSelectionScreen();
    }
}

void MultiTimer::drawTimerSelectionScreen() {
    if (!display) return;
    display->clearDisplay();
    display->setTextSize(1);
    display->setTextColor(SSD1306_WHITE);
    display->setCursor(0, 0);
    display->println("Custom Timers");
    display->println("=============");

    if (timers.empty()) {
        display->setCursor(0, 20);
        display->println("No timers saved.");
        display->println("Use Settings -> Timers");
        display->println("to create one.");
    } else {
        int maxItems = min(4, (int)timers.size());
        for (int i = 0; i < maxItems; ++i) {
            int idx = i; // top N only for 128x64
            int y = 16 + i * 10;
            if (idx == selectedTimerIndex) {
                display->fillRect(0, y - 1, SCREEN_WIDTH, 10, SSD1306_WHITE);
                display->setTextColor(SSD1306_BLACK);
            } else {
                display->setTextColor(SSD1306_WHITE);
            }
            display->setCursor(2, y);
            display->print(timers[idx].name);
        }
        display->setTextColor(SSD1306_WHITE);
        display->setCursor(0, 56);
        display->print("Y:Select Btn:Start");
    }
    display->display();
}

void MultiTimer::drawRunningScreen() {
    if (!display || !currentTimer) return;
    display->clearDisplay();
    display->setTextSize(1);
    display->setTextColor(SSD1306_WHITE);
    display->setCursor(0, 0);
    display->print("Timer: ");
    display->println(currentTimer->name);
    display->println("=============");

    const TimerPhase& phase = currentTimer->phases[currentPhaseIndex];
    display->setCursor(0, 16);
    display->print("Phase ");
    display->print(currentPhaseIndex + 1);
    display->print("/");
    display->print((int)currentTimer->phases.size());
    display->print(": ");
    display->println(phase.name);

    unsigned long phaseElapsed = phase.duration_seconds - remainingTime;
    int progress = (phase.duration_seconds > 0) ? (phaseElapsed * 100) / phase.duration_seconds : 0;
    progress = constrain(progress, 0, 100);
    display->setCursor(0, 28);
    display->print("Progress: ");
    display->print(progress);
    display->print("%");

    int barWidth = 120;
    int barHeight = 4;
    int barX = 4;
    int barY = 38;
    display->drawRect(barX, barY, barWidth, barHeight, SSD1306_WHITE);
    int fillWidth = (progress * barWidth) / 100;
    if (fillWidth > 0) display->fillRect(barX, barY, fillWidth, barHeight, SSD1306_WHITE);

    display->setTextSize(2);
    display->setCursor(20, 48);
    display->print(formatTime(remainingTime));
    display->display();
}

void MultiTimer::drawPhaseTransitionScreen() {
    if (!display || !currentTimer) return;
    display->clearDisplay();
    display->setTextSize(1);
    display->setTextColor(SSD1306_WHITE);
    display->setCursor(0, 20);
    display->println("Phase Complete!");
    if (currentPhaseIndex < (int)currentTimer->phases.size() - 1) {
        display->print("Next: ");
        display->println(currentTimer->phases[currentPhaseIndex + 1].name);
    } else {
        display->println("All phases complete!");
    }
    display->display();
}

void MultiTimer::drawFinishedScreen() {
    if (!display) return;
    static bool flashState = false;
    static unsigned long lastFlash = 0;
    unsigned long currentTime = millis();
    if (currentTime - lastFlash > 500) { flashState = !flashState; lastFlash = currentTime; }
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
    if (!isRunning || !currentTimer) return;
    unsigned long currentTime = millis();
    unsigned long phaseElapsed = (currentTime - currentPhaseStartTime) / 1000;
    const auto& phase = currentTimer->phases[currentPhaseIndex];
    remainingTime = (phaseElapsed >= phase.duration_seconds) ? 0 : (phase.duration_seconds - phaseElapsed);
}

void MultiTimer::advanceToNextPhase() {
    if (!currentTimer || currentPhaseIndex >= (int)currentTimer->phases.size() - 1) return;
    currentPhaseIndex++;
    currentPhaseStartTime = millis();
    remainingTime = currentTimer->phases[currentPhaseIndex].duration_seconds;
}

void MultiTimer::reset() {
    isRunning = false;
    isFinished = false;
    currentPhaseStartTime = 0;
    currentPhaseIndex = 0;
    remainingTime = 0;
    currentTimer = nullptr;
    selectedTimerIndex = 0;
}

void MultiTimer::pause() { if (isRunning) isRunning = false; }
void MultiTimer::resume() {
    if (!isRunning && !isFinished && currentTimer) {
        const auto& phase = currentTimer->phases[currentPhaseIndex];
        currentPhaseStartTime = millis() - ((phase.duration_seconds - remainingTime) * 1000);
        isRunning = true;
    }
}

unsigned long MultiTimer::getRemainingTime() const { return remainingTime; }
int MultiTimer::getCurrentPhaseIndex() const { return currentPhaseIndex; }
String MultiTimer::getCurrentPhaseName() const {
    if (currentTimer && currentPhaseIndex < (int)currentTimer->phases.size()) return currentTimer->phases[currentPhaseIndex].name;
    return "";
}
