#ifndef TIMEMANAGER_H
#define TIMEMANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <RTClib.h>
#include <time.h>
#include <Adafruit_SSD1306.h>

class TimeManager {
private:
    RTC_DS3231* rtc;
    Adafruit_SSD1306* display;
    
    // NTP Configuration
    const char* ntpServer = "pool.ntp.org";
    const long gmtOffset_sec = 0;  // Will be set based on timezone
    const int daylightOffset_sec = 3600;
    
    // Time synchronization status
    bool timeSynced;
    unsigned long lastSyncTime;
    const unsigned long syncInterval = 24 * 60 * 60 * 1000; // 24 hours
    
    // Internal methods
    bool syncWithNTP();
    void setRTCTime(const DateTime& ntpTime);
    String formatTime(const DateTime& time);
    String formatDate(const DateTime& time);
    
public:
    // Constructor
    TimeManager(RTC_DS3231* rtcInstance, Adafruit_SSD1306* displayInstance);
    
    // Initialization and synchronization
    bool initialize();
    bool syncTime();
    bool isTimeSynced() const;
    
    // Time retrieval
    DateTime getCurrentTime();
    String getCurrentTimeString();
    String getCurrentDateString();
    
    // Display methods
    void displayCurrentTime();
    void displayTimeSyncStatus();
    
    // Utility methods
    unsigned long getUnixTimestamp();
    bool isAlarmTime(const DateTime& alarmTime);
    int getDayOfWeek();
    
    // Configuration
    void setTimezone(int gmtOffsetHours, bool daylightSaving = true);
};

#endif // TIMEMANAGER_H
