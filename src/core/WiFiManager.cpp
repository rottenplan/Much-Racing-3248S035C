#include "WiFiManager.h"

WiFiManager::WiFiManager() {
    _ssid = "";
    _pass = "";
    _lastAttemptTime = 0;
    _isConnecting = false;
}

void WiFiManager::begin() {
    loadCredentials();
    
    if (_ssid.length() > 0) {
        Serial.print("WiFi: Auto-connecting to ");
        Serial.println(_ssid);
        WiFi.mode(WIFI_STA);
        WiFi.begin(_ssid.c_str(), _pass.c_str());
        _isConnecting = true;
        _lastAttemptTime = millis();
    } else {
        Serial.println("WiFi: No saved credentials found.");
    }
}

void WiFiManager::update() {
    // Keep alive or retry logic if needed
    // For now, let's just monitor connection status
    if (_isConnecting) {
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("WiFi: Connected successfully!");
            _isConnecting = false;
        } else if (millis() - _lastAttemptTime > 15000) {
            Serial.println("WiFi: Auto-connect timeout.");
            _isConnecting = false;
        }
    }
}

bool WiFiManager::connect(const char* ssid, const char* pass) {
    _ssid = ssid;
    _pass = pass;
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(_ssid.c_str(), _pass.c_str());
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        attempts++;
        Serial.print(".");
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi: Manual connection success!");
        saveCredentials(_ssid, _pass);
        return true;
    } else {
        Serial.println("\nWiFi: Manual connection failed.");
        return false;
    }
}

void WiFiManager::disconnect() {
    WiFi.disconnect(true);
}

void WiFiManager::saveCredentials(String ssid, String pass) {
    _ssid = ssid;
    _pass = pass;
    
    // Save to NVS (Preferences)
    _prefs.begin("laptimer", false);
    _prefs.putString("wifi_ssid", _ssid);
    _prefs.putString("wifi_pass", _pass);
    _prefs.end();
    
    // Also try to save to SD card as requested by user
    if (SD.cardSize() > 0) {
        File f = SD.open("/wifi.txt", FILE_WRITE);
        if (f) {
            f.println(_ssid);
            f.println(_pass);
            f.close();
            Serial.println("WiFi: Credentials saved to SD card.");
        }
    }
}

void WiFiManager::loadCredentials() {
    // 1. Try SD Card first (User mentioned SD card specifically)
    if (loadFromSD()) {
        return;
    }
    
    // 2. Fallback to NVS (Preferences)
    _prefs.begin("laptimer", true);
    _ssid = _prefs.getString("wifi_ssid", "");
    _pass = _prefs.getString("wifi_pass", "");
    _prefs.end();
    
    if (_ssid.length() > 0) {
        Serial.println("WiFi: Credentials loaded from NVS.");
    }
}

bool WiFiManager::loadFromSD() {
    if (SD.cardSize() == 0) return false;
    
    if (SD.exists("/wifi.txt")) {
        File f = SD.open("/wifi.txt", FILE_READ);
        if (f) {
            _ssid = f.readStringUntil('\n');
            _pass = f.readStringUntil('\n');
            _ssid.trim();
            _pass.trim();
            f.close();
            
            if (_ssid.length() > 0) {
                Serial.println("WiFi: Credentials loaded from SD card.");
                return true;
            }
        }
    }
    return false;
}

bool WiFiManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}
