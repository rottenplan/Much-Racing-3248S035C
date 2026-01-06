#include "LapTimerScreen.h"
#include "../../core/GPSManager.h"
#include "../../core/SessionManager.h"
#include "../fonts/Org_01.h"
#include <algorithm> // Untuk min_element

extern GPSManager gpsManager;
extern SessionManager sessionManager;

// Konstanta untuk Tata Letak UI
#define STATUS_BAR_HEIGHT 20
#define LIST_ITEM_HEIGHT 30

// Tentukan Area Tombol
#define STOP_BTN_Y 200 // Dipindahkan KE BAWAH untuk jarak yang lebih baik
#define STOP_BTN_H 35

void LapTimerScreen::onShow() {
  _lastUpdate = 0;
  _lastTouchTime = millis(); // Prevent ghost touch on entry
  _isRecording = false; // Mulai tidak merekam
  _finishSet = false;
  _lapCount = 0;
  _state = STATE_TRACK_SELECT; // Mulai di Pilihan Track
  _raceMode = MODE_BEST; // Default mode
  _bestLapTime = 0;
  _lapTimes.clear();
  _listScroll = 0;
  _menuSelectionIdx = -1;
  
  loadTracks();
  
  // Start in Sub-Menu
  _state = STATE_MENU;
  TFT_eSPI *tft = _ui->getTft();
  // tft->fillScreen(COLOR_BG); 
  drawMenu();
}

void LapTimerScreen::loadTracks() {
  _tracks.clear();
  
  Track sonoma;
  sonoma.name = "Sonoma Raceway";
  sonoma.lat = 38.1613;
  sonoma.lon = -122.4561;
  sonoma.configs.push_back({"Default"});
  sonoma.configs.push_back({"Alternative long"});
  sonoma.configs.push_back({"Long"});
  sonoma.configs.push_back({"Short"});
  
  _tracks.push_back(sonoma);
  // Tambahkan lebih banyak jika diperlukan
}


