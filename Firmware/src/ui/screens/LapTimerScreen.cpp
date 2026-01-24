#include "LapTimerScreen.h"
#include "../../core/GPSManager.h"
#include "../../core/SessionManager.h"
#include "../fonts/Org_01.h"
#include <algorithm> // Untuk min_element

extern GPSManager gpsManager;
extern SessionManager sessionManager;

// Konstanta untuk Tata Letak UI
#define STATUS_BAR_HEIGHT 20
#define LIST_ITEM_HEIGHT 30

// Tentukan Area Tombol
#define STOP_BTN_Y 200 // Dipindahkan KE BAWAH untuk jarak yang lebih baik
#define STOP_BTN_H 35

void LapTimerScreen::onShow() {
  _lastUpdate = 0;
  _lastTouchTime = millis(); // Prevent ghost touch on entry
  _lastBackTapTime = 0;
  _isRecording = false; // Mulai tidak merekam
  _finishSet = false;
  _lapCount = 0;
  _state = STATE_TRACK_LIST; // Mulai di Pilihan Track
  _raceMode = MODE_BEST;     // Default mode
  _bestLapTime = 0;
  _bestLapTime = 0;
  _lapTimes.clear();
  _listScroll = 0;
  _menuSelectionIdx = -1;
  _maxRpmSession = 0; // Reset Max RPM

  // Reset Flicker Tracking
  _lastSpeed = -1.0;
  _lastSats = -1;
  _lastRpmRender = -1;
  _lastMaxRpmRender = 0;
  _lastLapCountRender = -1;
  _lastLapCountRender = -1;
  _lastRecordedStateRender = (RecordingState)-1;
  _lastLastLapTimeRender = -1;
  _lastBestLapTimeRender = -1;

  _finishLineInside = false;
  _lastFinishCross = 0;

  // Initialize GPS recording state
  _recordingState = RECORD_IDLE;
  _recordedPoints.clear();
  _recordStartLat = 0;
  _recordStartLon = 0;
  _recordingStartTime = 0;
  _lastPointTime = 0;
  _totalDistance = 0;

  loadTracks();

  // Start in Sub-Menu
  _state = STATE_MENU;
  TFT_eSPI *tft = _ui->getTft();
  tft->fillScreen(_ui->getBackgroundColor()); // Ensure clean background
  gpsManager.setRawDataCallback(nullptr);     // Ensure no log overlay
  _needsStaticRedraw = true;
  drawMenu();
}

#include <ArduinoJson.h>

#include <ArduinoJson.h>

void LapTimerScreen::loadTracks() {
  _tracks.clear();

  double curLat = gpsManager.getLatitude();
  double curLon = gpsManager.getLongitude();

  // Load from SD Card if available
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

          // Filter 50km Radius
          double dist = gpsManager.distanceBetween(curLat, curLon, tLat, tLon);
          if (dist > 50000)
            continue;

          Track newTrack;
          newTrack.name = t["name"].as<String>();
          newTrack.lat = tLat;
          newTrack.lon = tLon;
          newTrack.isCustom = true; // Loaded from SD

          // Configs
          JsonArray configs = t["configs"];
          if (configs.size() > 0) {
            for (JsonVariant c : configs) {
              newTrack.configs.push_back({c.as<String>()});
            }
          } else {
            newTrack.configs.push_back({"Default"});
          }

          if (t.containsKey("path")) {
            newTrack.pathFile = t["path"].as<String>();
          }

          _tracks.push_back(newTrack);
        }
        Serial.println("Tracks loaded from SD");
      }
    }
  }

  // Factory Tracks (Hardcoded)
  // Check dist for them too
  Track sonoma;
  sonoma.name = "Test Track (Bordeaux)"; // Renamed to match img
  sonoma.lat = 44.8378;                  // Bordeaux approx
  sonoma.lon = -0.5792;
  sonoma.isCustom = false; // Factory
  sonoma.isCustom = false; // Factory
  sonoma.configs.push_back({"Default"});
  sonoma.pathFile = ""; // No file for factory (or hardcode points later)

  // Always add test track for DEBUG/UI TESTING
  _tracks.push_back(sonoma);

  if (gpsManager.isFixed()) {
    double d =
        gpsManager.distanceBetween(curLat, curLon, sonoma.lat, sonoma.lon);
    // if (d < 50000)
    //   _tracks.push_back(sonoma);
  } else {
    // For Debug/Sim without GPS, maybe add it?
    // User said "If GPS data is unavailable... cannot enter the menu".
    // So logic in `update` prevents us getting here without GPS.
    // So here safely assume GPS is fixed.
    // I'll leave the check enabled.
    double d =
        gpsManager.distanceBetween(curLat, curLon, sonoma.lat, sonoma.lon);
    // if (d < 50000)
    //   _tracks.push_back(sonoma);
  }

  // Add a fake "Nearby" one for testing if list is empty?
  // _tracks.push_back(sonoma); // FORCE ADD FOR UI TESTING (Remove later)
}

void LapTimerScreen::loadTrackPath(String filename) {
  _recordedPoints.clear();

  if (!SD.exists(filename)) {
    Serial.println("Track path file not found: " + filename);
    return;
  }

  File file = SD.open(filename, FILE_READ);
  if (!file)
    return;

  // Read CSV: lat,lon (one per line)
  // Limit points to save RAM (e.g. max 1000)
  int count = 0;
  while (file.available() && count < 1000) {
    String line = file.readStringUntil('\n');
    line.trim();
    if (line.length() > 0) {
      int commaIndex = line.indexOf(',');
      if (commaIndex > 0) {
        String latStr = line.substring(0, commaIndex);
        String lonStr = line.substring(commaIndex + 1);

        GPSPoint p;
        p.lat = latStr.toDouble();
        p.lon = lonStr.toDouble();
        p.timestamp = 0; // Static path
        _recordedPoints.push_back(p);
        count++;
      }
    }
  }
  file.close();
  Serial.println("Loaded " + String(count) + " points from " + filename);
}

