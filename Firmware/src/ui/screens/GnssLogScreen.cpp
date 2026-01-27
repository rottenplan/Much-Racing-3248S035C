#include "GnssLogScreen.h"
#include "../../core/GPSManager.h"
#include "../fonts/Org_01.h"

extern GPSManager gpsManager;
extern TFT_eSPI tft; // Or access via _ui->getTft()

void GnssLogScreen::onShow() {
  // Clear full screen
  TFT_eSPI *tft = _ui->getTft();
  _ui->drawCarbonBackground(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                            SCREEN_HEIGHT - STATUS_BAR_HEIGHT); // COLOR_BG
  _ui->drawStatusBar(true);

  _lines.clear();
  _paused = false;
  _lastDataTime = 0;
  _lastStatusConnected = false;
  _buffer = "";
  _needsRedraw = true;

  // --- 1. HEADER ---
  tft->drawFastHLine(0, 20, SCREEN_WIDTH, COLOR_SECONDARY);

  // Title
  tft->setTextColor(TFT_WHITE, TFT_BLACK);
  tft->setTextDatum(TC_DATUM);
  tft->setFreeFont(&Org_01);
  tft->setTextSize(2); // Match Session Summary
  tft->drawString("GPS LOG", SCREEN_WIDTH / 2, 28);

  // Back Button (Blue Triangle) - Bottom Left
  tft->fillTriangle(10, 220, 22, 214, 22, 226, TFT_BLUE);

  // --- 2. CHECKBOXES (Top area) ---
  drawCheckboxes();

  // --- 3. LOG CARD ---
  // Dynamic Width
  int boxX = 40;
  int boxW = SCREEN_WIDTH - 80; // 400px on 480w
  int boxY = 95;
  int boxH = SCREEN_HEIGHT - 95 - 15; // Bottom margin 15

  tft->fillRoundRect(boxX, boxY, boxW, boxH, 8, 0x18E3);
  tft->drawRoundRect(boxX, boxY, boxW, boxH, 8, TFT_DARKGREY);

  // Register Callback
  gpsManager.setRawDataCallback([this](uint8_t c) {
    if (_paused)
      return;

    if (c == '\n' || c == '\r') {
      if (_buffer.length() > 0) {
        // Truncate to fit new box width (Approx 1 char = 8px? 400px = 50 chars)
        int maxChars = (SCREEN_WIDTH - 100) / 8;
        if (_buffer.length() > maxChars) {
          _buffer = _buffer.substring(0, maxChars);
        }
        _lines.push_back(_buffer);
        if (_lines.size() > 8) // Max 8 lines for H=110
          _lines.erase(_lines.begin());
        _buffer = "";
        _needsRedraw = true; // Trigger redraw
      }
    } else {
      if (_buffer.length() < 60)
        _buffer += (char)c; // Prevent infinite growth
    }
  });
}

void GnssLogScreen::onHide() {
  gpsManager.setRawDataCallback(nullptr);
  _lines.clear();
  _buffer = "";
}

void GnssLogScreen::drawCheckboxes() {
  TFT_eSPI *tft = _ui->getTft();

  // Reset Font
  tft->setFreeFont(NULL);
  tft->setTextFont(1);
  tft->setTextSize(1);
  tft->setTextDatum(TL_DATUM);
  tft->setTextColor(TFT_SILVER, TFT_BLACK); // Silver on Black

  int yTop = 55; // Lowered to fit under new header

  // Helper to draw a "checkbox" style item
  auto drawCheckItem = [&](int x, String label, bool checked) {
    // Checkbox square (Larger 16x16)
    tft->drawRect(x, yTop, 16, 16, TFT_DARKGREY);

    if (checked) {
      tft->fillRect(x + 3, yTop + 3, 10, 10, TFT_GREEN); // Green check
    } else {
      tft->fillRect(x + 3, yTop + 3, 10, 10, TFT_BLACK); // Uncheck
    }
    tft->drawString(label, x + 20, yTop + 1); // Adjusted text pos
  };

  uint8_t m = gpsManager.getGnssMode();
  // Mapping logic
  bool checkGPS = true; // Always on
  bool checkGLO = (m == 0 || m == 1 || m == 2 || m == 7);
  bool checkSBAS = (m == 0 || m == 1 || m == 2 || m == 3 || m == 4 || m == 6);
  bool checkGAL = (m == 0 || m == 2 || m == 3);

  // Dynamic Centering
  // Items: GNSS(64), UBLOX(72), SBAS(64), GAL(50) ~ Total 300?
  // Spacing: 80px per slot?
  // 4 items * 80 = 320px
  int totalW = 320;
  int startX = (SCREEN_WIDTH - totalW) / 2;

  drawCheckItem(startX + 0, "GNSS", checkGPS);
  drawCheckItem(startX + 80, "UBLOX", checkGLO);
  drawCheckItem(startX + 160, "SBAS", checkSBAS);
  drawCheckItem(startX + 240, "GAL", checkGAL);
}

