#include "SettingsScreen.h"
#include "../../config.h"
#include "../../core/GPSManager.h"
#include "../../core/SessionManager.h"
#include "../../core/SyncManager.h"
#include "../../core/WiFiManager.h"
#include "../fonts/Org_01.h"
#include "../fonts/Picopixel.h"
#include "TimeSettingScreen.h"

extern SessionManager sessionManager;
extern WiFiManager wifiManager;
extern SyncManager syncManager;

// Static pointer for callback
static TFT_eSPI *static_tft = nullptr;

void sdProgressCallback(int percent, String status) {
  if (!static_tft)
    return;

  // Draw Status
  static_tft->setTextColor(TFT_WHITE, COLOR_BG);
  static_tft->setTextDatum(TC_DATUM); // Top Center
  static_tft->setTextSize(1);
  static_tft->setFreeFont(&Org_01);

  // Clear text area (approx y=100-120)
  static_tft->fillRect(0, 100, 320, 30, COLOR_BG);
  static_tft->drawString(status, 320 / 2, 110);

  // Draw Bar
  int barW = 200;
  int barH = 10;
  int barX = (320 - barW) / 2;
  int barY = 140;

  // Outline
  static_tft->drawRect(barX - 1, barY - 1, barW + 2, barH + 2, TFT_WHITE);

  // Fill
  int fillW = (barW * percent) / 100;
  static_tft->fillRect(barX, barY, fillW, barH, TFT_GREEN);
  static_tft->fillRect(barX + fillW, barY, barW - fillW, barH,
                       COLOR_BG); // Clear rest
}

void SettingsScreen::onShow() {
  _selectedIdx = -1;
  _lastSelectedIdx = -1;
  _scrollOffset = 0;        // Reset scroll
  _currentMode = MODE_MAIN; // Start at Main
  loadSettings();           // Reload to ensure sync

  TFT_eSPI *tft = _ui->getTft();
  tft->fillScreen(COLOR_BG);
  drawList(0, true);
}