void LapTimerScreen::saveTrackToGPX(String filename) {
  if (_recordedPoints.empty()) {
    Serial.println("No points to save!");
    return;
  }

  // Ensure directory exists
  if (!SD.exists("/tracks")) {
    SD.mkdir("/tracks");
  }

  File file = SD.open(filename, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing: " + filename);
    return;
  }

  Serial.println("Saving GPX to: " + filename);

  // 1. Header
  file.println("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
  file.println("<gpx version=\"1.1\" creator=\"MuchRacing\" "
               "xmlns=\"http://www.topografix.com/GPX/1/1\">");

  // 2. Metadata / Track Info
  file.println("  <trk>");
  // Use current track name or generic
  String trackName =
      (_currentTrackName.length() > 0) ? _currentTrackName : " Recorded Track";
  file.println("    <name>" + trackName + "</name>");
  file.println("    <trkseg>");

  // 3. Points
  for (const auto &p : _recordedPoints) {
    file.printf("      <trkpt lat=\"%.7f\" lon=\"%.7f\">\n", p.lat, p.lon);

    // Optional: Add Elevation or Time if available in GPSPoint struct
    // Standard timestamp format: 2023-10-25T14:30:00Z
    // Currently we store raw millis() or similar in timestamp, need real time?
    // If GPSManager has real UTC time, ideally we'd use that.
    // For now, no time tag to avoid confusing parsers with bad data.

    file.println("      </trkpt>");
  }

  // 4. Footer
  file.println("    </trkseg>");
  file.println("  </trk>");
  file.println("</gpx>");

  file.close();
  Serial.println("GPX Saved Successfully.");
}

void LapTimerScreen::update() {
  UIManager::TouchPoint p = _ui->getTouchPoint();
  bool touched = (p.x != -1);

  if (_state == STATE_MENU) {
    if (touched) {
      // Global Debounce for Menu
      if (millis() - _lastTouchTime < 200)
        return;
      _lastTouchTime = millis();

      // 1. Back/Home (< 60x60)
      if (p.x < 60 && p.y < 60) {
        if (_menuSelectionIdx == -2) {
          if (millis() - _lastBackTapTime < 500) {
            _ui->switchScreen(SCREEN_MENU);
            _lastBackTapTime = 0;
          } else {
            _lastBackTapTime = millis();
          }
        } else {
          _menuSelectionIdx = -2;
          drawMenu();
          _lastBackTapTime = millis();
        }
        return;
      }

      // 2. Button Logic
      // startY = 55, btnHeight = 35, gap = 8
      int startY = 55;
      int btnHeight = 35;
      int gap = 8;
      int x = (SCREEN_WIDTH - 240) / 2;
      int btnWidth = 240;

      // Check if X is within button width (centered)
      if (p.x > x && p.x < x + btnWidth) {
        int touchedIdx = -1;

        // Check Y coordinates for each button
        for (int i = 0; i < 4; i++) {
          int btnY = startY + (i * (btnHeight + gap));
          if (p.y > btnY && p.y < btnY + btnHeight) {
            touchedIdx = i;
            break;
          }
        }

        if (touchedIdx != -1) {
          // Debounce handled above

          // Double Tap Logic
          if (_menuSelectionIdx == touchedIdx) {
            // Second tap on SAME button -> Execute Action

            // Execute Action
            if (touchedIdx == 0) { // Select Track
              if (false &&
                  !gpsManager.isFixed()) { // BYPASS GPS CHECK FOR UI TESTING
                _state = STATE_NO_GPS;
                _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                                        SCREEN_HEIGHT - STATUS_BAR_HEIGHT,
                                        _ui->getBackgroundColor());
                drawNoGPS();
              } else {
                // Go to Searching Screen first
                _state = STATE_SEARCHING;
                _searchStartTime = millis();
                _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                                        SCREEN_HEIGHT - STATUS_BAR_HEIGHT,
                                        _ui->getBackgroundColor());
                drawSearching();
              }
            } else if (touchedIdx == 1) { // Race Screen
              _state = STATE_RACING;
              // fillRect removed, handled by drawRacingStatic's fillScreen

              drawRacingStatic();
              drawRacing();
            } else if (touchedIdx == 2) { // Session Summary
              _state = STATE_SUMMARY;
              _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                                      SCREEN_HEIGHT - STATUS_BAR_HEIGHT,
                                      _ui->getBackgroundColor());
              drawSummary();
            } else if (touchedIdx == 3) { // Record Track
              _recordingState = RECORD_IDLE;
              _recordedPoints.clear();
              _state = STATE_RECORD_TRACK;
              _lastRecordedStateRender = (RecordingState)-1; // Force Redraw
              _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                                      SCREEN_HEIGHT - STATUS_BAR_HEIGHT,
                                      _ui->getBackgroundColor());
              drawRecordTrack();
            }
          } else {
            // First tap (or different button) -> Highlight Only
            _menuSelectionIdx = touchedIdx;
            drawMenu();
          }

          return;
        }
      }
    }

  } else if (_state == STATE_NO_GPS) {
    if (touched) {
      if (millis() - _lastTouchTime < 200)
        return;
      _lastTouchTime = millis();

      int btnY = SCREEN_HEIGHT - 60;
      // Retry (Left) x=20, w=130
      if (p.y > btnY && p.y < btnY + 40) {
        if (p.x > 20 && p.x < 150) {
          // Retry Logic DISABLED
          /*
          if (gpsManager.isFixed()) {
            _state = STATE_SEARCHING;
            _searchStartTime = millis();
            _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                                    SCREEN_HEIGHT - STATUS_BAR_HEIGHT,
                                    COLOR_BG);
            drawSearching();
          } else {
            // Feedback (Redraw to blink)
            drawNoGPS();
          }
          */
        }
        // Continue (Right) x=170, w=130 -> Back to Menu
        else if (p.x > 170 && p.x < 300) {
          _state = STATE_MENU;
          _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                                  SCREEN_HEIGHT - STATUS_BAR_HEIGHT,
                                  _ui->getBackgroundColor());
          drawMenu();
          _ui->drawStatusBar();
        }
      }
    }

  } else if (_state == STATE_SEARCHING) {
    // Auto-transition after delay
    if (millis() - _searchStartTime > 2000) {
      loadTracks();
      // If no tracks loaded, maybe stay in searching or show "No Tracks"?
      // For now, go to List, list handles empty state.
      _selectedTrackIdx = -1;
      _state = STATE_TRACK_LIST;
      _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                              SCREEN_HEIGHT - STATUS_BAR_HEIGHT,
                              _ui->getBackgroundColor());
      drawTrackList();
    }

  } else if (_state == STATE_TRACK_LIST) {
    if (touched) {
      if (millis() - _lastTouchTime < 200)
        return;
      _lastTouchTime = millis();

      // 1. Back Arrow (if clicked top left still works, though text changed)
      // Title is "Nearby Tracks" at x=10. Back behavior?
      // User didn't specify Back on List, but implied menu access.
      // Let's keep Back check on left just in case < 60x60
      if (p.x < 60 && p.y < 60) {
        if (millis() - _lastBackTapTime < 500) {
          _state = STATE_MENU;
          _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                                  SCREEN_HEIGHT - STATUS_BAR_HEIGHT, COLOR_BG);
          drawMenu();
          _ui->drawStatusBar();
          _lastBackTapTime = 0;
        } else {
          _lastBackTapTime = millis();
        }
        return;
      }

      // 2. New Track Button (Top Right)
      // btnX = SCREEN_WIDTH - 110, Y=22, W=100, H=20
      if (p.x > SCREEN_WIDTH - 110 && p.y < 50) {
        // Go to Record Track Screen
        _state = STATE_RECORD_TRACK;
        _needsStaticRedraw = true;
        return;
      }

      // 3. Track Selection (Open Popup)
      int startY = 60;
      int itemH = 45;
      int gap = 8;
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

      // Popup Coords calculation again
      int w = 220;
      int h = 150;
      int x = (SCREEN_WIDTH - w) / 2;
      int y = (SCREEN_HEIGHT - h) / 2 + 10;

      // Check if touch inside popup
      if (p.x > x && p.x < x + w && p.y > y && p.y < y + h) {
        // Row Check
        int itemH = 25;
        int relY = p.y - (y + 10);
        int idx = relY / itemH;

        if (idx == 0) { // Select
          Track &t = _tracks[_selectedTrackIdx];
          _currentTrackName = t.name;
          _selectedConfigIdx = 0;

          // Load Track Path if available
          if (t.pathFile.length() > 0) {
            loadTrackPath(t.pathFile);
          } else {
            _recordedPoints.clear(); // Clear any previous points
          }

          _state = STATE_SUMMARY;
          _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                                  SCREEN_HEIGHT - STATUS_BAR_HEIGHT,
                                  _ui->getBackgroundColor());
          drawSummary();
          _ui->drawStatusBar();
        } else if (idx == 1) { // Select & Edit
          // Go to Details Screen
          _state = STATE_TRACK_DETAILS;
          _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                                  SCREEN_HEIGHT - STATUS_BAR_HEIGHT,
                                  _ui->getBackgroundColor());
          drawTrackDetails();
        } else if (idx == 2) { // Invert
                               // TODO: Logic
        } else if (idx == 3) { // Reinit Best Lap
          if (_selectedTrackIdx >= 0 && _selectedTrackIdx < _tracks.size()) {
            _tracks[_selectedTrackIdx].bestLap = 0;
          }
          // Close Popup
          _state = STATE_TRACK_LIST;
          _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                                  SCREEN_HEIGHT - STATUS_BAR_HEIGHT,
                                  _ui->getBackgroundColor());
          drawTrackList();
        } else if (idx == 4) { // Remove
          if (_selectedTrackIdx >= 0 && _selectedTrackIdx < _tracks.size()) {
            if (_tracks[_selectedTrackIdx].isCustom) {
              _tracks.erase(_tracks.begin() + _selectedTrackIdx);
            }
          }
          _state = STATE_TRACK_LIST;
          _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                                  SCREEN_HEIGHT - STATUS_BAR_HEIGHT, COLOR_BG);
          drawTrackList();
        }
      } else {
        // Click Outside -> Close Popup
        _state = STATE_TRACK_LIST;
        _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                                SCREEN_HEIGHT - STATUS_BAR_HEIGHT,
                                _ui->getBackgroundColor());
        drawTrackList();
      }
    }
  } else if (_state == STATE_TRACK_DETAILS) {
    if (touched) {
      if (millis() - _lastTouchTime < 200)
        return;
      _lastTouchTime = millis();

      // 1. Back Button (Top Left)
      if (p.x < 60 && p.y < 60) {
        if (millis() - _lastBackTapTime < 500) {
          _state = STATE_TRACK_LIST;
          _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                                  SCREEN_HEIGHT - STATUS_BAR_HEIGHT,
                                  _ui->getBackgroundColor());
          drawTrackList();
          _lastBackTapTime = 0;
        } else {
          _lastBackTapTime = millis();
        }
        return;
      }

      // 2. Select Button
      // x = (W - 140)/2, y=190, w=140, h=35
      int btnW = 140;
      int btnX = (SCREEN_WIDTH - btnW) / 2;
      int btnY = 190;
      int btnH = 35;

      if (p.x > btnX && p.x < btnX + btnW && p.y > btnY - 10 &&
          p.y < btnY + btnH + 10) {
        // SELECT ACTION
        Track &t = _tracks[_selectedTrackIdx];
        _currentTrackName = t.name;
        _selectedConfigIdx = 0;

        // Load Track Path logic
        if (t.pathFile.length() > 0) {
          loadTrackPath(t.pathFile);
        } else {
          _recordedPoints.clear();
        }

        _state = STATE_SUMMARY;
        _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                                SCREEN_HEIGHT - STATUS_BAR_HEIGHT,
                                _ui->getBackgroundColor());
        drawSummary();
        _ui->drawStatusBar();
      }
    }
  } else if (_state == STATE_SUMMARY) {

    // --- LOGIKA STATUS RINGKASAN ---
    if (touched) {
      // 1. KEMBALI/MENU (Kiri Atas)
      if (p.x < 60 && p.y < 60) {
        if (millis() - _lastTouchTime < 200)
          return;
        _lastTouchTime = millis();

        if (_menuSelectionIdx == -2) {
          if (millis() - _lastBackTapTime < 500) {
            // Back to Sub-Menu
            _state = STATE_MENU;
            _menuSelectionIdx = -1;
            _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                                    SCREEN_HEIGHT - STATUS_BAR_HEIGHT,
                                    _ui->getBackgroundColor());
            drawMenu();
            _ui->drawStatusBar();
            _lastBackTapTime = 0;
          } else {
            _lastBackTapTime = millis();
          }
        } else {
          _menuSelectionIdx = -2;
          drawSummary();
          _lastBackTapTime = millis();
        }
        return;
      }

      // 2. Restart / New Session (Tap anywhere else?)
      // For now, let's keep it simple: Tap bottom right to go to Track Select?
      if (p.x > 200 && p.y > 200) {
        _state = STATE_TRACK_LIST;
        _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                                SCREEN_HEIGHT - STATUS_BAR_HEIGHT,
                                _ui->getBackgroundColor());
        drawTrackList();
        _ui->drawStatusBar();
        return;
      }
    }

  } else if (_state == STATE_RECORD_TRACK) {
    extern GPSManager gpsManager;

    if (touched) {
      // Back button (Top Left)
      if (p.x < 60 && p.y < 60) {
        if (millis() - _lastBackTapTime < 500) {
          _state = STATE_MENU;
          _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                                  SCREEN_HEIGHT - STATUS_BAR_HEIGHT, COLOR_BG);
          drawMenu();
          _ui->drawStatusBar();
          _lastBackTapTime = 0;
        } else {
          _lastBackTapTime = millis();
        }
        return;
      }

      // Constants for Touch Logic (Must match drawRecordTrack)
      int cardY = 55;
      int cardH = 40;
      int clearY = cardY + cardH + 5;
      int gridY = clearY + 10;
      int boxH = 50;
      int btnY = gridY + boxH + 20; // Y ~ 180
      int btnH = 42;

      if (_recordingState == RECORD_IDLE) {
        // START button (Centered)
        // btnX ~ (320-140)/2 = 90. Width 140.
        // Hit Area: X 90-230, Y 180-222
        if (p.x > 80 && p.x < 240 && p.y > btnY - 10 &&
            p.y < btnY + btnH + 10) {

          if (true ||
              (gpsManager.isFixed() && gpsManager.getSatellites() >= 6)) {
            // Logic... same as before
            if (_menuSelectionIdx == 10) {
              _menuSelectionIdx = -1;
              _recordingState = RECORD_ACTIVE;
              _recordStartLat = gpsManager.getLatitude();
              _recordStartLon = gpsManager.getLongitude();
              _recordingStartTime = millis();
              _lastPointTime = millis();
              _recordedPoints.clear();

              GPSPoint firstPoint;
              firstPoint.lat = _recordStartLat;
              firstPoint.lon = _recordStartLon;
              firstPoint.timestamp = millis();
              _recordedPoints.push_back(firstPoint);

              drawRecordTrack();
            } else {
              _menuSelectionIdx = 10;
              drawRecordTrack();
            }
          }
        }
      } else if (_recordingState == RECORD_ACTIVE) {
        // STOP button (Centered, Same Pos)
        if (p.x > 80 && p.x < 240 && p.y > btnY - 10 &&
            p.y < btnY + btnH + 10) {

          if (_menuSelectionIdx == 11) {
            _menuSelectionIdx = -1;
            _recordingState = RECORD_COMPLETE;
            drawRecordTrack();
          } else {
            _menuSelectionIdx = 11;
            drawRecordTrack();
          }
        }
      } else if (_recordingState == RECORD_COMPLETE) {
        // SAVE | DISCARD Buttons
        // btnW=100, gap=20.
        // SaveX = (320 - 220)/2 = 50. Range 50-150.
        // DelX = 50 + 100 + 20 = 170. Range 170-270.

        // SAVE (Left)
        if (p.x > 40 && p.x < 160 && p.y > btnY - 10 &&
            p.y < btnY + btnH + 10) {
          if (_menuSelectionIdx == 12) {
            _menuSelectionIdx = -1;
            String filename = "/tracks/track_" + String(millis()) + ".gpx";
            saveTrackToGPX(filename);
            _ui->showToast("Saved to SD Card!", 2000);
            _finishLat = _recordStartLat;
            _finishLon = _recordStartLon;
            _finishSet = true;
            _currentTrackName = "Recorded Track";
            _state = STATE_RACING;
            _lapCount = 0;
            _bestLapTime = 0;
            _lapTimes.clear();
            _currentLapStart = millis();
            _currentLapStart = millis();
            // fillRect removed, handled by drawRacingStatic's fillScreen

            drawRacingStatic();
            drawRacing();
          } else {
            _menuSelectionIdx = 12;
            drawRecordTrack();
          }
          return;
        }

        // DISCARD (Right)
        if (p.x > 160 && p.x < 280 && p.y > btnY - 10 &&
            p.y < btnY + btnH + 10) {
          if (_menuSelectionIdx == 13) {
            _menuSelectionIdx = -1;
            _recordingState = RECORD_IDLE;
            _recordedPoints.clear();
            drawRecordTrack();
          } else {
            _menuSelectionIdx = 13;
            drawRecordTrack();
          }
          return;
        }
      }
    }

    // GPS Recording Loop (when ACTIVE)
    if (_recordingState == RECORD_ACTIVE) {
      unsigned long now = millis();
      if (now - _lastPointTime > 2000) {
        if (gpsManager.isFixed()) {
          double currentLat = gpsManager.getLatitude();
          double currentLon = gpsManager.getLongitude();
          if (_recordedPoints.size() > 0) {
            GPSPoint &lastPoint = _recordedPoints.back();
            double dist = gpsManager.distanceBetween(
                lastPoint.lat, lastPoint.lon, currentLat, currentLon);
            if (dist > 5) {
              GPSPoint newPoint;
              newPoint.lat = currentLat;
              newPoint.lon = currentLon;
              newPoint.timestamp = now;
              _recordedPoints.push_back(newPoint);
              double distToStart = gpsManager.distanceBetween(
                  _recordStartLat, _recordStartLon, currentLat, currentLon);
              if (distToStart < 15 && _recordedPoints.size() > 20) {
                _recordingState = RECORD_COMPLETE;
              }
            }
          }
        }
        _lastPointTime = now;
      }
    }

    // UI Refresh
    unsigned long now = millis();
    if (_needsStaticRedraw) {
      drawRecordTrackStatic();
      _needsStaticRedraw = false;
      _lastUpdate = now;
    }
    if (now - _lastUpdate > 500) {
      drawRecordTrack();
      _lastUpdate = now;
    }

  } else if (_state == STATE_NO_GPS) {
    if (touched) {
      if (p.x > 170 && p.x < 300 && p.y > SCREEN_HEIGHT - 60) {
        _state = STATE_MENU;
        _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                                SCREEN_HEIGHT - STATUS_BAR_HEIGHT,
                                _ui->getBackgroundColor());
        drawMenu();
        _ui->drawStatusBar();
      }
    }
  } else if (_state == STATE_RACING) {
    if (touched) {
      if (p.x < 50 && p.y > 200) {
        if (millis() - _lastTouchTime < 200)
          return;
        _lastTouchTime = millis();
        _state = STATE_SUMMARY;
        if (sessionManager.isLogging()) {
          String dateStr =
              gpsManager.getDateString() + " " + gpsManager.getTimeString();
          sessionManager.appendToHistoryIndex("Track Session", dateStr,
                                              _lapCount, _bestLapTime, "TRACK");
        }
        sessionManager.stopSession();
        _isRecording = false;
        _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                                SCREEN_HEIGHT - STATUS_BAR_HEIGHT, COLOR_BG);
        drawSummary();
        return;
      }
      if (p.x > 80 && p.x < 240 && p.y > STOP_BTN_Y) {
        _state = STATE_SUMMARY;
        if (sessionManager.isLogging()) {
          String dateStr =
              gpsManager.getDateString() + " " + gpsManager.getTimeString();
          sessionManager.appendToHistoryIndex("Track Session", dateStr,
                                              _lapCount, _bestLapTime, "TRACK");
        }
        sessionManager.stopSession();
        _isRecording = false;
        _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                                SCREEN_HEIGHT - STATUS_BAR_HEIGHT, COLOR_BG);
        drawSummary();
        return;
      }
    }

    if (_finishSet)
      checkFinishLine();

    if (sessionManager.isLogging() && (millis() - _lastUpdate > 100)) {
      String data = String(millis()) + "," +
                    String(gpsManager.getLatitude(), 6) + "," +
                    String(gpsManager.getLongitude(), 6) + "," +
                    String(gpsManager.getSpeedKmph()) + "," +
                    String(gpsManager.getSatellites()) + "," +
                    String(gpsManager.getAltitude(), 2) + "," +
                    String(gpsManager.getHeading(), 2);
      sessionManager.logData(data);
    }

    if (_needsStaticRedraw) {
      drawRacingStatic();
      _lastUpdate = 0;
      _needsStaticRedraw = false;
    }

    if (millis() - _lastUpdate > 100) {
      drawRacing();
      _lastUpdate = millis();
    }
  }
}

