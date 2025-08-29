#include "AlarmClock.h"
#include "StorageManager.h"
#include "configs.h"
#include "NotificationManager.h"
#include "PushNotifier.h"

extern NotificationManager notificationManager;

AlarmClock::AlarmClock(Adafruit_SSD1306* displayInstance, RTC_DS3231* rtcInstance, PushNotifier* notifier) 
    : display(displayInstance), rtc(rtcInstance), pushNotifier(notifier), lastAlarmCheck(0), alarmTriggerTime(0), currentAlarmIndex(-1), isRinging(false), lastDisplayUpdate(0) {
}

void AlarmClock::startSetup() {
    setupState = 0;
    tempHour = 7;
    tempMinute = 30;
    tempSoundTrack = 3;
    // Default no-op; use addAlarm to create alarms
    drawSetupScreen();
}

void AlarmClock::handleSetupInput() {
    // Handle Y-axis movement to cycle hour/minute/sound
    if (can_move()) {
        int y_move = get_y_movement();
        if (y_move != 0) {
            if (y_move == -1) {
                setupState = (setupState == 0) ? 2 : setupState - 1;
            } else if (y_move == 1) {
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
                case 0: // hour
                    if (x_move == 1) tempHour = (tempHour < 23) ? tempHour + 1 : 0;
                    else tempHour = (tempHour > 0) ? tempHour - 1 : 23;
                    break;
                case 1: // minute
                    if (x_move == 1) tempMinute = (tempMinute < 59) ? tempMinute + 1 : 0;
                    else tempMinute = (tempMinute > 0) ? tempMinute - 1 : 59;
                    break;
                case 2: // sound
                    if (x_move == 1) tempSoundTrack = (tempSoundTrack < 50) ? tempSoundTrack + 1 : 1;
                    else tempSoundTrack = (tempSoundTrack > 1) ? tempSoundTrack - 1 : 50;
                    break;
            }
            drawSetupScreen();
        }
    }
}

bool AlarmClock::isSetupComplete() const {
    return true; // Always complete since we have default values
}

void AlarmClock::loadAlarms(StorageManager& storage) {
    alarms = storage.loadAlarms();
}

void AlarmClock::addAlarm(const Alarm& newAlarm) {
    // Check duplicates
    for (const auto& a : alarms) {
        if (a.hour == newAlarm.hour && a.minute == newAlarm.minute) return;
    }
    alarms.push_back(newAlarm);
}

void AlarmClock::removeAlarm(int index) {
    if (index >= 0 && index < (int)alarms.size()) alarms.erase(alarms.begin() + index);
}

const std::vector<Alarm>& AlarmClock::getAlarms() const {
    return alarms;
}

void AlarmClock::enableAlarm(bool enable) {
    for (auto& a : alarms) a.enabled = enable;
}

void AlarmClock::disableAlarm() {
    for (auto& a : alarms) a.enabled = false;
    isRinging = false;
    currentAlarmIndex = -1;
}

bool AlarmClock::isAlarmSet() const {
    return !alarms.empty();
}

bool AlarmClock::isAlarmEnabled() const {
    for (const auto& a : alarms) if (a.enabled) return true;
    return false;
}

bool AlarmClock::isAlarmTriggered() const {
    return isRinging;
}

void AlarmClock::updateAlarm() {
    // Always check alarms if alarms exist
    if (alarms.empty()) return;
    unsigned long currentTime = millis();
    
    // Check alarm time every second
    if (currentTime - lastAlarmCheck >= 1000) {
        checkAlarmTime();
        lastAlarmCheck = currentTime;
    }
    
    // Update display periodically
    if (currentTime - lastDisplayUpdate >= displayUpdateInterval) {
        drawCurrentScreen();
        lastDisplayUpdate = currentTime;
    }
}

void AlarmClock::checkAlarmTime() {
    if (!rtc) return;
    
    DateTime now = rtc->now();
    
    // Iterate alarms and trigger when a matching enabled alarm is found at exact 0 seconds
    for (size_t i = 0; i < alarms.size(); ++i) {
        const Alarm& a = alarms[i];
        if (!a.enabled) continue;
        if (now.hour() == a.hour && now.minute() == a.minute && now.second() == 0) {
            // Trigger the first matching alarm
            isRinging = true;
            currentAlarmIndex = (int)i;
            alarmTriggerTime = millis();
            Serial.println("ALARM TRIGGERED (multi)!");
            // Play alarm sound (use current volume)
            notificationManager.playAlert(a.sound_track);
            // Push notification
            if (pushNotifier) pushNotifier->sendAll("Chrono-Cubo Alarm", "Time to wake up!");
            drawAlarmTriggeredScreen();
            return;
        }
    }
}

void AlarmClock::acknowledgeAlarm() {
    isRinging = false;
    currentAlarmIndex = -1;
    // Optionally disable alarm after acknowledgment
    // alarmEnabled = false;
}

void AlarmClock::drawCurrentScreen() {
    if (isRinging) {
        drawAlarmTriggeredScreen();
    } else if (!alarms.empty()) {
        drawAlarmStatusScreen();
    } else {
        drawSetupScreen();
    }
}