void LapTimerScreen::update() {
  UIManager::TouchPoint p = _ui->getTouchPoint();
  bool touched = (p.x != -1);



  if (_state == STATE_MENU) {
      if (touched) {
          // 1. Back/Home (< 60x60)
          if (p.x < 60 && p.y < 60) {
              if (millis() - _lastTouchTime < 200) return;
              _lastTouchTime = millis();

              if (_menuSelectionIdx == -2) {
                   _ui->switchScreen(SCREEN_MENU);
              } else {
                   _menuSelectionIdx = -2;
                   drawMenu();
              }
              return;
          }
          
          // 2. Button Logic
          // startY = 55, btnHeight = 35, gap = 8
          int startY = 55;
          int btnHeight = 35; 
          int gap = 8;
          int x = (SCREEN_WIDTH - 240) / 2;
          int btnWidth = 240;

          // Check if X is within button width (centered)
          if (p.x > x && p.x < x + btnWidth) {
              int touchedIdx = -1;

              // Check Y coordinates for each button
              for(int i=0; i<4; i++) {
                  int btnY = startY + (i * (btnHeight + gap));
                  if (p.y > btnY && p.y < btnY + btnHeight) {
                      touchedIdx = i;
                      break;
                  }
              }

              if (touchedIdx != -1) {
                  // Debounce Check (e.g. 200ms)
                  if (millis() - _lastTouchTime < 200) {
                      return; 
                  }
                  _lastTouchTime = millis();

                  // Double Tap Logic
                  if (_menuSelectionIdx == touchedIdx) {
                      // Second tap on SAME button -> Execute Action
                      
                      // Execute Action
                      if (touchedIdx == 0) { // Select Track
                          _selectedTrackIdx = 0; 
                          _selectedConfigIdx = -1; // Reset selection
                          _state = STATE_TRACK_SELECT;
                          _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT - STATUS_BAR_HEIGHT, COLOR_BG);
                          drawTrackSelect();
                      } else if (touchedIdx == 1) { // Race Screen
                          _state = STATE_RACING;
                          _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT - STATUS_BAR_HEIGHT, COLOR_BG);
                          drawRacingStatic();
                          drawRacing();
                      } else if (touchedIdx == 2) { // Session Summary
                          _state = STATE_SUMMARY;
                          _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT - STATUS_BAR_HEIGHT, COLOR_BG);
                          drawSummary();
                      } else if (touchedIdx == 3) { // Create Custom Track
                          _tempStartLat = 0;
                          _tempStartLon = 0;
                          _tempSplitCount = 0;
                          _state = STATE_CREATE_TRACK;
                          _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT - STATUS_BAR_HEIGHT, COLOR_BG);
                          drawCreateTrack();
                      }
                  } else {
                      // First tap (or different button) -> Highlight Only
                      _menuSelectionIdx = touchedIdx;
                      drawMenu(); 
                  }
                  
                  return;
              }
          }
      }
      
  } else if (_state == STATE_TRACK_SELECT) {
      if (touched) {
          // 1. Back Button (< 60x60)
          if (p.x < 60 && p.y < 60) {
               if (millis() - _lastTouchTime < 200) return;
               _lastTouchTime = millis();
               
               if (_selectedConfigIdx == -2) {
                   // Execute Back
                   _state = STATE_MENU;
                   _menuSelectionIdx = -1;
                   _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT - STATUS_BAR_HEIGHT, COLOR_BG);
                   drawMenu();
                   _ui->drawStatusBar();
               } else {
                   _selectedConfigIdx = -2;
                   drawTrackSelect();
               }
               return;
          }
          
          
          // 3. Config List Selection
          int startY = 80;
          int itemH = 30;
          if (p.y > startY && p.y < 200) {
              // Debounce
              if (millis() - _lastTouchTime < 200) return;
              _lastTouchTime = millis();

              int idx = (p.y - startY) / itemH;
              if (_selectedTrackIdx < _tracks.size()) {
                  Track &t = _tracks[_selectedTrackIdx];
                  if (idx >= 0 && idx < t.configs.size()) {
                       // Double Tap Logic
                       if (idx == _selectedConfigIdx) {
                           // Second Tap: Execute
                           _currentTrackName = t.name; 
                           _state = STATE_SUMMARY; 
                           _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT - STATUS_BAR_HEIGHT, COLOR_BG);
                           drawSummary();
                           _ui->drawStatusBar();
                       } else {
                           // First Tap: Select/Highlight
                           _selectedConfigIdx = idx;
                           drawTrackSelect();
                       }
                       return;
                  }
              }
          }
      }
  } else if (_state == STATE_SUMMARY) {

    // --- LOGIKA STATUS RINGKASAN ---
    if (touched) {
      // 1. KEMBALI/MENU (Kiri Atas)
      if (p.x < 70 && p.y < 70) { 
        if (millis() - _lastTouchTime < 200) return;
        _lastTouchTime = millis();

        if (_menuSelectionIdx == -2) {
            // Back to Sub-Menu
            _state = STATE_MENU;
            _menuSelectionIdx = -1;
            _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT - STATUS_BAR_HEIGHT, COLOR_BG);
            drawMenu();
            _ui->drawStatusBar();
        } else {
            _menuSelectionIdx = -2;
            drawSummary();
        }
        return;
      }
      
      // 2. Restart / New Session (Tap anywhere else?)
      // For now, let's keep it simple: Tap bottom right to go to Track Select?
      if (p.x > 200 && p.y > 200) {
          _state = STATE_TRACK_SELECT;
          _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT - STATUS_BAR_HEIGHT, COLOR_BG);
          drawTrackSelect();
          _ui->drawStatusBar();
          return;
      }
    }

  } else if (_state == STATE_CREATE_TRACK) {
      if (touched) {
          // 4 Quadrants
          bool left = (p.x < SCREEN_WIDTH/2);
          bool top = (p.y < SCREEN_HEIGHT/2);
          
          if (top && left) {
              // Set Start/Finish (Top Left)
              if (gpsManager.isFixed()) {
                  _tempStartLat = gpsManager.getLatitude();
                  _tempStartLon = gpsManager.getLongitude();
                  // Visual Feedback?
                  _ui->getTft()->fillCircle(20, 20, 5, TFT_GREEN);
              }
          } else if (top && !left) {
              // Cancel (Top Right)
              // Back to Sub-Menu
              _state = STATE_MENU;
              _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT - STATUS_BAR_HEIGHT, COLOR_BG);
              drawMenu();
              _ui->drawStatusBar();
              return;
          } else if (!top && left) {
              // Add Split (Bottom Left)
              if (gpsManager.isFixed() && _tempSplitCount < 4) {
                  _tempSplitCount++;
                  // Simpan split di mana? Belum ada struct split di TrackConfig
                  // Untuk sekarang hanya counter visual
                  drawCreateTrack(); // Redraw counter
              }
          } else if (!top && !left) {
              // Save Track (Bottom Right)
              if (_tempStartLat != 0) {
                   // Create Custom Track Entry
                   _finishLat = _tempStartLat;
                   _finishLon = _tempStartLon;
                   _finishSet = true;
                   
                   _currentTrackName = "Custom Track";
                   
                   // Start Racing Immediately
                   _state = STATE_RACING;
                   _isRecording = true;
                   sessionManager.startSession();
                   _lapCount = 1;
                   _currentLapStart = millis();
                   _bestLapTime = 0;
                   _lapTimes.clear();
                   
                   _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT - STATUS_BAR_HEIGHT, COLOR_BG);
                   drawRacingStatic();
                   drawRacing();
                   return;
              }
          }
           // Debounce touch?
           delay(200);
      }
      
  } else if (_state == STATE_NO_GPS) {
      if (touched) {
          // Retry or Continue
          // Retry: Check GPS again
          if (p.x < SCREEN_WIDTH/2) {
             // Retry
             _ui->getTft()->fillScreen(TFT_BLACK);
             if (gpsManager.isFixed()) {
                 _state = STATE_TRACK_SELECT;
                 drawTrackSelect();
             } else {
                 drawNoGPS(); // Redraw (maybe add "Still No Fix" msg)
             }
          } else {
             // Continue Anyway (Manual/Custom)
             _state = STATE_TRACK_SELECT;
             _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT - STATUS_BAR_HEIGHT, COLOR_BG);
             drawTrackSelect();
          }
      }
  } else {
    // --- LOGIKA STATUS BALAPAN ---
    // Sentuh: BERHENTI/SELESAI
    if (touched) {
       // 1. Back Button (Top Left)
       if (p.x < 60 && p.y < 60) {
           if (millis() - _lastTouchTime < 200) return;
           _lastTouchTime = millis();
           
           if (_menuSelectionIdx == -2) {
               // Execute Stop Logic
               if (sessionManager.isLogging()) {
                   String dateStr = gpsManager.getDateString() + " " + gpsManager.getTimeString();
                   sessionManager.appendToHistoryIndex("Track Session", dateStr, _lapCount, _bestLapTime);
               }
               sessionManager.stopSession();
               _isRecording = false;
               
               // Return to Menu
               _state = STATE_MENU;
                _menuSelectionIdx = -1;
                _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT - STATUS_BAR_HEIGHT, COLOR_BG);
                drawMenu();
           } else {
               _menuSelectionIdx = -2;
               // Highlight Arrow
               TFT_eSPI *tft = _ui->getTft();
               tft->setTextColor(COLOR_HIGHLIGHT, TFT_BLACK);
               tft->setTextDatum(TL_DATUM);
               tft->setFreeFont(&Org_01);
               tft->setTextSize(2);
               tft->drawString("<", 10, 25);
           }
           return;
       }
      // Tombol Berhenti (Bawah Tengah) - Tetap ada
      if (p.x > 80 && p.x < 240 && p.y > STOP_BTN_Y) {
        _state = STATE_SUMMARY;

        // Simpan Riwayat
        if (sessionManager.isLogging()) {
          // Dapatkan string Tanggal/Waktu?
          String dateStr =
              gpsManager.getDateString() + " " + gpsManager.getTimeString();
          sessionManager.appendToHistoryIndex("Track Session", dateStr,
                                              _lapCount, _bestLapTime);
        }

        sessionManager.stopSession();
        _isRecording = false;
        _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT - STATUS_BAR_HEIGHT, COLOR_BG);
        drawSummary();
        return;
      }

      // Mode Toggle (Kiri < 80)
      if (p.x < 80 && p.y > 60 && p.y < 200) {
          // Toggle Mode
          if (_raceMode == MODE_BEST) _raceMode = MODE_LAST;
          else if (_raceMode == MODE_LAST) _raceMode = MODE_PREDICTIVE;
          else _raceMode = MODE_BEST;
          
          // Clear Bottom Area for refresh
          _ui->getTft()->fillRect(0, 200, SCREEN_WIDTH, 40, TFT_BLACK); // Clear bottom black mostly
          // Redraw White Box for Label
          _ui->getTft()->fillRect(0, 200, 100, 40, TFT_WHITE);
          
          // Re-draw static parts if needed, but dynamic handles labels now.
          return;
      }

      // Set Selesai Manual (jika belum diset)
      if (!_finishSet && gpsManager.isFixed() && p.y < 200) {
        _finishLat = gpsManager.getLatitude();
        _finishLon = gpsManager.getLongitude();
        _finishSet = true;
        _ui->getTft()->fillCircle(SCREEN_WIDTH - 20, 35, 5, TFT_GREEN);
      }
    }

    // Logic
    if (_finishSet)
      checkFinishLine();

    // Pencatatan (1Hz atau 10Hz?)
    if (sessionManager.isLogging() && (millis() - _lastUpdate > 100)) {
      String data = String(millis()) + "," +
                    String(gpsManager.getLatitude(), 6) + "," +
                    String(gpsManager.getLongitude(), 6) + "," +
                    String(gpsManager.getSpeedKmph()) + "," +
                    String(gpsManager.getSatellites());
      sessionManager.logData(data);
    }

    // UI Update (10Hz)
    if (millis() - _lastUpdate > 100) {
      drawRacing();
      _ui->drawStatusBar();
      _lastUpdate = millis();
    }
  }
}

