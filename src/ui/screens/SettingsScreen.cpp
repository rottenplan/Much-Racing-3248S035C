#include "SettingsScreen.h"
#include "../../config.h"
#include "../../core/SessionManager.h"
#include "../fonts/Org_01.h"
#include "../fonts/Org_01.h"
#include "../fonts/Picopixel.h"
#include "../../core/GPSManager.h"
#include "../../core/WiFiManager.h"

extern SessionManager sessionManager;
extern WiFiManager wifiManager;

// Static pointer for callback
static TFT_eSPI* static_tft = nullptr;

void sdProgressCallback(int percent, String status) {
    if (!static_tft) return;
    
    // Draw Status
    static_tft->setTextColor(TFT_WHITE, COLOR_BG);
    static_tft->setTextDatum(TC_DATUM); // Top Center
    static_tft->setTextSize(1);
    static_tft->setFreeFont(&Org_01);
    
    // Clear text area (approx y=100-120)
    static_tft->fillRect(0, 100, 320, 30, COLOR_BG); 
    static_tft->drawString(status, 320/2, 110);
    
    // Draw Bar
    int barW = 200;
    int barH = 10;
    int barX = (320 - barW) / 2;
    int barY = 140;
    
    // Outline
    static_tft->drawRect(barX-1, barY-1, barW+2, barH+2, TFT_WHITE);
    
    // Fill
    int fillW = (barW * percent) / 100;
    static_tft->fillRect(barX, barY, fillW, barH, TFT_GREEN);
    static_tft->fillRect(barX + fillW, barY, barW - fillW, barH, COLOR_BG); // Clear rest
}

void SettingsScreen::onShow() {
  _selectedIdx = -1; 
  _lastSelectedIdx = -1;
  _currentMode = MODE_MAIN; // Start at Main
  loadSettings(); // Reload to ensure sync
  
  TFT_eSPI *tft = _ui->getTft();
  tft->fillScreen(COLOR_BG);
  drawList(0, true);
}

