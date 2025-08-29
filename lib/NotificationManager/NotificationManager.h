#ifndef NOTIFICATIONMANAGER_H
#define NOTIFICATIONMANAGER_H

#include <Arduino.h>
#include "DFRobotDFPlayerMini.h"

class NotificationManager {
private:
    DFRobotDFPlayerMini dfPlayer;
    bool isAlertActive;
    unsigned long lastFlashTime;
    bool ledState;
    int currentVolume;

public:
    NotificationManager();
    void begin();
    void playAlert(int trackNumber = 1);
    // overloaded variant to specify volume for urgent alarms
    void playAlert(int trackNumber, int vol);
    void stopAlert();
    void update(); // Call this in the main loop to handle LED flashing
    // Play a short UI advert/feedback sound which interrupts the current track
    void playAdvert(uint16_t trackNumber);
    // Query whether an alert is currently active (playing)
    bool isAlerting() const;
    // Volume control for DFPlayer (0-30)
    void setVolume(int vol);
    int getVolume() const;
};

#endif