// --- PEMBANTU PENGGAMBARAN ---

void LapTimerScreen::drawMenu() {
    TFT_eSPI *tft = _ui->getTft();
    
    // Header
    tft->drawFastHLine(0, 20, SCREEN_WIDTH, COLOR_SECONDARY);
    tft->setTextDatum(TC_DATUM);
    tft->setFreeFont(&Org_01);
    tft->setTextSize(2);
    tft->setTextColor(COLOR_TEXT, COLOR_BG);
    tft->drawString("LAP TIMER", SCREEN_WIDTH / 2, 25);
    
    // Back Arrow
    tft->setTextDatum(TL_DATUM);
    tft->drawString("<", 10, 25);
    
    // Buttons
    int startY = 55;
    int btnHeight = 35; 
    int btnWidth = 240;
    int gap = 8;
    int x = (SCREEN_WIDTH - btnWidth) / 2;
    
    const char* menuItems[] = {
        "SELECT TRACK",
        "RACE SCREEN",
        "SESSION SUMMARY",
        "CREATE CUSTOM TRACK"
    };
    
    for (int i = 0; i < 4; i++) {
        int y = startY + (i * (btnHeight + gap));
        
        // Determine Color based on selection
        uint16_t btnColor = (i == _menuSelectionIdx) ? TFT_RED : TFT_DARKGREY;
        
        tft->fillRoundRect(x, y, btnWidth, btnHeight, 5, btnColor);
        tft->setTextColor(TFT_WHITE, btnColor);
        tft->setTextDatum(MC_DATUM);
        tft->drawString(menuItems[i], SCREEN_WIDTH / 2, y + btnHeight/2 + 2);
    }
    _ui->drawStatusBar();
}

