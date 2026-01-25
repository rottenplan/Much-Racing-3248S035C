#include "GpsStatusScreen.h"
#include "../../core/GPSManager.h"
#include "../fonts/Org_01.h"

extern GPSManager gpsManager;

// Safe Area Offset (Status Bar Height)
#define TOP_OFFSET 25

void GpsStatusScreen::onShow() {
  TFT_eSPI *tft = _ui->getTft();
  // Clear entire screen to black to prevent artifacts
  tft->fillScreen(TFT_BLACK);

  // Force Status Bar Redraw to ensure no artifacts at top
  _ui->drawStatusBar(true);

  // Draw Top-Left Date/Time Box Frame (Optional, or just text)
  // Let's keep it clean black background.

  // Initial values reset
  _lastSats = -1;
  _lastHz = -1;
  _lastLat = -999;
  _lastLng = -999;
  _lastHdop = -1.0;   // Reset HDOP tracker
  _lastFixed = false; // Force re-draw of radar rings

  // Clear any potential leftover artifacts manually if needed,
  // but fillScreen should handle it. The 'white lines' might be status bar
  // related, but let's ensure clean state here.

  // Back Button (Blue Triangle) - Bottom Left
  tft->fillTriangle(10, 220, 22, 214, 22, 226, TFT_BLUE);

  // Log Button (Orange Label) - Bottom Right
  tft->fillRoundRect(260, 210, 50, 20, 4, TFT_ORANGE);
  tft->setTextColor(TFT_BLACK, TFT_ORANGE);
  tft->setTextDatum(MC_DATUM);
  tft->setTextFont(1);
  tft->drawString("LOG", 285, 221);

  drawStatus();
}

void GpsStatusScreen::update() {
  // Touch Handling
  UIManager::TouchPoint p = _ui->getTouchPoint();
  if (p.x != -1) {
    // Back Button Area
    // Back Button Area (Bottom Left)
    if (p.x < 60 && p.y > 200) {
      static unsigned long lastBackTap = 0;
      if (millis() - lastBackTap < 500) {
        _ui->switchScreen(SCREEN_MENU);
        lastBackTap = 0;
      } else {
        lastBackTap = millis();
      }
      return;
    }

    // Check for "LOG" Button Area (Bottom Right)
    if (p.x > 260 && p.y > 200) {
      _ui->switchScreen(SCREEN_GNSS_LOG);
      return;
    }

    // Check for Double Tap on body (Keep as backup)
    unsigned long now = millis();
    if (now - _lastTapTime < 500) {
      // Double Tap!
      _ui->switchScreen(SCREEN_GNSS_LOG);
      _lastTapTime = 0; // Reset
      return;
    }
    _lastTapTime = now;
  }

  drawStatus();
}

// Safe Area Offset (Status Bar Height)
#define TOP_OFFSET 25

