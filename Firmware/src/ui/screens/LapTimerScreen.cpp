#include "LapTimerScreen.h"
#include "../../core/GPSManager.h"
#include "../../core/SessionManager.h"
#include "../fonts/Org_01.h"
#include <ArduinoJson.h>
#include <algorithm>

extern GPSManager gpsManager;
extern SessionManager sessionManager;

#define STATUS_BAR_HEIGHT 20
#define LIST_ITEM_HEIGHT 30

void LapTimerScreen::onShow() {
  _lastUpdate = 0;
  _lastTouchTime = millis();
  _lastBackTapTime = 0;
  _lapCount = 0;
  _state = STATE_TRACK_LIST;
  _bestLapTime = 0;
  _lapTimes.clear();
  _menuSelectionIdx = -1;
  _maxRpmSession = 0;

  _lastSpeed = -1.0;
  _lastSats = -1;
  _lastRpmRender = -1;
  _lastMaxRpmRender = 0;
  _lastLapCountRender = -1;

  loadTracks();

  _state = STATE_MENU;
  TFT_eSPI *tft = _ui->getTft();
  tft->fillScreen(_ui->getBackgroundColor());
  gpsManager.setRawDataCallback(nullptr);
  _needsStaticRedraw = true;
  drawMenu();
}

void LapTimerScreen::loadTracks() {
  _tracks.clear();
  double curLat = gpsManager.getLatitude();
  double curLon = gpsManager.getLongitude();

  if (SD.exists("/tracks.json")) {
    File file = SD.open("/tracks.json", FILE_READ);
    if (file) {
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, file);
      file.close();
      if (!error && doc["tracks"].is<JsonArray>()) {
        JsonArray trackArray = doc["tracks"];
        for (JsonVariant t : trackArray) {
          double tLat = t["lat"].as<double>();
          double tLon = t["lon"].as<double>();
          double dist = gpsManager.distanceBetween(curLat, curLon, tLat, tLon);
          if (dist > 50000)
            continue;

          Track newTrack;
          newTrack.name = t["name"].as<String>();
          newTrack.lat = tLat;
          newTrack.lon = tLon;
          newTrack.isCustom = true;

          JsonArray configs = t["configs"];
          if (configs.size() > 0) {
            for (JsonVariant c : configs) {
              newTrack.configs.push_back({c.as<String>()});
            }
          } else {
            newTrack.configs.push_back({"Default"});
          }
          if (t["path"].is<String>())
            newTrack.pathFile = t["path"].as<String>();
          _tracks.push_back(newTrack);
        }
      }
    }
  }

  Track factory;
  factory.name = "Test Track (Bordeaux)";
  factory.lat = 44.8378;
  factory.lon = -0.5792;
  factory.isCustom = false;
  factory.configs.push_back({"Default"});
  factory.pathFile = "";
  _tracks.push_back(factory);
}

void LapTimerScreen::loadTrackPath(String filename) {
  // Logic mostly for map preview in manager
}

void LapTimerScreen::transitionToSummary() {
  ActiveSessionData data;
  data.trackName = _currentTrackName;
  data.bestLapTime = _bestLapTime;
  data.lapCount = _lapCount;
  data.lapTimes = _lapTimes;
  data.maxRpm = _maxRpmSession;
  _ui->setLastSession(data);
  _ui->switchScreen(SCREEN_SESSION_SUMMARY);
}

