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

public:
    NotificationManager();
    void begin();
    void playAlert(int trackNumber = 1, int volume = 20);
    void stopAlert();
    void update(); // Call this in the main loop to handle LED flashing
};

#endif