void GpsStatusScreen::drawStatus() {
  TFT_eSPI *tft = _ui->getTft();

  int sats = gpsManager.getSatellites();
  double hdop = gpsManager.getHDOP();
  int hz = gpsManager.getUpdateRate();
  double lat = gpsManager.getLatitude();
  double lng = gpsManager.getLongitude();

  // Colors
  uint16_t COLOR_CARD = 0x18E3;
  uint16_t COLOR_LABEL = TFT_SILVER;
  uint16_t COLOR_VALUE = TFT_WHITE;

  // --- 1. DATE/TIME & LAT/LON CARD (Left Side) ---
  // If first run or interval, redraw card base
  static unsigned long lastUpdate = 0;
  bool forceRedraw = (_lastSats == -1);

  if (forceRedraw || millis() - lastUpdate > 1000) {
    lastUpdate = millis();
    int cardX = 10;
    int cardY = TOP_OFFSET + 10;
    int cardW = 160;
    int cardH = 135;

    // Draw Card Background (only if needed or clean update)
    if (forceRedraw) {
      tft->fillRoundRect(cardX, cardY, cardW, cardH, 8, COLOR_CARD); // Charcoal
      tft->drawRoundRect(cardX, cardY, cardW, cardH, 8, TFT_DARKGREY);

      // Labels
      tft->setTextColor(COLOR_LABEL, COLOR_CARD);
      tft->setTextDatum(TL_DATUM);
      tft->setFreeFont(&Org_01);
      tft->setTextSize(1);
      tft->drawString("LOCATION", cardX + 10, cardY + 5);

      tft->drawLine(cardX + 5, cardY + 20, cardX + cardW - 5, cardY + 20,
                    TFT_DARKGREY);
    }

    // Dynamic Values - Time
    int h, m, s, d, mo, y;
    gpsManager.getLocalTime(h, m, s, d, mo, y);

    char dateBuf[32];
    sprintf(dateBuf, "%02d/%02d/%04d", d, mo, y);
    char timeBuf[16];
    sprintf(timeBuf, "%02d:%02d:%02d", h, m, s);

    tft->setTextColor(TFT_SKYBLUE, COLOR_CARD);
    tft->setTextDatum(TL_DATUM);
    tft->setTextFont(2);
    tft->drawString(dateBuf, cardX + 10, cardY + 25);

    tft->setTextFont(4); // Big Time
    tft->setTextColor(TFT_WHITE, COLOR_CARD);
    tft->drawString(timeBuf, cardX + 10, cardY + 42);

    // Dynamic Values - Lat/Lon
    // LAT
    tft->setTextColor(COLOR_LABEL, COLOR_CARD);
    tft->setFreeFont(&Org_01);
    tft->setTextSize(1);
    tft->drawString("LAT", cardX + 10, cardY + 75);

    tft->setTextColor(TFT_WHITE, COLOR_CARD);
    tft->setTextFont(2);
    tft->drawString(String(lat, 6), cardX + 40, cardY + 72);

    // LON
    tft->setTextColor(COLOR_LABEL, COLOR_CARD);
    tft->setFreeFont(&Org_01);
    tft->setTextSize(1);
    tft->drawString("LON", cardX + 10, cardY + 95);

    tft->setTextColor(TFT_WHITE, COLOR_CARD);
    tft->setTextFont(2);
    tft->drawString(String(lng, 6), cardX + 40, cardY + 92);

    // Alt / Heading tiny
    int alt = (int)gpsManager.getAltitude();
    int head = (int)gpsManager.getHeading();
    tft->setTextColor(TFT_ORANGE, COLOR_CARD);
    tft->setFreeFont(&Org_01);
    tft->drawString("ALT: " + String(alt) + "m", cardX + 10, cardY + 115);
    tft->drawString("DIR: " + String(head), cardX + 90, cardY + 115);
  }

  // --- 2. SATELLITE INFO (Bottom Card) ---
  if (forceRedraw || sats != _lastSats || hz != _lastHz) {
    int cardX = 10;
    int cardY = TOP_OFFSET + 150; // Moved up to 175
    int cardW = SCREEN_WIDTH - 20;
    int cardH = 60; // Increased height to fit content

    if (forceRedraw) {
      tft->fillRoundRect(cardX, cardY, cardW, cardH, 8, 0x10A2); // Slate
    }

    // Clear text area inside card
    tft->fillRoundRect(cardX + 2, cardY + 25, cardW - 4, 33, 0, 0x10A2);

    tft->setTextColor(TFT_SILVER, 0x10A2);
    tft->setTextDatum(TL_DATUM);
    tft->setFreeFont(&Org_01);
    tft->setTextSize(1);

    if (forceRedraw) {
      tft->drawString("STATUS", cardX + 10, cardY + 5);
      // Labels moved UP
      tft->drawString("SATS", cardX + 30, cardY + 18);
      tft->drawString("Hz", cardX + 80, cardY + 18);
      tft->drawString("HDOP", cardX + 130, cardY + 18);
    }

    // Sat Count (Values moved DOWN)
    int valY = cardY + 42;

    tft->setTextColor(TFT_GREEN, 0x10A2);
    tft->setTextFont(4);
    tft->setTextDatum(MC_DATUM);
    tft->drawString(String(sats), cardX + 30, valY);

    // Hz
    tft->setTextColor(TFT_CYAN, 0x10A2);
    tft->setTextFont(4);
    tft->drawString(String(hz), cardX + 80, valY);

    // HDOP
    tft->setTextColor(TFT_YELLOW, 0x10A2);
    tft->setTextFont(4);
    tft->drawString(String(hdop, 1), cardX + 130, valY);

    // Fix Quality
    String fixStr = gpsManager.isFixed() ? "3D FIX" : "NO FIX";
    uint16_t fixColor = gpsManager.isFixed() ? TFT_GREEN : TFT_RED;
    tft->setTextColor(fixColor, 0x10A2);
    tft->setTextFont(2);
    tft->setTextDatum(MR_DATUM);
    tft->drawString(fixStr, cardX + cardW - 10, valY);

    _lastSats = sats;
    _lastHz = hz;
  }

  // --- 3. RADAR (Right Side) ---
  int radarX = 180;
  int radarY = TOP_OFFSET + 10;
  int radarW = 130;
  int radarH = 135;
  int cX = radarX + radarW / 2;
  int cY = radarY + radarH / 2;
  int r = 55;

  if (forceRedraw) {
    // Container
    tft->fillRoundRect(radarX, radarY, radarW, radarH, 8,
                       TFT_BLACK); // Keep Black for contrast
    tft->drawRoundRect(radarX, radarY, radarW, radarH, 8, TFT_DARKGREY);

    // Circles
    tft->drawCircle(cX, cY, r, TFT_DARKGREY);
    tft->drawCircle(cX, cY, r * 0.6, TFT_DARKGREY);
    tft->drawCircle(cX, cY, r * 0.2, TFT_DARKGREY);

    // Crosshair
    tft->drawLine(cX - r, cY, cX + r, cY, TFT_DARKGREY);
    tft->drawLine(cX, cY - r, cX, cY + r, TFT_DARKGREY);

    // Labels
    tft->setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft->setTextDatum(MC_DATUM);
    tft->setFreeFont(&Org_01);
    tft->setTextSize(1);
    tft->drawString("N", cX, cY - r - 5);
  }

  if (!_lastFixed) {
    // Logic for satellites if we had them or just static crosshair
    _lastFixed = true;
  }
}