void LapTimerScreen::update() {
  UIManager::TouchPoint p = _ui->getTouchPoint();
  bool touched = (p.x != -1);

  if (_state == STATE_MENU) {
    if (touched) {
      if (millis() - _lastTouchTime < 200)
        return;
      _lastTouchTime = millis();

      if (_ui->isBackButtonTouched(p)) {
        _ui->switchScreen(SCREEN_MENU);
        return;
      }

      int startY = 55, btnH = 35, gap = 8, btnW = 360;
      int x = (SCREEN_WIDTH - btnW) / 2;
      if (p.x > x && p.x < x + btnW) {
        int idx = (p.y - startY) / (btnH + gap);
        if (idx >= 0 && idx < 4) {
          if (_menuSelectionIdx == idx) {
            if (idx == 0) {
              _state = STATE_SEARCHING;
              _searchStartTime = millis();
              drawSearching();
            } else if (idx == 1) {
              // Dashboard requires selected track. If empty, go to search.
              if (_selectedTrackIdx != -1)
                _ui->switchScreen(SCREEN_RACING_DASHBOARD);
              else {
                _state = STATE_SEARCHING;
                _searchStartTime = millis();
                drawSearching();
              }
            } else if (idx == 2) {
              transitionToSummary();
            } else if (idx == 3) {
              _ui->switchScreen(SCREEN_TRACK_RECORDER);
            }
          } else {
            _menuSelectionIdx = idx;
            drawMenu();
          }
        }
      }
    }
  } else if (_state == STATE_SEARCHING) {
    if (millis() - _searchStartTime > 2000) {
      loadTracks();
      _state = STATE_TRACK_LIST;
      drawTrackList();
    }
  } else if (_state == STATE_TRACK_LIST) {
    if (touched) {
      if (millis() - _lastTouchTime < 200)
        return;
      _lastTouchTime = millis();
      if (_ui->isBackButtonTouched(p)) {
        _state = STATE_MENU;
        drawMenu();
        return;
      }

      if (p.x > SCREEN_WIDTH - 110 && p.y < 50) {
        _state = STATE_CREATE_TRACK;
        _createStep = 0;
        drawCreateTrack();
        return;
      }

      int startY = 60, itemH = 45, gap = 8;
      if (p.y > startY) {
        int idx = (p.y - startY) / (itemH + gap);
        if (idx >= 0 && idx < _tracks.size()) {
          _selectedTrackIdx = idx;
          _state = STATE_TRACK_MENU;
          drawTrackOptionsPopup();
        }
      }
    }
  } else if (_state == STATE_TRACK_MENU) {
    if (touched) {
      if (millis() - _lastTouchTime < 200)
        return;
      _lastTouchTime = millis();
      // Logic for popup options...
      _state = STATE_TRACK_LIST;
      drawTrackList();
    }
  } else if (_state == STATE_TRACK_DETAILS) {
    if (touched) {
      if (millis() - _lastTouchTime < 200)
        return;
      _lastTouchTime = millis();
      if (_ui->isBackButtonTouched(p)) {
        _state = STATE_TRACK_LIST;
        drawTrackList();
        return;
      }

      int btnW = 140, btnX = (SCREEN_WIDTH - btnW) / 2, btnY = 190;
      if (p.x > btnX && p.x < btnX + btnW && p.y > btnY - 10 &&
          p.y < btnY + 45) {
        _ui->setSelectedTrack(_tracks[_selectedTrackIdx]);
        _ui->switchScreen(SCREEN_RACING_DASHBOARD);
      }
    }
  }
}

void LapTimerScreen::drawMenu() {
  TFT_eSPI *tft = _ui->getTft();
  tft->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                SCREEN_HEIGHT - STATUS_BAR_HEIGHT, TFT_BLACK);
  tft->setTextDatum(TC_DATUM);
  tft->setFreeFont(&Org_01);
  tft->setTextSize(2);
  tft->setTextColor(TFT_WHITE, TFT_BLACK);
  tft->drawString("LAP TIMER", SCREEN_WIDTH / 2, 25);
  _ui->drawBackButton();

  int startY = 55, btnH = 35, gap = 8, btnW = 360,
      x = (SCREEN_WIDTH - btnW) / 2;
  const char *items[] = {"SELECT TRACK", "RACE SCREEN", "SESSION SUMMARY",
                         "RECORD TRACK"};
  for (int i = 0; i < 4; i++) {
    uint16_t color = (i == _menuSelectionIdx) ? TFT_RED : TFT_DARKGREY;
    tft->fillRoundRect(x, startY + i * (btnH + gap), btnW, btnH, 5, color);
    tft->setTextColor(TFT_WHITE, color);
    tft->setTextDatum(MC_DATUM);
    tft->drawString(items[i], SCREEN_WIDTH / 2,
                    startY + i * (btnH + gap) + btnH / 2 + 2);
  }
  _ui->drawStatusBar();
}

