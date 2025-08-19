#include "NotificationManager.h"
#include <HardwareSerial.h>
#include "configs.h"

// We'll use a dedicated HardwareSerial for DFPlayer on ESP32 (UART1 or UART2).
// On ESP32-C3, Serial1 maps to GPIO via begin(tx, rx)
static HardwareSerial DFSerial(1);

NotificationManager::NotificationManager()
    : isAlertActive(false), lastFlashTime(0), ledState(false) {}

void NotificationManager::begin() {
    // Initialize LED pin
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    // Initialize serial for DFPlayer
    // Note: DFPlayer expects 9600 baud
    DFSerial.begin(9600, SERIAL_8N1, DFPLAYER_RX_PIN, DFPLAYER_TX_PIN);

    // Initialize DFPlayer
    if (!dfPlayer.begin(DFSerial)) {
        // Could not communicate with DFPlayer; keep going without audio
        Serial.println("[NotificationManager] DFPlayer init failed. Check wiring and SD card.");
        return;
    }

    dfPlayer.setTimeOut(500);
    dfPlayer.volume(20); // default volume
    dfPlayer.EQ(DFPLAYER_EQ_NORMAL);
    Serial.println("[NotificationManager] DFPlayer ready.");
}

void NotificationManager::playAlert(int trackNumber, int volume) {
    if (volume < 0) volume = 0;
    if (volume > 30) volume = 30; // DFPlayer volume range 0-30

    if (dfPlayer.available()) {
        // Drain any pending notifications
        while (dfPlayer.available()) { dfPlayer.readType(); }
    }

    dfPlayer.volume(volume);
    // Plays /mp3/00XX.mp3 where XX matches trackNumber
    dfPlayer.playMp3Folder(trackNumber);

    isAlertActive = true;
    lastFlashTime = millis();
    ledState = true;
    digitalWrite(LED_PIN, ledState ? HIGH : LOW);
}

void NotificationManager::stopAlert() {
    if (isAlertActive) {
        dfPlayer.stop();
    }
    isAlertActive = false;
    ledState = false;
    digitalWrite(LED_PIN, LOW);
}

void NotificationManager::update() {
    if (!isAlertActive) return;

    unsigned long now = millis();
    if (now - lastFlashTime >= 250) {
        lastFlashTime = now;
        ledState = !ledState;
        digitalWrite(LED_PIN, ledState ? HIGH : LOW);
    }
}
