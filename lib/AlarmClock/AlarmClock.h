#ifndef ALARMCLOCK_H
#define ALARMCLOCK_H

#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <RTClib.h>
#include "KeyInput.h"
#include "PushNotifier.h"

class AlarmClock {
private:
    Adafruit_SSD1306* display;
    RTC_DS3231* rtc;
    PushNotifier* pushNotifier;
    
    // Alarm state
    bool alarmSet;
    bool alarmTriggered;
    bool alarmEnabled;
    int alarmHour;
    int alarmMinute;
    int alarmSoundTrack;
    unsigned long lastAlarmCheck;
    unsigned long alarmTriggerTime;
    
    // Setup state
    int setupState; // 0 for hour, 1 for minute, 2 for sound
    
    // Display variables
    unsigned long lastDisplayUpdate;
    const unsigned long displayUpdateInterval = 1000; // Update display every 1 second
    
    // Internal methods
    void drawSetupScreen();
    void drawAlarmStatusScreen();
    void drawAlarmTriggeredScreen();
    String formatTime(int hour, int minute) const;
    void checkAlarmTime();
    
public:
    // Constructor
    AlarmClock(Adafruit_SSD1306* displayInstance, RTC_DS3231* rtcInstance, PushNotifier* notifier);
    
    // Setup methods
    void startSetup();
    void handleSetupInput();
    bool isSetupComplete() const;
    
    // Alarm management
    void setAlarm(int hour, int minute);
    void enableAlarm(bool enable = true);
    void disableAlarm();
    bool isAlarmSet() const;
    bool isAlarmEnabled() const;
    bool isAlarmTriggered() const;
    
    // Running methods
    void updateAlarm();
    void acknowledgeAlarm();
    
    // Display methods
    void drawCurrentScreen();
    
    // Control methods
    void reset();
    
    // Getters
    int getAlarmHour() const;
    int getAlarmMinute() const;
    String getAlarmTimeString() const;
};

#endif // ALARMCLOCK_H
