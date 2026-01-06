#include "SettingsScreen.h"
#include "../../config.h"
#include "../../core/SessionManager.h"
#include "../fonts/Org_01.h"
#include "../fonts/Org_01.h"
#include "../fonts/Picopixel.h"
#include "../../core/GPSManager.h"
#include <WiFi.h>


extern SessionManager sessionManager;

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
  _selectedIdx = -1; // Reset selection logic
  _currentMode = MODE_MAIN; // Start at Main
  loadSettings(); // Reload to ensure sync
  
  TFT_eSPI *tft = _ui->getTft();
  tft->fillScreen(COLOR_BG);
  drawList(0);
}

void SettingsScreen::loadSettings() {
  _settings.clear();
  
  if (_currentMode == MODE_MAIN) {
      _settings.push_back({"CLOCK SETTING", TYPE_ACTION});
      
      // Auto Off (Removed)
      // SettingItem autoOff = {"AUTO SCREEN OFF", TYPE_ACTION, "auto_off"};
      // _settings.push_back(autoOff);
      
      // Drag Meter Settings Sub-menu
      _settings.push_back({"DRAG METER SETTINGS", TYPE_ACTION});
      
      // GPS Status Sub-menu
      _settings.push_back({"GPS STATUS", TYPE_ACTION});

      // SD Card Test Sub-menu
      _settings.push_back({"SD CARD TEST", TYPE_ACTION});

      // RPM Settings Sub-menu
      _settings.push_back({"RPM SETTING", TYPE_ACTION});

      // WiFi Scan Sub-menu
      _settings.push_back({"WIFI SCAN", TYPE_ACTION});

      
  } else if (_currentMode == MODE_DRAG) {
      _prefs.begin("laptimer", true); // Read-only
// ... existing drag code ...
      // Reset Reference
      SettingItem resetRef = {"Reset Drag Ref", TYPE_ACTION, "reset_ref"};
      _settings.push_back(resetRef);
      
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
    
    // Terapkan efek langsung
    if (item.name == "Brightness") {
      int duty = 255;
      switch (item.currentOptionIdx) {
      case 0: duty = 64; break; // 25%
      case 1: duty = 128; break; // 50%
      case 2: duty = 192; break; // 75%
      case 3: duty = 255; break; // 100%
      }
      ledcWrite(0, duty); // Saluran 0
    }
  } else if (item.type == TYPE_TOGGLE) {
    _prefs.putBool(item.key.c_str(), item.checkState);
  }
  _prefs.end();
}

