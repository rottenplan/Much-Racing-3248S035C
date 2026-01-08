#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include "GPSManager.h"
#include <Arduino.h>
#include <Preferences.h>
#include <SD.h>
#include <WebServer.h>
#include <WiFi.h>

class WiFiManager {
public:
  WiFiManager();
  void begin();
  void update();
  void setGPS(GPSManager *gps) { _gps = gps; } // Inject GPS

  // Connection Methods
  bool connect(const char *ssid, const char *pass);
  bool tryAutoConnect(); // Try with saved credentials
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

  WebServer _server; // Web Server Object
  GPSManager *_gps = nullptr;

  bool loadFromSD();
  void startAP(); // Start Hotspot
  void handleRoot();
  void handleApiLive();
};

#endif
