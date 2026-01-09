#include "GnssLogScreen.h"
#include "../../core/GPSManager.h"
#include "../fonts/Org_01.h"

extern GPSManager gpsManager;
extern TFT_eSPI tft; // Or access via _ui->getTft()

void GnssLogScreen::onShow() {
  _ui->getTft()->fillScreen(TFT_BLACK);
  _ui->drawStatusBar(true); // Force clean status bar
  _lines.clear();
  _paused = false;
  _lastDataTime = 0;
  _lastStatusConnected = false;
  _buffer = "";
  _needsRedraw = true;

  TFT_eSPI *tft = _ui->getTft();

  // 1. Top Header: [v] GNSS  [v] UBLOX  [v] SBAS  [v] GAL
  tft->setFreeFont(NULL); // CRITICAL: Reset any custom font to ensure Font 1 is
                          // standard GLCD
  tft->setTextFont(
      1); // Explicitly reset to standard font to prevent "enlarging"
  tft->setTextSize(1);
  tft->setTextDatum(TL_DATUM);
  tft->setTextColor(TFT_WHITE, TFT_BLACK);

  int yTop = 35; // Moved down from 10 to avoid Status Bar (0-25)
  int xStep = 80;

  // Helper to draw a "checkbox" style item
  auto drawCheckItem = [&](int x, String label) {
    tft->drawRect(x, yTop, 12, 12, TFT_WHITE);
    tft->drawLine(x + 2, yTop + 6, x + 4, yTop + 9,
                  TFT_WHITE); // Checkmark part 1
    tft->drawLine(x + 4, yTop + 9, x + 9, yTop + 2,
                  TFT_WHITE); // Checkmark part 2
    tft->drawString(label, x + 16, yTop);
  };

  // Evenly distribute roughly
  drawCheckItem(10, "GNSS");
  drawCheckItem(90, "UBLOX");
  drawCheckItem(170, "SBAS");
  drawCheckItem(250, "GAL");

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

void GnssLogScreen::update() {
  // Touch Handling
  UIManager::TouchPoint p = _ui->getTouchPoint();
  if (p.x != -1) {
    if (p.y > 200 && p.x < 60) {
      // Back (Triangle area)
      gpsManager.setRawDataCallback(nullptr); // Disable callback
      _ui->switchScreen(SCREEN_GPS_STATUS);
      return;
    }

    if (p.y > 85 && p.y < 210) {
      // Toggle Pause (Tap on log area)
      _paused = !_paused;
      _needsRedraw = true;
      // Maybe draw a pause icon overlay?
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
