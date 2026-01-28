#include "SessionSummaryScreen.h"
#include "../../core/GPSManager.h"
#include "../fonts/Org_01.h"

extern GPSManager gpsManager;

void SessionSummaryScreen::onShow() {
  _ui->setTitle("SESSION SUMMARY");
  drawSummary();
}

void SessionSummaryScreen::update() {
  UIManager::TouchPoint p = _ui->getTouchPoint();
  if (p.x != -1) {
    if (_ui->isBackButtonTouched(p)) {
      _ui->switchScreen(SCREEN_LAP_TIMER);
    }
  }
}

void SessionSummaryScreen::drawSummary() {
  TFT_eSPI *tft = _ui->getTft();
  ActiveSessionData &data = _ui->getLastSession();

  tft->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                SCREEN_HEIGHT - STATUS_BAR_HEIGHT, TFT_BLACK);
  _ui->drawBackButton();

  int headY = 18;
  tft->setTextDatum(MC_DATUM);
  tft->setFreeFont(&Org_01);
  tft->setTextSize(2);
  tft->setTextColor(TFT_WHITE, TFT_BLACK);
  tft->drawString("SESSION SUMMARY", SCREEN_WIDTH / 2, headY + 15);

  // --- Layout ---
  int leftW = 260;
  int rightW = SCREEN_WIDTH - leftW - 30;
  int contentY = 65;

  // 1. Best Lap Card
  int cardX = 10;
  int cardY = contentY;
  int cardH = 65;
  tft->fillRoundRect(cardX, cardY, leftW, cardH, 8, 0x18E3);
  tft->drawRoundRect(cardX, cardY, leftW, cardH, 8, TFT_DARKGREY);

  tft->setTextColor(TFT_SILVER, 0x18E3);
  tft->setTextFont(2);
  tft->setTextDatum(TL_DATUM);
  tft->drawString("BEST LAP", cardX + 10, cardY + 8);

  if (!data.lapTimes.empty()) {
    // Find best lap
    unsigned long best = data.lapTimes[0];
    int bestIdx = 0;
    for (int i = 1; i < data.lapTimes.size(); i++) {
      if (data.lapTimes[i] < best) {
        best = data.lapTimes[i];
        bestIdx = i;
      }
    }

    tft->fillRoundRect(cardX + leftW - 60, cardY + 8, 50, 16, 4, TFT_GOLD);
    tft->setTextColor(TFT_BLACK, TFT_GOLD);
    tft->setTextDatum(MC_DATUM);
    tft->drawString("LAP " + String(bestIdx + 1), cardX + leftW - 35,
                    cardY + 16);

    int ms = best % 1000;
    int s = (best / 1000) % 60;
    int m = (best / 60000);
    char buf[16];
    sprintf(buf, "%d:%02d.%02d", m, s, ms / 10);

    tft->setTextColor(TFT_WHITE, 0x18E3);
    tft->setTextFont(6);
    tft->drawString(buf, cardX + leftW / 2, cardY + cardH / 2 + 8);
  } else {
    tft->setTextColor(TFT_DARKGREY, 0x18E3);
    tft->setTextFont(4);
    tft->setTextDatum(MC_DATUM);
    tft->drawString("--:--.--", cardX + leftW / 2, cardY + cardH / 2 + 8);
  }

  // 2. Stats Grid
  int gridY = cardY + cardH + 10;
  int boxW = (leftW - 10) / 2;
  int boxH = 45;

  tft->fillRoundRect(cardX, gridY, boxW, boxH, 6, 0x10A2);
  tft->setTextColor(TFT_SILVER, 0x10A2);
  tft->setTextFont(1);
  tft->setTextDatum(TL_DATUM);
  tft->drawString("TOTAL LAPS", cardX + 8, gridY + 5);
  tft->setTextColor(TFT_SKYBLUE, 0x10A2);
  tft->setTextFont(4);
  tft->setTextDatum(MC_DATUM);
  tft->drawNumber(data.lapCount, cardX + boxW / 2, gridY + 25);

  tft->fillRoundRect(cardX + boxW + 10, gridY, boxW, boxH, 6, 0x10A2);
  tft->setTextColor(TFT_SILVER, 0x10A2);
  tft->setTextFont(1);
  tft->setTextDatum(TL_DATUM);
  tft->drawString("MAX RPM", cardX + boxW + 18, gridY + 5);
  tft->setTextColor(TFT_ORANGE, 0x10A2);
  tft->drawNumber(data.maxRpm, cardX + boxW + 10 + boxW / 2, gridY + 25);

  // 3. Recent Laps
  int listY = gridY + boxH + 10;
  tft->drawFastHLine(cardX, listY, leftW, TFT_DARKGREY);
  tft->setTextFont(1);
  tft->setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft->setTextDatum(TL_DATUM);
  tft->drawString("RECENT LAPS", cardX, listY + 5);

  int rowY = listY + 20;
  int itemsToShow = 4;
  int startIdx = (data.lapTimes.size() > itemsToShow)
                     ? data.lapTimes.size() - itemsToShow
                     : 0;

  unsigned long bestTimeGlobal = 9999999;
  for (auto t : data.lapTimes)
    if (t < bestTimeGlobal)
      bestTimeGlobal = t;

  for (int i = (int)data.lapTimes.size() - 1; i >= startIdx; i--) {
    unsigned long t = data.lapTimes[i];
    int ms = t % 1000;
    int s = (t / 1000) % 60;
    int m = (t / 60000);
    char buf[32];
    sprintf(buf, "%d:%02d.%02d", m, s, ms / 10);

    tft->setTextFont(1);
    tft->setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    tft->drawString(String(i + 1) + ".", cardX, rowY);
    tft->setTextColor((t == bestTimeGlobal) ? TFT_GREEN : TFT_WHITE, TFT_BLACK);
    tft->setTextFont(2);
    tft->drawString(buf, cardX + 30, rowY);

    if (t != bestTimeGlobal && bestTimeGlobal != 9999999) {
      long delta = (long)t - (long)bestTimeGlobal;
      sprintf(buf, "+%d.%02d", (int)(delta / 1000), (int)(delta % 1000) / 10);
      tft->setTextColor(TFT_RED, TFT_BLACK);
      tft->setTextDatum(TR_DATUM);
      tft->drawString(buf, cardX + leftW - 5, rowY);
      tft->setTextDatum(TL_DATUM);
    }
    rowY += 21;
  }

  // 4. Map Preview
  int mapX = leftW + 20;
  int mapY = contentY;
  int mapH = SCREEN_HEIGHT - contentY - 10;
  tft->fillRoundRect(mapX, mapY, rightW, mapH, 8, 0x10A2);
  tft->drawRoundRect(mapX, mapY, rightW, mapH, 8, TFT_DARKGREY);

  drawTrackMap(mapX + 5, mapY + 5, rightW - 10, mapH - 10, data.trackPoints);
}