void SettingsScreen::loadSettings() {
  _settings.clear();

  if (_currentMode == MODE_MAIN) {
    _settings.push_back({"CLOCK SETTING", TYPE_ACTION});

    _prefs.begin("laptimer", false);

    // Power Save (Auto Off)
    SettingItem powerSave = {"POWER SAVE", TYPE_VALUE, "power_save"};
    powerSave.options = {"1 min", "5 min", "10 min", "30 min", "Never"};
    powerSave.currentOptionIdx =
        _prefs.getInt("power_save", 1); // Default 5 min
    _settings.push_back(powerSave);

    // Brightness
    SettingItem brightness = {"BRIGHTNESS", TYPE_VALUE, "brightness"};
    brightness.options = {"10%", "20%", "30%", "40%", "50%",
                          "60%", "70%", "80%", "90%", "100%"};
    brightness.currentOptionIdx =
        _prefs.getInt("brightness", 9); // Default 100%
    _settings.push_back(brightness);

    // Units
    SettingItem units = {"UNITS", TYPE_VALUE, "units"};
    units.options = {"Metric (km/h)", "Imperial (mph)"};
    units.currentOptionIdx = _prefs.getInt("units", 0); // Default Metric
    _settings.push_back(units);

    _prefs.end();

    // GPS Status Sub-menu REMOVED
    // _settings.push_back({"GPS STATUS", TYPE_ACTION});
    // _settings.push_back({"GNSS LOG", TYPE_ACTION}); // Moved to GPS Status
    // Double Tap
    _settings.push_back({"GNSS FINE TUNING", TYPE_ACTION});

    // SD Card Test Sub-menu
    _settings.push_back({"SD CARD TEST", TYPE_ACTION});

    // RPM Settings Sub-menu
    _settings.push_back({"RPM SETTING", TYPE_ACTION});

    // Engine Hours
    _settings.push_back({"ENGINE HOURS", TYPE_ACTION});

    // WiFi Setup
    _settings.push_back({"WIFI SETUP", TYPE_ACTION});

    // Cloud Sync
    _settings.push_back({"SYNC WITH CLOUD", TYPE_ACTION});

    // Account Management
    _settings.push_back({"REMOVE ACCOUNT", TYPE_ACTION});

  } else if (_currentMode == MODE_ENGINE) {
    _prefs.begin("laptimer", false);

    // Engine Hours (Read-only display)
    SettingItem engineHours = {"TOTAL HOURS", TYPE_VALUE, "engine_hours"};
    float hours = _prefs.getFloat("engine_hours", 0.0);
    engineHours.options = {String(hours, 1) + " hrs"};
    engineHours.currentOptionIdx = 0;
    _settings.push_back(engineHours);

    _prefs.end();
  } else if (_currentMode == MODE_RPM) {
    _prefs.begin("laptimer", false);

    // Pulses Per Revolution (PPR)
    SettingItem ppr = {"PULSE PER REV", TYPE_VALUE, "rpm_ppr"};
    ppr.options = {"1.0", "0.5 (1p/2r)", "2.0 (Default)", "3.0 (3p/1r)",
                   "4.0 (4p/1r)"};
    ppr.currentOptionIdx = _prefs.getInt("rpm_ppr_idx", 2);
    if (ppr.currentOptionIdx < 0 || ppr.currentOptionIdx >= ppr.options.size())
      ppr.currentOptionIdx = 2;

    _settings.push_back(ppr);

    _prefs.end();
  } else if (_currentMode == MODE_CLOCK) {
    _prefs.begin("laptimer", false);

    // UTC Time Zone
    SettingItem utcZone = {"UTC TIME ZONE", TYPE_VALUE, "utc_offset_idx"};
    for (int i = -12; i <= 12; i++) {
      char buf[16];
      if (i == 0)
        sprintf(buf, "UTC +00:00");
      else
        sprintf(buf, "UTC %s%02d:00", (i > 0 ? "+" : ""), i);
      utcZone.options.push_back(String(buf));
    }
    utcZone.currentOptionIdx =
        _prefs.getInt("utc_offset_idx", 12); // Default UTC+0
    extern GPSManager gpsManager;
    gpsManager.setUtcOffset(utcZone.currentOptionIdx - 12); // Ensure sync
    _settings.push_back(utcZone);

    _settings.push_back({"MANUAL CLOCK ADJUST", TYPE_ACTION});

    _settings.push_back({"MANUAL CLOCK ADJUST", TYPE_ACTION});

    _prefs.end();
  } else if (_currentMode == MODE_GNSS_CONFIG) {
    _prefs.begin("laptimer", false);

    // 1. GNSS Mode
    SettingItem mode = {"GNSS MODE", TYPE_VALUE, "gnss_mode"};
    mode.options = {
        "All (10Hz)",         // 0
        "GPS+GLO+SBAS(16Hz)", // 1
        "GPS+GAL+GLO(10Hz)",  // 2
        "GPS+GAL+SBAS(20Hz)", // 3
        "GPS+SBAS (25Hz)",    // 4
        "GPS Only (25Hz)",    // 5
        "GPS+BEI+SBAS(12Hz)", // 6
        "GPS+GLO (16Hz)"      // 7
    };
    mode.currentOptionIdx = _prefs.getInt("gnss_mode", 1);
    _settings.push_back(mode);

    // 2. GNSS Coordinate Projection
    SettingItem proj = {"COORD PROJECTION", TYPE_VALUE, "gnss_proj"};
    proj.options = {"No Projection", "Projection (Def)"};
    // Map bool to index: true -> 1, false -> 0
    // But we need to check how we stored it? We stored as bool.
    // Let's rely on internal state or load fresh.
    bool projState = _prefs.getBool("gnss_proj", true);
    proj.currentOptionIdx = projState ? 1 : 0;
    _settings.push_back(proj);

    // 3. Frequency
    SettingItem freq = {"FREQUENCY", TYPE_VALUE, "gnss_freq_limit"};
    freq.options = {"Max Possible", "Force 10Hz"};
    freq.currentOptionIdx = _prefs.getInt("gnss_freq_limit", 0);
    _settings.push_back(freq);

    // 4. Dynamic Model
    SettingItem dyn = {"DYNAMIC MODEL", TYPE_VALUE, "gnss_model"};
    dyn.options = {
        "Portable",         // 0
        "Stationary",       // 1
        "Pedestrian",       // 2
        "Automotive (Def)", // 3
        "At Sea",           // 4
        "Airborne <1g",     // 5
        "Airborne <2g",     // 6
        "Airborne <4g"      // 7
    };
    dyn.currentOptionIdx = _prefs.getInt("gnss_model", 3);
    _settings.push_back(dyn);

    // 5. SBAS Config
    SettingItem sbas = {"SBAS SYSTEM", TYPE_VALUE, "gnss_sbas"};
    sbas.options = {
        "EUROPE (EGNOS)",   // 0
        "USA (WAAS)",       // 1
        "RUSSIA (SDCM)",    // 2
        "JAPAN (MSAS)",     // 3
        "INDIA (GAGAN)",    // 4
        "AUS (SouthPAN)",   // 5
        "S.AMERICA (NONE)", // 6
        "MID-EAST (NONE)",  // 7
        "AFRICA (NONE)",    // 8
        "CHINA (BDSBAS)",   // 9
        "KOREA (KASS)"      // 10
    };
    sbas.currentOptionIdx = _prefs.getInt("gnss_sbas", 0);
    _settings.push_back(sbas);

    // 6. GNSS RX PIN
    SettingItem rxPin = {"GNSS RX PIN", TYPE_VALUE, "gps_rx_pin"};
    // Options: 17, 16, 1, 3
    rxPin.options = {"17 (Def)", "16", "1 (TX0)", "3 (RX0)"};

    extern GPSManager gpsManager;
    int curRx = gpsManager.getRxPin();
    rxPin.currentOptionIdx = 0; // Default 17
    int validRxCheck[] = {17, 16, 1, 3};
    for (int i = 0; i < 4; i++) {
      if (validRxCheck[i] == curRx) {
        rxPin.currentOptionIdx = i;
        break;
      }
    }
    _settings.push_back(rxPin);

    // 7. GNSS TX PIN
    SettingItem txPin = {"GNSS TX PIN", TYPE_VALUE, "gps_tx_pin"};
    txPin.options = {"17 (Def)", "16", "1 (TX0)", "3 (RX0)"};
    int curTx = gpsManager.getTxPin();
    txPin.currentOptionIdx = 0; // Default 17
    int validTxCheck[] = {17, 16, 1, 3};
    for (int i = 0; i < 4; i++) {
      if (validTxCheck[i] == curTx) {
        txPin.currentOptionIdx = i;
        break;
      }
    }
    _settings.push_back(txPin);

    // 8. GNSS BAUD RATE
    SettingItem baud = {"GNSS BAUD RATE", TYPE_VALUE, "gps_baud"};
    baud.options = {"9600 bps", "19200 bps", "38400 bps", "57600 bps",
                    "115200 bps"};
    int curBaud = gpsManager.getBaud();
    baud.currentOptionIdx = 0; // Default 9600
    int validBauds[] = {9600, 19200, 38400, 57600, 115200};
    for (int i = 0; i < 5; i++) {
      if (validBauds[i] == curBaud) {
        baud.currentOptionIdx = i;
        break;
      }
    }
    _settings.push_back(baud);

    _prefs.end();
  }
}

