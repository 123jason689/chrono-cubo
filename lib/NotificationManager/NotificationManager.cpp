#include "NotificationManager.h"
#include <HardwareSerial.h>
#include "configs.h"

// We'll use a dedicated HardwareSerial for DFPlayer on ESP32 (UART1 or UART2).
// On ESP32-C3, Serial1 maps to GPIO via begin(tx, rx)
static HardwareSerial DFSerial(1);

NotificationManager::NotificationManager()
    : isAlertActive(false), lastFlashTime(0), ledState(false), currentVolume(20) {}

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
    dfPlayer.volume(currentVolume); // default volume
    dfPlayer.EQ(DFPLAYER_EQ_NORMAL);
    Serial.println("[NotificationManager] DFPlayer ready.");
}

void NotificationManager::playAlert(int trackNumber) {

    if (dfPlayer.available()) {
        // Drain any pending notifications
        while (dfPlayer.available()) { dfPlayer.readType(); }
    }

    dfPlayer.volume(currentVolume);
    
    // Plays /mp3/00XX.mp3 where XX matches trackNumber
    dfPlayer.playMp3Folder(trackNumber);

    isAlertActive = true;
    lastFlashTime = millis();
    ledState = true;
    digitalWrite(LED_PIN, ledState ? HIGH : LOW);
}

void NotificationManager::playAlert(int trackNumber, int vol) {
    if (vol < 0) vol = 0; if (vol > 30) vol = 30;
    currentVolume = vol;
    if (dfPlayer.available()) dfPlayer.volume(currentVolume);
    playAlert(trackNumber);
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

void NotificationManager::setVolume(int vol) {
    if (vol < 0) vol = 0;
    if (vol > 30) vol = 30;
    currentVolume = vol;
    // Apply to DFPlayer if possible
    if (dfPlayer.available()) dfPlayer.volume(currentVolume);
}

int NotificationManager::getVolume() const {
    return currentVolume;
}

void NotificationManager::playAdvert(uint16_t trackNumber) {
    // Use DFPlayer's advertisement/interrupt API which pauses current mp3 and plays advert
    // Note: playAdvertisement is available in DFRobotDFPlayerMini library
    if (dfPlayer.available()) {
        // DFRobotDFPlayerMini provides `advertise()` to play short advert/notification clips
        dfPlayer.advertise((uint8_t)trackNumber);
    } else {
        // Fallback: just play from mp3 folder
        dfPlayer.playMp3Folder(trackNumber);
    }
}

bool NotificationManager::isAlerting() const {
    return isAlertActive;
}
