#ifndef PUSHNOTIFIER_H
#define PUSHNOTIFIER_H

#include <Arduino.h>
#include <Preferences.h>

class PushNotifier {
private:
    String accountKey;
    Preferences* preferences;
    String urlEncode(String str);

public:
    PushNotifier(Preferences* prefs);
    void begin(); // Load key from preferences
    bool hasAccountKey();
    void setAccountKey(String key);
    void sendNotification(String title, String message);
};

#endif