void SettingsScreen::saveSetting(int idx) {
  if (idx < 0 || idx >= _settings.size())
    return;

  _prefs.begin("laptimer", false);
  SettingItem &item = _settings[idx];

  if (item.type == TYPE_VALUE) {
    _prefs.putInt(item.key.c_str(), item.currentOptionIdx);

    // Apply immediate effects
    if (item.key == "brightness") {
      // Map 0-9 (10%-100%) to PWM duty cycle
      int duty = map(item.currentOptionIdx, 0, 9, 26, 255); // 10% to 100%
      _ui->setBrightness(duty);
    }

    if (item.key == "power_save") {
      unsigned long ms = 0;
      switch (item.currentOptionIdx) {
      case 0:
        ms = 60000;
        break; // 1 min
      case 1:
        ms = 300000;
        break; // 5 min
      case 2:
        ms = 600000;
        break; // 10 min
      case 3:
        ms = 1800000;
        break; // 30 min
      case 4:
        ms = 0;
        break; // Never
      }
      _ui->setAutoOff(ms);
    }

    // GPS Config Handlers
    extern GPSManager gpsManager;

    if (item.key == "utc_offset_idx") {
      int offset = item.currentOptionIdx - 12;
      gpsManager.setUtcOffset(offset);
      _ui->drawStatusBar(true);
    }

    if (item.key == "gnss_mode") {
      gpsManager.setGnssMode(item.currentOptionIdx);
    }
    if (item.key == "gnss_proj") {
      gpsManager.setProjection(item.currentOptionIdx == 1);
    }
    if (item.key == "gnss_model") {
      gpsManager.setDynamicModel(item.currentOptionIdx);
    }
    if (item.key == "gnss_sbas") {
      gpsManager.setSBASConfig(item.currentOptionIdx);
    }
    if (item.key == "gnss_freq_limit") {
      // Recalculate based on current mode but cap at 10Hz if selected
      if (item.currentOptionIdx == 1)
        gpsManager.setFrequencyLimit(10);
      else
        gpsManager.setGnssMode(_prefs.getInt("gnss_mode", 1)); // Re-apply max
    }

    if (item.key == "gps_rx_pin" || item.key == "gps_tx_pin") {
      // Map index to pin value
      int validRxCheck[] = {17, 16, 1, 3};
      int validTxCheck[] = {17, 16, 1, 3};

      // Get fresh pin values from other settings if just changing one
      int newRx = gpsManager.getRxPin();
      int newTx = gpsManager.getTxPin();

      if (item.key == "gps_rx_pin") {
        if (item.currentOptionIdx >= 0 && item.currentOptionIdx < 4)
          newRx = validRxCheck[item.currentOptionIdx];
      }
      if (item.key == "gps_tx_pin") {
        if (item.currentOptionIdx >= 0 && item.currentOptionIdx < 4)
          newTx = validTxCheck[item.currentOptionIdx];
      }

      // Apply new pins
      gpsManager.setPins(newRx, newTx);
    }

    if (item.key == "gps_baud") {
      int validBauds[] = {9600, 19200, 38400, 57600, 115200};
      if (item.currentOptionIdx >= 0 && item.currentOptionIdx < 5) {
        gpsManager.setBaud(validBauds[item.currentOptionIdx]);
      }
    }

  } else if (item.type == TYPE_TOGGLE) {
    _prefs.putBool(item.key.c_str(), item.checkState);
  }
  _prefs.end();
}

