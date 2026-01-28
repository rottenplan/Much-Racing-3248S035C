#include "TrackRecorderScreen.h"
#include "../../core/GPSManager.h"
#include "../../core/SessionManager.h"
#include "../fonts/Org_01.h"
#include <FS.h>
#include <SD.h>

extern GPSManager gpsManager;
extern SessionManager sessionManager;

void TrackRecorderScreen::onShow() {
  _state = REC_IDLE;
  _recordedPoints.clear();
  _recordingStartTime = 0;
  _lastPointTime = 0;
  _lastTouchTime = millis();

  _lastStateRender = (RecorderState)-1;
  _lastGpsFixed = false;
  _lastSats = -1;

  _ui->setTitle("RECORD TRACK");
  drawStatic();
}

void TrackRecorderScreen::onHide() {
  // Stop any active recording if screen is switched?
  // Maybe better to alert or just stop.
  if (_state == REC_ACTIVE) {
    sessionManager.stopSession();
  }
}

void TrackRecorderScreen::update() {
  UIManager::TouchPoint p = _ui->getTouchPoint();
  bool touched = (p.x != -1);

  if (touched) {
    if (millis() - _lastTouchTime < 200)
      return;
    _lastTouchTime = millis();

    // 1. Back Button
    if (_ui->isBackButtonTouched(p)) {
      _ui->switchScreen(SCREEN_LAP_TIMER);
      return;
    }

    // 2. State-Specific Buttons
    int btnW = UI_BTN_W;
    int btnY = SCREEN_HEIGHT - 60;
    int btnX = (SCREEN_WIDTH - btnW) / 2;

    if (_state == REC_IDLE) {
      // START Button
      if (p.x > btnX && p.x < btnX + btnW && p.y > btnY - 20) {
        // GPS Check (Optional bypass for testing)
        if (true || gpsManager.isFixed()) {
          _state = REC_ACTIVE;
          _recordStartLat = gpsManager.getLatitude();
          _recordStartLon = gpsManager.getLongitude();
          _recordingStartTime = millis();
          _lastPointTime = millis();
          _recordedPoints.clear();

          GPSPoint first;
          first.lat = _recordStartLat;
          first.lon = _recordStartLon;
          first.timestamp = millis();
          _recordedPoints.push_back(first);

          _lastStateRender = (RecorderState)-1; // Force Redraw
        }
      }
    } else if (_state == REC_ACTIVE) {
      // STOP Button
      int stopW = 140;
      int stopX = (SCREEN_WIDTH - stopW) / 2;
      if (p.x > stopX && p.x < stopX + stopW && p.y > btnY - 20) {
        _state = REC_COMPLETE;
        _lastStateRender = (RecorderState)-1;
      }
    } else if (_state == REC_COMPLETE) {
      // KEEP | DISCARD
      int keepW = 120;
      int gap = 20;
      int startX = (SCREEN_WIDTH - (keepW * 2 + gap)) / 2;

      // KEEP (Left)
      if (p.x > startX && p.x < startX + keepW && p.y > btnY - 10) {
        String filename = "/tracks/track_" + String(millis()) + ".gpx";
        saveTrackToGPX(filename);
        _ui->showToast("Saved to SD Card!", 2000);

        // Return to Lap Timer
        _ui->switchScreen(SCREEN_LAP_TIMER);
      }
      // DISCARD (Right)
      else if (p.x > startX + keepW + gap &&
               p.x < startX + keepW + gap + keepW && p.y > btnY - 10) {
        _state = REC_IDLE;
        _recordedPoints.clear();
        _lastStateRender = (RecorderState)-1;
      }
    }
  }

  // Recording Logic
  if (_state == REC_ACTIVE) {
    unsigned long now = millis();
    if (now - _lastPointTime > 2000) {
      if (gpsManager.isFixed()) {
        double currentLat = gpsManager.getLatitude();
        double currentLon = gpsManager.getLongitude();

        if (_recordedPoints.size() > 0) {
          GPSPoint &last = _recordedPoints.back();
          double dist = gpsManager.distanceBetween(last.lat, last.lon,
                                                   currentLat, currentLon);
          if (dist > 5) {
            GPSPoint p;
            p.lat = currentLat;
            p.lon = currentLon;
            p.timestamp = now;
            _recordedPoints.push_back(p);

            // Check if returned to start (Loop closure)
            double distToStart = gpsManager.distanceBetween(
                _recordStartLat, _recordStartLon, currentLat, currentLon);
            if (distToStart < 15 && _recordedPoints.size() > 20) {
              _state = REC_COMPLETE;
            }
          }
        }
      }
      _lastPointTime = now;
    }
  }

  drawDynamic();
}

