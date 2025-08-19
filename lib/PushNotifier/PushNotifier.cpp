#include "PushNotifier.h"
#include <WiFi.h>
#include <HTTPClient.h>

PushNotifier::PushNotifier(Preferences* prefs) : preferences(prefs) {}

void PushNotifier::begin() {
    if (!preferences) return;
    preferences->begin("notifier", true); // read-only
    accountKey = preferences->getString("alertzyKey", "");
    preferences->end();
}

bool PushNotifier::hasAccountKey() {
    return accountKey.length() > 0;
}

void PushNotifier::setAccountKey(String key) {
    accountKey = key;
    if (!preferences) return;
    preferences->begin("notifier", false);
    preferences->putString("alertzyKey", accountKey);
    preferences->end();
}

String PushNotifier::urlEncode(String str) {
    String encoded = "";
    char c;
    char code0;
    char code1;
    for (size_t i = 0; i < str.length(); i++) {
        c = str.charAt(i);
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            encoded += c;
        } else if (c == ' ') {
            encoded += '+';
        } else {
            code0 = (c >> 4) & 0xF;
            code1 = c & 0xF;
            encoded += '%';
            encoded += (code0 > 9) ? (code0 - 10) + 'A' : code0 + '0';
            encoded += (code1 > 9) ? (code1 - 10) + 'A' : code1 + '0';
        }
    }
    return encoded;
}

void PushNotifier::sendNotification(String title, String message) {
    if (WiFi.status() != WL_CONNECTED) return;
    if (!hasAccountKey()) return;

    HTTPClient http;
    const char* endpoint = "https://alertzy.app/send";
    if (!http.begin(endpoint)) {
        return;
    }

    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String body = String("accountKey=") + accountKey +
                  "&title=" + urlEncode(title) +
                  "&message=" + urlEncode(message);

    int httpCode = http.POST(body);
    // Optionally log httpCode for debugging
    (void)httpCode;
    http.end();
}