void SettingsScreen::update() {
  static unsigned long lastSettingTouch = 0;
  UIManager::TouchPoint p = _ui->getTouchPoint();
  if (p.x == -1)
    return;

  // Tombol Kembali (Area Header 20-60)
  if (p.x < 60 && p.y < 60) {
    if (millis() - lastSettingTouch < 200) return;
    lastSettingTouch = millis();

    // Special handling for GPS/SD/WiFi Mode (Single tap back)
    if (_currentMode == MODE_GPS || _currentMode == MODE_SD_TEST || _currentMode == MODE_WIFI || _currentMode == MODE_WIFI_PASS) {
        if (p.x < 50 && p.y < 50) { // Back Button
              if (_currentMode == MODE_WIFI_PASS) {
                  // Back to WiFi List
                  _currentMode = MODE_WIFI;
                  _ui->getTft()->fillScreen(COLOR_BG);
                  _ui->drawStatusBar(true);
                  drawWiFiScan();
                  return;
              }
              
              // Back to Main for GPS, SD_TEST, WIFI
              _currentMode = MODE_MAIN;
              loadSettings();
              _ui->getTft()->fillScreen(COLOR_BG);
              _ui->drawStatusBar(true);
              drawList(0);
              return;
         }

        // Mode Specific Touch
        if (millis() - lastSettingTouch > 200) {
            lastSettingTouch = millis();
            
            if (_currentMode == MODE_WIFI) {
                // Check List Touches
                // List starts Y=70, ItemH=25
                int listY = 70;
                int itemH = 25;
                if (p.y > listY) {
                    int idx = (p.y - listY) / itemH;
                    if (idx >= 0 && idx < _scanCount && idx < 8) { // Limit 8 items
                        _targetSSID = WiFi.SSID(idx);
                        
                        // Check if Open Network
                        if (WiFi.encryptionType(idx) == WIFI_AUTH_OPEN) {
                             // Direct Connect? Or just empty pass?
                             _enteredPass = "";
                             // Auto connect logic or show "Connect" button?
                             // Let's go to keyboard anyway but user just hits Enter
                        }
                        
                        _enteredPass = "";
                        _currentMode = MODE_WIFI_PASS;
                        _ui->getTft()->fillScreen(COLOR_BG);
                        _ui->drawStatusBar(true);
                        drawKeyboard(); // Initial Draw
                    }
                }
            } else if (_currentMode == MODE_WIFI_PASS) {
                // Keyboard Touch Logic
                // Simple Grid Detection
                // Keyboard Area: Y=120 to 240
                if (p.y >= 120) {
                     // Key Width is approx 320/10 = 32px
                     int keyW = 32; 
                     int keyH = 30;
                     int row = (p.y - 120) / keyH;
                     int col = p.x / keyW;
                     
                     char key = 0;
                     // Mappings
                     const char* r0 = "qwertyuiop"; // 10
                     const char* r1 = "asdfghjkl";  // 9 (offset?)
                     const char* r2 = "zxcvbnm";    // 7
                     
                     if (row == 0) { // QWERTY...
                         if (col < 10) key = r0[col];
                     } else if (row == 1) { // ASDF...
                         // Indent 16px? If so, map x
                         // Simplified: just align left for now or detect closest
                         if (col < 9) key = r1[col];
                     } else if (row == 2) { // ZXCV...
                         // col 0: Shift (Toggle?)
                         // col 1-7: Letters
                         // col 8-9: Backspace
                         if (col >= 1 && col <= 7) key = r2[col-1];
                         else if (col >= 8) key = 8; // BS
                     } else if (row == 3) { // Space/Enter
                         // 0-1: 123
                         // 2-6: Space (' ')
                         // 7-9: Enter (13)
                         if (col >= 2 && col <= 6) key = ' ';
                         else if (col >= 7) key = 13; // Enter
                     }
                     
                     if (key > 0) {
                         if (key == 8) { // Backspace
                             if (_enteredPass.length() > 0) _enteredPass.remove(_enteredPass.length()-1);
                         } else if (key == 13) { // Enter
                             // CONNECT
                             _ui->getTft()->fillRect(0, 80, 320, 30, COLOR_BG);
                             _ui->getTft()->drawString("Connecting...", 160, 95);
                             
                             WiFi.begin(_targetSSID.c_str(), _enteredPass.c_str());
                             
                             unsigned long t = millis();
                             bool connected = false;
                             while (millis() - t < 5000) { // 5s Timeout for UI feedback
                                 if (WiFi.status() == WL_CONNECTED) {
                                     connected = true;
                                     break;
                                 }
                                 delay(100);
                             }
                             
                             _ui->getTft()->fillRect(0, 80, 320, 30, COLOR_BG); // Clear msg
                             if (connected) {
                                 _ui->getTft()->setTextColor(TFT_GREEN, COLOR_BG);
                                 _ui->getTft()->drawString("CONNECTED! IP: " + WiFi.localIP().toString(), 160, 95);
                             } else {
                                 _ui->getTft()->setTextColor(TFT_RED, COLOR_BG);
                                 _ui->getTft()->drawString("FAILED!", 160, 95);
                             }
                             delay(2000); // Show result
                             
                             // Return to List
                             _currentMode = MODE_WIFI;
                             _ui->getTft()->fillScreen(COLOR_BG);
                             _ui->drawStatusBar(true);
                             drawWiFiScan();
                             return;
                             
                         } else {
                             _enteredPass += key;
                         }
                         
                         // Redraw Input Box
                         drawKeyboard(); 
                     }
                }
            }
        }
        return;
    }

    if (_selectedIdx == -2) {
       // Back pressed
       if (_currentMode == MODE_MAIN) {
           _ui->switchScreen(SCREEN_MENU);
       } else {
           // Go back to Main
           _currentMode = MODE_MAIN;
           loadSettings();
           _ui->getTft()->fillScreen(COLOR_BG); // Clear
           _ui->drawStatusBar(true);
           drawList(0);
       }
    } else {
       _selectedIdx = -2;
       drawList(_scrollOffset);
    }
    return;
  }

  // Daftar Sentuh
  int listY = 55; // Header at 60, but we can start slightly higher or overlap line slightly? 
  // Let's stick to 60 for safety, margin is tight.
  // User wants 10 lines. 240 - 60 = 180. 180 / 10 = 18.
  listY = 60;
  int itemH = 18; // 18px * 10 = 180px. Exactly fits bottom.


    // Debounce Check
    if (millis() - lastSettingTouch > 200) {
        // Only handle standard List Touch for list-based modes
        if (_currentMode == MODE_MAIN || _currentMode == MODE_DRAG || _currentMode == MODE_RPM) {
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
                          drawList(_scrollOffset);
                      }
                 }
            }
        }
        
        lastSettingTouch = millis();
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
      drawList(_scrollOffset);
  } else if (item.type == TYPE_TOGGLE) {
      item.checkState = !item.checkState;
      saveSetting(idx);
      drawList(_scrollOffset);
  } else if (item.type == TYPE_ACTION) {
      if (item.name == "CLOCK SETTING") {
          _ui->switchScreen(SCREEN_TIME_SETTINGS);
          return; // STOP EXECUTION HERE
      } else if (item.name == "RPM SETTING") {
          _currentMode = MODE_RPM;
          loadSettings();
          _ui->getTft()->fillScreen(COLOR_BG);
          _ui->drawStatusBar(true); // Redraw Status Bar
          drawList(0);
      } else if (item.name == "GPS STATUS") {
          _currentMode = MODE_GPS;
          _ui->getTft()->fillScreen(COLOR_BG);
          _ui->drawStatusBar(true); // Redraw Status Bar
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
      } else if (item.name == "WIFI SCAN") {
          // Show "Scanning..." first
          _ui->getTft()->fillScreen(COLOR_BG);
          _ui->drawStatusBar(true);
          drawHeader("WIFI SCANNER");
          _ui->getTft()->setTextDatum(MC_DATUM);
          _ui->getTft()->drawString("Scanning...", SCREEN_WIDTH/2, 120);
          
          _scanCount = WiFi.scanNetworks();
          _currentMode = MODE_WIFI;
          
          _ui->getTft()->fillScreen(COLOR_BG);
          _ui->drawStatusBar(true); 
          drawWiFiScan();
      } else if (item.key == "reset_ref") {
          _prefs.begin("laptimer", false);
          _prefs.remove("drag_ref"); // Key for reference run time
          _prefs.end();
      }
  }
}

