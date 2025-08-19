#include "PushNotifier.h"
#include <WiFi.h>
#include <HTTPClient.h>

PushNotifier::PushNotifier(Preferences* prefs) : preferences(prefs) {}

void PushNotifier::begin() {
    // No-op here; accounts are injected via setAccounts from StorageManager
}

void PushNotifier::setAccounts(const std::vector<AlertzyAccount>& list) {
    accounts = list;
}

const std::vector<AlertzyAccount>& PushNotifier::getAccounts() const {
    return accounts;
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

void PushNotifier::sendNotification(String title, String message, const std::vector<uint8_t>& key_indices) {
    if (WiFi.status() != WL_CONNECTED) return;
    if (accounts.empty()) return;

    // Build a single underscore-concatenated accountKey string per Alertzy docs
    String combinedKeys = "";
    bool first = true;
    for (uint8_t idx : key_indices) {
        if (idx >= accounts.size()) continue;
        const String& key = accounts[idx].key;
        if (key.length() == 0) continue;
        if (!first) combinedKeys += "_";
        combinedKeys += key;
        first = false;
    }
    if (combinedKeys.length() == 0) return;

    HTTPClient http;
    const char* endpoint = "https://alertzy.app/send";
    if (!http.begin(endpoint)) {
        return;
    }

    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String body = String("accountKey=") + combinedKeys +
                  "&title=" + urlEncode(title) +
                  "&message=" + urlEncode(message);

    int httpCode = http.POST(body);
    (void)httpCode;
    http.end();
}

void PushNotifier::sendAll(String title, String message) {
    std::vector<uint8_t> indices;
    indices.reserve(accounts.size());
    for (uint8_t i = 0; i < accounts.size(); ++i) indices.push_back(i);
    sendNotification(title, message, indices);
}