void LapTimerScreen::drawTrackSelect() {
  TFT_eSPI *tft = _ui->getTft();
  
  // 1. Header (Select Track)
  tft->drawFastHLine(0, 20, SCREEN_WIDTH, COLOR_SECONDARY); // Garis status
  
  tft->setTextDatum(TC_DATUM);
  tft->setFreeFont(&Org_01); // Use standard font
  tft->setTextSize(2); // Slightly larger
  tft->setTextColor(COLOR_TEXT, COLOR_BG);
  tft->drawString("Select Track", SCREEN_WIDTH / 2, 25); // Just below status bar
  
  // Back Arrow
  tft->setTextDatum(TL_DATUM);
  tft->setFreeFont(&Org_01); // Ensure font is set
  tft->setTextSize(2);       // Ensure size is set
  tft->drawString("<", 10, 25);

  // Separator
  tft->drawFastHLine(0, 50, SCREEN_WIDTH, COLOR_TEXT);
  
  if (_tracks.empty()) return;
  
  Track &t = _tracks[_selectedTrackIdx];
  
  // 2. Track Name
  tft->setTextDatum(TC_DATUM);
  tft->setTextSize(1);
  tft->setTextFont(2); // Standard font for name
  tft->drawString(t.name, SCREEN_WIDTH / 2, 60);
  
  // 3. Config List
  int startY = 80;
  int itemH = 30;
  
  tft->setTextDatum(TL_DATUM);
  for (int i = 0; i < t.configs.size(); i++) {
      int y = startY + (i * itemH);
      
      // Highlight selection
      if (i == _selectedConfigIdx) {
          tft->fillRoundRect(10, y, SCREEN_WIDTH - 20, itemH, 5, COLOR_HIGHLIGHT);
          tft->setTextColor(COLOR_BG, COLOR_HIGHLIGHT);
      } else {
          tft->setTextColor(COLOR_TEXT, COLOR_BG);
      }
      
      // Draw Config Name
      tft->drawString(t.configs[i].name, 20, y + 5); // Added slight Y offset for center
  }
  _ui->drawStatusBar();
  
  // 4. Create Custom Track (Bottom) - REMOVED (Moved to Sub-Menu)
}

