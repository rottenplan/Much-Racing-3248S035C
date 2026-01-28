#include "RacingDashboardScreen.h"
#include "../../core/GPSManager.h"
#include "../../core/SessionManager.h"
#include "../fonts/Org_01.h"

extern GPSManager gpsManager;
extern SessionManager sessionManager;

// Define STOP Button Area
#define STOP_BTN_Y 200

void RacingDashboardScreen::onShow() {
  _currentTrack = _ui->getSelectedTrack();
  _currentLapStart = millis();
  _lastLapTime = 0;
  _bestLapTime = 0;
  _lapCount = 0;
  _lapTimes.clear();
  _maxRpmSession = 0;
  _finishLineInside = false;
  _lastFinishCross = 0;

  _lastSpeed = -1.0;
  _lastSats = -1;
  _lastRpmRender = -1;
  _needsStaticRedraw = true;

  // Start Logging
  sessionManager.startSession();

  _ui->setTitle(_currentTrack.name);
  drawStatic();
}

void RacingDashboardScreen::onHide() {
  // If moving to summary, we already stopped. But defensive stop:
  if (sessionManager.isLogging()) {
    sessionManager.stopSession();
  }
}

void RacingDashboardScreen::update() {
  UIManager::TouchPoint p = _ui->getTouchPoint();
  if (p.x != -1) {
    // Stop Button
    int cx = SCREEN_WIDTH / 2;
    if (p.x > cx - 100 && p.x < cx + 100 && p.y > STOP_BTN_Y) {
      if (sessionManager.isLogging()) {
        String dateStr =
            gpsManager.getDateString() + " " + gpsManager.getTimeString();
        sessionManager.appendToHistoryIndex(_currentTrack.name, dateStr,
                                            _lapCount, _bestLapTime, "TRACK");
        sessionManager.stopSession();
      }

      // Prepare data for summary
      ActiveSessionData data;
      data.trackName = _currentTrack.name;
      data.lapTimes = _lapTimes;
      data.bestLapTime = _bestLapTime;
      data.lapCount = _lapCount;
      data.maxRpm = _maxRpmSession;
      // data.trackPoints = _currentTrack.trackPoints; // Track struct doesn't
      // have trackPoints yet

      _ui->setLastSession(data);
      _ui->switchScreen(SCREEN_SESSION_SUMMARY);
      return;
    }
  }

  // Logic: Finish Line
  checkFinishLine();

  // Logic: Logging
  if (sessionManager.isLogging() && (millis() - _lastUpdate > 100)) {
    String data = String(millis()) + "," + String(gpsManager.getLatitude(), 6) +
                  "," + String(gpsManager.getLongitude(), 6) + "," +
                  String(gpsManager.getSpeedKmph()) + "," +
                  String(gpsManager.getSatellites()) + "," +
                  String(gpsManager.getAltitude(), 2) + "," +
                  String(gpsManager.getHeading(), 2);
    sessionManager.logData(data);
  }

  // UI Redraw
  if (_needsStaticRedraw) {
    drawStatic();
    _needsStaticRedraw = false;
    _lastUpdate = 0;
  }

  if (millis() - _lastUpdate > 100) {
    drawDynamic();
    _lastUpdate = millis();
  }
}

