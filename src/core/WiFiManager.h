#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>
#include <Preferences.h>
#include <SD.h>
#include <Arduino.h>

class WiFiManager {
public:
    WiFiManager();
    void begin();
    void update();
    
    // Connection Methods
    bool connect(const char* ssid, const char* pass);
    void disconnect();
    
    // Persistence
    void saveCredentials(String ssid, String pass);
    void loadCredentials();
    
    // Status
    bool isConnected();
    String getSSID() { return _ssid; }
    
private:
    String _ssid;
    String _pass;
    Preferences _prefs;
    unsigned long _lastAttemptTime;
    bool _isConnecting;
    
    bool loadFromSD();
};

#endif