void LapTimerScreen::drawCreateTrack() {
    TFT_eSPI *tft = _ui->getTft();
    // Background Black
    tft->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT - STATUS_BAR_HEIGHT, TFT_BLACK);
    tft->drawFastHLine(0, 20, SCREEN_WIDTH, COLOR_SECONDARY); // Separator
    
    int cx = SCREEN_WIDTH / 2;
    int cy = SCREEN_HEIGHT / 2;
    
    tft->setTextColor(TFT_WHITE, TFT_BLACK);
    tft->setFreeFont(&Org_01);
    tft->setTextSize(1); // Standard
    
    // Top Left: Set Start/Finish
    tft->setTextDatum(TL_DATUM);
    tft->drawString("Set Start/Finish", 10, 30); // Adjusted to 30
    tft->drawString("Line", 10, 50); // Adjusted to 50
    if (_tempStartLat != 0) {
        tft->setTextColor(TFT_GREEN, TFT_BLACK);
        tft->drawString("SET", 10, 70); // Adjusted to 70
        tft->setTextColor(TFT_WHITE, TFT_BLACK);
    }

    // Top Right: Cancel
    tft->setTextDatum(TR_DATUM);
    tft->drawString("Cancel", SCREEN_WIDTH - 10, 30); // Adjusted to 30
    
    // Bottom Left: Add Split
    tft->setTextDatum(BL_DATUM);
    tft->drawString("Add Split " + String(_tempSplitCount + 1), 10, SCREEN_HEIGHT - 40);
    tft->drawString("of 4", 10, SCREEN_HEIGHT - 20);
    
    // Bottom Right: Save Track
    tft->setTextDatum(BR_DATUM);
    tft->drawString("Save Track", SCREEN_WIDTH - 10, SCREEN_HEIGHT - 20);
    
    // Separators (Optional, looks better without hard lines for this pure text look?)
    // Reference image has no visible lines, just text quadrants.
    // But we can add soft guide lines.
    // tft->drawFastVLine(cx, 0, SCREEN_HEIGHT, TFT_DARKGREY);
    // tft->drawFastHLine(0, cy, SCREEN_WIDTH, TFT_DARKGREY);
    
    _ui->drawStatusBar();
}

void LapTimerScreen::drawNoGPS() {
    TFT_eSPI *tft = _ui->getTft();
    tft->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT - STATUS_BAR_HEIGHT, TFT_BLACK);
    tft->drawFastHLine(0, 20, SCREEN_WIDTH, COLOR_SECONDARY);
    
    tft->setTextColor(TFT_RED, TFT_BLACK);
    tft->setFreeFont(&Org_01);
    tft->setTextDatum(MC_DATUM);
    tft->setTextSize(2);
    tft->drawString("NO GPS FIX", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 40);
    
    tft->setTextColor(TFT_WHITE, TFT_BLACK);
    tft->setTextSize(1);
    tft->drawString("Cannot locate nearby tracks.", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
    
    // Buttons
    int btnY = SCREEN_HEIGHT - 60;
    
    // Retry (Left)
    tft->fillRoundRect(20, btnY, 130, 40, 5, TFT_DARKGREY);
    tft->setTextColor(TFT_WHITE, TFT_DARKGREY);
    tft->drawString("Retry", 85, btnY + 22);
    
    // Continue (Right)
    tft->fillRoundRect(170, btnY, 130, 40, 5, TFT_DARKGREY);
    tft->setTextColor(TFT_WHITE, TFT_DARKGREY);
    tft->drawString("Continue", 235, btnY + 22);
    
    _ui->drawStatusBar();
}