void RacingDashboardScreen::drawStatic() {
  TFT_eSPI *tft = _ui->getTft();
  tft->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                SCREEN_HEIGHT - STATUS_BAR_HEIGHT, TFT_BLACK);

  // Layout Constants
  int topBarH = STATUS_BAR_HEIGHT;
  int rpmH = 40;
  int midY = topBarH + rpmH + 15;
  int midH = 140;
  int mapW = 160;
  int metricsX = 15 + mapW + 10;
  int metricsW = SCREEN_WIDTH - metricsX - 15;
  int speedH = midH / 2 - 5;
  int timeH = midH / 2 - 5;
  int gridY = midY + midH + 10;
  int gridH = SCREEN_HEIGHT - gridY - 10;
  int cardW = (SCREEN_WIDTH - 20) / 3;

  // 1. Map Box
  tft->fillRoundRect(10, midY, mapW, midH, 8, 0x10A2);
  tft->drawRoundRect(10, midY, mapW, midH, 8, TFT_DARKGREY);

  // 2. Metrics Boxes (Speed & Time)
  tft->fillRoundRect(metricsX, midY, metricsW, speedH, 8, 0x18E3);
  tft->drawRoundRect(metricsX, midY, metricsW, speedH, 8, TFT_DARKGREY);
  tft->fillRoundRect(metricsX, midY + speedH + 10, metricsW, timeH, 8, 0x18E3);
  tft->drawRoundRect(metricsX, midY + speedH + 10, metricsW, timeH, 8,
                     TFT_DARKGREY);

  // Labels
  tft->setTextColor(TFT_SILVER, 0x18E3);
  tft->setTextFont(2);
  tft->setTextDatum(TL_DATUM);
  tft->drawString("SPEED", metricsX + 10, midY + 5);
  tft->drawString("TIME", metricsX + 10, midY + speedH + 15);

  // 3. Grid Boxes (Sats, Last, Best)
  for (int i = 0; i < 3; i++) {
    int x = 10 + i * (cardW + 5);
    tft->fillRoundRect(x, gridY, cardW, gridH, 8, 0x10A2);
    tft->drawRoundRect(x, gridY, cardW, gridH, 8, TFT_DARKGREY);
  }

  tft->setTextColor(TFT_SILVER, 0x10A2);
  tft->setTextFont(1);
  tft->drawString("GPS SATS", 15, gridY + 5);
  tft->drawString("LAST LAP", 15 + cardW + 5, gridY + 5);
  tft->drawString("BEST LAP", 15 + (cardW + 5) * 2, gridY + 5);

  // STOP Button Background
  int cx = SCREEN_WIDTH / 2;
  tft->fillRoundRect(cx - 70, STOP_BTN_Y + 10, 140, 40, 8, TFT_RED);
  tft->setTextColor(TFT_WHITE, TFT_RED);
  tft->setTextFont(4);
  tft->setTextDatum(MC_DATUM);
  tft->drawString("STOP", cx, STOP_BTN_Y + 30);

  drawTrackMap(15, midY + 10, mapW - 10, midH - 20);
}