void SettingsScreen::update() {
  static unsigned long lastSettingTouch = 0;

  // WiFi Scanning Animation (runs without touch)
  if (_isScanning) {
    if (millis() - _lastScanAnim > 500) {
      _lastScanAnim = millis();
      _scanAnimStep = (_scanAnimStep + 1) % 4;

      TFT_eSPI *tft = _ui->getTft();
      tft->setTextColor(TFT_WHITE, COLOR_BG);
      tft->setTextDatum(MC_DATUM);
      String dots = "";
      for (int i = 0; i < _scanAnimStep; i++)
        dots += ".";
      tft->drawString("Scanning" + dots + "   ", SCREEN_WIDTH / 2, 120);
    }

    int n = WiFi.scanComplete();
    if (n >= 0) {
      _isScanning = false;
      _scanCount = n;
      _ui->getTft()->fillScreen(COLOR_BG);
      _ui->drawStatusBar(true);
      drawWiFiList();
      _lastWiFiTouch = millis();
    }
    return; // Block other input while scanning
  }

  UIManager::TouchPoint p = _ui->getTouchPoint();
  if (p.x == -1)
    return;

  // Tombol Kembali (Bottom-Left corner, y > 210)
  if (p.x < 60 && p.y > 210) {
    if (millis() - lastSettingTouch < 200)
      return;
    lastSettingTouch = millis();

    // Visual Feedback (Selection)
    if (_selectedIdx != -2) {
      _selectedIdx = -2;
      drawList(_scrollOffset, false);
    }

    // Logic Back
    if (_currentMode == MODE_MAIN) {
      _ui->switchScreen(SCREEN_MENU);
    } else {
      // Return to Main Settings
      _currentMode = MODE_MAIN;
      _ui->setTitle("SETTINGS");
      loadSettings();
      _ui->getTft()->fillScreen(COLOR_BG);
      _ui->drawStatusBar(true);
      _scrollOffset = 0;
      drawList(0, true);
    }
    return;
  }

  // Scroll Down Button (Bottom-Right, y > 210, x > 290)
  if (p.x > 290 && p.y > 210) {
    if (millis() - lastSettingTouch < 200)
      return;
    lastSettingTouch = millis();

    int listY = 30;
    int itemH = 20;
    int maxY = 210;
    int visibleItems = (maxY - listY) / itemH;

    if (_scrollOffset + visibleItems < _settings.size()) {
      _scrollOffset++;
      drawList(_scrollOffset, true); // Force redraw.
    }
    return;
  }

  // Scroll Up Button (Bottom-Right, y > 210, x 260-290)
  if (p.x > 260 && p.x <= 290 && p.y > 210) {
    if (millis() - lastSettingTouch < 200)
      return;
    lastSettingTouch = millis();

    if (_scrollOffset > 0) {
      _scrollOffset--;
      drawList(_scrollOffset, true);
    }
    return;
  }

  // Daftar Sentuh (now starts at y=30, with gap after status bar)
  int listY = 30;
  int itemH = 20; // Match drawList

  // Debounce Check
  if (millis() - lastSettingTouch > 200) {
    // Only handle standard List Touch for list-based modes
    if (_currentMode == MODE_MAIN || _currentMode == MODE_RPM ||
        _currentMode == MODE_CLOCK || _currentMode == MODE_ENGINE ||
        _currentMode == MODE_GNSS_CONFIG) {
      int idx = _scrollOffset + ((p.y - listY) / itemH);

      if (idx >= 0 && idx < _settings.size()) {
        unsigned long now = millis();
        if (_lastTapIdx == idx && (now - _lastTapTime < 500)) {
          // Double Tap Confirmed -> Execute
          handleTouch(idx);

          // Reset logic depend on action... usually reset tap tracking
          _lastTapIdx = -1;
        } else {
          // First Tap -> Select/Prime
          _lastTapIdx = idx;
          _lastTapTime = now;

          if (_selectedIdx != idx) {
            _selectedIdx = idx;
            drawList(_scrollOffset, false);
          }
        }
      }
    }

    lastSettingTouch = millis();
  } // End List Touch

  // WiFi List Mode
  if (_currentMode == MODE_WIFI && p.y > 60) {
    if (millis() - _lastWiFiTouch > 300) {
      _lastWiFiTouch = millis();
      int idx = (p.y - 60) / 25;
      if (idx >= 0 && idx < _scanCount) {
        _selectedWiFiIdx = idx;
        _targetSSID = WiFi.SSID(idx);
        _enteredPass = "";
        _currentMode = MODE_WIFI_PASS;
        _ui->getTft()->fillScreen(COLOR_BG);
        _ui->drawStatusBar(true);
        drawKeyboard();
        _lastKeyboardTouch = millis(); // Prevent immediate key press
        return;
      }
    }
  }

  // WiFi Keyboard Mode - handle key presses
  if (_currentMode == MODE_WIFI_PASS) {
    if (millis() - _lastKeyboardTouch > 200) {
      _lastKeyboardTouch = millis();

      // Keyboard input
      KeyboardComponent::KeyResult res = _keyboard.handleTouch(p.x, p.y, 115);
      switch (res.type) {
      case KeyboardComponent::KEY_CHAR: {
        char c = res.value;
        if (!_isUppercase && c >= 'A' && c <= 'Z')
          c += ('a' - 'A');
        _enteredPass += c;
        drawKeyboard(false);
        break;
      }
      case KeyboardComponent::KEY_SHIFT:
        _isUppercase = !_isUppercase;
        drawKeyboard(false);
        break;
      case KeyboardComponent::KEY_DEL:
        if (_enteredPass.length() > 0) {
          _enteredPass.remove(_enteredPass.length() - 1);
          drawKeyboard(false);
        }
        break;
      case KeyboardComponent::KEY_SPACE:
        _enteredPass += " ";
        drawKeyboard(false);
        break;
      case KeyboardComponent::KEY_OK:
        connectWiFi();
        break;
      default:
        // Password Visibility Toggle (Touch area near the password field)
        if (p.x >= SCREEN_WIDTH - 60 && p.x < SCREEN_WIDTH - 10 && p.y >= 80 &&
            p.y < 105) {
          _showPassword = !_showPassword;
          drawKeyboard(false);
        }
        break;
      }
    }
  }

  // Continuous Update for GPS Mode
  if (_currentMode == MODE_GPS) {
    drawGPSStatus();
  }
}