void LapTimerScreen::drawSummary() {
  TFT_eSPI *tft = _ui->getTft();
  tft->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT - STATUS_BAR_HEIGHT, TFT_BLACK);
  tft->drawFastHLine(0, 20, SCREEN_WIDTH, COLOR_SECONDARY);

  // 1. Back Arrow (Kiri Atas)
  tft->setTextColor(TFT_WHITE, TFT_BLACK);
  tft->setTextDatum(TL_DATUM);
  tft->setFreeFont(&Org_01); // Standard Font
  tft->setTextSize(2);       // Standard Standard Size
  tft->drawString("<", 10, 25); 

  // 2. Best Lap (Kiri Atas - Green)
  int bestIdx = -1;
  unsigned long bestTime = 0;
  
  // Cari Best Lap Index
  if (!_lapTimes.empty()) {
      bestTime = _lapTimes[0];
      bestIdx = 0;
      for(int i=1; i<_lapTimes.size(); i++) {
          if (_lapTimes[i] < bestTime) {
              bestTime = _lapTimes[i];
              bestIdx = i;
          }
      }
  }

  int leftX = 20;
  int topY = 60;

  tft->setTextDatum(TL_DATUM);
  tft->setTextColor(TFT_GREEN, TFT_BLACK); // Green Text
  tft->setTextFont(2);
  tft->setTextSize(1);
  String label = "Best lap #" + String(bestIdx + 1); // Lap Number (1-based)
  tft->drawString(label, leftX, topY);

  // Time Value (Large Green)
  char buf[32];
  if (bestIdx != -1) {
    int ms = bestTime % 1000;
    int s = (bestTime / 1000) % 60;
    int m = (bestTime / 60000);
    sprintf(buf, "%d:%02d.%02d", m, s, ms / 10);
  } else {
    sprintf(buf, "--:--.--");
  }

  tft->setTextFont(6); // Large Font
  tft->setTextSize(1);
  tft->drawString(buf, leftX, topY + 25);

  // 3. Theoretical Best (Kiri Bawah - White)
  int theoryY = 160;
  tft->setTextColor(TFT_WHITE, TFT_BLACK);
  tft->setTextFont(2);
  tft->drawString("Theoretical best", leftX, theoryY);
  
  // Mockup Theoretical: Best - 200ms
  unsigned long theoryTime = (bestTime > 200) ? (bestTime - 200) : bestTime;
  if (bestIdx != -1) {
    int ms = theoryTime % 1000;
    int s = (theoryTime / 1000) % 60;
    int m = (theoryTime / 60000);
    sprintf(buf, "%d:%02d.%02d", m, s, ms / 10);
  } else {
      sprintf(buf, "--:--.--");
  }
  
  tft->setTextFont(6);
  tft->drawString(buf, leftX, theoryY + 25);

  // 4. List (Kanan - Next Best Laps)
  // Sort indices by time (skipping actual best)
  std::vector<std::pair<unsigned long, int>> sortedLaps;
  for(int i=0; i<_lapTimes.size(); i++) {
      if (i == bestIdx) continue; // Skip the absolute best
      sortedLaps.push_back({_lapTimes[i], i + 1}); // Store time and Lap#
  }
  std::sort(sortedLaps.begin(), sortedLaps.end()); // Sort by time

  int listX = SCREEN_WIDTH / 2 + 40;
  int listY = 40;
  int itemsToShow = 3;

  tft->setTextFont(4); // Medium Font
  tft->setTextDatum(TR_DATUM); // Align Right logic? Or TL of list item?
  // Let's draw TL based.
  
  for(int i=0; i<itemsToShow && i<sortedLaps.size(); i++) {
      unsigned long t = sortedLaps[i].first;
      int lapNum = sortedLaps[i].second;
      
      int ms = t % 1000;
      int s = (t / 1000) % 60;
      int m = (t / 60000);
      sprintf(buf, "%02d. %d:%02d.%02d", lapNum, m, s, ms / 10);
      
      tft->setTextColor(TFT_WHITE, TFT_BLACK);
      tft->setTextDatum(TR_DATUM); // Rata Kanan Screen
      tft->drawString(buf, SCREEN_WIDTH - 10, listY + (i * 40));
  }
  
  _ui->drawStatusBar();
}