// --- PEMBANTU PENGGAMBARAN ---

void LapTimerScreen::drawMenu() {
  TFT_eSPI *tft = _ui->getTft();

  // Header
  tft->drawFastHLine(0, 20, SCREEN_WIDTH, COLOR_SECONDARY);
  tft->setTextDatum(TC_DATUM);
  tft->setFreeFont(&Org_01);
  tft->setTextSize(2);
  tft->setTextColor(COLOR_TEXT, COLOR_BG);
  tft->drawString("LAP TIMER", SCREEN_WIDTH / 2, 25);

  // Back Arrow
  tft->setTextDatum(TL_DATUM);
  tft->drawString("<", 10, 25);

  // Buttons
  int startY = 55;
  int btnHeight = 35;
  int btnWidth = 240;
  int gap = 8;
  int x = (SCREEN_WIDTH - btnWidth) / 2;

  const char *menuItems[] = {"SELECT TRACK", "RACE SCREEN", "SESSION SUMMARY",
                             "RECORD TRACK"};

  for (int i = 0; i < 4; i++) {
    int y = startY + (i * (btnHeight + gap));

    // Determine Color based on selection
    uint16_t btnColor = (i == _menuSelectionIdx) ? TFT_RED : TFT_DARKGREY;

    tft->fillRoundRect(x, y, btnWidth, btnHeight, 5, btnColor);
    tft->setTextColor(TFT_WHITE, btnColor);
    tft->setTextDatum(MC_DATUM);
    tft->drawString(menuItems[i], SCREEN_WIDTH / 2, y + btnHeight / 2 + 2);
  }
  _ui->drawStatusBar();
}

