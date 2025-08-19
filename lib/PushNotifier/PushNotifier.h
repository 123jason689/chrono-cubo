#ifndef PUSHNOTIFIER_H
#define PUSHNOTIFIER_H

#include <Arduino.h>
#include <Preferences.h>
#include <vector>
#include "DataModels.h"

class PushNotifier {
private:
    Preferences* preferences;
    std::vector<AlertzyAccount> accounts;
    String urlEncode(String str);

public:
    PushNotifier(Preferences* prefs);
    void begin();

    // Accounts management (data provided by StorageManager)
    void setAccounts(const std::vector<AlertzyAccount>& list);
    const std::vector<AlertzyAccount>& getAccounts() const;

    // Send notifications to selected accounts by indices
    void sendNotification(String title, String message, const std::vector<uint8_t>& key_indices);
    // Convenience: send to all accounts
    void sendAll(String title, String message);
};

#endif
