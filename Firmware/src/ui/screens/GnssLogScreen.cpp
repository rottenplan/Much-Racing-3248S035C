#include "GnssLogScreen.h"
#include "../../core/GPSManager.h"
#include "../fonts/Org_01.h"
#include <Arduino.h>
#include <TFT_eSPI.h>

extern GPSManager gpsManager;
extern TFT_eSPI tft; // Or access via _ui->getTft()

// Constants for Layout
#define CHECK_Y_START 50
#define CHECK_H 40
#define LOG_BOX_Y 110
#define LOG_BOX_BOTTOM 215

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
  tft->setTextSize(2);
  tft->drawString("GNSS LOGGER", SCREEN_WIDTH / 2, 28);

  // Back Button (Blue Triangle) - Bottom Left
  _ui->drawBackButton();

  // Draw Initial UI Elements
  drawCheckboxes();
  drawControls();
  drawLogBoxArea();

  // Register Callback
  gpsManager.setRawDataCallback([this](uint8_t c) {
    if (_paused)
      return;

    if (c == '\n' || c == '\r') {
      if (_buffer.length() > 0) {
        // Truncate to fit
        int maxChars = (SCREEN_WIDTH - 60) / 8;
        if (_buffer.length() > maxChars) {
          _buffer = _buffer.substring(0, maxChars);
        }
        _lines.push_back(_buffer);
        if (_lines.size() > 8) // Max lines
          _lines.erase(_lines.begin());
        _buffer = "";
        _needsRedraw = true;
      }
    } else {
      if (_buffer.length() < 60)
        _buffer += (char)c;
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
  tft->setTextDatum(MC_DATUM);

  uint8_t m = gpsManager.getGnssMode();
  bool checkGPS = true; // Always on
  bool checkGLO = (m == 0 || m == 1 || m == 2 || m == 7);
  bool checkSBAS = (m == 0 || m == 1 || m == 2 || m == 3 || m == 4 || m == 6);
  bool checkGAL = (m == 0 || m == 2 || m == 3);

  // 4 Buttons evenly spaced
  // W=100, Gap=10 -> 4*100 + 3*10 = 430. Fits in 480.
  int btnW = 100;
  int gap = 10;
  int totalW = (btnW * 4) + (gap * 3);
  int startX = (SCREEN_WIDTH - totalW) / 2;
  int y = CHECK_Y_START;

  auto drawBtn = [&](int idx, String label, bool active) {
    int x = startX + (idx * (btnW + gap));
    uint16_t color = active ? TFT_GREEN : 0x39E7; // Green or Dark Grey
    uint16_t txtColor = active ? TFT_BLACK : TFT_WHITE;

    // Fill
    tft->fillRoundRect(x, y, btnW, CHECK_H, 4, color);
    if (!active)
      tft->drawRoundRect(x, y, btnW, CHECK_H, 4, TFT_SILVER);

    tft->setTextColor(txtColor, color);
    tft->drawString(label, x + btnW / 2, y + CHECK_H / 2);
  };

  drawBtn(0, "GPS", checkGPS);
  drawBtn(1, "GLO", checkGLO);
  drawBtn(2, "SBAS", checkSBAS);
  drawBtn(3, "GAL", checkGAL);
}

void GnssLogScreen::drawControls() {
  TFT_eSPI *tft = _ui->getTft();

  // Resume/Pause Button (Right side, below checks)
  // Clear Button (Left side, below checks) - OR use Bottom Right like Status
  // Screen?

  // Let's put specific control buttons:
  // [ PAUSE/RESUME ]  [ CLEAR ]
  // at bottom right?

  // Back button is bottom left.
  // Let's place RESUME/PAUSE at Bottom Right.

  int btnW = 80;
  int btnH = 30;
  int x = SCREEN_WIDTH - btnW - 10;
  int y = SCREEN_HEIGHT - 35; // Bottom area

  uint16_t color = _paused ? TFT_GREEN : TFT_ORANGE;
  String label = _paused ? "RESUME" : "PAUSE";

  tft->fillRoundRect(x, y, btnW, btnH, 4, color);
  tft->setTextColor(TFT_BLACK, color);
  tft->setTextDatum(MC_DATUM);
  tft->setFreeFont(NULL);
  tft->setTextFont(2);
  tft->drawString(label, x + btnW / 2, y + btnH / 2);

  // Clear Button - Left of Reuse/Pause?
  // Or maybe just center bottom?
  // Let's put CLEAR at Center Bottom
  int clrW = 60;
  int clrX = SCREEN_WIDTH / 2 - clrW / 2;
  int clrY = SCREEN_HEIGHT - 35;

  tft->fillRoundRect(clrX, clrY, clrW, btnH, 4, TFT_RED);
  tft->setTextColor(TFT_WHITE, TFT_RED);
  tft->drawString("CLR", clrX + clrW / 2, clrY + btnH / 2);
}

void GnssLogScreen::drawLogBoxArea() {
  TFT_eSPI *tft = _ui->getTft();
  int boxX = 20;
  int boxW = SCREEN_WIDTH - 40;
  int boxH = LOG_BOX_BOTTOM - LOG_BOX_Y; // 215 - 110 = 105

  tft->drawRect(boxX, LOG_BOX_Y, boxW, boxH, TFT_DARKGREY);
}

void GnssLogScreen::update() {
  UIManager::TouchPoint p = _ui->getTouchPoint();

  if (p.x != -1) {
    if (_ui->isBackButtonTouched(p)) {
      static unsigned long lastBack = 0;
      if (millis() - lastBack < 500) {
        gpsManager.setRawDataCallback(nullptr);
        _ui->switchScreen(SCREEN_GPS_STATUS);
        lastBack = 0;
      } else {
        lastBack = millis();
      }
      return;
    }

    // CHECKBOXES
    if (p.y >= CHECK_Y_START && p.y <= CHECK_Y_START + CHECK_H) {
      // Calculate index
      int btnW = 100;
      int gap = 10;
      int totalW = (btnW * 4) + (gap * 3);
      int startX = (SCREEN_WIDTH - totalW) / 2;

      if (p.x >= startX && p.x <= startX + totalW) {
        int idx = (p.x - startX) / (btnW + gap);
        if (idx >= 0 && idx <= 3) {
          uint8_t m = gpsManager.getGnssMode();
          bool glo = (m == 0 || m == 1 || m == 2 || m == 7);
          bool sbas =
              (m == 0 || m == 1 || m == 2 || m == 3 || m == 4 || m == 6);
          bool gal = (m == 0 || m == 2 || m == 3);

          // Toggle logic
          if (idx == 1)
            glo = !glo;
          if (idx == 2)
            sbas = !sbas;
          if (idx == 3)
            gal = !gal;
          // Idx 0 (GPS) ignored as it's always on

          // Resolve Mode
          uint8_t newMode = 1;

          if (glo && gal && sbas)
            newMode = 0;
          else if (glo && !gal && sbas)
            newMode = 1;
          else if (glo && gal && !sbas)
            newMode = 2;
          else if (!glo && gal && sbas)
            newMode = 3;
          else if (!glo && !gal && sbas)
            newMode = 4;
          else if (!glo && !gal && !sbas)
            newMode = 5;
          else if (glo && !gal && !sbas)
            newMode = 7;

          if (newMode != m) {
            gpsManager.setGnssMode(newMode);
            drawCheckboxes();
            delay(100); // Debounce visual
          }
        }
      }
    }

    // CONTROL BUTTONS (Bottom Area)
    if (p.y > SCREEN_HEIGHT - 40) {
      // PAUSE/RESUME (Right)
      if (p.x > SCREEN_WIDTH - 100) {
        _paused = !_paused;
        drawControls();
        delay(200);
      }
      // CLEAR (Center)
      else if (p.x > SCREEN_WIDTH / 2 - 40 && p.x < SCREEN_WIDTH / 2 + 40) {
        _lines.clear();
        _buffer = "";
        _needsRedraw = true;
        delay(200);
      }
    }
  }

  // CONNECTION STATUS UPDATES
  bool connected = (millis() - _lastDataTime < 1000) && (_lastDataTime != 0);
  if (connected != _lastStatusConnected) {
    _lastStatusConnected = connected;
    // Just indicator dot? Or text?
    // Let's use a small dot next to header
    TFT_eSPI *tft = _ui->getTft();
    tft->fillCircle(SCREEN_WIDTH - 20, 28, 5, connected ? TFT_GREEN : TFT_RED);
  }

  static unsigned long lastDrawTime = 0;
  if (_needsRedraw && (millis() - lastDrawTime > 100)) {
    drawLines();
    _needsRedraw = false;
    lastDrawTime = millis();
  }
}

void GnssLogScreen::drawLines() {
  TFT_eSPI *tft = _ui->getTft();

  // Define Log Area
  int boxX = 20;
  int boxW = SCREEN_WIDTH - 40;
  int boxH = LOG_BOX_BOTTOM - LOG_BOX_Y;

  // Clear inside
  tft->fillRect(boxX + 1, LOG_BOX_Y + 1, boxW - 2, boxH - 2,
                TFT_BLACK); // Black background for log

  tft->setTextFont(1);
  tft->setTextSize(1);
  tft->setFreeFont(NULL);
  tft->setTextColor(TFT_GREEN, TFT_BLACK);
  tft->setTextDatum(TL_DATUM);

  int x = boxX + 5;
  int y = LOG_BOX_Y + 5;

  for (const auto &line : _lines) {
    tft->drawString(line, x, y);
    y += 12; // Spacing
  }
}