void LapTimerScreen::drawSearching() {
  TFT_eSPI *tft = _ui->getTft();
  int cx = SCREEN_WIDTH / 2;
  int cy = SCREEN_HEIGHT / 2 - 20;

  // --- Draw Icon ---
  tft->setTextColor(TFT_WHITE, COLOR_BG);

  // 1. Map (Trapezoid-like)
  int mapY = cy + 10;
  tft->drawLine(cx - 20, mapY, cx + 20, mapY, TFT_WHITE); // Top
  tft->drawLine(cx + 20, mapY, cx + 30, mapY + 25,
                TFT_WHITE); // Right Slope
  tft->drawLine(cx + 30, mapY + 25, cx - 30, mapY + 25,
                TFT_WHITE);                                    // Bottom
  tft->drawLine(cx - 30, mapY + 25, cx - 20, mapY, TFT_WHITE); // Left Slope

  // Dotted Path inside (Mock)
  for (int i = 0; i < 3; i++) {
    tft->fillCircle(cx - 15 + (i * 15), mapY + 12, 2, TFT_WHITE);
  }

  // 2. Pin (Above Map)
  int pinY = cy - 10;
  tft->fillCircle(cx, pinY, 8, TFT_WHITE); // Head
  tft->fillTriangle(cx - 8, pinY, cx + 8, pinY, cx, pinY + 15,
                    TFT_WHITE);            // Point
  tft->fillCircle(cx, pinY, 3, TFT_BLACK); // Hole

  // --- Draw Text ---
  tft->setTextDatum(TC_DATUM);
  tft->setFreeFont(&Org_01); // Standard font
  tft->setTextSize(1);
  tft->drawString("Searching nearby Tracks", cx, cy + 50);

  _ui->drawStatusBar();
}

void LapTimerScreen::drawTrackList() {
  TFT_eSPI *tft = _ui->getTft();

  // Clear Screen (Below StatusBar)
  tft->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                SCREEN_HEIGHT - STATUS_BAR_HEIGHT, TFT_BLACK);

  // --- 1. HEADER ---
  int headY = 20; // Y-coord for line
  tft->drawFastHLine(0, headY, SCREEN_WIDTH, COLOR_SECONDARY);

  // Title "Nearby Tracks" (Moved to Center for consistency)
  tft->setTextDatum(TC_DATUM);
  tft->setFreeFont(&Org_01);
  tft->setTextSize(2);
  tft->setTextColor(TFT_WHITE, TFT_BLACK); // Global White Text
  tft->drawString("SELECT TRACK", SCREEN_WIDTH / 2, headY + 8);

  // Back Button (<) Left
  tft->setTextDatum(TL_DATUM);
  tft->setTextSize(1);
  tft->drawString("<", 10, 25);

  // "New Track" Button (Top Right) -> "+" Icon style
  int btnX = SCREEN_WIDTH - 40;
  int btnY = 25;
  // Draw Circle Button
  // tft->fillCircle(btnX + 10, btnY + 10, 15, 0x10A2); // Slate Circle
  // tft->drawCircle(btnX + 10, btnY + 10, 15, TFT_WHITE);
  // tft->drawString("+", btnX + 5, btnY + 2);

  // Or "NEW" Text Button
  int newW = 50;
  int newH = 20;
  int newX = SCREEN_WIDTH - newW - 10;
  tft->fillRoundRect(newX, 25, newW, newH, 4, 0x10A2);
  tft->drawRoundRect(newX, 25, newW, newH, 4, TFT_WHITE);
  tft->setTextDatum(MC_DATUM);
  tft->setTextSize(1);
  tft->drawString("NEW", newX + newW / 2, 25 + newH / 2 + 1);

  // --- 2. LIST ---
  int startY = 60;
  int itemH = 45; // Taller for card style
  int itemW = SCREEN_WIDTH - 20;
  int itemX = 10;
  int gap = 8;

  tft->setTextDatum(TL_DATUM);

  if (_tracks.empty()) {
    tft->setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft->setTextDatum(MC_DATUM);
    tft->drawString("No tracks found.", SCREEN_WIDTH / 2, 100);
    tft->drawString("Enable GPS or Create New.", SCREEN_WIDTH / 2, 125);
    _ui->drawStatusBar();
    return;
  }

  // Draw Items
  for (size_t i = 0; i < _tracks.size(); i++) {
    int y = startY + (i * (itemH + gap));
    if (y + itemH > SCREEN_HEIGHT)
      break; // Pagination limit

    // Card BG
    tft->fillRoundRect(itemX, y, itemW, itemH, 6, 0x18E3); // Charcoal
    tft->drawRoundRect(itemX, y, itemW, itemH, 6, TFT_DARKGREY);

    // Track Icon (Left)
    // tft->fillCircle(itemX + 20, y + itemH / 2, 10, TFT_BLACK);
    // Draw Flag or Dot
    tft->fillCircle(itemX + 20, y + itemH / 2, 4,
                    _tracks[i].isCustom ? TFT_CYAN : TFT_GOLD);

    // Name
    tft->setTextColor(TFT_WHITE, 0x18E3);
    tft->setTextFont(2); // Mid size
    tft->setTextDatum(ML_DATUM);
    tft->drawString(_tracks[i].name, itemX + 40, y + itemH / 2 - 5);

    // Detail (Lat/Lon or Configs)
    tft->setTextColor(TFT_SILVER, 0x18E3);
    tft->setTextFont(1);
    char buf[32];
    sprintf(buf, "%d Configs", _tracks[i].configs.size());
    tft->drawString(buf, itemX + 40, y + itemH / 2 + 10);

    // Arrow Right
    tft->setTextColor(TFT_DARKGREY, 0x18E3);
    tft->drawString(">", itemX + itemW - 15, y + itemH / 2);
  }

  _ui->drawStatusBar();
}

void LapTimerScreen::drawTrackOptionsPopup() {
  TFT_eSPI *tft = _ui->getTft();

  // Popup Dimensions
  int w = 220;
  int h = 150;
  int x = (SCREEN_WIDTH - w) / 2;
  int y = (SCREEN_HEIGHT - h) / 2 + 10;

  // Draw Box (Black with White Border)
  tft->fillRoundRect(x, y, w, h, 5, TFT_BLACK);
  tft->drawRoundRect(x, y, w, h, 5, TFT_WHITE);

  // Options
  const char *options[] = {"Select", "Select & Edit", "Invert",
                           "Reinit best Lap", "Remove"};
  int startY = y + 10;
  int itemH = 25;

  tft->setTextDatum(TL_DATUM);
  tft->setFreeFont(&Org_01);
  tft->setTextSize(1);
  tft->setTextColor(TFT_WHITE, TFT_BLACK);

  for (int i = 0; i < 5; i++) {
    tft->drawString(options[i], x + 15, startY + (i * itemH));
  }
}

void LapTimerScreen::drawTrackDetails() {
  TFT_eSPI *tft = _ui->getTft();
  if (_selectedTrackIdx < 0 || _selectedTrackIdx >= _tracks.size())
    return;
  Track &t = _tracks[_selectedTrackIdx];

  // Title
  tft->setTextDatum(TC_DATUM);
  tft->setFreeFont(&Org_01);
  tft->setTextSize(2);
  tft->setTextColor(TFT_WHITE, TFT_BLACK);
  tft->drawString("TRACK DETAILS", SCREEN_WIDTH / 2, 28);

  // Back Arrow
  tft->setTextDatum(TL_DATUM);
  tft->setTextSize(1);
  tft->drawString("<", 10, 25);

  // --- LAYOUT ---
  // Style: Big Card for Map (Top), Info Card (Bottom Left), Action Buttons
  // (Bottom Right) Actually, standard layout: Map Card: Full width or large
  // square? Let's do: Map (Left), Info (Right) -> then Buttons Bottom. Map:
  // x=10, y=55, w=145, h=120 Info: x=165, y=55, w=145, h=120 Buttons: y=185

  int mapX = 10;
  int mapY = 55;
  int mapW = 145;
  int mapH = 120;

  int infoX = 165;
  int infoY = 55;
  int infoW = 145;
  int infoH = 120;

  // 1. MAP CARD
  tft->fillRoundRect(mapX, mapY, mapW, mapH, 8, 0x18E3); // Charcoal
  tft->drawRoundRect(mapX, mapY, mapW, mapH, 8, TFT_DARKGREY);

  // Draw Map Placeholder or Actual if points exist
  // We can reuse drawTrackMap from LapTimer but we need to pass bounds
  // drawTrackMap(mapX, mapY, mapW, mapH); -> This function relies on
  // _recordedPoints being loaded! In update(), when selecting "Select & Edit",
  // we did NOT load points yet? CHECK update(): "Select & Edit" -> _state =
  // STATE_TRACK_DETAILS. No loading points. We should load points roughly if
  // available? Or just draw the simplistic mock. For now, simple mock or text
  // "Map Preview".
  tft->setTextColor(TFT_SILVER, 0x18E3);
  tft->setTextDatum(MC_DATUM);
  tft->drawString("Map Preview", mapX + mapW / 2, mapY + mapH / 2);

  // 2. INFO CARD
  tft->fillRoundRect(infoX, infoY, infoW, infoH, 8, 0x18E3);
  tft->drawRoundRect(infoX, infoY, infoW, infoH, 8, TFT_DARKGREY);

  tft->setTextDatum(TL_DATUM);
  tft->setTextColor(TFT_SILVER, 0x18E3);
  tft->drawString("NAME:", infoX + 10, infoY + 10);

  tft->setTextColor(TFT_WHITE, 0x18E3);
  tft->setTextFont(2);
  // Wrap text if needed?
  tft->drawString(t.name, infoX + 10, infoY + 25);

  tft->setTextFont(1);
  tft->setTextColor(TFT_SILVER, 0x18E3);
  tft->drawString("BEST LAP:", infoX + 10, infoY + 55);

  tft->setTextColor(TFT_GOLD, 0x18E3);
  tft->setTextFont(2);
  tft->drawString("01:10.5", infoX + 10, infoY + 70); // Placeholder

  tft->setTextFont(1);
  tft->setTextColor(TFT_SILVER, 0x18E3);
  char confBuf[32];
  sprintf(confBuf, "Configs: %d", t.configs.size());
  tft->drawString(confBuf, infoX + 10, infoY + 95);

  // 3. ACTION BUTTONS
  int btnY = 190;
  int btnH = 35;
  int btnW = 140;

  // SELECT Button (Green/Slate) -> centered? Or split?
  // Let's put SELECT center-right, BACK center-left?
  // User asked for "Select & Edit". Maybe "EDIT" button?
  // We'll implemented "SELECT" for now.

  int selX = (SCREEN_WIDTH - btnW) / 2;
  tft->fillRoundRect(selX, btnY, btnW, btnH, 6, 0x05E0); // Greenish
  tft->drawRoundRect(selX, btnY, btnW, btnH, 6, TFT_WHITE);

  tft->setTextColor(TFT_WHITE, 0x05E0);
  tft->setTextDatum(MC_DATUM);
  tft->setTextFont(2);
  tft->drawString("SELECT TRACK", selX + btnW / 2, btnY + btnH / 2 + 1);

  _ui->drawStatusBar();
}

