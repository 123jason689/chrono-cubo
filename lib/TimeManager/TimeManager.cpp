#include "TimeManager.h"
#include "configs.h"

TimeManager::TimeManager(RTC_DS3231* rtcInstance, Adafruit_SSD1306* displayInstance) 
    : rtc(rtcInstance), display(displayInstance), timeSynced(false), lastSyncTime(0) {
}

bool TimeManager::initialize() {
    // Initialize RTC
    if (!rtc->begin()) {
        Serial.println("Couldn't find RTC");
        return false;
    }
    
    // Check if RTC lost power and set time if needed
    if (rtc->lostPower()) {
        Serial.println("RTC lost power, setting time...");
        // Set RTC to the date & time this sketch was compiled
        rtc->adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
    
    // Configure NTP
    configTime(0, 0, ntpServer);
    setTimezone(0); // Default to GMT, can be changed later
    
    return true;
}

bool TimeManager::syncTime() {
    if (!WiFi.isConnected()) {
        Serial.println("WiFi not connected, cannot sync time");
        return false;
    }
    
    // Check if we need to sync (every 24 hours)
    if (timeSynced && (millis() - lastSyncTime < syncInterval)) {
        return true; // Already synced recently
    }
    
    Serial.println("Syncing time with NTP server...");
    
    // Try to sync with NTP
    if (syncWithNTP()) {
        timeSynced = true;
        lastSyncTime = millis();
        Serial.println("Time synchronized successfully");
        return true;
    }
    
    Serial.println("Failed to sync time with NTP");
    return false;
}

bool TimeManager::syncWithNTP() {
    // Wait for NTP time to be set
    time_t now = 0;
    int attempts = 0;
    const int maxAttempts = 10;
    
    while (now < 24 * 3600 && attempts < maxAttempts) {
        Serial.print("Attempting NTP sync... ");
        now = time(nullptr);
        attempts++;
        delay(1000);
    }
    
    if (now < 24 * 3600) {
        return false; // Failed to get valid time
    }
    
    // Convert to DateTime and set RTC
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
        return false;
    }
    
    DateTime ntpTime(
        timeinfo.tm_year + 1900,
        timeinfo.tm_mon + 1,
        timeinfo.tm_mday,
        timeinfo.tm_hour,
        timeinfo.tm_min,
        timeinfo.tm_sec
    );
    
    setRTCTime(ntpTime);
    return true;
}

void TimeManager::setRTCTime(const DateTime& ntpTime) {
    rtc->adjust(ntpTime);
    Serial.print("RTC set to: ");
    Serial.println(formatTime(ntpTime));
}

DateTime TimeManager::getCurrentTime() {
    return rtc->now();
}

String TimeManager::getCurrentTimeString() {
    DateTime now = getCurrentTime();
    return formatTime(now);
}

String TimeManager::getCurrentDateString() {
    DateTime now = getCurrentTime();
    return formatDate(now);
}

String TimeManager::formatTime(const DateTime& time) {
    char timeStr[9];
    sprintf(timeStr, "%02d:%02d:%02d", time.hour(), time.minute(), time.second());
    return String(timeStr);
}

String TimeManager::formatDate(const DateTime& time) {
    char dateStr[11];
    sprintf(dateStr, "%04d-%02d-%02d", time.year(), time.month(), time.day());
    return String(dateStr);
}

void TimeManager::displayCurrentTime() {
    if (!display) return;
    
    DateTime now = getCurrentTime();
    
    display->clearDisplay();
    display->setTextSize(2);
    display->setTextColor(SSD1306_WHITE);
    display->setCursor(0, 0);
    
    // Display time
    display->println(formatTime(now));
    
    // Display date
    display->setTextSize(1);
    display->setCursor(0, 30);
    display->println(formatDate(now));
    
    // Display day of week
    const char* days[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    display->setCursor(0, 45);
    display->print(days[now.dayOfTheWeek()]);
    
    // Display sync status
    display->setCursor(80, 45);
    if (timeSynced) {
        display->print("SYNC");
    } else {
        display->print("NO SYNC");
    }
    
    display->display();
}

void TimeManager::displayTimeSyncStatus() {
    if (!display) return;
    
    display->clearDisplay();
    display->setTextSize(1);
    display->setTextColor(SSD1306_WHITE);
    display->setCursor(0, 0);
    display->println("Time Sync Status");
    display->println();
    
    if (timeSynced) {
        display->println("Time: Synced");
        display->print("Last sync: ");
        display->println(lastSyncTime / 1000);
    } else {
        display->println("Time: Not synced");
    }
    
    display->println();
    display->print("Current: ");
    display->println(getCurrentTimeString());
    
    display->display();
}

bool TimeManager::isTimeSynced() const {
    return timeSynced;
}

unsigned long TimeManager::getUnixTimestamp() {
    DateTime now = getCurrentTime();
    return now.unixtime();
}

bool TimeManager::isAlarmTime(const DateTime& alarmTime) {
    DateTime now = getCurrentTime();
    return (now.hour() == alarmTime.hour() && 
            now.minute() == alarmTime.minute() && 
            now.second() == 0);
}

int TimeManager::getDayOfWeek() {
    DateTime now = getCurrentTime();
    return now.dayOfTheWeek();
}

void TimeManager::setTimezone(int gmtOffsetHours, bool daylightSaving) {
    long gmtOffset = gmtOffsetHours * 3600;
    int daylightOffset = daylightSaving ? 3600 : 0;
    configTime(gmtOffset, daylightOffset, ntpServer);
}