void LapTimerScreen::drawRacingStatic() {
  TFT_eSPI *tft = _ui->getTft();
  tft->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT - STATUS_BAR_HEIGHT, TFT_BLACK); // Pastikan layar hitam (di bawah status bar)

  // 1. Header (Nama Track)
  // Tidak ada, atau kecil di atas? Gambar referensi menunjukkan "Serres Automotive" di bar status?
  // Kita akan gunakan bar status untuk nama track jika memungkinkan, atau draw di y=25
  // Asumsi: Status bar diurus UIManager. Kita gambar di bawahnya.
  
  // 1. Back Arrow
  tft->setTextColor(TFT_WHITE, TFT_BLACK);
  tft->setTextDatum(TL_DATUM);
  tft->setFreeFont(&Org_01); 
  tft->setTextSize(2);      
  tft->drawString("<", 10, 25);
  
  // 3. Separator Horizontal Utama
  tft->drawFastHLine(0, 90, SCREEN_WIDTH, TFT_WHITE);
  tft->drawFastHLine(0, 200, SCREEN_WIDTH, TFT_WHITE);
  
  // 4. Lap Box (Kanan Atas) - Latar Putih
  tft->fillRect(SCREEN_WIDTH / 2 + 20, 25, SCREEN_WIDTH / 2 - 20, 65, TFT_WHITE); // Height increased to match line 90
  
  // 5. Box BEST (Kiri Bawah) - Latar Putih
  int bottomY = 200;
  tft->fillRect(0, bottomY, 100, 40, TFT_WHITE);
  
  _ui->drawStatusBar();
}

