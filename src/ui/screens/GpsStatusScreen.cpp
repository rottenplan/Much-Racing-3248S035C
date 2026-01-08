#include "GpsStatusScreen.h"
#include "../../core/GPSManager.h"
#include "../fonts/Org_01.h"

extern GPSManager gpsManager;

// Safe Area Offset (Status Bar Height)
#define TOP_OFFSET 25

void GpsStatusScreen::onShow() {
  TFT_eSPI *tft = _ui->getTft();
  tft->fillScreen(TFT_BLACK); // "Negative UI" = Dark Mode

  // Draw Top-Left Date/Time Box Frame (Optional, or just text)
  // Let's keep it clean black background.

  // Initial values reset
  _lastSats = -1;
  _lastHz = -1;
  _lastLat = 0;
  _lastLng = 0;
  _lastFixed = false; // Force re-draw of radar rings

  drawStatus();
}

void GpsStatusScreen::update() {
  // Touch to Back
  UIManager::TouchPoint p = _ui->getTouchPoint();
  if (p.x != -1) {
    if (p.x < 60 && p.y > TOP_OFFSET && p.y < (TOP_OFFSET + 40)) {
      _ui->switchScreen(SCREEN_MENU);
      return;
    }
  }

  drawStatus();
}

void GpsStatusScreen::drawStatus() {
  TFT_eSPI *tft = _ui->getTft();

  int sats = gpsManager.getSatellites();
  double hdop = gpsManager.getHDOP();
  int hz = gpsManager.getUpdateRate();
  double lat = gpsManager.getLatitude();
  double lng = gpsManager.getLongitude();

  // --- 1. Date/Time (White Text on Black) ---
  int h, m, s, d, mo, y;
  gpsManager.getLocalTime(h, m, s, d, mo, y);

  // We need to update this every second ideally (Use static timer or check
  // distinct second)
  static unsigned long lastTimeDraw = 0;
  if (millis() - lastTimeDraw > 1000) {
    lastTimeDraw = millis();

    // Clear area (if needed, but text overwrite might work if consistent width)
    tft->fillRect(0, TOP_OFFSET, 180, 45, TFT_BLACK);

    tft->setTextColor(TFT_WHITE, TFT_BLACK);
    tft->setTextDatum(TL_DATUM);
    tft->setFreeFont(&Org_01);
    tft->setTextSize(1);

    char dateBuf[32];
    sprintf(dateBuf, "%02d/%02d/%04d UTC+07:00", mo, d, y);
    tft->drawString(dateBuf, 5, TOP_OFFSET + 5);

    tft->setTextFont(4); // Large Font
    tft->setTextSize(1);
    char timeBuf[16];
    sprintf(timeBuf, "%02d:%02d:%02d", h, m, s);
    tft->drawString(timeBuf, 5, TOP_OFFSET + 20);
  }

  // --- 2. Lat/Lng (White Text on Black) ---
  if (abs(lat - _lastLat) > 0.00001 || abs(lng - _lastLng) > 0.00001) {
    tft->setTextColor(TFT_WHITE, TFT_BLACK);
    tft->setTextDatum(TL_DATUM);
    tft->setFreeFont(&Org_01);
    tft->setTextSize(2);

    int yLat = TOP_OFFSET + 65;
    int yLng = TOP_OFFSET + 95;

    // Labels (Simulating the pixel font look)
    tft->drawString("Lat:", 10, yLat);
    tft->drawString("Lng:", 10, yLng);

    // Values
    tft->setTextFont(4);
    tft->setTextSize(1);

    // Clear areas
    tft->fillRect(60, yLat - 5, 120, 25, TFT_BLACK);
    tft->fillRect(60, yLng - 5, 120, 25, TFT_BLACK);

    tft->drawString(String(lat, 6), 60, yLat - 5);
    tft->drawString(String(lng, 6), 60, yLng - 5);

    _lastLat = lat;
    _lastLng = lng;
  }

  // --- 3. Sat Counts (Text) ---
  if (sats != _lastSats) {
    tft->setFreeFont(&Org_01);
    tft->setTextSize(1);
    tft->setTextColor(TFT_WHITE, TFT_BLACK);
    tft->setTextDatum(TL_DATUM);

    int yRow1 = TOP_OFFSET + 135;
    int yRow2 = TOP_OFFSET + 150;

    tft->fillRect(10, yRow1, 150, 35, TFT_BLACK);

    char buf[32];
    sprintf(buf, "GPS:%d Sat. GAL:0 Sat.", sats);
    tft->drawString(buf, 10, yRow1);

    sprintf(buf, "GLO:0 Sat. BEI:0 Sat.");
    tft->drawString(buf, 10, yRow2);

    _lastSats = sats;
  }

  // --- 4. Footer Info ---
  if (abs(hdop - _lastHdop) > 0.01 || hz != _lastHz) {
    tft->setFreeFont(&Org_01);
    tft->setTextSize(1);
    tft->setTextColor(TFT_WHITE, TFT_BLACK);
    tft->setTextDatum(BL_DATUM);

    int yFooter = 235; // Fixed near bottom (320x240 screen)

    tft->fillRect(0, yFooter - 15, 200, 20, TFT_BLACK);

    char footerBuf[64];
    int fixQ = gpsManager.isFixed() ? 3 : 0;

    // Added Hz to footer or check if it fits
    sprintf(footerBuf, "SAM-M10Q [H] A:%d HDOP:%.2f", fixQ, hdop);
    tft->drawString(footerBuf, 5, yFooter);

    _lastHdop = hdop;
    _lastHz = hz;
  }

  // --- 5. Radar Plot (Right Side) ---
  // Center X = 240, Center Y = 120 + Offset?
  // If we offset Y, it might clip bottom.
  // Let's keep Y center at 120 + (OFFSET/2) -> 132?
  // 120 + 12 = 132.
  int cX = 240;
  int cY = 120 + (TOP_OFFSET / 2);
  int r = 65; // Slightly smaller to fit with offset

  if (!_lastFixed) {
    // Radar lines are WHITE on BLACK.
    tft->drawCircle(cX, cY, r, TFT_WHITE);
    tft->drawCircle(cX, cY, r * 0.6, TFT_WHITE);
    tft->drawCircle(cX, cY, r * 0.2, TFT_WHITE);

    tft->drawLine(cX - r, cY, cX + r, cY, TFT_WHITE);
    tft->drawLine(cX, cY - r, cX, cY + r, TFT_WHITE);

    // Labels N/E/S/W (Inverted: White Circle, Black Text OR Black Circle White
    // Text) "Negative UI" -> Usually White Lines on Black. Let's use White
    // Circle, Black Text for contrast pop.
    auto drawDir = [&](String d, int x, int y) {
      tft->fillCircle(x, y, 7, TFT_WHITE);
      tft->setTextColor(TFT_BLACK, TFT_WHITE);
      tft->setTextDatum(MC_DATUM);
      tft->setFreeFont(&Org_01);
      tft->setTextSize(1);
      tft->drawString(d, x, y + 1);
    };
    drawDir("N", cX, cY - r);
    drawDir("S", cX, cY + r);
    drawDir("E", cX + r, cY);
    drawDir("W", cX - r, cY);

    _lastFixed = true;
  }
}
