#include "WiFiManager.h"
#include "web_static.h"
#include <ArduinoJson.h>

WiFiManager::WiFiManager() : _server(80) {
  _ssid = "";
  _pass = "";
  _lastAttemptTime = 0;
  _isConnecting = false;
}

void WiFiManager::begin() {
  loadCredentials();

  // ALWAYS Start AP Mode
  startAP();

  // Enable CORS
  _server.enableCORS(true);

  // Start Web Server
  _server.on("/", HTTP_GET, std::bind(&WiFiManager::handleRoot, this));
  _server.on("/api/live", HTTP_GET,
             std::bind(&WiFiManager::handleApiLive, this));
  _server.begin();
  Serial.println("Web Server Started on Port 80");

  // Try Auto Connect if creds exist
  if (_ssid.length() > 0) {
    Serial.print("WiFi: Auto-connecting to ");
    Serial.println(_ssid);
    WiFi.mode(WIFI_AP_STA);
    WiFi.begin(_ssid.c_str(), _pass.c_str());
    _isConnecting = true;
    _lastAttemptTime = millis();
  } else {
    Serial.println("WiFi: No saved credentials. Running AP Only.");
  }
}

void WiFiManager::startAP() {
  const char *apSSID = "MuchRacing-GPS";
  const char *apPass = "12345678";

  WiFi.softAP(apSSID, apPass);

  Serial.print("AP Started: ");
  Serial.println(apSSID);
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());
}

void WiFiManager::update() {
  _server.handleClient(); // Handle Web Requests

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

void WiFiManager::handleRoot() { _server.send(200, "text/html", INDEX_HTML); }

void WiFiManager::handleApiLive() {
  if (!_gps) {
    _server.send(500, "application/json",
                 "{\"error\":\"No GPS Manager Linked\"}");
    return;
  }

  JsonDocument doc;
  doc["speed"] = _gps->getSpeedKmph();
  doc["rpm"] = _gps->getRPM();
  doc["trip"] = _gps->getTotalTrip();
  doc["sats"] = _gps->getSatellites();
  doc["lat"] = _gps->getLatitude();
  doc["lng"] = _gps->getLongitude();

  String json;
  serializeJson(doc, json);
  _server.send(200, "application/json", json);
}

bool WiFiManager::connect(const char *ssid, const char *pass) {
  _ssid = ssid;
  _pass = pass;

  WiFi.mode(WIFI_AP_STA);
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

int WiFiManager::scanNetworks() {
  Serial.println("WiFi: Scanning...");
  WiFi.mode(WIFI_AP_STA);
  int n = WiFi.scanNetworks();
  Serial.print("WiFi: Found ");
  Serial.print(n);
  Serial.println(" networks");
  return n;
}

String WiFiManager::getSSID(int index) { return WiFi.SSID(index); }

int WiFiManager::getRSSI(int index) { return WiFi.RSSI(index); }

void WiFiManager::disconnect() {
  WiFi.disconnect(true);
  WiFi.mode(WIFI_AP);
}

void WiFiManager::saveCredentials(String ssid, String pass) {
  _ssid = ssid;
  _pass = pass;

  _prefs.begin("laptimer", false);
  _prefs.putString("wifi_ssid", _ssid);
  _prefs.putString("wifi_pass", _pass);
  _prefs.end();

  if (SD.cardSize() > 0) {
    if (SD.exists("/wifi.txt") || SD.open("/wifi.txt", FILE_WRITE)) {
      File f = SD.open("/wifi.txt", FILE_WRITE);
      if (f) {
        f.println(_ssid);
        f.println(_pass);
        f.close();
      }
    }
  }
}

void WiFiManager::loadCredentials() {
  if (loadFromSD())
    return;

  _prefs.begin("laptimer", true);
  _ssid = _prefs.getString("wifi_ssid", "");
  _pass = _prefs.getString("wifi_pass", "");
  _prefs.end();
}

bool WiFiManager::loadFromSD() {
  if (SD.cardSize() == 0)
    return false;

  if (SD.exists("/wifi.txt")) {
    File f = SD.open("/wifi.txt", FILE_READ);
    if (f) {
      _ssid = f.readStringUntil('\n');
      _pass = f.readStringUntil('\n');
      _ssid.trim();
      _pass.trim();
      f.close();
      if (_ssid.length() > 0)
        return true;
    }
  }
  return false;
}

bool WiFiManager::tryAutoConnect() {
  if (_ssid.length() == 0) {
    loadCredentials();
  }

  if (_ssid.length() > 0) {
    // Use connect logic but standard blocking or async?
    // Re-using connect() which is blocking-ish (loop with delay)
    return connect(_ssid.c_str(), _pass.c_str());
  }
  return false;
}

bool WiFiManager::isConnected() { return WiFi.status() == WL_CONNECTED; }