void LapTimerScreen::drawRacing() {
  TFT_eSPI *tft = _ui->getTft();
  
  // 1. Kecepatan (Kiri Atas - Besar Putih di Hitam)
  tft->setTextColor(TFT_WHITE, TFT_BLACK);
  tft->setTextDatum(TL_DATUM);
  tft->setTextFont(7); // Font 7-segmen besar
  tft->setTextSize(1);
  
  // Reduced padding to avoid erasing KPH label
  tft->setTextPadding(110); 
  tft->drawFloat(gpsManager.getSpeedKmph(), 0, 45, 30); 
  tft->setTextPadding(0);

  // Re-draw Labels dynamically to prevent overwrites
  // Label "KPH" (Kecil Putih di kiri)
  tft->setTextColor(TFT_WHITE, TFT_BLACK);
  tft->setTextDatum(TL_DATUM);
  tft->setFreeFont(&Org_01);
  tft->setTextSize(1);
  tft->drawString("KPH", 25, 27);

  // Label "LAP" (Kecil Hitam di dalam kotak Putih)
  tft->setTextColor(TFT_BLACK, TFT_WHITE);
  tft->setTextDatum(TL_DATUM);
  tft->setFreeFont(&Org_01); // Use Org_01 for consistency and small size
  tft->setTextSize(1);
  tft->drawString("LAP", SCREEN_WIDTH / 2 + 25, 27); // Moved to Top-Left of Box 

  // 2. Lap Count (Kanan Atas - Besar Hitam di Putih)
  tft->setTextColor(TFT_BLACK, TFT_WHITE);
  tft->setTextDatum(TR_DATUM); // Rata Kanan
  tft->setTextFont(7); 
  tft->setTextSize(1);
  tft->setTextPadding(100);
  // Gambar di dalam kotak putih (x > 180)
  tft->drawNumber(_lapCount, SCREEN_WIDTH - 10, 30); 
  tft->setTextPadding(0);

  // 3. Main Time (Tengah - Besar Putih)
  unsigned long currentLap = 0;
  if (_isRecording)
    currentLap = millis() - _currentLapStart;
  
  int ms = currentLap % 1000;
  int s = (currentLap / 1000) % 60;
  int m = (currentLap / 60000);
  char buf[32];
  
  // Format MM:SS.d untuk timer berjalan
  sprintf(buf, "%d:%02d.%01d", m, s, ms / 100); 

  tft->setTextColor(TFT_WHITE, TFT_BLACK);
  tft->setTextDatum(MC_DATUM); // Tengah Layar
  tft->setTextFont(7); // Font paling besar 7
  tft->setTextSize(1); // (Atau skala 2 untuk font 4?) Font 7 numerik bagus.
  tft->setTextPadding(SCREEN_WIDTH);
  tft->drawString(buf, SCREEN_WIDTH / 2, 140); // Tengah vertikal area (80-200)
  tft->setTextPadding(0);

  // 4. Bottom Section (Mode Dependent)
  int bottomY = 200;
  
  // Label Box (Background Putih sudah digambar di Static, tapi kita overwrite text)
  tft->setTextColor(TFT_BLACK, TFT_WHITE);
  tft->setTextDatum(MC_DATUM);
  tft->setFreeFont(&Org_01);
  tft->setTextSize(2); 

  String label = "";
  String valueStr = "--:--.--";
  
  if (_raceMode == MODE_BEST) {
      label = "BEST";
      if (_bestLapTime > 0) {
          int bms = _bestLapTime % 1000;
          int bs = (_bestLapTime / 1000) % 60;
          int bm = (_bestLapTime / 60000);
          char tmp[32];
          sprintf(tmp, "%d:%02d.%02d", bm, bs, bms / 10);
          valueStr = String(tmp);
      }
  } else if (_raceMode == MODE_LAST) {
      label = "LAST";
      if (_lastLapTime > 0) {
          int lms = _lastLapTime % 1000;
          int ls = (_lastLapTime / 1000) % 60;
          int lm = (_lastLapTime / 60000);
          char tmp[32];
          sprintf(tmp, "%d:%02d.%02d", lm, ls, lms / 10);
          valueStr = String(tmp);
      }
  } else if (_raceMode == MODE_PREDICTIVE) {
      // In Predictive Mode, Show the Delta Number in the White Box
      valueStr = "-00.26"; 
      label = valueStr; 
  }
  
  // Draw Label/Value in White Box (0-100)
  tft->setTextColor(TFT_BLACK, TFT_WHITE);
  tft->setTextDatum(MC_DATUM);
  tft->setTextPadding(90);
  tft->drawString(label, 50, bottomY + 20);
  tft->setTextPadding(0);
  
  // Draw Right Side Content
  if (_raceMode == MODE_PREDICTIVE) {
     // Bar Graph Simulation
     // Background Bar
     tft->fillRect(110, bottomY + 10, SCREEN_WIDTH - 120, 20, TFT_DARKGREY);
     // Value Bar (Green/Red)
     // Misal -0.26 (Green)
     tft->fillRect(SCREEN_WIDTH/2, bottomY + 10, 40, 20, TFT_GREEN);
     tft->drawFastVLine(SCREEN_WIDTH/2, bottomY+5, 30, TFT_WHITE); // Center Marker
      
  } else {
      // Standard Time Value (Right Side)
      tft->setTextColor(TFT_WHITE, TFT_BLACK);
      tft->setTextFont(4); 
      tft->setTextDatum(TR_DATUM);
      tft->setTextPadding(200);
      tft->drawString(valueStr, SCREEN_WIDTH - 10, 210); 
      tft->setTextPadding(0);
  }
}

void LapTimerScreen::checkFinishLine() {
  double dist = gpsManager.distanceBetween(gpsManager.getLatitude(),
                                           gpsManager.getLongitude(),
                                           _finishLat, _finishLon);

  // Logika Deteksi Mulai/Lap
  static bool inside = false;
  static unsigned long lastCross = 0;

  if (dist < 20) {                                   // Radius 20m
    if (!inside && (millis() - lastCross > 10000)) { // Debounce 10s
      // Lap Baru / Mulai
      if (!_isRecording) {
        _isRecording = true;
        sessionManager.startSession();
        _lapCount = 1;
      } else {
        unsigned long lapTime = millis() - _currentLapStart;
        _lastLapTime = lapTime;
        _lapTimes.push_back(lapTime); // Tambahkan ke riwayat
        if (_bestLapTime == 0 || lapTime < _bestLapTime)
          _bestLapTime = lapTime;
        _lapCount++;
      }
      _currentLapStart = millis();
      lastCross = millis();
      inside = true;
    }
  } else {
    if (dist > 25)
      inside = false;
  }
}
