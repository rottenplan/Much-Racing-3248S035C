#include "ui/screens/SpeedometerScreen.h"
#include "../../core/GPSManager.h"
#include "ui/UIManager.h"
#include <Arduino.h>

extern UIManager uiManager;
extern GPSManager gpsManager;

// Colors
#define COL_BG 0x0000
#define COL_CARD_BG 0x18E3
#define COL_CARD_BORDER 0x4A69
#define COL_ARC_OFF 0x2124
#define COL_LOW TFT_GREEN
#define COL_MID 0xFFE0
#define COL_HIGH 0xF800
#define COL_MAIN TFT_WHITE
#define COL_SUB 0xBDF7
#define COL_ACCENT 0x04DF

SpeedometerScreen::SpeedometerScreen(TFT_eSPI *tft)
    : _lastSpeed(-1), _lastSats(-1), _lastTrip(-1), _maxSpeed(0), _maxRPM(0) {}

void SpeedometerScreen::onShow() {
  _ui->getTft()->fillScreen(COL_BG);
  drawDashboard(true);
}

void SpeedometerScreen::onHide() {}

void SpeedometerScreen::update() {
  UIManager::TouchPoint p = _ui->getTouchPoint();
  if (p.x != -1) {
    if (_ui->isBackButtonTouched(p)) {
      _ui->switchScreen(SCREEN_MENU);
      return;
    }
  }

  float speed = gpsManager.getSpeedKmph();
  int sats = gpsManager.getSatellites();
  float trip = gpsManager.getTotalTrip();
  int currentRPM = gpsManager.getRPM();

  // Draw loop
  static unsigned long lastUpd = 0;
  if (millis() - lastUpd > 50) {
    lastUpd = millis();

    bool speedChanged = ((int)speed != (int)_lastSpeed);
    bool rpmChanged = (abs(currentRPM - _lastRPM) > 50);
    bool satsChanged = (sats != _lastSats);
    bool tripChanged = (abs(trip - _lastTrip) > 0.1);

    if (speedChanged || rpmChanged || satsChanged || tripChanged) {
      if (speed > _maxSpeed)
        _maxSpeed = speed;
      if (currentRPM > _maxRPM)
        _maxRPM = currentRPM;

      _lastSpeed = speed;
      _lastRPM = currentRPM;
      _lastSats = sats;
      _lastTrip = trip;

      drawDashboard(false);
    }
  }
}

void SpeedometerScreen::drawDashboard(bool force) {
  TFT_eSPI *tft = _ui->getTft();
  int cx = SCREEN_WIDTH / 2;
  int cy = 180;
  int r = 120; // Balanced radius

  // --- 1. SIDE DATA COLUMNS (HUD STYLE) ---
  int sideY = 60;
  int sideStep = 70;
  int leftX = 10;
  int rightX = SCREEN_WIDTH - 90;
  int widgetW = 80;
  int widgetH = 50;

  auto drawHudWidget = [&](int x, int y, const char *label, const char *val,
                           uint16_t color, bool rightAlign) {
    if (force) {
      // Decorative bar
      tft->fillRect(x + (rightAlign ? widgetW - 4 : 0), y, 4, widgetH, color);
      // Label
      tft->setTextFont(1);
      tft->setTextColor(COL_SUB, COL_BG);
      tft->setTextDatum(rightAlign ? TR_DATUM : TL_DATUM);
      tft->drawString(label, x + (rightAlign ? widgetW - 8 : 8), y + 4);
    }
    // Value
    tft->setTextFont(4);
    tft->setTextColor(color, COL_BG);
    tft->setTextDatum(rightAlign ? BR_DATUM : BL_DATUM);
    tft->drawString(val, x + (rightAlign ? widgetW - 8 : 8), y + widgetH - 4);
  };

  char b1[16], b2[16], b3[16], b4[16];
  sprintf(b1, "%d", _maxRPM);
  sprintf(b2, "%.0f", _maxSpeed);
  sprintf(b3, "%.1f", _lastTrip);
  sprintf(b4, "%d", _lastSats);

  // Left Column
  drawHudWidget(leftX, sideY, "TRIP", b3, TFT_CYAN, false);
  drawHudWidget(leftX, sideY + sideStep, "MAX KMH", b2, COL_MAIN, false);

  // Right Column
  drawHudWidget(rightX, sideY, "SATS", b4,
                (_lastSats > 5) ? TFT_GREEN : TFT_RED, true);
  drawHudWidget(rightX, sideY + sideStep, "MAX RPM", b1, COL_MAIN, true);

  // --- 2. CENTRAL PRO TACHOMETER ---
  // Wide arc with tick marks
  float startA = 135.0; // Wider sweep
  float endA = 405.0;
  float sweep = endA - startA;
  int segments = 60;

  for (int i = 0; i <= segments; i++) {
    float angle = startA + (i * (sweep / segments));
    float rpmVal = i * (10000.0 / segments);

    // Gradient Logic
    uint16_t color = 0x18E3; // Dark grey base
    bool active = (_lastRPM > rpmVal);

    if (active) {
      if (rpmVal < 6000)
        color = TFT_CYAN; // Cool start
      else if (rpmVal < 8500)
        color = TFT_GOLD; // Power band
      else
        color = TFT_MAGENTA; // Redline
    }

    float rad = angle * DEG_TO_RAD;
    float cosA = cos(rad);
    float sinA = sin(rad);

    // Dynamic thickness: Active segments are thicker
    int innerR = r - (active ? 18 : 12);

    // Draw thick segment line
    tft->drawLine(cx + cosA * innerR, cy + sinA * innerR, cx + cosA * r,
                  cy + sinA * r, color);

    // Draw slightly offset line for thickness
    float rad2 = (angle + 0.5) * DEG_TO_RAD;
    tft->drawLine(cx + cos(rad2) * innerR, cy + sin(rad2) * innerR,
                  cx + cos(rad2) * r, cy + sin(rad2) * r, color);
  }

  // --- 3. CENTER STAGE ---
  // Gear 'Badge'
  int gear = (_lastSpeed < 2) ? 0 : (_lastSpeed / 20) + 1;
  if (gear > 6)
    gear = 6;

  if (force) {
    // Gear box background look
    tft->drawRect(cx - 20, cy - 85, 40, 30, 0x18E3);
  }
  tft->setTextFont(4);
  tft->setTextColor(TFT_YELLOW, COL_BG);
  tft->setTextDatum(MC_DATUM);
  if (gear == 0)
    tft->drawString("N", cx, cy - 70);
  else
    tft->drawNumber(gear, cx, cy - 70);

  // Main Speed
  tft->setTextColor(COL_MAIN, COL_BG);
  tft->setTextFont(7);
  tft->setTextSize(2);
  tft->setTextPadding(200);
  tft->drawNumber((int)_lastSpeed, cx, cy + 10);
  tft->setTextPadding(0);

  // Unit Label
  tft->setTextFont(2);
  tft->setTextColor(COL_ACCENT, COL_BG);
  tft->drawString("KM/H", cx, cy + 65);

  // --- 4. NAVIGATION ---
  if (force)
    _ui->drawBackButton();
}