void AlarmClock::drawSetupScreen() {
    if (!display) return;
    
    display->clearDisplay();
    display->setTextSize(1);
    display->setTextColor(SSD1306_WHITE);
    
    // Title
    display->setCursor(0, 0);
    display->println("Sleep Alarm Setup");
    display->println("================");
    
    // Instructions
    display->setCursor(0, 16);
    display->println("Y: Select field");
    display->println("X: Adjust value");
    display->println("Btn: Set alarm");
    
    // Time display
    display->setTextSize(2);
    display->setCursor(12, 36);
    display->print(formatTime(tempHour, tempMinute));

    // Sound display
    display->setTextSize(1);
    display->setCursor(0, 54);
    display->print("Sound: < Track ");
    display->print(tempSoundTrack);
    display->print(" >");

    // Active field indicator
    display->setCursor(0, 28);
    if (setupState == 0) display->print("^ Hour");
    else if (setupState == 1) display->print("      ^ Minute");
    
    display->display();
}

void AlarmClock::drawAlarmStatusScreen() {
    if (!display || !rtc) return;
    
    DateTime now = rtc->now();
    
    display->clearDisplay();
    display->setTextSize(1);
    display->setTextColor(SSD1306_WHITE);
    
    // Title
    display->setCursor(0, 0);
    display->println("Sleep Alarm Status");
    display->println("==================");
    
    // Current time
    display->setCursor(0, 16);
    display->print("Current: ");
    char currentTimeStr[9];
    sprintf(currentTimeStr, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
    display->println(currentTimeStr);
    
    // Next alarm time (find next enabled alarm)
    display->setCursor(0, 28);
    display->print("Next Alarm: ");
    if (alarms.empty()) {
        display->print("- none -");
    } else {
        // Find next alarm chronologically
        int nowMinutes = now.hour() * 60 + now.minute();
        int bestIdx = -1; int bestMinutes = 24*60;
        for (size_t i = 0; i < alarms.size(); ++i) {
            if (!alarms[i].enabled) continue;
            int am = alarms[i].hour * 60 + alarms[i].minute;
            int delta = am - nowMinutes;
            if (delta < 0) delta += 24*60;
            if (bestIdx == -1 || delta < bestMinutes) { bestMinutes = delta; bestIdx = (int)i; }
        }
        if (bestIdx == -1) {
            display->print("- none -");
        } else {
            display->print(formatTime(alarms[bestIdx].hour, alarms[bestIdx].minute));
        }
    }
    
    // Status
    display->setCursor(0, 40);
    if (isAlarmEnabled()) {
        display->print("Status: ENABLED");
    } else {
        display->print("Status: DISABLED");
    }
    
    // Time until alarm
    // Time until next alarm
    if (alarms.empty()) {
        display->setCursor(0, 52);
        display->print("No alarms set");
    } else {
        int nowMinutes = now.hour() * 60 + now.minute();
        int bestIdx = -1; int bestDelta = 24*60;
        for (size_t i = 0; i < alarms.size(); ++i) {
            if (!alarms[i].enabled) continue;
            int am = alarms[i].hour * 60 + alarms[i].minute;
            int delta = am - nowMinutes;
            if (delta < 0) delta += 24*60;
            if (bestIdx == -1 || delta < bestDelta) { bestDelta = delta; bestIdx = (int)i; }
        }
        if (bestIdx == -1) {
            display->setCursor(0, 52);
            display->print("No enabled alarms");
        } else {
            display->setCursor(0, 52);
            display->print("Time until: ");
            display->print(bestDelta / 60);
            display->print("h ");
            display->print(bestDelta % 60);
            display->print("m");
        }
    }
    
    display->display();
}

void AlarmClock::drawAlarmTriggeredScreen() {
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
        display->println("WAKE UP!");
        display->println();
        
        display->setTextSize(1);
        display->setCursor(20, 40);
        display->print("Alarm: ");
        if (currentAlarmIndex >= 0 && currentAlarmIndex < (int)alarms.size()) {
            display->println(formatTime(alarms[currentAlarmIndex].hour, alarms[currentAlarmIndex].minute));
        } else {
            display->println(formatTime(tempHour, tempMinute));
        }
        
        display->setCursor(20, 50);
        display->println("Press button to stop");
    }
    
    display->display();
}

String AlarmClock::formatTime(int hour, int minute) const {
    char timeStr[6];
    sprintf(timeStr, "%02d:%02d", hour, minute);
    return String(timeStr);
}

void AlarmClock::reset() {
    alarms.clear();
    isRinging = false;
    currentAlarmIndex = -1;
    setupState = 0;
    lastAlarmCheck = 0;
    alarmTriggerTime = 0;
}

int AlarmClock::getAlarmHour() const {
    if (!alarms.empty()) return alarms[0].hour;
    return tempHour;
}

int AlarmClock::getAlarmMinute() const {
    if (!alarms.empty()) return alarms[0].minute;
    return tempMinute;
}

String AlarmClock::getAlarmTimeString() const {
    if (!alarms.empty()) return formatTime(alarms[0].hour, alarms[0].minute);
    return formatTime(tempHour, tempMinute);
}

int AlarmClock::getAlarmSoundTrack() const {
    if (currentAlarmIndex >= 0 && currentAlarmIndex < (int)alarms.size()) return alarms[currentAlarmIndex].sound_track;
    // Fallback to first alarm or temp
    if (!alarms.empty()) return alarms[0].sound_track;
    return tempSoundTrack;
}

int AlarmClock::getSetupHour() const { return tempHour; }
int AlarmClock::getSetupMinute() const { return tempMinute; }
int AlarmClock::getSetupSoundTrack() const { return tempSoundTrack; }