void SettingsScreen::loadSettings() {
  _settings.clear();
  
  if (_currentMode == MODE_MAIN) {
      _settings.push_back({"CLOCK SETTING", TYPE_ACTION});
      
      _prefs.begin("laptimer", true);
      
      // Power Save (Auto Off)
      SettingItem powerSave = {"POWER SAVE", TYPE_VALUE, "power_save"};
      powerSave.options = {"1 min", "5 min", "10 min", "30 min", "Never"};
      powerSave.currentOptionIdx = _prefs.getInt("power_save", 1); // Default 5 min
      _settings.push_back(powerSave);
      
      // Brightness
      SettingItem brightness = {"BRIGHTNESS", TYPE_VALUE, "brightness"};
      brightness.options = {"10%", "20%", "30%", "40%", "50%", "60%", "70%", "80%", "90%", "100%"};
      brightness.currentOptionIdx = _prefs.getInt("brightness", 9); // Default 100%
      _settings.push_back(brightness);
      
      // Units
      SettingItem units = {"UNITS", TYPE_VALUE, "units"};
      units.options = {"Metric (km/h)", "Imperial (mph)"};
      units.currentOptionIdx = _prefs.getInt("units", 0); // Default Metric
      _settings.push_back(units);
      
      _prefs.end();
      
      // GPS Status Sub-menu
      _settings.push_back({"GPS STATUS", TYPE_ACTION});

      // SD Card Test Sub-menu
      _settings.push_back({"SD CARD TEST", TYPE_ACTION});

      // RPM Settings Sub-menu
      _settings.push_back({"RPM SETTING", TYPE_ACTION});
      
      // Engine Hours
      _settings.push_back({"ENGINE HOURS", TYPE_ACTION});
      
      // WiFi Setup
      _settings.push_back({"WIFI SETUP", TYPE_ACTION});





      
  } else if (_currentMode == MODE_ENGINE) {
      _prefs.begin("laptimer", true);
      
      // Engine Hours (Read-only display)
      SettingItem engineHours = {"TOTAL HOURS", TYPE_VALUE, "engine_hours"};
      float hours = _prefs.getFloat("engine_hours", 0.0);
      engineHours.options = {String(hours, 1) + " hrs"};
      engineHours.currentOptionIdx = 0;
      _settings.push_back(engineHours);
      
      _prefs.end();
  } else if (_currentMode == MODE_RPM) {
      _prefs.begin("laptimer", true);
      
      // Pulses Per Revolution (PPR)
      SettingItem ppr = {"PULSE PER REV", TYPE_VALUE, "rpm_ppr"};
      ppr.options = {"1.0", "0.5 (1p/2r)", "2.0 (Default)", "3.0 (3p/1r)", "4.0 (4p/1r)"};
      ppr.currentOptionIdx = _prefs.getInt("rpm_ppr_idx", 2); 
      if (ppr.currentOptionIdx < 0 || ppr.currentOptionIdx >= ppr.options.size()) ppr.currentOptionIdx = 2;
      
      _settings.push_back(ppr);
      
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
      switch(item.currentOptionIdx) {
          case 0: ms = 60000; break;      // 1 min
          case 1: ms = 300000; break;     // 5 min
          case 2: ms = 600000; break;     // 10 min
          case 3: ms = 1800000; break;    // 30 min
          case 4: ms = 0; break;          // Never
      }
      _ui->setAutoOff(ms);
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
          for(int i=0; i<_scanAnimStep; i++) dots += ".";
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
    if (millis() - lastSettingTouch < 200) return;
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
        loadSettings();
        _ui->getTft()->fillScreen(COLOR_BG);
        _ui->drawStatusBar(true);
        drawList(0, true);
    }
    return;
  }

  // Daftar Sentuh (now starts at y=30, with gap after status bar)
  int listY = 30;
  int itemH = 20; // Match drawList


    // Debounce Check
    if (millis() - lastSettingTouch > 200) {
        // Only handle standard List Touch for list-based modes
        if (_currentMode == MODE_MAIN || _currentMode == MODE_RPM) {
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
            
            int keyW = 28;
            int keyH = 25; 
            int startY = 115; 
            
            // QWERTY keyboard rows
            String rows[] = {"1234567890", "QWERTYUIOP", "ASDFGHJKL", "ZXCVBNM"};
            
            for (int row = 0; row < 4; row++) {
                String keys = rows[row];
                int numKeys = keys.length();
                int totalW = numKeys * keyW;
                int startX = (SCREEN_WIDTH - totalW) / 2;
                
                for (int col = 0; col < numKeys; col++) {
                    int x = startX + (col * keyW);
                    int y = startY + (row * keyH);
                    
                    if (p.x >= x && p.x < x + keyW && p.y >= y && p.y < y + keyH) {
                        char c = keys[col];
                        if (!_isUppercase && c >= 'A' && c <= 'Z') {
                            c = c + ('a' - 'A'); // Convert to lowercase
                        }
                        _enteredPass += c;
                        // NO FULL REDRAW - Only update password field
                        drawKeyboard(false); 
                        return;
                    }
                }
            }
            
            int specialY = startY + (4 * keyH);
            int shiftW = 45;
            int delW = 45;
            int spaceW = 80;
            int okW = 55;
            int gap = 5;
            int totalW = shiftW + delW + spaceW + okW + (3 * gap);
            int startX = (SCREEN_WIDTH - totalW) / 2;
            
            int shiftX = startX;
            int delX = shiftX + shiftW + gap;
            int spaceX = delX + delW + gap;
            int okX = spaceX + spaceW + gap;
            
            // Password Visibility Toggle (Touch area near the password field)
            // Field is at y=80, h=25. Let's add a button area on the right
            if (p.x >= SCREEN_WIDTH - 60 && p.x < SCREEN_WIDTH - 10 && p.y >= 80 && p.y < 105) {
                _showPassword = !_showPassword;
                drawKeyboard(false); // Update only the password field
                return;
            }

            // SHIFT
            if (p.x >= shiftX && p.x < shiftX + shiftW && p.y >= specialY && p.y < specialY + keyH) {
                _isUppercase = !_isUppercase;
                // NO FULL REDRAW - Only update SHIFT and char display
                drawKeyboard(false);
            }
            // Del
            else if (p.x >= delX && p.x < delX + delW && p.y >= specialY && p.y < specialY + keyH) {
                 if (_enteredPass.length() > 0) {
                    _enteredPass.remove(_enteredPass.length() - 1); 
                    // NO FULL REDRAW
                    drawKeyboard(false);
                }
            }
            // Space
            else if (p.x >= spaceX && p.x < spaceX + spaceW && p.y >= specialY && p.y < specialY + keyH) {
                _enteredPass += " ";
                // NO FULL REDRAW
                drawKeyboard(false);
            }
            // OK
            else if (p.x >= okX && p.x < okX + okW && p.y >= specialY && p.y < specialY + keyH) {
                connectWiFi();
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
          _ui->switchScreen(SCREEN_TIME_SETTINGS);
          return; // STOP EXECUTION HERE
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
      } else if (item.name == "RPM SETTING") {
          _currentMode = MODE_RPM;
          loadSettings();
          _ui->getTft()->fillScreen(COLOR_BG);
          _ui->drawStatusBar(true); // Redraw Status Bar
          drawList(0, true);
      } else if (item.name == "GPS STATUS") {
          _currentMode = MODE_GPS;
          _ui->getTft()->fillScreen(COLOR_BG);
          _ui->drawStatusBar(true); // Redraw Status Bar
          drawGPSStatus(true);
      } else if (item.name == "SD CARD TEST") {
          _currentMode = MODE_SD_TEST;
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
      }
  }
}