void LapTimerScreen::drawRecordTrackStatic() {
  TFT_eSPI *tft = _ui->getTft();

  // Background
  tft->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                SCREEN_HEIGHT - STATUS_BAR_HEIGHT, TFT_BLACK);

  // --- 1. HEADER ---
  // --- 1. HEADER ---
  int headY = 20;
  tft->drawFastHLine(0, headY, SCREEN_WIDTH, COLOR_SECONDARY);
  tft->setTextColor(TFT_WHITE, TFT_BLACK);
  tft->setTextDatum(TC_DATUM);
  tft->setFreeFont(&Org_01);
  tft->setTextSize(2);
  tft->drawString("TRACK RECORDER", SCREEN_WIDTH / 2, headY + 8);

  // Back Button (<)
  tft->setTextDatum(TL_DATUM);
  tft->setTextSize(1);
  tft->drawString("<", 10, 25);

  _ui->drawStatusBar();

  // Reset trackers to force dynamic redraw
  _lastRecordGpsFixed = false;
  _lastRecordSats = -1;
}

void LapTimerScreen::drawRecordTrack() {
  TFT_eSPI *tft = _ui->getTft();
  extern GPSManager gpsManager;

  // --- LAYOUT CONSTANTS ---
  // --- LAYOUT CONSTANTS ---
  const int H_BTN = 42;
  const int Y_BTN = STATUS_BAR_HEIGHT + 5; // Top, under Status Bar (25)

  const int Y_HEADER = Y_BTN + H_BTN + 10; // ~77
  const int H_HEADER = 30;

  const int Y_METRICS = Y_HEADER + H_HEADER + 10; // ~117
  const int Y_METRICS_VAL = Y_METRICS + 22;

  const int Y_FEEDBACK = Y_METRICS_VAL + 50; // ~190

  // --- 1. STATE CHANGE DETECTION ---
  bool stateChanged = (_recordingState != _lastRecordedStateRender);

  if (stateChanged) {
    // Clear Button Area (to prevent overlap when switching states)
    tft->fillRect(0, Y_BTN, SCREEN_WIDTH, H_BTN + 5, TFT_BLACK);

    // --- 1. HEADER (Premium Style) ---
    int headY = 20;
    tft->drawFastHLine(0, headY, SCREEN_WIDTH, COLOR_SECONDARY);

    // Title
    tft->setTextColor(TFT_WHITE, TFT_BLACK);
    tft->setTextDatum(TC_DATUM);
    tft->setFreeFont(&Org_01);
    tft->setTextSize(2);
    tft->drawString("TRACK RECORDER", SCREEN_WIDTH / 2, headY + 8);

    // Back button (<)
    tft->setTextDatum(TL_DATUM);
    tft->setTextSize(1);
    tft->drawString("<", 10, 25);
  }

  // --- 2. GPS STATUS (Premium Card Style) ---
  int cardX = 10;
  int cardY = 55;
  int cardW = SCREEN_WIDTH - 20;
  int cardH = 40; // Compact height

  bool gpsFixed = gpsManager.isFixed();
  int sats = gpsManager.getSatellites();

  if (gpsFixed != _lastRecordGpsFixed || sats != _lastRecordSats ||
      stateChanged) {
    if (stateChanged) {
      // Card Background
      tft->fillRoundRect(cardX, cardY, cardW, cardH, 8, 0x18E3); // Charcoal
      tft->drawRoundRect(cardX, cardY, cardW, cardH, 8, TFT_DARKGREY);

      // Label
      tft->setTextColor(TFT_SILVER, 0x18E3);
      tft->setTextDatum(ML_DATUM);
      tft->setTextFont(2);
      tft->drawString("GPS STATUS", cardX + 10, cardY + cardH / 2);
    }

    // Status Text
    uint16_t statusColor = TFT_RED;
    String statusText = "NO FIX";
    if (gpsFixed) {
      if (sats >= 6) {
        statusColor = TFT_GREEN;
        statusText = "READY";
      } else {
        statusColor = TFT_YELLOW;
        statusText = "WEAK";
      }
    }

    tft->setTextColor(statusColor, 0x18E3);
    tft->setTextDatum(MR_DATUM);
    tft->setTextPadding(120);
    tft->setTextFont(2);
    tft->drawString(statusText + " (" + String(sats) + ")", cardX + cardW - 10,
                    cardY + cardH / 2);
    tft->setTextPadding(0);

    _lastRecordGpsFixed = gpsFixed;
    _lastRecordSats = sats;
  }

  // --- 3. MAIN CONTENT DRAWING ---
  if (stateChanged) {
    // Clear Content Area (Below GPS Card)
    int clearY = cardY + cardH + 5;
    tft->fillRect(0, clearY, SCREEN_WIDTH, SCREEN_HEIGHT - clearY, TFT_BLACK);

    // Grid Layout for Metrics (Slate Boxes)
    int gridY = clearY + 10;
    int boxW = (SCREEN_WIDTH - 25) / 2;
    int boxH = 50;
    int box1X = 10;
    int box2X = 15 + boxW;

    // --- STATIC ELEMENTS PER STATE ---
    if (_recordingState == RECORD_IDLE) {
      // Instructions
      tft->setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      tft->setTextFont(2);
      tft->setTextDatum(MC_DATUM);
      tft->drawString("Go to Start Line", SCREEN_WIDTH / 2, gridY + 15);
      tft->drawString("& Tap Start", SCREEN_WIDTH / 2, gridY + 35);

    } else if (_recordingState == RECORD_ACTIVE) {
      // DRAW GRID BOXES
      // Box 1: Points
      tft->fillRoundRect(box1X, gridY, boxW, boxH, 6, 0x10A2); // Slate
      tft->setTextColor(TFT_SILVER, 0x10A2);
      tft->setTextFont(1);
      tft->setTextDatum(TL_DATUM);
      tft->drawString("POINTS", box1X + 8, gridY + 5);

      // Box 2: Time
      tft->fillRoundRect(box2X, gridY, boxW, boxH, 6, 0x10A2); // Slate
      tft->setTextColor(TFT_SILVER, 0x10A2);
      tft->setTextFont(1);
      tft->setTextDatum(TL_DATUM);
      tft->drawString("TIME", box2X + 8, gridY + 5);

      // STOP Button below grid
      int btnY = gridY + boxH + 20;

      // STOP Button
      int btnW = 140;
      int btnX = (SCREEN_WIDTH - btnW) / 2;
      uint16_t btnColor = (_menuSelectionIdx == 11) ? 0x8000 : TFT_RED;
      uint16_t txtColor = TFT_WHITE;

      tft->fillRoundRect(btnX, btnY, btnW, H_BTN, 8, btnColor);
      if (_menuSelectionIdx == 11)
        tft->drawRoundRect(btnX, btnY, btnW, H_BTN, 8, TFT_WHITE);

      tft->setTextColor(txtColor, btnColor);
      tft->setTextFont(4);
      tft->setTextDatum(MC_DATUM);
      tft->drawString("STOP", SCREEN_WIDTH / 2, btnY + H_BTN / 2 + 2);

    } else if (_recordingState == RECORD_COMPLETE) {

      // Success Message in Grid area
      tft->setTextColor(TFT_GREEN, TFT_BLACK);
      tft->setTextFont(4);
      tft->setTextDatum(MC_DATUM);
      // FIX: Move UP (was gridY + 25)
      tft->drawString("DONE!", SCREEN_WIDTH / 2, gridY + 10);

      int btnY = gridY + boxH + 20;

      // Buttons: SAVE | DISCARD
      int btnW = 100;
      int gap = 20;
      int startX = (SCREEN_WIDTH - (btnW * 2 + gap)) / 2;

      uint16_t saveColor = (_menuSelectionIdx == 12) ? 0x03E0 : TFT_GREEN;
      uint16_t saveTxt = (_menuSelectionIdx == 12) ? TFT_WHITE : TFT_BLACK;

      tft->fillRoundRect(startX, btnY, btnW, H_BTN, 6, saveColor);
      if (_menuSelectionIdx == 12)
        tft->drawRoundRect(startX, btnY, btnW, H_BTN, 6, TFT_WHITE);

      tft->setTextColor(saveTxt, saveColor);
      tft->setTextDatum(MC_DATUM);
      tft->drawString("SAVE", startX + btnW / 2, btnY + H_BTN / 2);

      uint16_t delColor = (_menuSelectionIdx == 13) ? 0x8000 : TFT_RED;

      tft->fillRoundRect(startX + btnW + gap, btnY, btnW, H_BTN, 6, delColor);
      if (_menuSelectionIdx == 13)
        tft->drawRoundRect(startX + btnW + gap, btnY, btnW, H_BTN, 6,
                           TFT_WHITE);

      tft->setTextColor(TFT_WHITE, delColor);
      tft->drawString("DEL", startX + btnW + gap + btnW / 2, btnY + H_BTN / 2);
    }
    _lastRecordedStateRender = _recordingState;
  }

  // --- 4. DYNAMIC UPDATES ---
  int clearY = cardY + cardH + 5;
  int gridY = clearY + 10;
  int boxW = (SCREEN_WIDTH - 25) / 2;
  int box1X = 10;
  int box2X = 15 + boxW;

  if (_recordingState == RECORD_IDLE) {
    static bool lastReady = false;
    bool ready = true; // BYPASS

    if (ready != lastReady || stateChanged) {
      int btnW = 140;
      int btnX = (SCREEN_WIDTH - btnW) / 2;
      int btnY = gridY + 50;

      if (ready) {
        uint16_t btnColor = (_menuSelectionIdx == 10) ? 0x05E0 : TFT_GREEN;
        uint16_t txtColor = (_menuSelectionIdx == 10) ? TFT_WHITE : TFT_BLACK;

        tft->fillRoundRect(btnX, btnY, btnW, H_BTN, 8, btnColor);
        if (_menuSelectionIdx == 10)
          tft->drawRoundRect(btnX, btnY, btnW, H_BTN, 8, TFT_WHITE);

        tft->setTextColor(txtColor, btnColor);
        tft->setTextFont(4);
        tft->setTextDatum(MC_DATUM);
        tft->drawString("START", SCREEN_WIDTH / 2, btnY + H_BTN / 2 + 2);

        // Clear Msg
        tft->fillRect(0, btnY - 30, SCREEN_WIDTH, 25, TFT_BLACK);

      } else {
        tft->fillRect(btnX, btnY, btnW, H_BTN, TFT_BLACK);
        tft->setTextColor(TFT_ORANGE, TFT_BLACK);
        tft->setTextFont(2);
        tft->setTextDatum(MC_DATUM);
        tft->drawString("WAITING FOR GPS...", SCREEN_WIDTH / 2, btnY + 20);
      }
      lastReady = ready;
    }
  } else if (_recordingState == RECORD_ACTIVE) {
    // Dynamic Values inside Boxes

    // Points Value (Left Box)
    tft->setTextColor(TFT_SKYBLUE, 0x10A2);
    tft->setTextFont(4);
    tft->setTextDatum(MC_DATUM);
    tft->setTextPadding(boxW - 10);
    tft->drawNumber(_recordedPoints.size(), box1X + boxW / 2, gridY + 28);

    // Time Value (Right Box)
    tft->setTextColor(TFT_WHITE, 0x10A2);
    unsigned long elapsed = (millis() - _recordingStartTime) / 1000;
    tft->setTextPadding(boxW - 10);
    tft->drawString(String(elapsed) + "s", box2X + boxW / 2, gridY + 28);
    tft->setTextPadding(0);

    // Feedback
    int feedY = 230;
    double currentLat = gpsManager.getLatitude();
    double currentLon = gpsManager.getLongitude();
    double distToStart = gpsManager.distanceBetween(
        _recordStartLat, _recordStartLon, currentLat, currentLon);

    tft->setTextFont(2);
    tft->setTextDatum(MC_DATUM);
    tft->setTextPadding(200);

    if (distToStart < 20 && _recordedPoints.size() > 10) {
      tft->setTextColor(TFT_GREEN, TFT_BLACK);
      tft->drawString("FINISH DETECTED!", SCREEN_WIDTH / 2, feedY);
    } else {
      tft->setTextColor(TFT_LIGHTGREY, TFT_BLACK);
      tft->drawString("Dist: " + String(distToStart, 0) + "m", SCREEN_WIDTH / 2,
                      feedY);
    }
    tft->setTextPadding(0);

  } else if (_recordingState == RECORD_COMPLETE) {
    if (stateChanged) {
      tft->setTextColor(TFT_WHITE, TFT_BLACK);
      tft->setTextFont(2);
      tft->setTextDatum(MC_DATUM);

      unsigned long elapsed =
          (_recordedPoints.back().timestamp - _recordingStartTime) / 1000;
      String stats = String(_recordedPoints.size()) + " Pts  |  " +
                     String(elapsed) + " Sec";

      // FIX: Move text UP to avoid overlap with buttons at gridY + 70
      // Previous: gridY + 70 (Overlapped)
      // New: Stats at gridY + 35
      tft->drawString(stats, SCREEN_WIDTH / 2, gridY + 35);
    }
  }
}