void SettingsScreen::handleTouch(int idx) {
  if (idx < 0 || idx >= _settings.size())
    return;

  SettingItem &item = _settings[idx];

  if (item.type == TYPE_VALUE) {
    item.currentOptionIdx++;
    if (item.currentOptionIdx >= item.options.size()) {
      item.currentOptionIdx = 0;
    }
    saveSetting(idx);
    drawList(_scrollOffset, false);
  } else if (item.type == TYPE_TOGGLE) {
    item.checkState = !item.checkState;
    saveSetting(idx);
    drawList(_scrollOffset, false);
  } else if (item.type == TYPE_ACTION) {
    if (item.name == "CLOCK SETTING") {
      _currentMode = MODE_CLOCK;
      loadSettings();
      _ui->getTft()->fillScreen(COLOR_BG);
      _ui->drawStatusBar(true);
      drawList(0, true);
      return;
    } else if (item.name == "MANUAL CLOCK ADJUST") {
      _ui->switchScreen(SCREEN_TIME_SETTINGS);
      return;
    } else if (item.name == "ENGINE HOURS") {
      _currentMode = MODE_ENGINE;
      loadSettings();
      _ui->getTft()->fillScreen(COLOR_BG);
      _ui->drawStatusBar(true);
      drawList(0, true);
    } else if (item.name == "WIFI SETUP") {
      // Start WiFi scan
      _currentMode = MODE_WIFI;
      TFT_eSPI *tft = _ui->getTft();
      tft->fillScreen(COLOR_BG);
      _ui->drawStatusBar(true);

      tft->setTextColor(TFT_WHITE, COLOR_BG);
      tft->setTextDatum(MC_DATUM);
      tft->drawString("Scanning...", SCREEN_WIDTH / 2, 120);

      WiFi.mode(WIFI_STA);
      WiFi.disconnect();
      delay(100);
      WiFi.scanNetworks(true); // Async scan
      _isScanning = true;
      _lastScanAnim = millis();
      _scanAnimStep = 0;
    } else if (item.name == "SYNC WITH CLOUD") {
      // Trigger cloud sync
      TFT_eSPI *tft = _ui->getTft();
      tft->fillScreen(COLOR_BG);
      _ui->drawStatusBar(true);

      tft->setTextColor(TFT_CYAN, COLOR_BG);
      tft->setTextDatum(MC_DATUM);
      tft->setTextSize(1);
      tft->setFreeFont(&Org_01);
      tft->drawString("Syncing...", SCREEN_WIDTH / 2, 100);

      // Get user credentials from NVS
      _prefs.begin("muchrace", false);
      String username = _prefs.getString("username", "");
      String password = _prefs.getString("password", "");
      _prefs.end();

      Serial.print("DEBUG: Checking 'muchrace' namespace. Username: '");
      Serial.print(username);
      Serial.println("'");

      Serial.print("DEBUG: Checking 'muchrace' namespace. Username: '");
      Serial.print(username);
      Serial.println("'");

      if (username.length() == 0) {
        tft->fillRect(0, 80, SCREEN_WIDTH, 80, COLOR_BG);
        tft->setTextColor(TFT_RED, COLOR_BG);
        tft->drawString("No account!", SCREEN_WIDTH / 2, 80);
        tft->drawString("Run Setup again", SCREEN_WIDTH / 2,
                        100); // More helpful message
        delay(2000);
      } else {
        // Use API_URL from config.h
        String apiUrl = API_URL;

        // Perform sync
        bool success = syncManager.syncSettings(
            apiUrl.c_str(), username.c_str(), password.c_str());

        // Show result
        tft->fillRect(0, 80, SCREEN_WIDTH, 100, COLOR_BG);
        if (success) {
          tft->setTextColor(TFT_GREEN, COLOR_BG);
          tft->drawString("Sync Success!", SCREEN_WIDTH / 2, 100);
          delay(2000);
        } else {
          tft->setTextColor(TFT_RED, COLOR_BG);
          tft->drawString("Sync Failed!", SCREEN_WIDTH / 2, 100);
          delay(2000);
        }
      }

      _currentMode = MODE_MAIN;
      loadSettings();
      tft->fillScreen(COLOR_BG);
      _ui->drawStatusBar(true);
      drawList(0, true);
    } else if (item.name == "REMOVE ACCOUNT") {
      // Remove account credentials from storage
      TFT_eSPI *tft = _ui->getTft();
      tft->fillScreen(COLOR_BG);
      _ui->drawStatusBar(true);

      tft->setTextColor(TFT_YELLOW, COLOR_BG);
      tft->setTextDatum(MC_DATUM);
      tft->setTextFont(2); // Use Standard Font 2 (Sans Serif)
      tft->setTextSize(1);
      tft->drawString("Removing Account...", SCREEN_WIDTH / 2, 100);

      // Clear user credentials from NVS
      _prefs.begin("muchrace", false);
      _prefs.clear(); // Clear all user data (including setup_done)
      _prefs.end();

      // Also clear WiFi credentials
      _prefs.begin("wifi", false);
      _prefs.clear();
      _prefs.end();

      delay(1000);

      // Show confirmation
      tft->fillRect(0, 80, SCREEN_WIDTH, 120, COLOR_BG); // Clear larger area
      tft->setTextColor(TFT_GREEN, COLOR_BG);
      tft->setTextFont(2); // Ensure Font 2
      tft->drawString("Account Removed!", SCREEN_WIDTH / 2, 100);
      tft->setTextColor(TFT_WHITE, COLOR_BG);
      tft->drawString("Device will restart", SCREEN_WIDTH / 2, 130);
      tft->drawString("on next setup...", SCREEN_WIDTH / 2, 150);

      delay(3000);

      // Return to settings menu
      delay(3000);

      // Automatic Restart
      ESP.restart();
    } else if (item.name == "RPM SETTING") {
      _currentMode = MODE_RPM;
      loadSettings();
      _ui->getTft()->fillScreen(COLOR_BG);
      _ui->drawStatusBar(true); // Redraw Status Bar
      drawList(0, true);
    } else if (item.name == "GNSS FINE TUNING") {
      _currentMode = MODE_GNSS_CONFIG;
      loadSettings();
      _ui->getTft()->fillScreen(COLOR_BG);
      _ui->drawStatusBar(true);
      drawList(0, true);
    } else if (item.name == "SD CARD TEST") {
      _currentMode = MODE_SD_TEST;
      _ui->setTitle("SD CARD TEST");
      TFT_eSPI *tft = _ui->getTft();
      tft->fillScreen(COLOR_BG);
      _ui->drawStatusBar(true); // Redraw Status Bar

      // Draw "Running..."
      _sdResult = {false, "", "", "", 0, 0}; // Reset
      drawSDTest();

      // Setup static pointer for callback
      static_tft = tft;

      // Run Test (Blocking with Callback)
      _sdResult = sessionManager.runFullTest(sdProgressCallback);

      // Clear static
      static_tft = nullptr;

      // Redraw with results
      tft->fillScreen(COLOR_BG);
      _ui->drawStatusBar(true); // Redraw Status Bar
      drawSDTest();

      /* WiFi Handlers removed
      // ...
      */
    } else if (item.key == "reset_ref") {
      _prefs.begin("laptimer", false);
      _prefs.remove("drag_ref"); // Key for reference run time
      _prefs.end();
      /* WiFi Handlers removed
      // ...
      */
    }
  }
}