void GnssLogScreen::update() {
  // Touch Handling
  UIManager::TouchPoint p = _ui->getTouchPoint();
  if (p.x != -1) {
    if (p.y > 200 && p.x < 60) {
      // Back (Triangle area)
      static unsigned long lastBackTap = 0;
      if (millis() - lastBackTap < 500) {
        gpsManager.setRawDataCallback(nullptr); // Disable callback
        _ui->switchScreen(SCREEN_GPS_STATUS);
        lastBackTap = 0;
      } else {
        lastBackTap = millis();
      }
      return;
    }

    // Toggle Pause (Tap on log area or Toggle Button?)
    // Area: Y 95 - 205
    if (p.x > 40 && p.x < 280 && p.y > 90 && p.y < 210) {
      _paused = !_paused;
      _needsRedraw = true;
    }
  }

  if (p.y < 90) { // Expanded touch area from 60 to 90
    // Checkbox Area
    int tapX = p.x;

    // Dynamic Centering Matching drawCheckboxes
    int totalW = 320;
    int startX = (SCREEN_WIDTH - totalW) / 2;
    // 80px per slot starting at startX

    uint8_t m = gpsManager.getGnssMode();
    bool glo = (m == 0 || m == 1 || m == 2 || m == 7);
    bool sbas = (m == 0 || m == 1 || m == 2 || m == 3 || m == 4 || m == 6);
    bool gal = (m == 0 || m == 2 || m == 3);

    if (tapX > startX && tapX < startX + 70) { // GNSS Area
                                               // GNSS always on
    } else if (tapX > startX + 80 && tapX < startX + 150) {
      glo = !glo;
    } else if (tapX > startX + 160 && tapX < startX + 230) {
      sbas = !sbas;
    } else if (tapX > startX + 240 && tapX < startX + 310) {
      gal = !gal;
    }

    // Resolve new mode
    uint8_t newMode = 1; // Default fallback

    if (glo && gal && sbas)
      newMode = 0; // All
    else if (glo && !gal && sbas)
      newMode = 1; // GPS+GLO+SBAS
    else if (glo && gal && !sbas)
      newMode = 2; // GPS+GAL+GLO
    else if (!glo && gal && sbas)
      newMode = 3; // GPS+GAL+SBAS
    else if (!glo && !gal && sbas)
      newMode = 4; // GPS+SBAS
    else if (!glo && !gal && !sbas)
      newMode = 5; // GPS Only
    else if (glo && !gal && !sbas)
      newMode = 7; // GPS+GLO

    if (newMode != m) {
      gpsManager.setGnssMode(newMode);
      // _ui->switchScreen(SCREEN_GNSS_LOG); // Reload to redraw checks <--
      // REMOVED
      drawCheckboxes(); // Update only checkboxes
      return;
    }
  }

  // Status Indicator
  bool connected = (millis() - _lastDataTime < 1000) && (_lastDataTime != 0);
  if (connected != _lastStatusConnected) {
    _lastStatusConnected = connected;
    _needsRedraw = true;

    // Update status immediately
    TFT_eSPI *tft = _ui->getTft();
    tft->setTextSize(1);
    tft->setTextDatum(TR_DATUM);
    // Draw status at top right, aligned with checks Y=35
    if (connected) {
      tft->setTextColor(TFT_GREEN, TFT_BLACK);
      tft->drawString("GPS: CONNECTED  ", 310, 35);
    } else {
      tft->setTextColor(TFT_RED, TFT_BLACK);
      tft->drawString("GPS: NO DATA    ", 310, 35);
    }
  }

  static unsigned long lastDrawTime = 0;
  if (_needsRedraw && (millis() - lastDrawTime > 300)) { // Throttle to ~3 FPS
    drawLines();
    _needsRedraw = false;
    lastDrawTime = millis();
  }
}

void GnssLogScreen::drawLines() {
  TFT_eSPI *tft = _ui->getTft();

  // Revert to Standard Font
  tft->setTextFont(1);
  tft->setTextSize(1);
  tft->setFreeFont(NULL);

  // Green text for logs
  tft->setTextColor(TFT_GREEN, 0x18E3);
  tft->setTextDatum(TL_DATUM);
  tft->setTextPadding(220); // Width reduced to avoid overwriting right border

  // Margin 10px from X=40 -> X=50. Y=105.
  int innerX = 50;
  int innerY = 105;

  int y = innerY;
  for (const auto &line : _lines) {
    tft->drawString(line, innerX, y);
    y += 10; // Standard spacing
  }
}