void LapTimerScreen::drawNoGPS() {
  TFT_eSPI *tft = _ui->getTft();

  // Colors (Renamed to avoid config.h macro conflicts)
  uint16_t L_COLOR_BG = TFT_BLACK;
  uint16_t L_COLOR_CARD = 0x18E3; // Charcoal
  uint16_t L_COLOR_BTN = 0x10A2;  // Slate
  uint16_t L_COLOR_TEXT = TFT_WHITE;
  uint16_t L_COLOR_LABEL = TFT_SILVER;

  tft->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                SCREEN_HEIGHT - STATUS_BAR_HEIGHT, L_COLOR_BG);

  // --- HEADER ---
  int headY = 20;
  tft->drawFastHLine(0, headY, SCREEN_WIDTH, COLOR_SECONDARY);

  tft->setTextColor(TFT_WHITE, L_COLOR_BG);
  tft->setTextDatum(TC_DATUM);
  tft->setFreeFont(&Org_01);
  tft->setTextSize(2);
  tft->drawString("GPS STATUS", SCREEN_WIDTH / 2, headY + 8);

  // --- MESSAGE CARD ---
  int cardW = 260;
  int cardH = 100;
  int cardX = (SCREEN_WIDTH - cardW) / 2;
  int cardY = (SCREEN_HEIGHT - cardH) / 2;

  tft->fillRoundRect(cardX, cardY, cardW, cardH, 8, L_COLOR_CARD);
  tft->drawRoundRect(cardX, cardY, cardW, cardH, 8, TFT_DARKGREY);

  // Message Text
  tft->setTextColor(TFT_RED, L_COLOR_CARD);
  tft->setTextSize(1);
  tft->setTextDatum(MC_DATUM);
  tft->drawString("NO SATELLITES FIX", SCREEN_WIDTH / 2, cardY + 30);

  tft->setTextColor(L_COLOR_LABEL, L_COLOR_CARD);
  tft->drawString("Cannot record track.", SCREEN_WIDTH / 2, cardY + 55);
  tft->drawString("Please check GPS antenna.", SCREEN_WIDTH / 2, cardY + 75);

  // --- CONTINUE BUTTON ---
  int btnW = 140;
  int btnH = 36;
  int btnX = (SCREEN_WIDTH - btnW) / 2;
  int btnY = SCREEN_HEIGHT - 50;

  tft->fillRoundRect(btnX, btnY, btnW, btnH, 6, L_COLOR_BTN);
  tft->drawRoundRect(btnX, btnY, btnW, btnH, 6, TFT_WHITE);

  tft->setTextColor(TFT_WHITE, L_COLOR_BTN);
  tft->setTextSize(1);
  tft->drawString("CONTINUE", SCREEN_WIDTH / 2, btnY + (btnH / 2) - 2);

  _ui->drawStatusBar();
}

void LapTimerScreen::drawSummary() {
  TFT_eSPI *tft = _ui->getTft();
  tft->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                SCREEN_HEIGHT - STATUS_BAR_HEIGHT, TFT_BLACK);

  // --- 1. HEADER ---
  tft->drawFastHLine(0, 20, SCREEN_WIDTH, COLOR_SECONDARY);

  // Title
  tft->setTextColor(TFT_WHITE, TFT_BLACK);
  tft->setTextDatum(TC_DATUM);
  tft->setFreeFont(&Org_01);
  tft->setTextSize(2);
  tft->drawString("SESSION SUMMARY", SCREEN_WIDTH / 2, 28);

  // Back Button (<)
  tft->setTextDatum(TL_DATUM);
  tft->drawString("<", 10, 25);

  // --- DATA PROCESSING ---
  int bestIdx = -1;
  unsigned long bestTime = 0;
  unsigned long totalTime = 0;

  if (!_lapTimes.empty()) {
    bestTime = _lapTimes[0];
    bestIdx = 0;
    for (int i = 0; i < _lapTimes.size(); i++) {
      totalTime += _lapTimes[i];
      if (_lapTimes[i] < bestTime) {
        bestTime = _lapTimes[i];
        bestIdx = i;
      }
    }
  }

  // --- 2. BEST LAP CARD (Premium Look) ---
  int cardX = 10;
  int cardY = 55;
  int cardW = SCREEN_WIDTH - 20;
  int cardH = 65;

  tft->fillRoundRect(cardX, cardY, cardW, cardH, 8,
                     0x18E3); // Dark Grey/Charcoalish
  tft->drawRoundRect(cardX, cardY, cardW, cardH, 8, TFT_DARKGREY);

  // Label
  tft->setTextSize(1);
  tft->setTextFont(2);
  tft->setTextColor(TFT_SILVER, 0x18E3);
  tft->setTextDatum(TL_DATUM);
  tft->drawString("BEST LAP", cardX + 10, cardY + 8);

  if (bestIdx != -1) {
    // Lap Number Tag
    String lapTag = "LAP " + String(bestIdx + 1);
    int tagW = tft->textWidth(lapTag);
    tft->fillRoundRect(cardX + cardW - tagW - 15, cardY + 8, tagW + 10, 16, 4,
                       TFT_GOLD);
    tft->setTextColor(TFT_BLACK, TFT_GOLD);
    tft->setTextDatum(MC_DATUM);
    tft->drawString(lapTag, cardX + cardW - 10 - tagW / 2, cardY + 16);

    // Time Value (Big & Center)
    int ms = bestTime % 1000;
    int s = (bestTime / 1000) % 60;
    int m = (bestTime / 60000);
    char buf[16];
    sprintf(buf, "%d:%02d.%02d", m, s, ms / 10);

    tft->setTextColor(TFT_WHITE, 0x18E3);
    tft->setTextFont(6); // Large Font
    tft->setTextDatum(MC_DATUM);
    tft->drawString(buf, cardX + cardW / 2, cardY + cardH / 2 + 8);
  } else {
    tft->setTextColor(TFT_DARKGREY, 0x18E3);
    tft->setTextFont(4);
    tft->setTextDatum(MC_DATUM);
    tft->drawString("--:--.--", cardX + cardW / 2, cardY + cardH / 2 + 5);
  }

  // --- 3. STATS GRID (Rounded Boxes) ---
  int gridY = cardY + cardH + 10;
  int boxW = (SCREEN_WIDTH - 25) / 2;
  int boxH = 45;

  // Box 1: Total Laps
  tft->fillRoundRect(10, gridY, boxW, boxH, 6,
                     0x10A2); // Darker Slate
  tft->setTextColor(TFT_SILVER, 0x10A2);
  tft->setTextFont(1);
  tft->setTextDatum(TL_DATUM);
  tft->drawString("TOTAL LAPS", 18, gridY + 5);

  tft->setTextFont(4);
  tft->setTextColor(TFT_SKYBLUE, 0x10A2);
  tft->setTextDatum(MC_DATUM);
  tft->drawNumber(_lapCount, 10 + boxW / 2, gridY + 25);

  // Box 2: Max RPM or Theoretical
  tft->fillRoundRect(15 + boxW, gridY, boxW, boxH, 6, 0x10A2);
  tft->setTextColor(TFT_SILVER, 0x10A2);
  tft->setTextFont(1);
  tft->setTextDatum(TL_DATUM);
  tft->drawString("MAX RPM", 23 + boxW, gridY + 5);

  tft->setTextFont(4);
  tft->setTextColor(TFT_ORANGE, 0x10A2);
  tft->setTextDatum(MC_DATUM);
  tft->drawNumber(_maxRpmSession, 15 + boxW + boxW / 2, gridY + 25);

  // --- 4. DATA LIST (Top Laps) ---
  int listY = gridY + boxH + 10;
  tft->drawFastHLine(20, listY, SCREEN_WIDTH - 40, TFT_DARKGREY);

  // Header tiny
  tft->setTextFont(1);
  tft->setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft->setTextDatum(TL_DATUM);
  tft->drawString("RECENT LAPS", 20, listY + 5);

  // Sort by recent? Or show last few?
  // Let's show last 3 laps (if available) to verify
  // consistency
  int itemsToShow = 3;
  int startIdx =
      (_lapTimes.size() > itemsToShow) ? _lapTimes.size() - itemsToShow : 0;

  int rowY = listY + 20;
  for (int i = _lapTimes.size() - 1; i >= startIdx; i--) {
    if (i < 0)
      break;

    unsigned long t = _lapTimes[i];
    int ms = t % 1000;
    int s = (t / 1000) % 60;
    int m = (t / 60000);
    char buf[32];

    // Color diff: Green if best?
    uint16_t color = (i == bestIdx) ? TFT_GREEN : TFT_WHITE;

    sprintf(buf, "%d:%02d.%02d", m, s, ms / 10);

    tft->setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    tft->drawString(String(i + 1) + ".", 20, rowY);

    tft->setTextColor(color, TFT_BLACK);
    tft->setTextFont(2);
    tft->drawString(buf, 60, rowY);

    // Delta? (vs Best)
    if (bestIdx != -1 && i != bestIdx) {
      long delta = t - bestTime;
      int d_ms = delta % 1000;
      int d_s = delta / 1000;
      sprintf(buf, "+%d.%02d", d_s, d_ms / 10);
      tft->setTextColor(TFT_RED, TFT_BLACK); // Slower
      tft->setTextDatum(TR_DATUM);
      tft->drawString(buf, SCREEN_WIDTH - 20, rowY);
      tft->setTextDatum(TL_DATUM);
    }

    rowY += 20;
  }

  // Done button hint?
  // tft->setTextColor(TFT_DARKGREY, TFT_BLACK);
  // tft->setTextDatum(BC_DATUM);
  // tft->setTextFont(1);
  // tft->drawString("Tap to Exit", SCREEN_WIDTH/2,
  // SCREEN_HEIGHT - 5);

  _ui->drawStatusBar();
}