void SettingsScreen::drawHeader(String title, uint16_t backColor) {
  TFT_eSPI *tft = _ui->getTft();

  // Draw horizontal line at bottom gap (above the arrow area, like status bar
  // line) tft->drawFastHLine(0, 210, SCREEN_WIDTH, COLOR_SECONDARY);

  // Back Button at bottom-left - Blue filled triangle pointing LEFT
  // Same size as MenuScreen arrows: 20px wide, 8px tall
  // Points: left tip at (5, 225), right-top at (25, 221), right-bottom at (25,
  // 229)
  tft->fillTriangle(5, 225, 25, 217, 25, 233, COLOR_ACCENT);
}

void SettingsScreen::drawGPSStatus(bool force) {
  TFT_eSPI *tft = _ui->getTft();

  if (force) {
    tft->fillScreen(COLOR_BG);
    _ui->drawStatusBar(true);

    // Static Header
    tft->setTextColor(COLOR_HIGHLIGHT, COLOR_BG);
    tft->setTextDatum(TL_DATUM);
    tft->setFreeFont(&Org_01);
    tft->setTextSize(2);
    tft->drawString("<", 10, 25);

    // Static layout elements
    int yStats = 45; // Shifted up from 62 to prevent overlap
    int hStatsHeader = 18;
    tft->fillRect(10, yStats, 160, hStatsHeader, TFT_WHITE);
    tft->setTextColor(TFT_BLACK, TFT_WHITE);
    tft->setTextSize(1);
    tft->drawString("GPS STATUS", 15, yStats + 9);

    int listH = 6 * 15 + 8; // 6 items * 15px + pad
    tft->drawRect(10, yStats + hStatsHeader, 160, listH, TFT_WHITE);

    // Radar
    int cX = 245, cY = 120, r = 55;
    tft->drawCircle(cX, cY, r, TFT_WHITE);
    tft->drawCircle(cX, cY, r * 0.66, COLOR_SECONDARY);
    tft->drawCircle(cX, cY, r * 0.33, COLOR_SECONDARY);
    tft->drawFastHLine(cX - r, cY, 2 * r, COLOR_SECONDARY);
    tft->drawFastVLine(cX, cY - r, 2 * r, COLOR_SECONDARY);

    auto drawCard = [&](String l, int x, int y) {
      tft->fillCircle(x, y, 9, TFT_WHITE);
      tft->setTextColor(TFT_BLACK, TFT_WHITE);
      tft->setTextDatum(MC_DATUM);
      tft->drawString(l, x, y + 1);
    };
    drawCard("N", cX, cY - r);
    drawCard("S", cX, cY + r);
    drawCard("E", cX + r, cY);
    drawCard("W", cX - r, cY);

    // Reset trackers
    _lastSats = -1;
    _lastHdopValue = -1.0;
    _lastLat = 0;
    _lastLon = 0;
    _lastFixed = false;
  }

  // Rate Limiting
  static unsigned long lastGPSDraw = 0;
  if (!force && millis() - lastGPSDraw < 1000)
    return;
  lastGPSDraw = millis();

  extern GPSManager gpsManager;
  int sats = gpsManager.getSatellites();
  double hdop = gpsManager.getHDOP();
  double lat = gpsManager.getLatitude();
  double lon = gpsManager.getLongitude();
  bool fixed = gpsManager.isFixed();

  int yStats = 45; // Match static layout
  int hStatsHeader = 18;
  int hItem = 15;
  int curY = yStats + hStatsHeader + 4;

  auto drawRowValue = [&](String label, String val, String lastVal, int y) {
    if (force || val != lastVal) {
      tft->setTextDatum(ML_DATUM);
      tft->setTextColor(TFT_WHITE, COLOR_BG);
      tft->setFreeFont(&Org_01);
      tft->setTextSize(1);

      // Draw label and separator only on force
      if (force) {
        tft->drawString(label, 15, y + (hItem / 2));
        tft->drawString(":", 60, y + (hItem / 2));
        tft->drawFastHLine(10, y + hItem, 160, COLOR_SECONDARY);
      }

      // Clear and draw value
      tft->fillRect(75, y, 90, hItem - 1, COLOR_BG);
      tft->drawString(val, 75, y + (hItem / 2));
    }
  };

  drawRowValue("GPS", String(sats) + " Sat",
               force ? "" : String(_lastSats) + " Sat", curY);
  curY += hItem;
  drawRowValue("GLO", "- Sat", force ? "" : "- Sat", curY);
  curY += hItem;
  drawRowValue("GAL", "- Sat", force ? "" : "- Sat", curY);
  curY += hItem;
  drawRowValue("BEI", "- Sat", force ? "" : "- Sat", curY);
  curY += hItem;
  drawRowValue("MBN", "A : -", force ? "" : "A : -", curY);
  curY += hItem;
  drawRowValue("HDOP", String(hdop, 2), force ? "" : String(_lastHdopValue, 2),
               curY);

  if (force || lat != _lastLat || lon != _lastLon) {
    int tableBottom = yStats + hStatsHeader + (6 * hItem + 8);
    int yLat = tableBottom + 10;
    tft->fillRect(10, yLat, 200, 40, COLOR_BG);
    tft->setTextColor(TFT_WHITE, COLOR_BG);
    tft->setTextDatum(TL_DATUM);
    tft->drawString("LAT : " + String(lat, 6), 15, yLat);
    tft->drawString("LNG : " + String(lon, 6), 15, yLat + 18);
  }

  // Polar Plot Satellites (Simulated for feedback in code)
  if (fixed != _lastFixed || fixed) {
    int cX = 245, cY = 120, r = 55;
    // Only clear plot area if state changed or we need to redraw blinkers
    if (fixed) {
      // Clear old dots (simplest is clear small r+5 area around dots, but radar
      // is fast) Just redraw radar lines to "clean" old dots
      tft->drawCircle(cX, cY, r * 0.66, COLOR_SECONDARY);
      tft->drawCircle(cX, cY, r * 0.33, COLOR_SECONDARY);
      tft->drawFastHLine(cX - r, cY, 2 * r, COLOR_SECONDARY);
      tft->drawFastVLine(cX, cY - r, 2 * r, COLOR_SECONDARY);

      unsigned long t = millis() / 1000;
      float rad = ((t * 5) % 360) * DEG_TO_RAD;
      tft->fillCircle(cX + cos(rad) * (r * 0.5), cY + sin(rad) * (r * 0.5), 3,
                      TFT_GREEN);
      rad = ((t * 2 + 120) % 360) * DEG_TO_RAD;
      tft->fillCircle(cX + cos(rad) * (r * 0.8), cY + sin(rad) * (r * 0.8), 3,
                      TFT_GREEN);
    }
  }

  _lastSats = sats;
  _lastHdopValue = hdop;
  _lastLat = lat;
  _lastLon = lon;
  _lastFixed = fixed;
}

