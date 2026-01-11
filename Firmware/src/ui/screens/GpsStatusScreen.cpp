#include "GpsStatusScreen.h"
#include "../../core/GPSManager.h"
#include "../fonts/Org_01.h"

extern GPSManager gpsManager;

// Safe Area Offset (Status Bar Height)
#define TOP_OFFSET 25

void GpsStatusScreen::onShow() {
  TFT_eSPI *tft = _ui->getTft();
  // Clear only content area
  tft->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                SCREEN_HEIGHT - STATUS_BAR_HEIGHT, COLOR_BG);

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
      if (millis() - lastBackTap < 1000) {
        _ui->switchScreen(SCREEN_MENU);
        lastBackTap = 0;
      } else {
        lastBackTap = millis();
      }
      return;
    }

    // Check for Double Tap on body
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

void GpsStatusScreen::drawStatus() {
  TFT_eSPI *tft = _ui->getTft();

  int sats = gpsManager.getSatellites();
  double hdop = gpsManager.getHDOP();
  int hz = gpsManager.getUpdateRate();
  double lat = gpsManager.getLatitude();
  double lng = gpsManager.getLongitude();

  // --- 1. Date/Time (Highlighted in Middle) ---
  int h, m, s, d, mo, y;
  gpsManager.getLocalTime(h, m, s, d, mo, y);

  static unsigned long lastTimeDraw = 0;
  // If we just entered, force draw?
  // We can use _lastSats == -1 as a 'first run' indicator or just checking
  // millis. Ideally, lastTimeDraw should be reset in onShow, but it's static
  // local. Workaround: Use a member variable for drawing timer or just let it
  // update.

  if (millis() - lastTimeDraw > 1000 ||
      _lastSats == -1) { // Force update on first run
    lastTimeDraw = millis();

    // White Box Highlight aligned with Radar (approx Y=105)
    int yTime = TOP_OFFSET + 80;

    // Prepare Date String to calculate width
    tft->setFreeFont(&Org_01);
    tft->setTextSize(1);
    char dateBuf[32];
    sprintf(dateBuf, "%02d/%02d/%04d UTC+07:00", mo, d, y);
    int dateW = tft->textWidth(dateBuf);

    // Check Time Width (Font 4)
    tft->setTextFont(4);
    char timeBuf[16];
    sprintf(timeBuf, "%02d:%02d:%02d", h, m, s);
    int timeW = tft->textWidth(timeBuf);

    // Calculate Box Width (Max of Date/Time + padding), capped at 170 (Radar
    // starts at 175)
    int boxW = (dateW > timeW ? dateW : timeW) + 15;
    if (boxW > 170)
      boxW = 170;

    tft->fillRect(0, yTime, boxW, 45, TFT_WHITE);

    tft->setTextColor(TFT_BLACK, TFT_WHITE); // Inverted Text
    tft->setTextDatum(TL_DATUM);

    // Draw Date
    tft->setFreeFont(&Org_01);
    tft->setTextSize(1);
    tft->drawString(dateBuf, 5, yTime + 5);

    // Draw Time
    tft->setTextFont(4); // Large Font
    tft->setTextSize(1);
    tft->drawString(timeBuf, 5, yTime + 20);
  }

  // --- 2. Lat/Lng (Highlighted) ---
  // Draw only if changed significantly, but for highlighting box we might want
  // to redraw if not present? Actually, let's redraw the values on top of the
  // box. Ideally, draw the box once or only when values update? If we redraw
  // values, we need to clear previous. With a box, we can just refill the box
  // or part of it.

  // Let's refill the whole Lat/Lng area to be safe and clean.
  if (abs(lat - _lastLat) > 0.00001 ||
      abs(lng - _lastLng) > 0.00001) { // Or force redraw logic if needed
    int yLatBox = TOP_OFFSET + 10;
    int boxHeight = 55;
    int boxWidth = 170; // Match Date/Time box width cap approx

    // Draw White Box
    tft->fillRect(0, yLatBox, boxWidth, boxHeight, TFT_WHITE);

    tft->setTextColor(TFT_BLACK, TFT_WHITE); // Inverted
    tft->setTextDatum(TL_DATUM);

    // Labels & Values
    // LAT
    tft->setFreeFont(&Org_01);
    tft->setTextSize(1);
    tft->drawString("LAT", 5, yLatBox + 5);

    tft->setTextFont(4); // Large Font for Value
    tft->setTextSize(1);
    tft->drawString(String(lat, 6), 35, yLatBox + 2);

    // LNG
    int yLngRow = yLatBox + 28;
    tft->setFreeFont(&Org_01);
    tft->setTextSize(1);
    tft->drawString("LNG", 5, yLngRow + 5);

    tft->setTextFont(4);
    tft->setTextSize(1);
    tft->drawString(String(lng, 6), 35, yLngRow + 2);

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

    int yFooter = 190; // Moved up above Back Button

    tft->fillRect(5, yFooter - 15, 300, 20, TFT_BLACK); // Wider clear

    char footerBuf[64];
    int fixQ = gpsManager.isFixed() ? 3 : 0;
    int rx = gpsManager.getRxPin();
    int tx = gpsManager.getTxPin();
    int baud = gpsManager.getBaud();

    // Show Pins and Baud for debugging
    sprintf(footerBuf, "R:%d T:%d B:%d A:%d HDOP:%.2f", rx, tx, baud, fixQ,
            hdop);
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