void LapTimerScreen::drawRacingStatic() {
  TFT_eSPI *tft = _ui->getTft();

  // Use fillScreen to ensure complete clearing of previous screen or ghosting
  // This is safer than fillRect for full-screen modes
  tft->fillScreen(TFT_BLACK);

  // --- LAYOUT DEFINITIONS ---
  int topBarH = 25;

  // COMPACT LAYOUT
  int rpmY = STATUS_BAR_HEIGHT + 5;
  int rpmH = 20;
  int midY = rpmY + rpmH + 8;
  int midH = 85;
  int gridY = midY + midH + 8;

  int centerSplitX = SCREEN_WIDTH / 3 + 20;

  // 1. RPM BAR CONTAINER
  tft->drawRoundRect(5, rpmY, SCREEN_WIDTH - 10, rpmH, 4, TFT_DARKGREY);

  // --- 2. MAIN LAYOUT SPLIT ---

  // MAP CARD (Left)
  int mapX = 5;
  int mapW = centerSplitX - 10;
  tft->fillRoundRect(mapX, midY, mapW, midH, 8, 0x18E3); // Charcoal

  drawTrackMap(mapX, midY, mapW, midH);

  // MAIN TIMER AREA
  int timeAreaW = SCREEN_WIDTH - centerSplitX - 5;
  int timeCenterX = centerSplitX + timeAreaW / 2;
  int timeY = midY + 52;

  // Current Lap Label Box
  int lblW = 150;
  int lblH = 22;
  int lblY = midY + 4;

  // Draw Box
  tft->fillRoundRect(timeCenterX - lblW / 2, lblY, lblW, lblH, 4, 0x18E3);
  tft->setTextColor(TFT_SILVER, 0x18E3);
  tft->setTextFont(1);
  tft->setTextDatum(ML_DATUM);
  tft->drawString("CURRENT LAP", timeCenterX - lblW / 2 + 10,
                  lblY + lblH / 2 + 1);

  // --- 3. BOTTOM STATS GRID ---
  int gridH = SCREEN_HEIGHT - gridY - 5;
  int cardW = (SCREEN_WIDTH - 15) / 2;
  int cardH = (gridH - 5) / 2;

  uint16_t cardBg = 0x18E3;

  // Helper to draw formatted card
  auto drawCard = [&](int x, int y, String label) {
    tft->fillRoundRect(x, y, cardW, cardH, 8, cardBg);
    tft->setTextColor(TFT_SILVER, cardBg);
    tft->setTextFont(1);
    tft->setTextDatum(TL_DATUM);
    tft->drawString(label, x + 10, y + 6);
  };

  // Row 1 Left: LAST LAP
  drawCard(5, gridY, "LAST LAP");

  // Row 1 Right: BEST LAP
  drawCard(10 + cardW, gridY, "BEST LAP");

  // Row 2 Left: SPEED
  int row2Y = gridY + cardH + 5;
  drawCard(5, row2Y, "SPEED");

  // Row 2 Right: SATS
  drawCard(10 + cardW, row2Y, "SATS");

  // Redraw StatusBar on top of black fill
  _ui->drawStatusBar(true);
}

void LapTimerScreen::drawRPMBar(int rpm, int maxRpm) {
  TFT_eSPI *tft = _ui->getTft();
  int x = 7;
  int y = STATUS_BAR_HEIGHT + 7;
  int w = SCREEN_WIDTH - 14;
  int h = 16;

  if (maxRpm < 5000)
    maxRpm = 8000;

  int fillW = map(constrain(rpm, 0, maxRpm), 0, maxRpm, 0, w);

  // Gradient Color Logic
  uint16_t color = TFT_GREEN;
  if (rpm > maxRpm * 0.9)
    color = TFT_RED;
  else if (rpm > maxRpm * 0.7)
    color = TFT_YELLOW;

  // Draw Fill
  tft->fillRect(x, y, fillW, h, color);
  // Clear rest
  tft->fillRect(x + fillW, y, w - fillW, h, 0x10A2);
}