void SettingsScreen::drawSDTest() {
  TFT_eSPI *tft = _ui->getTft();
  drawHeader(""); // Just draw the back button

  int y = 45; // Move up from 80
  int gap = 22;
  int labelX = 20;
  int valX = 100; // Align values

  tft->setTextDatum(TL_DATUM);
  tft->setTextFont(2);

  if (!_sdResult.success && _sdResult.cardType == "") {
    tft->setTextColor(TFT_WHITE, COLOR_BG);
    tft->drawString("Running SD Test...", labelX, y);
    return;
  }

  if (!_sdResult.success && _sdResult.cardType == "NO CARD") {
    tft->setTextColor(TFT_RED, COLOR_BG);
    tft->drawString("NO SD CARD FOUND!", labelX, y);
    return;
  }

  // Result Display
  auto drawInfo = [&](String label, String value,
                      uint16_t valColor = TFT_WHITE) {
    tft->setTextColor(COLOR_HIGHLIGHT, COLOR_BG);
    tft->drawString(label, labelX, y);
    tft->setTextColor(valColor, COLOR_BG);
    tft->drawString(value, valX, y);
    y += gap;
  };

  drawInfo("Card Type:", _sdResult.cardType);
  drawInfo("Total:", _sdResult.sizeLabel);
  drawInfo("Used:", _sdResult.usedLabel);

  y += 5; // Extra gap before speed test
  tft->setTextColor(COLOR_HIGHLIGHT, COLOR_BG);
  tft->drawString("Speed Test Results:", labelX, y);
  y += gap;

  drawInfo(" Read:", String(_sdResult.readSpeedKBps, 0) + " KB/s", TFT_GREEN);
  drawInfo(" Write:", String(_sdResult.writeSpeedKBps, 0) + " KB/s", TFT_GREEN);
}

void SettingsScreen::drawList(int scrollOffset, bool force) {
  TFT_eSPI *tft = _ui->getTft();

  // Highlight Back Arrow if selected
  uint16_t backColor = (_selectedIdx == -2) ? COLOR_HIGHLIGHT : COLOR_TEXT;

  if (force) {
    _ui->drawStatusBar(true);
    // Draw horizontal line after status bar
    tft->drawFastHLine(0, 20, SCREEN_WIDTH, COLOR_SECONDARY);
  }

  // List (starts at y=30, with gap after status bar)
  int listY = 30;
  int itemH = 20;
  int maxY = 210; // Bottom limit for list

  // Clear list area to prevent ghosts when scrolling
  if (force) {
    tft->fillRect(0, listY, SCREEN_WIDTH, maxY - listY, COLOR_BG);
  }

  int visibleItems = (maxY - listY) / itemH;

  for (int i = 0; i < visibleItems; i++) {
    int idx = scrollOffset + i;
    if (idx >= _settings.size())
      break;

    SettingItem &item = _settings[idx];
    int y = listY + (i * itemH);

    int sIdx = idx;
    bool stateChanged = (sIdx == _selectedIdx || sIdx == _lastSelectedIdx);

    if (force || stateChanged) {
      // Background
      uint16_t bgColor = (sIdx == _selectedIdx) ? TFT_WHITE : COLOR_BG;
      uint16_t txtColor = (sIdx == _selectedIdx) ? TFT_BLACK : COLOR_TEXT;

      // Explicitly clear background
      tft->fillRect(0, y, SCREEN_WIDTH, itemH, bgColor);
      if (sIdx != _selectedIdx) {
        tft->drawFastHLine(0, y + itemH - 1, SCREEN_WIDTH, COLOR_SECONDARY);
      }

      // Name
      tft->setTextDatum(TL_DATUM);
      tft->setTextFont(1);
      tft->setTextSize(1);
      tft->setTextColor(txtColor, bgColor);

      String displayName = item.name;
      if (_currentMode == MODE_GNSS_CONFIG) {
        displayName = String(idx + 1) + ". " + item.name;
      }
      tft->drawString(displayName, 10, y + 5);

      // Value / Toggle / Action
      tft->setTextDatum(TR_DATUM);
      String valText = "";
      if (item.type == TYPE_VALUE) {
        if (item.currentOptionIdx >= 0 &&
            item.currentOptionIdx < item.options.size()) {
          valText = item.options[item.currentOptionIdx];
        }
      } else if (item.type == TYPE_TOGGLE) {
        valText = "";
      } else if (item.type == TYPE_ACTION) {
        valText = ">";
      }
      tft->drawString(valText, SCREEN_WIDTH - 10, y + 5);

      // Custom Render for Toggle
      if (item.type == TYPE_TOGGLE) {
        int swW = 24;
        int swH = 12;
        int swX = SCREEN_WIDTH - swW - 10;
        int swY = y + (itemH - swH) / 2;
        int r = swH / 2;

        if (item.checkState) {
          tft->fillRoundRect(swX, swY, swW, swH, r, TFT_GREEN);
          tft->fillCircle(swX + swW - r, swY + r, r - 2, TFT_WHITE);
        } else {
          tft->fillRoundRect(swX, swY, swW, swH, r, TFT_RED);
          tft->fillCircle(swX + r, swY + r, r - 2, TFT_WHITE);
        }
      }
    }
  }
  _lastSelectedIdx = _selectedIdx;

  // Draw bottom elements AFTER list
  if (force) {
    // Clear bottom area
    tft->fillRect(0, 210, SCREEN_WIDTH, 30, COLOR_BG);

    // Back Button (Left)
    tft->fillTriangle(5, 225, 25, 217, 25, 233, COLOR_ACCENT);

    // Scroll Buttons (Right)
    // Up Arrow (Left of the pair)
    if (scrollOffset > 0) {
      tft->fillTriangle(270, 233, 290, 233, 280, 217, COLOR_ACCENT);
    }

    // Down Arrow (Far Right)
    if (scrollOffset + visibleItems < _settings.size()) {
      tft->fillTriangle(300, 217, 320, 217, 310, 233, COLOR_ACCENT);
    }
  }
}

