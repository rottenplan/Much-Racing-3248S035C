#include "WiFiManager.h"
#include "web_static.h"
#include <ArduinoJson.h>
#include <Update.h>

WiFiManager::WiFiManager() : _server(80) {
  _ssid = "";
  _pass = "";
  _lastAttemptTime = 0;
  _isConnecting = false;
}

void WiFiManager::begin() {
  loadCredentials();

  if (!_enabled) {
    Serial.println("WiFi: Disabled by user settings.");
    return;
  }

  // ALWAYS Start AP Mode
  WiFi.mode(WIFI_AP_STA);
  startAP();

  // Start Web Server
  _server.on("/", HTTP_GET, std::bind(&WiFiManager::handleRoot, this));
  _server.on("/api/live", HTTP_GET,
             std::bind(&WiFiManager::handleApiLive, this));

  // OTA Update Routes
  _server.on("/update", HTTP_GET,
             std::bind(&WiFiManager::handleUpdateGet, this));

  // Session Manager Routes
  _server.on("/sessions", HTTP_GET,
             std::bind(&WiFiManager::handleSessionsPage, this));
  _server.on("/api/sessions", HTTP_GET,
             std::bind(&WiFiManager::handleApiSessions, this));
  _server.on("/download", HTTP_GET,
             std::bind(&WiFiManager::handleDownload, this));

  _server.on(

      "/update", HTTP_POST,
      [this]() { _server.sendHeader("Connection", "close"); },
      [this]() { handleUpdateUpload(); });

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
  if (!_enabled)
    return;

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

void WiFiManager::handleRoot() {
  _server.sendHeader("Access-Control-Allow-Origin", "*");
  _server.send(200, "text/html", INDEX_HTML);
}

void WiFiManager::handleApiLive() {
  // Serial.println("API Live: Request Received"); // Debug spam
  if (!_gps) {
    Serial.println("API Live: Error - No GPS Linked!");
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

void WiFiManager::handleUpdateGet() {
  _server.send(200, "text/html", UPDATE_HTML);
}

void WiFiManager::handleUpdateUpload() {
  HTTPUpload &upload = _server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    Serial.printf("Update: %s\n", upload.filename.c_str());
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { // start with max available size
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    /* flashing firmware to ESP*/
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (Update.end(true)) { // true to set the size to the current progress
      Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      _server.send(200, "text/plain", "Update Success! Rebooting...");
      delay(1000);
      ESP.restart();
    }
  }
}

void WiFiManager::handleSessionsPage() {
  _server.send(200, "text/html", SESSIONS_HTML);
}

void WiFiManager::handleApiSessions() {
  JsonDocument doc;
  JsonArray array = doc.to<JsonArray>();

  File root = SD.open("/sessions"); // Assuming sessions are here
  if (!root || !root.isDirectory()) {
    // Fallback to root if folder missing
    root = SD.open("/");
  }

  if (root) {
    File file = root.openNextFile();
    while (file) {
      String fileName = String(file.name());
      // Filter .csv or .gpx
      if (!file.isDirectory() &&
          (fileName.endsWith(".csv") || fileName.endsWith(".gpx"))) {
        JsonObject obj = array.add<JsonObject>();

        // Clean filename if it contains full path (depending on SD lib version)
        int lastSlash = fileName.lastIndexOf('/');
        String cleanName =
            (lastSlash >= 0) ? fileName.substring(lastSlash + 1) : fileName;

        obj["name"] = cleanName;

        // Size Formatting
        float kb = file.size() / 1024.0;
        if (kb > 1024)
          obj["size"] = String(kb / 1024.0, 2) + " MB";
        else
          obj["size"] = String(kb, 1) + " KB";

        // Full Path for download
        String fullPath = String(root.path()) + "/" + cleanName;
        if (String(root.path()) == "/")
          fullPath = "/" + cleanName;
        obj["path"] = fullPath;
      }
      file = root.openNextFile();
    }
  }

  String output;
  serializeJson(doc, output);
  _server.send(200, "application/json", output);
}

void WiFiManager::handleDownload() {
  if (!_server.hasArg("file")) {
    _server.send(400, "text/plain", "Bad Request");
    return;
  }

  String path = _server.arg("file");
  // Basic security: No parent dir traversing
  if (path.indexOf("..") != -1) {
    _server.send(403, "text/plain", "Forbidden");
    return;
  }

  if (SD.exists(path)) {
    File file = SD.open(path, FILE_READ);
    _server.streamFile(file, "application/octet-stream");
    file.close();
  } else {
    _server.send(404, "text/plain", "File Not Found");
  }
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
  _prefs.begin("laptimer", true);
  _enabled = _prefs.getBool("wifi_enabled", true); // Default to ON
  _ssid = _prefs.getString("wifi_ssid", "");
  _pass = _prefs.getString("wifi_pass", "");
  _prefs.end();

  if (loadFromSD()) {
    // SD Card Overrides stored SSID/Pass, but not enabled state (unless we add
    // it there too, but let's keep it simple)
    return;
  }
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

void WiFiManager::setEnabled(bool enabled) {
  if (_enabled == enabled)
    return;
  _enabled = enabled;

  _prefs.begin("laptimer", false);
  _prefs.putBool("wifi_enabled", _enabled);
  _prefs.end();

  if (_enabled) {
    begin(); // Re-run begin to start everything up
  } else {
    Serial.println("WiFi: Disabling...");
    _server.stop();
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
  }
}
