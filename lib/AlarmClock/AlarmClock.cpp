#include "AlarmClock.h"
#include "configs.h"
#include "NotificationManager.h"
#include "PushNotifier.h"

extern NotificationManager notificationManager;

AlarmClock::AlarmClock(Adafruit_SSD1306* displayInstance, RTC_DS3231* rtcInstance, PushNotifier* notifier) 
    : display(displayInstance), rtc(rtcInstance), pushNotifier(notifier), alarmSet(false), alarmTriggered(false), alarmEnabled(false),
      alarmHour(7), alarmMinute(30), alarmSoundTrack(3), lastAlarmCheck(0), alarmTriggerTime(0), setupState(0), lastDisplayUpdate(0) {
}

void AlarmClock::startSetup() {
    alarmHour = 7;
    alarmMinute = 30;
    alarmSoundTrack = 3;
    setupState = 0;
    alarmSet = false;
    alarmTriggered = false;
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
                    if (x_move == 1) alarmHour = (alarmHour < 23) ? alarmHour + 1 : 0;
                    else alarmHour = (alarmHour > 0) ? alarmHour - 1 : 23;
                    break;
                case 1: // minute
                    if (x_move == 1) alarmMinute = (alarmMinute < 59) ? alarmMinute + 1 : 0;
                    else alarmMinute = (alarmMinute > 0) ? alarmMinute - 1 : 59;
                    break;
                case 2: // sound
                    if (x_move == 1) alarmSoundTrack = (alarmSoundTrack < 50) ? alarmSoundTrack + 1 : 1;
                    else alarmSoundTrack = (alarmSoundTrack > 1) ? alarmSoundTrack - 1 : 50;
                    break;
            }
            drawSetupScreen();
        }
    }
}

bool AlarmClock::isSetupComplete() const {
    return true; // Always complete since we have default values
}

void AlarmClock::setAlarm(int hour, int minute) {
    alarmHour = hour;
    alarmMinute = minute;
    alarmSet = true;
    alarmEnabled = true;
    alarmTriggered = false;
    // Sound track is taken from current setup selection
    Serial.print("Alarm set for: ");
    Serial.print(hour);
    Serial.print(":");
    Serial.println(minute);
}

void AlarmClock::enableAlarm(bool enable) {
    alarmEnabled = enable;
    if (!enable) {
        alarmTriggered = false;
    }
}

void AlarmClock::disableAlarm() {
    alarmEnabled = false;
    alarmTriggered = false;
}

bool AlarmClock::isAlarmSet() const {
    return alarmSet;
}

bool AlarmClock::isAlarmEnabled() const {
    return alarmEnabled && alarmSet;
}

bool AlarmClock::isAlarmTriggered() const {
    return alarmTriggered;
}

void AlarmClock::updateAlarm() {
    if (!alarmEnabled || !alarmSet) return;
    
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
    
    // Check if current time matches alarm time (within 1 second)
    if (now.hour() == alarmHour && now.minute() == alarmMinute && now.second() == 0) {
        if (!alarmTriggered) {
            alarmTriggered = true;
            alarmTriggerTime = millis();
            Serial.println("ALARM TRIGGERED!");
            // Start audio/LED alert
            notificationManager.playAlert(alarmSoundTrack, 25);
            // Push notification
            if (pushNotifier) {
                pushNotifier->sendAll("Chrono-Cubo Alarm", "Time to wake up!");
            }
            drawAlarmTriggeredScreen();
        }
    }
}

void AlarmClock::acknowledgeAlarm() {
    alarmTriggered = false;
    // Optionally disable alarm after acknowledgment
    // alarmEnabled = false;
}

void AlarmClock::drawCurrentScreen() {
    if (alarmTriggered) {
        drawAlarmTriggeredScreen();
    } else if (alarmSet) {
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
    display->print(formatTime(alarmHour, alarmMinute));

    // Sound display
    display->setTextSize(1);
    display->setCursor(0, 54);
    display->print("Sound: < Track ");
    display->print(alarmSoundTrack);
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
    
    // Alarm time
    display->setCursor(0, 28);
    display->print("Alarm: ");
    display->print(formatTime(alarmHour, alarmMinute));
    
    // Status
    display->setCursor(0, 40);
    if (alarmEnabled) {
        display->print("Status: ENABLED");
    } else {
        display->print("Status: DISABLED");
    }
    
    // Time until alarm
    int currentMinutes = now.hour() * 60 + now.minute();
    int alarmMinutes = alarmHour * 60 + alarmMinute;
    int minutesUntilAlarm = alarmMinutes - currentMinutes;
    
    if (minutesUntilAlarm < 0) {
        minutesUntilAlarm += 24 * 60; // Next day
    }
    
    display->setCursor(0, 52);
    display->print("Time until: ");
    display->print(minutesUntilAlarm / 60);
    display->print("h ");
    display->print(minutesUntilAlarm % 60);
    display->print("m");
    
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
        display->println(formatTime(alarmHour, alarmMinute));
        
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
    alarmSet = false;
    alarmTriggered = false;
    alarmEnabled = false;
    alarmHour = 7;
    alarmMinute = 30;
    alarmSoundTrack = 3;
    setupState = 0;
    lastAlarmCheck = 0;
    alarmTriggerTime = 0;
}

int AlarmClock::getAlarmHour() const {
    return alarmHour;
}

int AlarmClock::getAlarmMinute() const {
    return alarmMinute;
}

String AlarmClock::getAlarmTimeString() const {
    return formatTime(alarmHour, alarmMinute);
}