void SettingsScreen::drawHeader(String title, uint16_t backColor) {
    TFT_eSPI *tft = _ui->getTft();
    
    // Header
    tft->setTextColor(backColor, COLOR_BG);
    tft->setTextDatum(TL_DATUM);
    tft->setFreeFont(&Org_01);
    tft->setTextSize(2); 
    tft->drawString("<", 10, 25);
    
    tft->setTextColor(COLOR_TEXT, COLOR_BG);
    tft->setTextDatum(TC_DATUM);
    tft->setTextSize(1);
    tft->setTextFont(2);
    tft->drawString(title, SCREEN_WIDTH / 2, 40);
    
    tft->drawFastHLine(0, 60, SCREEN_WIDTH, COLOR_SECONDARY);
}

void SettingsScreen::drawGPSStatus() {
    TFT_eSPI *tft = _ui->getTft();
    // Background is cleared in update() loop if needed? 
    // No, SettingsScreen::update calls this continuously.
    // We should probably clear screen ONCE on entry, then just overwrite text.
    // But SettingsScreen logic for other tabs uses fillRect to clear items.
    // For GPS Mode, we probably want to minimize flicker.
    // Let's assume background is black (COLOR_BG).
    
    // Header is NOT used in the reference provided by user (custom aesthetics).
    // Reference has "12/17/2025" logic at top.
    // But we should keep the "BACK" button logic consistent?
    // User asked "UI nya seperti ini" (Like this).
    // I will try to replicate the "Look" but keep the navigation standard (Back arrow).
    
    // 1. Draw Back Arrow (Standard Position aligned with typical header area)
    tft->setTextColor(COLOR_HIGHLIGHT, COLOR_BG);
    tft->setTextDatum(TL_DATUM);
    tft->setFreeFont(&Org_01);
    tft->setTextSize(2);
    tft->drawString("<", 10, 25); // Move to Y=25 to match other screens

    extern GPSManager gpsManager;
    
    // --- LAYOUT ---
    // Shifted down to avoid overlap with top status bar (if any)
    int startX = 35; // Indent for boxes to clear arrow area if side-by-side, or just general layout
    
    // Y Offsets
    // Date/Time Removed as per user request to avoid overlap
    // Centering Box with Polar Plot (cY=120)
    // Box Height ~116. Target Center 120. Start Y = 120 - 58 = 62.
    int yStats = 62; 
    int hStatsHeader = 18;
    int hItem = 15; // Compact to 15px
    
    // Data Preparation
    // String dateStr = gpsManager.getDateString(); // Removed
    int sats = gpsManager.getSatellites();
    double hdop = gpsManager.getHDOP();
    double lat = gpsManager.getLatitude();
    double lon = gpsManager.getLongitude();
    
    tft->setFreeFont(&Org_01);
    tft->setTextSize(1);
    
    // 4. GPS Status Group
    tft->fillRect(10, yStats, 160, hStatsHeader, TFT_WHITE);
    tft->setTextColor(TFT_BLACK, TFT_WHITE);
    tft->drawString("GPS STATUS", 15, yStats + 9);
    
    // List Box (Outline)
    int listH = 6 * hItem + 8; // 6 items
    tft->drawRect(10, yStats + hStatsHeader, 160, listH, TFT_WHITE);
    
    // Items
    tft->setTextColor(TFT_WHITE, COLOR_BG);
    int curY = yStats + hStatsHeader + 4;
    
    // Helper to draw row
    auto drawRow = [&](String label, String val, int y) {
        tft->setTextDatum(ML_DATUM);
        tft->drawString(label, 15, y + (hItem/2));
        tft->drawString(":", 60, y + (hItem/2));
        tft->drawString(val, 75, y + (hItem/2));
        tft->drawFastHLine(10, y + hItem, 160, COLOR_SECONDARY); 
    };

    drawRow("GPS", String(sats) + " Sat", curY); curY += hItem;
    drawRow("GLO", "- Sat", curY); curY += hItem;
    drawRow("GAL", "- Sat", curY); curY += hItem;
    drawRow("BEI", "- Sat", curY); curY += hItem;
    drawRow("MBN", "A : -", curY); curY += hItem;
    drawRow("HDOP", String(hdop, 2), curY);
    
    // 5. Lat/Lon (Bottom Left)
    // Position relative to table bottom to ensure no overlap
    int tableBottom = yStats + hStatsHeader + (6 * hItem + 8);
    int yLat = tableBottom + 10;
    
    tft->setTextColor(TFT_WHITE, COLOR_BG);
    tft->drawString("LAT : " + String(lat, 6), 15, yLat);
    tft->drawString("LNG : " + String(lon, 6), 15, yLat + 18);

    
    
    // --- RIGHT SIDE: POLAR PLOT ---
    // Center of Right Area
    // Screen 320. Left takes ~170. 
    // Center X = 170 + (150/2) = 245
    // Center Y = 120
    
    int cX = 245;
    int cY = 120;
    int r = 55; // Reduced from 70 to 55
    
    // Draw Radar Circles
    // Outer
    tft->drawCircle(cX, cY, r, TFT_WHITE);
    // Mid
    tft->drawCircle(cX, cY, r*0.66, COLOR_SECONDARY);
    // Inner
    tft->drawCircle(cX, cY, r*0.33, COLOR_SECONDARY);

    
    // Crosshairs
    tft->drawFastHLine(cX - r, cY, 2*r, COLOR_SECONDARY);
    tft->drawFastVLine(cX, cY - r, 2*r, COLOR_SECONDARY);
    
    // Labels N E S W
    // Draw Small Circles with Letters
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
    
    // Simulated Satellite Dots (Just for visual fidelity to reference if 'Fix' is present)
    if (gpsManager.isFixed()) {
        // Randomly consistent spots? 
        // Or based on actual time to rotate them?
        // Let's create a few pseudo-random spots based on time
        // Just 2-3 dots to show "active" status
        unsigned long t = millis() / 1000;
        
        int s1_ang = (t * 5) % 360;
        int s1_r = r * 0.5;
        float rad = s1_ang * DEG_TO_RAD;
        tft->fillCircle(cX + cos(rad)*s1_r, cY + sin(rad)*s1_r, 3, TFT_GREEN);
        
        int s2_ang = (t * 2 + 120) % 360;
        int s2_r = r * 0.8;
        rad = s2_ang * DEG_TO_RAD;
        tft->fillCircle(cX + cos(rad)*s2_r, cY + sin(rad)*s2_r, 3, TFT_GREEN);
    }
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

void SettingsScreen::drawWiFiScan() {
    TFT_eSPI *tft = _ui->getTft();
    drawHeader("WIFI SCANNER");
    
    tft->setTextColor(TFT_WHITE, COLOR_BG);
    tft->setTextDatum(TC_DATUM);
    tft->setFreeFont(&Org_01);
    tft->setTextSize(1);
    
    tft->drawString("Scanning Networks...", SCREEN_WIDTH/2, 100);
    
    // Use cached scan result
    int n = _scanCount;
    
    // Clear "Scanning..." (if any residue, though we clear screen before calling this)
    // tft->fillRect(0, 80, SCREEN_WIDTH, 40, COLOR_BG);
    
    tft->setTextDatum(TL_DATUM);
    int y = 70;
    int hItem = 25; // Larger touch targets for WiFi list
    
    if (n == 0) {
        tft->drawString("No networks found", 20, y);
    } else {
        tft->drawString(String(n) + " Networks Found:", 20, y);
        y += 25;
        
        // List up to 8 networks to fit screen
        int limit = (n > 8) ? 8 : n;
        
        for (int i = 0; i < limit; ++i) {
            String ssid = WiFi.SSID(i);
            int rssi = WiFi.RSSI(i);
            String enc = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "Open" : "Sec";
            
            // Format: SSID (RSSI) [Enc]
            String line = ssid.substring(0, 15); // Truncate long SSIDs
            
            tft->setTextColor(TFT_WHITE, COLOR_BG);
            tft->drawString(line, 20, y);
            
            tft->setTextDatum(TR_DATUM);
            String info = String(rssi) + "dB " + enc;
            tft->drawString(info, SCREEN_WIDTH - 20, y);
            tft->setTextDatum(TL_DATUM);
            
            y += hItem;
        }
    }
}

void SettingsScreen::drawList(int scrollOffset) {
  TFT_eSPI *tft = _ui->getTft();

  // Determine Title
  String title = "SETTINGS";
  if (_currentMode == MODE_DRAG) title = "DRAG SETTINGS";
  else if (_currentMode == MODE_RPM) title = "RPM SETTING";
  
  // Highlight Back Arrow if selected
  uint16_t backColor = (_selectedIdx == -2) ? COLOR_HIGHLIGHT : COLOR_TEXT;
  
  drawHeader(title, backColor);

  // List
  int listY = 60;
  int itemH = 18; // 10 Lines
  
  for (int i = 0; i < _settings.size(); i++) {
    SettingItem &item = _settings[i];
    int y = listY + (i * itemH);
    int sIdx = i;

    // Background
    uint16_t bgColor = (sIdx == _selectedIdx) ? TFT_WHITE : COLOR_BG;
    uint16_t txtColor = (sIdx == _selectedIdx) ? TFT_BLACK : COLOR_TEXT;

    if (sIdx == _selectedIdx) {
        tft->fillRect(0, y, SCREEN_WIDTH, itemH, bgColor);
    } else {
        // Explicitly clear background for unselected items to prevent artifacts
        tft->fillRect(0, y, SCREEN_WIDTH, itemH, COLOR_BG);
        tft->drawFastHLine(0, y + itemH - 1, SCREEN_WIDTH, COLOR_SECONDARY);
    }

    // Name
    tft->setTextDatum(TL_DATUM);
    // Use Font 1 (GLCD) scaled to 2 = ~16px height? Or Size 1 = 8px?
    // User asked "kecilkan lagi". Font 2 was ~16px.
    // If I use Font 1 Size 1, it's 8px. Legible but small.
    // If I use Font 1 Size 2, it's 16px (similar to Font 2).
    // Let's use Font 1 Size 1.5? No. 
    // Let's use Font 1 Size 1 but centered nicely? Or Font 2 scaled down? No scaling for bitmaps.
    // Let's try Font 1 (Default) Size 2. It is slightly more compact horizontally than Font 2.
    // AND it fits 18px height better than Font 2 which has large vertical padding.
    tft->setTextFont(1); 
    tft->setTextSize(1); // Size 1 is 8px. With 18px line, 5px pad top/bottom. Very clean.
    // Wait, Size 1 might be TOO small for "Settings".
    // Let's try Size 2 (16px). 1px pad top/bottom. Tight but maximizes use.
    // Actually, let's stick to Size 1 for "Max Density" compliance? 
    // "kecilkan lgi" -> make it smaller again.
    // I will use Size 1. It is very safe for 18px line.
    
    tft->setTextColor(txtColor, bgColor);
    tft->drawString(item.name, 10, y + 5); // Centered vertically (18-8)/2 = 5
    
    // Value / Toggle / Action
    tft->setTextDatum(TR_DATUM);
    String valText = "";
    if (item.type == TYPE_VALUE) {
        if (item.currentOptionIdx >= 0 && item.currentOptionIdx < item.options.size()) {
            valText = item.options[item.currentOptionIdx];
        }
    } else if (item.type == TYPE_TOGGLE) {
        valText = ""; // Drawn graphically below
    } else if (item.type == TYPE_ACTION) {
        valText = ">";
    }
    
    tft->drawString(valText, SCREEN_WIDTH - 10, y + 5);
    
    // Custom Render for Toggle
    if (item.type == TYPE_TOGGLE) {
        int swW = 24; // Smaller Switch
        int swH = 12;
        int swX = SCREEN_WIDTH - swW - 10; 
        int swY = y + (itemH - swH) / 2;
        int r = swH / 2; 

        if (item.checkState) {
          // ON (Green)
          tft->fillRoundRect(swX, swY, swW, swH, r, TFT_GREEN);
          tft->fillCircle(swX + swW - r, swY + r, r - 2, TFT_WHITE);
        } else {
          // OFF (Red)
          tft->fillRoundRect(swX, swY, swW, swH, r, TFT_RED);
          tft->fillCircle(swX + r, swY + r, r - 2, TFT_WHITE);
        }
    }
  }
}

void SettingsScreen::drawKeyboard() {
    TFT_eSPI *tft = _ui->getTft();
    
    // Header
    drawHeader("PASS: " + _targetSSID);
    
    // Input Box
    tft->drawRect(10, 70, 300, 40, TFT_WHITE);
    tft->setTextColor(TFT_WHITE, COLOR_BG);
    tft->setTextDatum(ML_DATUM);
    tft->setTextSize(2);
    tft->setFreeFont(&Picopixel); // Or standard
    tft->setTextFont(1); // Standard legible
    
    // Mask password?
    String masked = "";
    // for (int i=0; i<_enteredPass.length(); i++) masked += "*"; 
    // Show plain text for easier typing on tiny screen
    tft->drawString(_enteredPass + "_", 20, 90);
    
    // Keyboard Base Y = 120
    int yBase = 120;
    int keyW = 32;
    int keyH = 30;
    
    tft->setTextSize(1);
    tft->setTextDatum(MC_DATUM);
    
    const char* r0 = "qwertyuiop"; 
    const char* r1 = "asdfghjkl";
    const char* r2 = "zxcvbnm";
    
    auto drawKey = [&](char c, int r, int col, int w=1) {
        int x = col * keyW;
        int y = yBase + (r * keyH);
        
        // Key BG
        tft->drawRect(x+1, y+1, (keyW*w)-2, keyH-2, COLOR_SECONDARY);
        
        // Char
         tft->setTextColor(TFT_WHITE, COLOR_BG);
        if (c == 8) tft->drawString("<-", x + (keyW*w)/2, y + keyH/2);
        else if (c == 13) tft->drawString("OK", x + (keyW*w)/2, y + keyH/2);
        else if (c == ' ') tft->drawString("_", x + (keyW*w)/2, y + keyH/2); // Space
        else tft->drawChar(c, x + (keyW*w)/2, y + keyH/2, 2); 
    };
    
    // Row 0
    // QWERTYUIOP (10 keys)
    for(int i=0; i<10; i++) drawKey(r0[i], 0, i);
    
    // Row 1
    // ASDFGHJKL (9 keys)
    for(int i=0; i<9; i++) drawKey(r1[i], 1, i);
    
    // Row 2
    // ZXCVBNM + BS
    // Space shift? 
    // Let's do: Shift(dum) Z X C V B N M BS
    // But simplified: 1 (Z) to 7 (M)
    // Shift is complicating. Just lowercase for now.
    
    // drawKey('^', 2, 0); // Shift placeholder
    for(int i=0; i<7; i++) drawKey(r2[i], 2, i+1);
    drawKey(8, 2, 8, 2); // BS (Double width)
    
    // Row 3
    // 123? Space OK
    // drawKey('1', 3, 0); 
    // drawKey('2', 3, 1);
    drawKey(' ', 3, 2, 5); // Space (5 width)
    drawKey(13, 3, 7, 3); // Enter (3 width)
}

