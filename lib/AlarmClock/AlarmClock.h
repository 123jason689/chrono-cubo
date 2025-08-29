#ifndef ALARMCLOCK_H
#define ALARMCLOCK_H

#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <RTClib.h>
#include "KeyInput.h"
#include "PushNotifier.h"
#include "DataModels.h"

// Forward declaration
class StorageManager;

class AlarmClock {
private:
    Adafruit_SSD1306* display;
    RTC_DS3231* rtc;
    PushNotifier* pushNotifier;
    
    // Alarms list
    std::vector<Alarm> alarms;
    unsigned long lastAlarmCheck;
    unsigned long alarmTriggerTime;
    int currentAlarmIndex;
    
    // Setup state
    int setupState; // 0 for hour, 1 for minute, 2 for sound
    // Temporary setup values when creating a new alarm
    int tempHour;
    int tempMinute;
    int tempSoundTrack;
    
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
    // Load/save
    void loadAlarms(StorageManager& storage);
    void addAlarm(const Alarm& newAlarm);
    void removeAlarm(int index);
    const std::vector<Alarm>& getAlarms() const;

    // Ringing status
    bool isRinging;
    // Legacy compatibility methods
    void enableAlarm(bool enable = true);
    void disableAlarm();
    bool isAlarmSet() const;
    bool isAlarmEnabled() const;
    bool isAlarmTriggered() const;
    
    // Running methods
    void updateAlarm();
    void acknowledgeAlarm();

    // Alarm sound info for currently ringing alarm
    int getAlarmSoundTrack() const;
    
    // Display methods
    void drawCurrentScreen();
    
    // Control methods
    void reset();
    
    // Getters
    // Return primary alarm info (first alarm) or setup temp if none
    int getAlarmHour() const;
    int getAlarmMinute() const;
    String getAlarmTimeString() const;

    // Accessors for setup temporary values
    int getSetupHour() const;
    int getSetupMinute() const;
    int getSetupSoundTrack() const;
};

#endif // ALARMCLOCK_H