void SettingsScreen::drawHeader(String title, uint16_t backColor) {
    TFT_eSPI *tft = _ui->getTft();
    
    // Draw horizontal line at bottom gap (above the arrow area, like status bar line)
    // tft->drawFastHLine(0, 210, SCREEN_WIDTH, COLOR_SECONDARY);
    
    // Back Button at bottom-left - Blue filled triangle pointing LEFT
    // Same size as MenuScreen arrows: 20px wide, 8px tall
    // Points: left tip at (5, 225), right-top at (25, 221), right-bottom at (25, 229)
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
        int yStats = 62; 
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
        tft->drawCircle(cX, cY, r*0.66, COLOR_SECONDARY);
        tft->drawCircle(cX, cY, r*0.33, COLOR_SECONDARY);
        tft->drawFastHLine(cX - r, cY, 2*r, COLOR_SECONDARY);
        tft->drawFastVLine(cX, cY - r, 2*r, COLOR_SECONDARY);
        
        auto drawCard = [&](String l, int x, int y) {
            tft->fillCircle(x, y, 9, TFT_WHITE);
            tft->setTextColor(TFT_BLACK, TFT_WHITE);
            tft->setTextDatum(MC_DATUM);
            tft->drawString(l, x, y+1);
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
    if (!force && millis() - lastGPSDraw < 1000) return;
    lastGPSDraw = millis();

    extern GPSManager gpsManager;
    int sats = gpsManager.getSatellites();
    double hdop = gpsManager.getHDOP();
    double lat = gpsManager.getLatitude();
    double lon = gpsManager.getLongitude();
    bool fixed = gpsManager.isFixed();
    
    int yStats = 62;
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
                tft->drawString(label, 15, y + (hItem/2));
                tft->drawString(":", 60, y + (hItem/2));
                tft->drawFastHLine(10, y + hItem, 160, COLOR_SECONDARY); 
            }
            
            // Clear and draw value
            tft->fillRect(75, y, 90, hItem-1, COLOR_BG);
            tft->drawString(val, 75, y + (hItem/2));
        }
    };

    drawRowValue("GPS", String(sats) + " Sat", force ? "" : String(_lastSats) + " Sat", curY); curY += hItem;
    drawRowValue("GLO", "- Sat", force ? "" : "- Sat", curY); curY += hItem;
    drawRowValue("GAL", "- Sat", force ? "" : "- Sat", curY); curY += hItem;
    drawRowValue("BEI", "- Sat", force ? "" : "- Sat", curY); curY += hItem;
    drawRowValue("MBN", "A : -", force ? "" : "A : -", curY); curY += hItem;
    drawRowValue("HDOP", String(hdop, 2), force ? "" : String(_lastHdopValue, 2), curY);
    
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
            // Clear old dots (simplest is clear small r+5 area around dots, but radar is fast)
            // Just redraw radar lines to "clean" old dots
            tft->drawCircle(cX, cY, r*0.66, COLOR_SECONDARY);
            tft->drawCircle(cX, cY, r*0.33, COLOR_SECONDARY);
            tft->drawFastHLine(cX - r, cY, 2*r, COLOR_SECONDARY);
            tft->drawFastVLine(cX, cY - r, 2*r, COLOR_SECONDARY);

            unsigned long t = millis() / 1000;
            float rad = ((t * 5) % 360) * DEG_TO_RAD;
            tft->fillCircle(cX + cos(rad)*(r*0.5), cY + sin(rad)*(r*0.5), 3, TFT_GREEN);
            rad = ((t * 2 + 120) % 360) * DEG_TO_RAD;
            tft->fillCircle(cX + cos(rad)*(r*0.8), cY + sin(rad)*(r*0.8), 3, TFT_GREEN);
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
    drawHeader("SD CARD TEST");
    
    int y = 80;
    int gap = 25;
    
    tft->setTextDatum(TL_DATUM);
    tft->setTextColor(COLOR_TEXT, COLOR_BG);
    tft->setTextFont(2);
    
    if (!_sdResult.success && _sdResult.cardType == "") {
         tft->drawString("Running Test...", 20, y);
         return;
    }
    
    if (!_sdResult.success && _sdResult.cardType == "NO CARD") {
         tft->setTextColor(TFT_RED, COLOR_BG);
         tft->drawString("NO SD CARD FOUND!", 20, y);
         return;
    }
    
    // Result Display
    String typeStr = "Type: " + _sdResult.cardType;
    tft->drawString(typeStr, 20, y);
    
    y += gap;
    String sizeStr = "Total: " + _sdResult.sizeLabel;
    tft->drawString(sizeStr, 20, y);

    y += gap;
    String usedStr = "Used: " + _sdResult.usedLabel;
    tft->drawString(usedStr, 20, y);
    
    y += gap + 5;
    tft->drawString("Speed Test:", 20, y);
    
    y += gap;
    tft->setTextColor(TFT_GREEN, COLOR_BG); // Highlights
    String readStr = "Read: " + String(_sdResult.readSpeedKBps, 0) + " KB/s";
    tft->drawString(readStr, 40, y);
    
    y += gap;
    String writeStr = "Write: " + String(_sdResult.writeSpeedKBps, 0) + " KB/s";
    tft->drawString(writeStr, 40, y);
}