void TrackRecorderScreen::drawStatic() {
  TFT_eSPI *tft = _ui->getTft();
  tft->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                SCREEN_HEIGHT - STATUS_BAR_HEIGHT, TFT_BLACK);
  _ui->drawBackButton();
}

void TrackRecorderScreen::drawDynamic() {
  TFT_eSPI *tft = _ui->getTft();
  bool stateChanged = (_state != _lastStateRender);

  // 1. GPS Status Card
  int cardX = 10, cardY = 55, cardW = SCREEN_WIDTH - 20, cardH = 45;
  bool gpsFixed = gpsManager.isFixed();
  int sats = gpsManager.getSatellites();

  if (gpsFixed != _lastGpsFixed || sats != _lastSats || stateChanged) {
    tft->fillRoundRect(cardX, cardY, cardW, cardH, 8, 0x18E3);
    tft->drawRoundRect(cardX, cardY, cardW, cardH, 8, TFT_DARKGREY);
    tft->setTextColor(TFT_SILVER, 0x18E3);
    tft->setTextDatum(ML_DATUM);
    tft->setTextFont(2);
    tft->drawString("GPS STATUS", cardX + 15, cardY + cardH / 2);

    uint16_t color = gpsFixed ? (sats >= 5 ? TFT_GREEN : TFT_YELLOW) : TFT_RED;
    String txt = gpsFixed ? (sats >= 5 ? "READY" : "WEAK") : "NO FIX";
    tft->setTextColor(color, 0x18E3);
    tft->setTextDatum(MR_DATUM);
    tft->setTextFont(4);
    tft->drawString(txt, cardX + cardW - 15, cardY + cardH / 2 + 2);

    _lastGpsFixed = gpsFixed;
    _lastSats = sats;
  }

  int contentY = cardY + cardH + 10;
  int contentH = SCREEN_HEIGHT - contentY;

  if (_state == REC_IDLE) {
    if (stateChanged) {
      tft->fillRect(0, contentY, SCREEN_WIDTH, contentH, TFT_BLACK);
      tft->setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      tft->setTextFont(2);
      tft->setTextDatum(MC_DATUM);
      tft->drawString("Go to Start Line & Tap Start", SCREEN_WIDTH / 2,
                      contentY + 20);

      // Start Button
      tft->fillRoundRect((SCREEN_WIDTH - UI_BTN_W) / 2, SCREEN_HEIGHT - 60,
                         UI_BTN_W, 42, 8, TFT_GREEN);
      tft->setTextColor(TFT_BLACK, TFT_GREEN);
      tft->setTextFont(4);
      tft->drawString("START", SCREEN_WIDTH / 2, SCREEN_HEIGHT - 60 + 21);
    }
  } else if (_state == REC_ACTIVE) {
    if (stateChanged) {
      tft->fillRect(0, contentY, SCREEN_WIDTH, contentH, TFT_BLACK);
      // Time Card
      tft->fillRoundRect(10, contentY, SCREEN_WIDTH - 20, 90, 8, 0x10A2);
      tft->setTextColor(TFT_WHITE, 0x10A2);
      tft->setTextFont(1);
      tft->setTextDatum(TL_DATUM);
      tft->drawString("ELAPSED TIME", 20, contentY + 8);

      // Stop Button
      tft->fillRoundRect((SCREEN_WIDTH - 140) / 2, SCREEN_HEIGHT - 60, 140, 42,
                         8, TFT_RED);
      tft->setTextColor(TFT_WHITE, TFT_RED);
      tft->setTextFont(4);
      tft->setTextDatum(MC_DATUM);
      tft->drawString("STOP", SCREEN_WIDTH / 2, SCREEN_HEIGHT - 60 + 21);
    }

    // Update Time
    unsigned long elapsed = millis() - _recordingStartTime;
    char buf[16];
    sprintf(buf, "%02d:%02d.%d", (int)(elapsed / 60000),
            (int)(elapsed / 1000) % 60, (int)(elapsed % 1000) / 100);
    tft->setTextColor(TFT_WHITE, 0x10A2);
    tft->setTextFont(7);
    tft->setTextDatum(MC_DATUM);
    tft->drawString(buf, SCREEN_WIDTH / 2, contentY + 52);

    // Stats
    int subY = contentY + 100;
    int subW = (SCREEN_WIDTH - 25) / 2;
    if (stateChanged) {
      tft->fillRoundRect(10, subY, subW, 50, 6, 0x10A2);
      tft->fillRoundRect(15 + subW, subY, subW, 50, 6, 0x10A2);
      tft->setTextColor(TFT_SILVER, 0x10A2);
      tft->setTextFont(1);
      tft->setTextDatum(TL_DATUM);
      tft->drawString("POINTS", 18, subY + 5);
      tft->drawString("DIST (m)", 23 + subW, subY + 5);
    }

    tft->setTextColor(TFT_SKYBLUE, 0x10A2);
    tft->setTextFont(4);
    tft->setTextDatum(MC_DATUM);
    tft->drawNumber(_recordedPoints.size(), 10 + subW / 2, subY + 28);

    double dist = gpsManager.distanceBetween(_recordStartLat, _recordStartLon,
                                             gpsManager.getLatitude(),
                                             gpsManager.getLongitude());
    tft->setTextColor(TFT_ORANGE, 0x10A2);
    tft->drawNumber((int)dist, 15 + subW + subW / 2, subY + 28);

  } else if (_state == REC_COMPLETE) {
    if (stateChanged) {
      tft->fillRect(0, contentY, SCREEN_WIDTH, contentH, TFT_BLACK);
      tft->setTextColor(TFT_GREEN, TFT_BLACK);
      tft->setTextFont(4);
      tft->setTextDatum(MC_DATUM);
      tft->drawString("TRACK RECORDED!", SCREEN_WIDTH / 2, contentY + 20);

      String stats = String(_recordedPoints.size()) + " Pts | " +
                     String((millis() - _recordingStartTime) / 1000) + "s";
      tft->setTextColor(TFT_WHITE, TFT_BLACK);
      tft->setTextFont(2);
      tft->drawString(stats, SCREEN_WIDTH / 2, contentY + 50);

      int startX = (SCREEN_WIDTH - (120 * 2 + 20)) / 2;
      tft->fillRoundRect(startX, SCREEN_HEIGHT - 60, 120, 42, 6, TFT_GREEN);
      tft->fillRoundRect(startX + 140, SCREEN_HEIGHT - 60, 120, 42, 6, TFT_RED);
      tft->setTextColor(TFT_BLACK, TFT_GREEN);
      tft->drawString("KEEP", startX + 60, SCREEN_HEIGHT - 60 + 21);
      tft->setTextColor(TFT_WHITE, TFT_RED);
      tft->drawString("DISCARD", startX + 140 + 60, SCREEN_HEIGHT - 60 + 21);
    }
  }

  _lastStateRender = _state;
}

void TrackRecorderScreen::saveTrackToGPX(String filename) {
  if (_recordedPoints.empty())
    return;
  if (!SD.exists("/tracks"))
    SD.mkdir("/tracks");
  File file = SD.open(filename, FILE_WRITE);
  if (!file)
    return;

  file.println("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
  file.println("<gpx version=\"1.1\" creator=\"MuchRacing\" "
               "xmlns=\"http://www.topografix.com/GPX/1/"
               "1\"><trk><name>Recorded Track</name><trkseg>");
  for (const auto &p : _recordedPoints) {
    file.printf("<trkpt lat=\"%.7f\" lon=\"%.7f\"></trkpt>\n", p.lat, p.lon);
  }
  file.println("</trkseg></trk></gpx>");
  file.close();
}