void SessionSummaryScreen::drawTrackMap(int x, int y, int w, int h,
                                        const std::vector<GPSPoint> &points) {
  TFT_eSPI *tft = _ui->getTft();
  if (points.empty()) {
    tft->setTextColor(TFT_DARKGREY, 0x10A2);
    tft->setTextDatum(MC_DATUM);
    tft->drawString("NO MAP", x + w / 2, y + h / 2);
    return;
  }

  double minLat = 90.0, maxLat = -90.0, minLon = 180.0, maxLon = -180.0;
  for (const auto &p : points) {
    if (p.lat < minLat)
      minLat = p.lat;
    if (p.lat > maxLat)
      maxLat = p.lat;
    if (p.lon < minLon)
      minLon = p.lon;
    if (p.lon > maxLon)
      maxLon = p.lon;
  }

  double dLat = maxLat - minLat;
  double dLon = maxLon - minLon;
  if (dLat == 0)
    dLat = 0.0001;
  if (dLon == 0)
    dLon = 0.0001;

  double scale = min(w / dLon, h / dLat) * 0.9;
  int offsetX = x + (w - dLon * scale) / 2;
  int offsetY = y + (h - dLat * scale) / 2;

  for (size_t i = 1; i < points.size(); i++) {
    int x1 = offsetX + (points[i - 1].lon - minLon) * scale;
    int y1 = offsetY + (maxLat - points[i - 1].lat) * scale;
    int x2 = offsetX + (points[i].lon - minLon) * scale;
    int y2 = offsetY + (maxLat - points[i].lat) * scale;
    tft->drawLine(x1, y1, x2, y2, TFT_WHITE);
  }
}