// WiFi Functions

void SettingsScreen::drawWiFiList(bool force) {
  TFT_eSPI *tft = _ui->getTft();

  drawHeader("WIFI SETUP");

  // List networks
  int listY = 60;
  int itemH = 25;

  for (int i = 0; i < _scanCount && i < 8; i++) {
    String ssid = WiFi.SSID(i);
    int rssi = WiFi.RSSI(i);
    bool isSecure = (WiFi.encryptionType(i) != WIFI_AUTH_OPEN);

    int y = listY + (i * itemH);

    // Background
    uint16_t bgColor = (i == _selectedWiFiIdx) ? TFT_BLUE : COLOR_BG;
    uint16_t txtColor = (i == _selectedWiFiIdx) ? TFT_WHITE : COLOR_TEXT;

    tft->fillRect(0, y, SCREEN_WIDTH, itemH, bgColor);

    // SSID
    tft->setTextColor(txtColor, bgColor);
    tft->setTextDatum(TL_DATUM);
    tft->drawString(ssid, 10, y + 5);

    // Signal strength
    String signal = String(rssi) + "dBm";
    tft->setTextDatum(TR_DATUM);
    tft->drawString(signal, SCREEN_WIDTH - 30, y + 5);

    // Lock icon if secured
    if (isSecure) {
      tft->drawString("*", SCREEN_WIDTH - 10, y + 5);
    }
  }

  if (_scanCount == 0) {
    tft->setTextColor(COLOR_TEXT, COLOR_BG);
    tft->setTextDatum(MC_DATUM);
    tft->drawString("No networks found", SCREEN_WIDTH / 2, 120);
  }
}

void SettingsScreen::drawKeyboard(bool fullRedraw) {
  if (fullRedraw) {
    drawHeader("ENTER PASSWORD");
    // Show SSID
    TFT_eSPI *tft = _ui->getTft();
    tft->setTextColor(COLOR_TEXT, COLOR_BG);
    tft->setTextDatum(TC_DATUM);
    tft->drawString(_targetSSID, SCREEN_WIDTH / 2, 60);
  }

  // Redraw password field
  TFT_eSPI *tft = _ui->getTft();
  tft->fillRect(10, 80, SCREEN_WIDTH - 20, 25, TFT_DARKGREY);
  tft->setTextColor(TFT_WHITE, TFT_DARKGREY);
  tft->setTextDatum(TL_DATUM);
  tft->setTextFont(2);

  String displayPass = "";
  if (_showPassword) {
    displayPass = _enteredPass;
  } else {
    for (int i = 0; i < _enteredPass.length(); i++)
      displayPass += "*";
  }
  tft->drawString(displayPass, 15, 85);

  tft->setTextDatum(TR_DATUM);
  tft->setTextColor(COLOR_HIGHLIGHT, TFT_DARKGREY);
  tft->drawString(_showPassword ? "HIDE" : "SHOW", SCREEN_WIDTH - 15, 85);

  _keyboard.draw(tft, 115, _isUppercase);
}

void SettingsScreen::connectWiFi() {
  TFT_eSPI *tft = _ui->getTft();
  tft->fillScreen(COLOR_BG);
  _ui->drawStatusBar(true);

  tft->setTextColor(TFT_WHITE, COLOR_BG);
  tft->setTextDatum(MC_DATUM);
  tft->drawString("Connecting...", SCREEN_WIDTH / 2, 120);

  // Use core manager for connection
  bool success = wifiManager.connect(_targetSSID.c_str(), _enteredPass.c_str());

  if (success) {
    tft->fillRect(0, 80, SCREEN_WIDTH, 100,
                  COLOR_BG); // Clear "Connecting..." area
    tft->setTextColor(TFT_GREEN, COLOR_BG);
    tft->drawString("Connected!", SCREEN_WIDTH / 2, 120);
    delay(2000);

    // Return to main settings only on success
    _currentMode = MODE_MAIN;
    loadSettings();
    tft->fillScreen(COLOR_BG);
    _ui->drawStatusBar(true);
    drawList(0, true);
  } else {
    tft->fillRect(0, 80, SCREEN_WIDTH, 100,
                  COLOR_BG); // Clear "Connecting..." area
    tft->setTextColor(TFT_RED, COLOR_BG);
    tft->drawString("Failed!", SCREEN_WIDTH / 2, 120);
    delay(2000);

    // Stay in keyboard mode on failure
    tft->fillScreen(COLOR_BG);
    _ui->drawStatusBar(true);
    drawKeyboard(true); // Full redraw to restore keyboard UI
  }
}