void LapTimerScreen::drawSearching() {
  TFT_eSPI *tft = _ui->getTft();
  int cx = SCREEN_WIDTH / 2, cy = SCREEN_HEIGHT / 2 - 20;
  tft->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                SCREEN_HEIGHT - STATUS_BAR_HEIGHT, TFT_BLACK);
  tft->setTextColor(TFT_WHITE, TFT_BLACK);
  tft->setTextDatum(TC_DATUM);
  tft->drawString("Searching nearby Tracks...", cx, cy + 50);
  _ui->drawStatusBar();
}

void LapTimerScreen::drawTrackList() {
  TFT_eSPI *tft = _ui->getTft();
  tft->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                SCREEN_HEIGHT - STATUS_BAR_HEIGHT, TFT_BLACK);
  tft->setTextDatum(TC_DATUM);
  tft->setFreeFont(&Org_01);
  tft->setTextSize(2);
  tft->setTextColor(TFT_WHITE, TFT_BLACK);
  tft->drawString("SELECT TRACK", SCREEN_WIDTH / 2, 28);
  _ui->drawBackButton();

  int startY = 60, itemH = 45, itemW = SCREEN_WIDTH - 40, itemX = 20, gap = 8;
  for (size_t i = 0; i < _tracks.size(); i++) {
    int y = startY + i * (itemH + gap);
    if (y + itemH > SCREEN_HEIGHT)
      break;
    tft->fillRoundRect(itemX, y, itemW, itemH, 6, 0x18E3);
    tft->drawRoundRect(itemX, y, itemW, itemH, 6, TFT_DARKGREY);
    tft->setTextColor(TFT_WHITE, 0x18E3);
    tft->setTextFont(2);
    tft->setTextDatum(ML_DATUM);
    tft->drawString(_tracks[i].name, itemX + 40, y + itemH / 2);
  }
  _ui->drawStatusBar();
}

void LapTimerScreen::drawTrackOptionsPopup() {
  TFT_eSPI *tft = _ui->getTft();
  int w = 320, h = 170, x = (SCREEN_WIDTH - w) / 2,
      y = (SCREEN_HEIGHT - h) / 2 + 10;
  tft->fillRoundRect(x, y, w, h, 5, TFT_BLACK);
  tft->drawRoundRect(x, y, w, h, 5, TFT_WHITE);
  const char *opts[] = {"Select", "Select & Edit", "Invert", "Reinit best Lap",
                        "Remove"};
  tft->setTextDatum(TL_DATUM);
  tft->setTextColor(TFT_WHITE, TFT_BLACK);
  for (int i = 0; i < 5; i++)
    tft->drawString(opts[i], x + 15, y + 10 + i * 25);
}

void LapTimerScreen::drawTrackDetails() {
  TFT_eSPI *tft = _ui->getTft();
  tft->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                SCREEN_HEIGHT - STATUS_BAR_HEIGHT, TFT_BLACK);
  tft->setTextColor(TFT_WHITE, TFT_BLACK);
  tft->setTextDatum(MC_DATUM);
  tft->drawString("TRACK DETAILS", SCREEN_WIDTH / 2, 28);
  _ui->drawBackButton();
  // Map and Info boxes...
  _ui->drawStatusBar();
}

void LapTimerScreen::drawCreateTrack() {}
void LapTimerScreen::drawRenameTrack() {}
void LapTimerScreen::drawNoGPS() {}
void LapTimerScreen::checkFinishLine() {}
void LapTimerScreen::saveNewTrack(String name, double sLat, double sLon,
                                  double fLat, double fLon) {}
void LapTimerScreen::renameTrack(int index, String newName) {}