void SettingsScreen::drawList(int scrollOffset, bool force) {
  TFT_eSPI *tft = _ui->getTft();

  // Determine Title
  String title = "SETTINGS";
  if (_currentMode == MODE_RPM) title = "RPM SETTING";
  else if (_currentMode == MODE_ENGINE) title = "ENGINE HOURS";
  
  // Highlight Back Arrow if selected
  uint16_t backColor = (_selectedIdx == -2) ? COLOR_HIGHLIGHT : COLOR_TEXT;
  
  if (force) {
    _ui->drawStatusBar(true);
    // Draw horizontal line after status bar
    tft->drawFastHLine(0, 20, SCREEN_WIDTH, COLOR_SECONDARY);
  }

  // List (starts at y=30, with gap after status bar)
  int listY = 30;
  int itemH = 20; // Slightly taller rows for better touch targets
  int maxY = 210; // Leave room for bottom area (line at 210, arrow below)
  
  for (int i = 0; i < _settings.size(); i++) {
    SettingItem &item = _settings[i];
    int y = listY + (i * itemH);
    
    // Stop if we'd draw beyond the bottom area
    if (y + itemH > maxY) break;
    
    int sIdx = i;

    // Only draw if forced OR if this item's selection state changed
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
      tft->drawString(item.name, 10, y + 5); 
      
      // Value / Toggle / Action
      tft->setTextDatum(TR_DATUM);
      String valText = "";
      if (item.type == TYPE_VALUE) {
          if (item.currentOptionIdx >= 0 && item.currentOptionIdx < item.options.size()) {
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
  
  // Draw bottom elements AFTER list (so they're on top)
  if (force) {
    // Clear bottom area
    tft->fillRect(0, 210, SCREEN_WIDTH, 30, COLOR_BG);
    // Horizontal line separator
    // tft->drawFastHLine(0, 210, SCREEN_WIDTH, COLOR_SECONDARY);
    // Blue triangle arrow pointing LEFT (20px wide, 16px tall)
    tft->fillTriangle(5, 225, 25, 217, 25, 233, COLOR_ACCENT);
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
  TFT_eSPI *tft = _ui->getTft();
  
  if (fullRedraw) {
      drawHeader("ENTER PASSWORD");
      // Show SSID
      tft->setTextColor(COLOR_TEXT, COLOR_BG);
      tft->setTextDatum(TC_DATUM);
      tft->drawString(_targetSSID, SCREEN_WIDTH / 2, 60);
  }
  
  // ALWAYS Redraw entered password (it changes)
  tft->fillRect(10, 80, SCREEN_WIDTH - 20, 25, TFT_DARKGREY);
  tft->setTextColor(TFT_WHITE, TFT_DARKGREY);
  tft->setTextDatum(TL_DATUM);
  tft->setTextFont(2); 
  
  String displayPass = "";
  if (_showPassword) {
      displayPass = _enteredPass;
  } else {
      for (int i = 0; i < _enteredPass.length(); i++) displayPass += "*";
  }
  tft->drawString(displayPass, 15, 85);

  // Draw Visibility Toggle Button (EYE Icon replacement with text)
  tft->setTextDatum(TR_DATUM);
  tft->setTextColor(COLOR_HIGHLIGHT, TFT_DARKGREY);
  tft->drawString(_showPassword ? "HIDE" : "SHOW", SCREEN_WIDTH - 15, 85);
  
  int keyW = 28;
  int keyH = 25; 
  int startY = 115; 
  
  // QWERTY Keyboard Layout
  String rows[] = {
    "1234567890",
    "QWERTYUIOP",
    "ASDFGHJKL",
    "ZXCVBNM"
  };

  // If fullRedraw OR shift state changed, we must redraw the character keys
  // Actually, character keys change case, so we MUST redraw them if!fullRedraw too if SHIFT was pressed?
  // Let's optimize: Redraw keys ONLY if fullRedraw is true OR if SHIFT was just toggled.
  // Since we don't track "just toggled", let's redraw keys if fullRedraw is true.
  // BUT the user wants to see lowercase characters when SHIFT is off.
  // So if SHIFT toggles, we must redraw all keys.
  
  // [NEW LOGIC]: If fullRedraw is false, we are here because of a key press or SHIFT toggle.
  // If SHIFT was toggled, we must redraw keys.
  // How do we know if SHIFT was toggled? 
  // Let's just always redraw keys if we want to be safe, but skip fillScreen. 
  // Redrawing 30 keys without clearing screen is much faster than clearing whole screen.
  
  if (fullRedraw || true) { // Always redraw keys for now to ensure case is correct, but without clearing screen
      tft->setTextFont(1); // Small font for keys
      tft->setTextSize(1);
      
      for (int row = 0; row < 4; row++) {
        String keys = rows[row];
        int numKeys = keys.length();
        int totalW = numKeys * keyW;
        int startX = (SCREEN_WIDTH - totalW) / 2;
        
        for (int col = 0; col < numKeys; col++) {
          int x = startX + (col * keyW);
          int y = startY + (row * keyH);
          
          if (fullRedraw) tft->drawRect(x, y, keyW, keyH, TFT_WHITE);
          else tft->fillRect(x+1, y+1, keyW-2, keyH-2, COLOR_BG); // Clear center
          
          tft->setTextColor(TFT_WHITE, COLOR_BG);
          tft->setTextDatum(MC_DATUM);
          
          char c = keys[col];
          if (!_isUppercase && c >= 'A' && c <= 'Z') {
              c = c + ('a' - 'A');
          }
          tft->drawString(String(c), x + keyW/2, y + keyH/2);
        }
      }
      
      // Special keys
      int specialY = startY + (4 * keyH);
      int shiftW = 45;
      int delW = 45;
      int spaceW = 80;
      int okW = 55;
      int gap = 5;
      int totalW = shiftW + delW + spaceW + okW + (3 * gap);
      int startX = (SCREEN_WIDTH - totalW) / 2;
      
      int shiftX = startX;
      int delX = shiftX + shiftW + gap;
      int spaceX = delX + delW + gap;
      int okX = spaceX + spaceW + gap;

      // SHIFT
      uint16_t shiftColor = _isUppercase ? COLOR_HIGHLIGHT : COLOR_BG;
      uint16_t shiftTxtColor = _isUppercase ? TFT_BLACK : TFT_WHITE;
      
      tft->fillRect(shiftX, specialY, shiftW, keyH, shiftColor);
      if (!_isUppercase) tft->drawRect(shiftX, specialY, shiftW, keyH, TFT_WHITE);
      
      tft->setTextColor(shiftTxtColor, shiftColor);
      tft->setTextDatum(MC_DATUM);
      tft->drawString("SHFT", shiftX + shiftW/2, specialY + keyH/2);

      if (fullRedraw) {
          // Backspace
          tft->drawRect(delX, specialY, delW, keyH, TFT_WHITE);
          tft->setTextColor(TFT_WHITE, COLOR_BG);
          tft->drawString("DEL", delX + delW/2, specialY + keyH/2);

          // Space
          tft->drawRect(spaceX, specialY, spaceW, keyH, TFT_WHITE);
          tft->drawString("SPACE", spaceX + spaceW/2, specialY + keyH/2);
          
          // Connect
          tft->fillRect(okX, specialY, okW, keyH, TFT_GREEN);
          tft->setTextColor(TFT_BLACK, TFT_GREEN);
          tft->drawString("OK", okX + okW/2, specialY + keyH/2);
      }
  }
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
    tft->setTextColor(TFT_GREEN, COLOR_BG);
    tft->drawString("Connected!", SCREEN_WIDTH / 2, 160);
    delay(2000);

    // Return to main settings only on success
    _currentMode = MODE_MAIN;
    loadSettings();
    tft->fillScreen(COLOR_BG);
    _ui->drawStatusBar(true);
    drawList(0);
  } else {
    tft->setTextColor(TFT_RED, COLOR_BG);
    tft->drawString("Failed!", SCREEN_WIDTH / 2, 160);
    delay(2000);

    // Stay in keyboard mode on failure
    tft->fillScreen(COLOR_BG);
    _ui->drawStatusBar(true);
    drawKeyboard(true); // Full redraw to restore keyboard UI
  }
}






