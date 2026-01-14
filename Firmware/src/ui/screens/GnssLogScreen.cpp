#include "GnssLogScreen.h"
#include "../../core/GPSManager.h"
#include "../fonts/Org_01.h"

extern GPSManager gpsManager;
extern TFT_eSPI tft; // Or access via _ui->getTft()

void GnssLogScreen::onShow() {
  // Clear only content area
  _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                          SCREEN_HEIGHT - STATUS_BAR_HEIGHT, COLOR_BG);
  _ui->drawStatusBar(true); // Force clean status bar
  _lines.clear();
  _paused = false;
  _lastDataTime = 0;
  _lastStatusConnected = false;
  _buffer = "";
  _needsRedraw = true;

  TFT_eSPI *tft = _ui->getTft();

  // 1. Draw Checkboxes
  drawCheckboxes();

  // 2. Title "LOG"
  tft->setTextSize(2);             // Larger font
  tft->setTextDatum(TC_DATUM);     // Top Center
  tft->drawString("LOG", 160, 60); // Moved down from 40

  // 3. Log Box Frame
  // Y=85 to accommodate title. H=125 (Ends at 210, leaving room for Back
  // button)
  tft->drawRect(5, 85, 310, 125, TFT_WHITE);

  // 4. Back Button (Blue Triangle)
  // Bottom Left
  tft->fillTriangle(10, 220, 22, 214, 22, 226, TFT_BLUE);
  // Optional: "Back" text? Reference image shows just triangle?
  // Reference has clear triangle. Let's stick to triangle.

  // Register Callback
  gpsManager.setRawDataCallback([this](uint8_t c) {
    if (_paused)
      return;

    if (c == '\n' || c == '\r') {
      if (_buffer.length() > 0) {
        _lines.push_back(_buffer);
        if (_lines.size() > 10)
          _lines.erase(_lines.begin()); // reduced line count due to smaller box
        _buffer = "";
        _needsRedraw = true; // Trigger redraw
      }
    } else {
      if (_buffer.length() < 50)
        _buffer += (char)c; // Prevent infinite growth
    }
  });
}

void GnssLogScreen::drawCheckboxes() {
  TFT_eSPI *tft = _ui->getTft();

  // Reset Font
  tft->setFreeFont(NULL);
  tft->setTextFont(1);
  tft->setTextSize(1);
  tft->setTextDatum(TL_DATUM);
  tft->setTextColor(TFT_WHITE, TFT_BLACK);

  int yTop = 35;

  // Helper to draw a "checkbox" style item
  auto drawCheckItem = [&](int x, String label, bool checked) {
    // Clear area first (small box around it)
    // Label width approx 5 chars * 6px = 30px + 20px padding = 50px
    tft->fillRect(x, yTop, 60, 12, TFT_BLACK);

    tft->drawRect(x, yTop, 12, 12, TFT_WHITE);
    if (checked) {
      tft->fillRect(x + 2, yTop + 2, 8, 8, TFT_WHITE); // Filled if checked
    }
    tft->drawString(label, x + 16, yTop);
  };

  uint8_t m = gpsManager.getGnssMode();
  // Mapping logic
  bool checkGPS = true; // Always on
  bool checkGLO = (m == 0 || m == 1 || m == 2 || m == 7);
  bool checkSBAS = (m == 0 || m == 1 || m == 2 || m == 3 || m == 4 || m == 6);
  bool checkGAL = (m == 0 || m == 2 || m == 3);

  drawCheckItem(10, "GNSS", checkGPS);
  drawCheckItem(90, "UBLOX", checkGLO);
  drawCheckItem(170, "SBAS", checkSBAS);
  drawCheckItem(250, "GAL", checkGAL);
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

    if (p.y > 85 && p.y < 210) {
      // Toggle Pause (Tap on log area)
      _paused = !_paused;
      _needsRedraw = true;
    }

    if (p.y < 60) {
      // Checkbox Area
      int tapX = p.x;
      // 10, 90, 170, 250. Width ~60?

      uint8_t m = gpsManager.getGnssMode();
      bool glo = (m == 0 || m == 1 || m == 2 || m == 7);
      bool sbas = (m == 0 || m == 1 || m == 2 || m == 3 || m == 4 || m == 6);
      bool gal = (m == 0 || m == 2 || m == 3);

      if (tapX > 80 && tapX < 140) {
        glo = !glo;
      } else if (tapX > 160 && tapX < 220) {
        sbas = !sbas;
      } else if (tapX > 240 && tapX < 300) {
        gal = !gal;
      } else {
        // Ignore others (GNSS is locked / too far left)
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

  // Use Standard Font 1 (Tiny/Small, 5x7 monospaced-ish)
  tft->setTextFont(1);
  tft->setTextSize(1);

  // Draw with background color to overwrite previous text (Anti-Glitch)
  tft->setTextColor(TFT_GREEN, TFT_BLACK);
  tft->setTextDatum(TL_DATUM);
  tft->setTextPadding(300); // Pad to clear line width

  // Box Inner Area: X=6, Y=86, W=308, H=123
  int innerX = 8;
  int innerY = 90;

  // No full fillRect here! It causes flicker.
  // We rely on padding to clear lines.
  // However, we should ensure the area is black first in onShow.

  int y = innerY;
  for (const auto &line : _lines) {
    tft->drawString(line, innerX, y);
    y += 10; // Font 1 height is ~8px
  }
}