void LapTimerScreen::drawRacing() {
  TFT_eSPI *tft = _ui->getTft();

  // --- RE-DEFINE LAYOUT CONSTANTS ---
  int rpmY = STATUS_BAR_HEIGHT + 5;
  int rpmH = 20;
  int midY = rpmY + rpmH + 8;
  int midH = 85;

  int centerSplitX = SCREEN_WIDTH / 3 + 20;

  int gridY = midY + midH + 8;
  int gridH = SCREEN_HEIGHT - gridY - 5;
  int cardW = (SCREEN_WIDTH - 15) / 2;
  int cardH = (gridH - 5) / 2;
  int row2Y = gridY + cardH + 5;

  uint16_t cardBg = 0x18E3;

  // --- 1. RPM BAR ---
  int currentRpm = gpsManager.getRPM();
  if (currentRpm > _maxRpmSession)
    _maxRpmSession = currentRpm;
  if (abs(currentRpm - _lastRpmRender) > 50) {
    drawRPMBar(currentRpm, 10000);
    _lastRpmRender = currentRpm;
  }

  // --- 2. MAIN TIME ---
  unsigned long currentLap = 0;
  if (_isRecording)
    currentLap = millis() - _currentLapStart;

  int ms = (currentLap % 1000);
  int s = (currentLap / 1000) % 60;
  int m = (currentLap / 60000);
  char timeBuf[16];
  sprintf(timeBuf, "%02d:%02d.%02d", m, s, ms / 10);

  int timeAreaW = SCREEN_WIDTH - centerSplitX - 5;
  int timeCenterX = centerSplitX + timeAreaW / 2;
  int timeY = midY + 52;

  tft->setTextColor(TFT_WHITE, TFT_BLACK);
  tft->setTextFont(6);
  tft->setTextDatum(MC_DATUM);
  tft->setTextPadding(timeAreaW);
  tft->drawString(timeBuf, timeCenterX, timeY);
  tft->setTextPadding(0);

  // Lap Count (Inside Label Box)
  if (_lapCount != _lastLapCountRender) {
    // Redraw "L x" inside the "CURRENT LAP" box
    int lblW = 150;
    int lblH = 22;
    int lblY = midY + 4;

    // Background is charcoal (same as box)
    tft->setTextColor(TFT_GOLD, 0x18E3);
    tft->setTextFont(2);
    tft->setTextDatum(MR_DATUM);
    // Right Align inside box
    tft->setTextPadding(40);
    tft->drawString("L " + String(_lapCount + 1), timeCenterX + lblW / 2 - 10,
                    lblY + lblH / 2 + 1);
    tft->setTextPadding(0);

    _lastLapCountRender = _lapCount;
  }

  // --- 3. BOTTOM DATA GRID ---
  int valOffsetY = cardH / 2 + 5;

  // ROW 1 LEFT: LAST LAP
  long lastTimeVal = (_lastLapTime > 0) ? _lastLapTime : -1;
  if (lastTimeVal != _lastLastLapTimeRender) {
    String text = "--:--.--";
    if (lastTimeVal > 0) {
      int lms = lastTimeVal % 1000;
      int ls = (lastTimeVal / 1000) % 60;
      int lm = (lastTimeVal / 60000);
      char b[16];
      sprintf(b, "%02d:%02d.%02d", lm, ls, lms / 10);
      text = String(b);
    }
    tft->setTextColor(TFT_WHITE, cardBg);
    tft->setTextFont(4);
    tft->setTextDatum(MC_DATUM);
    tft->setTextPadding(cardW - 10);
    tft->drawString(text, 5 + cardW / 2, gridY + valOffsetY);
    tft->setTextPadding(0);
    _lastLastLapTimeRender = lastTimeVal;
  }

  // ROW 1 RIGHT: BEST LAP
  long bestTimeVal = (_bestLapTime > 0) ? _bestLapTime : -1;
  if (bestTimeVal != _lastBestLapTimeRender) {
    String text = "--:--.--";
    if (bestTimeVal > 0) {
      int bms = bestTimeVal % 1000;
      int bs = (bestTimeVal / 1000) % 60;
      int bm = (bestTimeVal / 60000);
      char b[16];
      sprintf(b, "%02d:%02d.%02d", bm, bs, bms / 10);
      text = String(b);
    }
    tft->setTextColor(TFT_GOLD, cardBg);
    tft->setTextFont(4);
    tft->setTextDatum(MC_DATUM);
    tft->setTextPadding(cardW - 10);
    tft->drawString(text, 10 + 1.5 * cardW, gridY + valOffsetY);
    tft->setTextPadding(0);
    _lastBestLapTimeRender = bestTimeVal;
  }

  // ROW 2 LEFT: SPEED
  float speed = gpsManager.getSpeedKmph();
  if (abs(speed - _lastSpeed) > 0.5) {
    tft->setTextColor(TFT_CYAN, cardBg);
    tft->setTextFont(6);
    tft->setTextDatum(MC_DATUM);
    tft->setTextPadding(cardW - 10);
    tft->drawFloat(speed, 1, 5 + cardW / 2, row2Y + valOffsetY);
    tft->setTextPadding(0);
    _lastSpeed = speed;
  }

  // ROW 2 RIGHT: SATS
  int sats = gpsManager.getSatellites();
  if (sats != _lastSats) {
    tft->setTextColor(TFT_WHITE, cardBg);
    tft->setTextFont(4);
    tft->setTextDatum(MC_DATUM);
    tft->setTextPadding(cardW - 10);
    tft->drawNumber(sats, 10 + 1.5 * cardW, row2Y + valOffsetY);
    tft->setTextPadding(0);
    _lastSats = sats;
  }
}

void LapTimerScreen::drawTrackMap(int x, int y, int w, int h) {
  TFT_eSPI *tft = _ui->getTft();

  // Logic to draw map from points
  // If no recorded points (or loaded track points), show
  // "No Map" or simple loop

  // For now, if we have _recordedPoints, we draw them
  // scaled
  if (_recordedPoints.empty()) {
    // Scaling mock track to fit 80% of the container
    int cx = x + w / 2;
    int cy = y + h / 2;

    // Scale factor: Fit '80px' nominal shape into min(w, h)
    // * 0.8
    float scale = (min(w, h) * 0.8) / 80.0;

    // Original coordinates scaled
    // (-20, -40) to (20, -40) -> Top Horizontal
    tft->drawLine(cx - 20 * scale, cy - 40 * scale, cx + 20 * scale,
                  cy - 40 * scale, TFT_WHITE);
    // (20, -40) to (30, -10) -> Top Right
    tft->drawLine(cx + 20 * scale, cy - 40 * scale, cx + 30 * scale,
                  cy - 10 * scale, TFT_WHITE);
    // (30, -10) to (10, 40) -> Bottom Right
    tft->drawLine(cx + 30 * scale, cy - 10 * scale, cx + 10 * scale,
                  cy + 40 * scale, TFT_WHITE);
    // (10, 40) to (-20, 30) -> Bottom Left
    tft->drawLine(cx + 10 * scale, cy + 40 * scale, cx - 20 * scale,
                  cy + 30 * scale, TFT_WHITE);
    // (-20, 30) to (-20, -40) -> Left Vertical
    tft->drawLine(cx - 20 * scale, cy + 30 * scale, cx - 20 * scale,
                  cy - 40 * scale, TFT_WHITE);

    // Finish line dot
    tft->fillCircle(cx - 20 * scale, cy - 40 * scale, 4 * scale, TFT_RED);
    return;
  }

  // Auto-scale logic
  double minLat = 90.0, maxLat = -90.0;
  double minLon = 180.0, maxLon = -180.0;

  // Find bounds
  // Use a sampling if too many points to be fast
  int step = 1;
  if (_recordedPoints.size() > 500)
    step = _recordedPoints.size() / 500;

  for (size_t i = 0; i < _recordedPoints.size(); i += step) {
    if (_recordedPoints[i].lat < minLat)
      minLat = _recordedPoints[i].lat;
    if (_recordedPoints[i].lat > maxLat)
      maxLat = _recordedPoints[i].lat;
    if (_recordedPoints[i].lon < minLon)
      minLon = _recordedPoints[i].lon;
    if (_recordedPoints[i].lon > maxLon)
      maxLon = _recordedPoints[i].lon;
  }

  if (minLat == maxLat || minLon == maxLon)
    return;

  // Scale factors
  // Expand bounds slightly (5%)
  double latRange = maxLat - minLat;
  double lonRange = maxLon - minLon;

  // Aspect ratio correction (basic equirectangular
  // approximation) Lat degrees are constant distance, Lon
  // degrees shrink by cos(lat)
  double avgLatRad = (minLat + maxLat) / 2.0 * DEG_TO_RAD;
  double lonScale = cos(avgLatRad);

  double aspect = (lonRange * lonScale) / latRange;

  int drawW = w - 20;
  int drawH = h - 20;
  double screenAspect = (double)drawW / drawH;

  double scaleX, scaleY;
  int offsetX = x + 10;
  int offsetY = y + 10;

  if (aspect > screenAspect) {
    // Wider than screen: fit to width
    scaleX = drawW / (lonRange * lonScale);
    scaleY = scaleX; // Keep aspect
    // Center Y
    double contentH = latRange * scaleY;
    offsetY += (drawH - contentH) / 2;
  } else {
    // Taller than screen: fit to height
    scaleY = drawH / latRange;
    scaleX = scaleY; // Keep aspect
    // Center X
    double contentW = (lonRange * lonScale) * scaleX;
    offsetX += (drawW - contentW) / 2;
  }

  // Draw points
  for (size_t i = 0; i < _recordedPoints.size() - step; i += step) {
    GPSPoint &p1 = _recordedPoints[i];
    GPSPoint &p2 = _recordedPoints[i + step];

    // Map to screen
    // Note: Y is inverted (Lat increases up, Screen Y
    // increases down)
    int x1 = offsetX + (int)((p1.lon - minLon) * lonScale * scaleX);
    int y1 = offsetY + (int)((maxLat - p1.lat) * scaleY);

    int x2 = offsetX + (int)((p2.lon - minLon) * lonScale * scaleX);
    int y2 = offsetY + (int)((maxLat - p2.lat) * scaleY);

    tft->drawLine(x1, y1, x2, y2, TFT_WHITE);
  }

  // Draw current position cursor?
  // Locate current pos
  double currLat = gpsManager.getLatitude();
  double currLon = gpsManager.getLongitude();
  int cx = offsetX + (int)((currLon - minLon) * lonScale * scaleX);
  int cy = offsetY + (int)((maxLat - currLat) * scaleY);

  // Verify bounds before drawing cursor
  if (cx >= x && cx < x + w && cy >= y && cy < y + h) {
    tft->fillCircle(cx, cy, 4, TFT_RED);
  }
}

void LapTimerScreen::checkFinishLine() {
  double dist = gpsManager.distanceBetween(gpsManager.getLatitude(),
                                           gpsManager.getLongitude(),
                                           _finishLat, _finishLon);

  // Logika Deteksi Mulai/Lap
  // Members: _finishLineInside, _lastFinishCross

  if (dist < 20) { // Radius 20m
    if (!_finishLineInside &&
        (millis() - _lastFinishCross > 10000)) { // Debounce 10s
      // Lap Baru / Mulai
      if (!_isRecording) {
        _isRecording = true;
        sessionManager.startSession();
        _lapCount = 1;
      } else {
        unsigned long lapTime = millis() - _currentLapStart;
        _lastLapTime = lapTime;
        _lapTimes.push_back(lapTime); // Tambahkan ke riwayat
        if (_bestLapTime == 0 || lapTime < _bestLapTime)
          _bestLapTime = lapTime;
        _lapCount++;

        // Log Lap Event
        // Format: LAP, LapNumber, LapTimeMs
        String lapLog = "LAP," + String(_lapCount) + "," + String(lapTime);
        sessionManager.logData(lapLog);

        // Reset max RPM for new lap if desired, or keep
        // session max? Usually Session Max is what you want
        // to see overall. But "RPM max" on screen might
        // imply "This Lap". Design usually shows Session or
        // Lap Max. Let's keep Session Max for now as it's
        // easier. If "This Lap", we reset here:
        // _maxRpmSession = 0;
      }
      _currentLapStart = millis();
      _lastFinishCross = millis();
      _finishLineInside = true;
    }
  } else {
    if (dist > 25)
      _finishLineInside = false;
  }
}
