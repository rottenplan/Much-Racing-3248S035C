#include "LapTimerScreen.h"
#include "../../core/GPSManager.h"
#include "../../core/SessionManager.h"
#include "../fonts/Org_01.h"
#include <algorithm> // For min_element

extern GPSManager gpsManager;
extern SessionManager sessionManager;

// Constants for UI Layout
#define STATUS_BAR_HEIGHT 20
#define HEADER_HEIGHT 40
#define LIST_ITEM_HEIGHT 30
#define BUTTON_HEIGHT 40

// Define Button Area
#define STOP_BTN_Y 200 // Moved DOWN for better spacing
#define STOP_BTN_H 35

void LapTimerScreen::onShow() {
  _lastUpdate = 0;
  _isRecording = false; // Start not recording
  _finishSet = false;
  _lapCount = 0;
  _state = STATE_SUMMARY; // Start in Summary/Menu View
  _bestLapTime = 0;
  _lapTimes.clear();
  _listScroll = 0;

  TFT_eSPI *tft = _ui->getTft();
  // tft->fillScreen(COLOR_BG); // Already cleared by UIManager
  drawSummary();
}

void LapTimerScreen::update() {
  UIManager::TouchPoint p = _ui->getTouchPoint();
  bool touched = (p.x != -1);



  if (_state == STATE_SUMMARY) {
    // --- SUMMARY STATE LOGIC ---
    if (touched) {
      // 1. START BUTTON (Bottom)
      // Rect: y > 200
      if (p.y > 200) {
        // Start Racing
        _state = STATE_RACING;
        _ui->getTft()->fillScreen(COLOR_BG);
        drawRacingStatic(); // Draw static elements once
        drawRacing(); // Initial draw of dynamic elements
        return;
      }

      // 2. BACK/MENU (Top Left)
      // Back Button (Header Area 20-60)
      if (p.x < 70 && p.y < 70) { // Standardized to 70x70
        _ui->switchScreen(SCREEN_MENU);
        return;
      }

      // 3. SCROLLING (Middle)
      // List is at y=70 to ~190
      if (p.y > 70 && p.y < 200) {
        if (p.y < 135) { // Upper half
          if (_listScroll > 0)
            _listScroll--;
        } else { // Lower half
          if (_listScroll < _lapTimes.size())
            _listScroll++;
        }
        drawLapList(_listScroll);
      }
    }

  } else {
    // --- RACING STATE LOGIC ---
    // Touch: STOP/FINISH
    if (touched) {
      // Stop Button (Bottom)
      if (p.y > STOP_BTN_Y) {
        _state = STATE_SUMMARY;

        // Save History
        if (sessionManager.isLogging()) {
          // Get Date/Time string?
          String dateStr =
              gpsManager.getDateString() + " " + gpsManager.getTimeString();
          sessionManager.appendToHistoryIndex("Track Session", dateStr,
                                              _lapCount, _bestLapTime);
        }

        sessionManager.stopSession();
        _isRecording = false;
        _ui->getTft()->fillScreen(COLOR_BG);
        drawSummary();
        return;
      }

      // Manual Finish Set (if not set)
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

    // Logging (1Hz or 10Hz?)
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

// --- DRAWING HELPERS ---

void LapTimerScreen::drawSummary() {
  TFT_eSPI *tft = _ui->getTft();

  // Header
  // Header (Below Status Bar)
  int headerY = STATUS_BAR_HEIGHT; // 20
  tft->fillRect(0, headerY, SCREEN_WIDTH, HEADER_HEIGHT, COLOR_SECONDARY);

  tft->setTextColor(COLOR_TEXT, COLOR_SECONDARY);
  tft->setTextDatum(MC_DATUM);
  tft->setTextFont(2);
  tft->setTextSize(1);
  tft->drawString("SESSION SUMMARY", SCREEN_WIDTH / 2,
                  headerY + (HEADER_HEIGHT / 2));

  // Back Button Icon
  tft->drawString("<", 15, headerY + (HEADER_HEIGHT / 2));

  // Best Lap Box (Top)
  int bestLapY = headerY + HEADER_HEIGHT + 5; // 20+40+5 = 65
  tft->setTextColor(COLOR_TEXT, COLOR_BG);
  tft->setTextDatum(TC_DATUM);
  tft->setTextFont(2);
  tft->setTextSize(1);
  tft->drawString("BEST LAP", SCREEN_WIDTH / 2, bestLapY);

  char buf[32];
  if (_bestLapTime > 0) {
    int ms = _bestLapTime % 1000;
    int s = (_bestLapTime / 1000) % 60;
    int m = (_bestLapTime / 60000);
    sprintf(buf, "%02d:%02d.%02d", m, s, ms / 10);
  } else {
    sprintf(buf, "--:--.--");
  }

  tft->setTextFont(4); // Use Font 4 for big numbers (approx 26px high)
  tft->setTextSize(1); // Standard size
  tft->drawString(buf, SCREEN_WIDTH / 2,
                  bestLapY + 25); // Adjusted Y for font 4

  // List (Middle)
  // Shift list up slightly
  drawLapList(_listScroll);

  // Start Button (Bottom)
  // Screen H=240. Status=20. Header=40. List=~150.
  // Button needs to be at ~200.
  int btnY = 200;
  int btnH = 35;

  tft->fillRoundRect(40, btnY, SCREEN_WIDTH - 80, btnH, 5, TFT_GREEN);
  tft->setTextColor(TFT_BLACK, TFT_GREEN);
  tft->setFreeFont(&Org_01);
  tft->setTextSize(2);
  tft->setTextDatum(MC_DATUM);
  tft->drawString("START", SCREEN_WIDTH / 2,
                  btnY + (btnH / 2) + 2); // +2 centering
}

void LapTimerScreen::drawLapList(int scrollOffset) {
  TFT_eSPI *tft = _ui->getTft();
  int startY = 90;     // Moved down to clear header (20+40+30)
  int itemsToShow = 4; // Show fewer items to fit

  // Clear List Area
  tft->fillRect(0, startY, SCREEN_WIDTH, itemsToShow * LIST_ITEM_HEIGHT,
                COLOR_BG);

  tft->setTextFont(2);
  tft->setTextSize(1);
  tft->setTextDatum(TL_DATUM);
  tft->setTextColor(COLOR_TEXT, COLOR_BG);

  for (int i = 0; i < itemsToShow; i++) {
    int idx = scrollOffset + i;
    if (idx >= _lapTimes.size())
      break;

    unsigned long t = _lapTimes[idx];
    int ms = t % 1000;
    int s = (t / 1000) % 60;
    int m = (t / 60000);
    char buf[32];
    sprintf(buf, "%d. %02d:%02d.%02d", idx + 1, m, s, ms / 10);

    int y = startY + (i * LIST_ITEM_HEIGHT);
    tft->drawString(buf, 20, y);

    // Highlight Best Lap?
    if (t == _bestLapTime && t > 0) {
      tft->drawRect(15, y - 2, SCREEN_WIDTH - 30, LIST_ITEM_HEIGHT - 2,
                    TFT_GOLD);
    }
  }
}

void LapTimerScreen::drawRacingStatic() {
  TFT_eSPI *tft = _ui->getTft();

  // Draw Static Labels
  tft->setTextColor(COLOR_TEXT, COLOR_BG);
  
  // 1. Speed Label (Top Left)
  tft->setTextDatum(TL_DATUM); // Top Left
  tft->setTextFont(2);
  tft->setTextSize(1);
  tft->drawString("km/h", 10, 50); // Label at 50

  // 2. Lap Label (Top Right)
  tft->setTextDatum(TR_DATUM); // Top Right
  tft->setTextFont(2);
  tft->drawString("LAP", SCREEN_WIDTH - 10, 50); // Label at 50

  // 3. Best Lap Label (Bottom Center, above STOP)
  tft->setTextDatum(TC_DATUM);
  tft->drawString("BEST LAP", SCREEN_WIDTH / 2, 165); // Moved UP to 165

  // Stop Button (Moved here, drawn ONCE)
  tft->fillRoundRect(40, STOP_BTN_Y, SCREEN_WIDTH - 80, STOP_BTN_H, 5, TFT_RED);
  tft->setTextColor(TFT_WHITE, TFT_RED);
  tft->setFreeFont(&Org_01);
  tft->setTextSize(2);
  tft->setTextDatum(MC_DATUM);
  tft->drawString("STOP", SCREEN_WIDTH / 2,
                  STOP_BTN_Y + (STOP_BTN_H / 2) + 2); // Match offset
}

void LapTimerScreen::drawRacing() {
  TFT_eSPI *tft = _ui->getTft();
  tft->setTextColor(COLOR_TEXT, COLOR_BG);

  // 1. Speed (Top Left)
  tft->setTextDatum(TL_DATUM);
  tft->setTextFont(4); // Medium Font
  tft->setTextSize(1);
  tft->setTextPadding(80); 
  tft->drawFloat(gpsManager.getSpeedKmph(), 0, 10, 20); // Value at 20
  tft->setTextPadding(0);

  // 2. Lap Count (Top Right)
  tft->setTextDatum(TR_DATUM);
  tft->setTextFont(4); 
  tft->setTextSize(1);
  tft->setTextPadding(50);
  tft->drawNumber(_lapCount, SCREEN_WIDTH - 10, 20); // Value at 20
  tft->setTextPadding(0);

  // 3. Current Lap Time (CENTER - Main Focus)
  unsigned long currentLap = 0;
  if (_isRecording)
    currentLap = millis() - _currentLapStart;
  int ms = currentLap % 1000;
  int s = (currentLap / 1000) % 60;
  int m = (currentLap / 60000);
  char buf[32];
  sprintf(buf, "%02d:%02d.%01d", m, s, ms / 100); // Format MM:SS.d

  tft->setTextDatum(MC_DATUM); // Center Screen
  tft->setTextFont(6); // Large Font
  tft->setTextSize(1);
  tft->setTextPadding(SCREEN_WIDTH); // Full width clear
  tft->drawString(buf, SCREEN_WIDTH / 2, 115); // Center at 115
  tft->setTextPadding(0);

  // 4. Best Lap Time (Bottom Center - Small)
  if (_bestLapTime > 0) {
      int bms = _bestLapTime % 1000;
      int bs = (_bestLapTime / 1000) % 60;
      int bm = (_bestLapTime / 60000);
      sprintf(buf, "%02d:%02d.%02d", bm, bs, bms / 10);
  } else {
      sprintf(buf, "--:--.--");
  }
  
  tft->setTextFont(2);
  tft->setTextDatum(TC_DATUM);
  tft->setTextPadding(100);
  tft->drawString(buf, SCREEN_WIDTH / 2, 185); // Value at 185 (Clear gap to 200)
  tft->setTextPadding(0);
}

void LapTimerScreen::checkFinishLine() {
  double dist = gpsManager.distanceBetween(gpsManager.getLatitude(),
                                           gpsManager.getLongitude(),
                                           _finishLat, _finishLon);

  // Start/Lap Detection Logic
  static bool inside = false;
  static unsigned long lastCross = 0;

  if (dist < 20) {                                   // 20m radius
    if (!inside && (millis() - lastCross > 10000)) { // Debounce 10s
      // New Lap / Start
      if (!_isRecording) {
        _isRecording = true;
        sessionManager.startSession();
        _lapCount = 1;
      } else {
        unsigned long lapTime = millis() - _currentLapStart;
        _lastLapTime = lapTime;
        _lapTimes.push_back(lapTime); // Add to history
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