void RacingDashboardScreen::drawDynamic() {
  TFT_eSPI *tft = _ui->getTft();

  // RPM
  int rpm = gpsManager.getRPM();
  if (abs(rpm - _lastRpmRender) > 50) {
    drawRPMBar(rpm, 10000);
    _lastRpmRender = rpm;
  }

  // Layout for values
  int mapW = 160;
  int metricsX = 15 + mapW + 10;
  int metricsW = SCREEN_WIDTH - metricsX - 15;
  int midY = STATUS_BAR_HEIGHT + 40 + 15;
  int speedH = 140 / 2 - 5;
  int gridY = midY + 140 + 10;
  int gridH = SCREEN_HEIGHT - gridY - 10;
  int cardW = (SCREEN_WIDTH - 20) / 3;

  // Speed
  float speed = gpsManager.getSpeedKmph();
  tft->setTextColor(TFT_CYAN, 0x18E3);
  tft->setTextFont(7);
  tft->setTextDatum(MC_DATUM);
  tft->drawFloat(speed, 1, metricsX + metricsW / 2, midY + speedH / 2 + 8);

  // Current Time
  unsigned long currentLap = millis() - _currentLapStart;
  char timeBuf[16];
  sprintf(timeBuf, "%02d:%02d", (int)(currentLap / 60000),
          (int)(currentLap / 1000) % 60);
  tft->setTextColor(TFT_WHITE, 0x18E3);
  tft->drawString(timeBuf, metricsX + metricsW / 2,
                  midY + speedH + 10 + speedH / 2 + 8);

  // GPS Sats
  int sats = gpsManager.getSatellites();
  bool fix = gpsManager.isFixed();
  tft->setTextColor(fix ? TFT_GREEN : TFT_RED, 0x10A2);
  tft->setTextFont(4);
  tft->drawNumber(sats, 10 + cardW / 2, gridY + gridH / 2 + 8);

  // Last Lap
  if (_lastLapTime > 0) {
    char lBuf[16];
    sprintf(lBuf, "%d:%02d.%d", (int)(_lastLapTime / 60000),
            (int)(_lastLapTime / 1000) % 60, (int)(_lastLapTime % 1000) / 100);
    tft->setTextColor(TFT_WHITE, 0x10A2);
    tft->drawString(lBuf, 10 + cardW + 5 + cardW / 2, gridY + gridH / 2 + 8);
  } else {
    tft->setTextColor(TFT_DARKGREY, 0x10A2);
    tft->drawString("-:--.-", 10 + cardW + 5 + cardW / 2,
                    gridY + gridH / 2 + 8);
  }

  // Best Lap
  if (_bestLapTime > 0) {
    char bBuf[16];
    sprintf(bBuf, "%d:%02d.%d", (int)(_bestLapTime / 60000),
            (int)(_bestLapTime / 1000) % 60, (int)(_bestLapTime % 1000) / 100);
    tft->setTextColor(TFT_GOLD, 0x10A2);
    tft->drawString(bBuf, 10 + (cardW + 5) * 2 + cardW / 2,
                    gridY + gridH / 2 + 8);
  } else {
    tft->setTextColor(TFT_DARKGREY, 0x10A2);
    tft->drawString("-:--.-", 10 + (cardW + 5) * 2 + cardW / 2,
                    gridY + gridH / 2 + 8);
  }
}

void RacingDashboardScreen::checkFinishLine() {
  if (_currentTrack.configs.empty())
    return;

  double lat = gpsManager.getLatitude();
  double lon = gpsManager.getLongitude();
  double dist = gpsManager.distanceBetween(lat, lon, _currentTrack.lat,
                                           _currentTrack.lon);

  if (dist < 15) { // 15m radius
    if (!_finishLineInside) {
      _finishLineInside = true;
      unsigned long now = millis();
      if (_lastFinishCross > 0) {
        _lastLapTime = now - _lastFinishCross;
        _lapTimes.push_back(_lastLapTime);
        if (_bestLapTime == 0 || _lastLapTime < _bestLapTime) {
          _bestLapTime = _lastLapTime;
        }
        _lapCount++;
        _currentLapStart = now;
      }
      _lastFinishCross = now;
    }
  } else if (dist > 25) {
    _finishLineInside = false;
  }
}

void RacingDashboardScreen::drawRPMBar(int rpm, int maxRpm) {
  TFT_eSPI *tft = _ui->getTft();
  int x = 10, y = STATUS_BAR_HEIGHT + 5, w = SCREEN_WIDTH - 20, h = 40;
  int fillW = map(constrain(rpm, 0, maxRpm), 0, maxRpm, 0, w);
  uint16_t color = (rpm > maxRpm * 0.9)   ? TFT_RED
                   : (rpm > maxRpm * 0.7) ? TFT_YELLOW
                                          : TFT_GREEN;
  tft->fillRect(x, y, fillW, h, color);
  tft->fillRect(x + fillW, y, w - fillW, h, TFT_BLACK);
  tft->drawRect(x, y, w, h, TFT_DARKGREY);
}

void RacingDashboardScreen::drawTrackMap(int x, int y, int w, int h) {
  // Simple map or mock
  _ui->getTft()->setTextColor(TFT_DARKGREY, 0x10A2);
  _ui->getTft()->setTextDatum(MC_DATUM);
  _ui->getTft()->drawString("MAP", x + w / 2, y + h / 2);
}
