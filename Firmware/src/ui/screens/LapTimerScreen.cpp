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
  _lapTimes.clear();
  _listScroll = 0;
  _menuSelectionIdx = -1;

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
  // tft->fillScreen(COLOR_BG); // Removed to avoid flash
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
  sonoma.configs.push_back({"Default"});

  // Always add test track for DEBUG if no GPS fix or just to see something?
  // User req: "lists all tracks within a 50 km radius".
  // If I strictly enforce, I might see nothing in dev.
  // I will enforce dist check if GPS is fixed.
  if (gpsManager.isFixed()) {
    double d =
        gpsManager.distanceBetween(curLat, curLon, sonoma.lat, sonoma.lon);
    if (d < 50000)
      _tracks.push_back(sonoma);
  } else {
    // For Debug/Sim without GPS, maybe add it?
    // User said "If GPS data is unavailable... cannot enter the menu".
    // So logic in `update` prevents us getting here without GPS.
    // So here safely assume GPS is fixed.
    // I'll leave the check enabled.
    double d =
        gpsManager.distanceBetween(curLat, curLon, sonoma.lat, sonoma.lon);
    if (d < 50000)
      _tracks.push_back(sonoma);
  }

  // Add a fake "Nearby" one for testing if list is empty?
  // _tracks.push_back(sonoma); // FORCE ADD FOR UI TESTING (Remove later)
}

void LapTimerScreen::update() {
  UIManager::TouchPoint p = _ui->getTouchPoint();
  bool touched = (p.x != -1);

  if (_state == STATE_MENU) {
    if (touched) {
      // 1. Back/Home (< 60x60)
      if (p.x < 60 && p.y < 60) {
        if (millis() - _lastTouchTime < 200)
          return;
        _lastTouchTime = millis();

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
          // Debounce Check (e.g. 200ms)
          if (millis() - _lastTouchTime < 200) {
            return;
          }
          _lastTouchTime = millis();

          // Double Tap Logic
          if (_menuSelectionIdx == touchedIdx) {
            // Second tap on SAME button -> Execute Action

            // Execute Action
            if (touchedIdx == 0) { // Select Track
              if (!gpsManager.isFixed()) {
                _state = STATE_NO_GPS;
                _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                                        SCREEN_HEIGHT - STATUS_BAR_HEIGHT,
                                        COLOR_BG);
                drawNoGPS();
              } else {
                // Go to Searching Screen first
                _state = STATE_SEARCHING;
                _searchStartTime = millis();
                _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                                        SCREEN_HEIGHT - STATUS_BAR_HEIGHT,
                                        COLOR_BG);
                drawSearching();
              }
            } else if (touchedIdx == 1) { // Race Screen
              _state = STATE_RACING;
              _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                                      SCREEN_HEIGHT - STATUS_BAR_HEIGHT,
                                      COLOR_BG);
              drawRacingStatic();
              drawRacing();
            } else if (touchedIdx == 2) { // Session Summary
              _state = STATE_SUMMARY;
              _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                                      SCREEN_HEIGHT - STATUS_BAR_HEIGHT,
                                      COLOR_BG);
              drawSummary();
            } else if (touchedIdx == 3) { // Record Track
              _recordingState = RECORD_IDLE;
              _recordedPoints.clear();
              _state = STATE_RECORD_TRACK;
              _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                                      SCREEN_HEIGHT - STATUS_BAR_HEIGHT,
                                      COLOR_BG);
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
          // Retry Logic
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
        }
        // Continue (Right) x=170, w=130 -> Back to Menu
        else if (p.x > 170 && p.x < 300) {
          _state = STATE_MENU;
          _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                                  SCREEN_HEIGHT - STATUS_BAR_HEIGHT, COLOR_BG);
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
                              SCREEN_HEIGHT - STATUS_BAR_HEIGHT, COLOR_BG);
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
        _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                                SCREEN_HEIGHT - STATUS_BAR_HEIGHT, COLOR_BG);
        drawRecordTrack();
        return;
      }

      // 3. Track Selection (Open Popup)
      int startY = 60;
      int itemH = 30;
      if (p.y > startY) {
        int idx = (p.y - startY) / itemH;
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

          _state = STATE_SUMMARY;
          _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                                  SCREEN_HEIGHT - STATUS_BAR_HEIGHT, COLOR_BG);
          drawSummary();
          _ui->drawStatusBar();
        } else if (idx == 1) { // Select & Edit
          // Go to Details Screen
          _state = STATE_TRACK_DETAILS;
          _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                                  SCREEN_HEIGHT - STATUS_BAR_HEIGHT, COLOR_BG);
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
                                  SCREEN_HEIGHT - STATUS_BAR_HEIGHT, COLOR_BG);
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
                                SCREEN_HEIGHT - STATUS_BAR_HEIGHT, COLOR_BG);
        drawTrackList();
      }
    }
  } else if (_state == STATE_TRACK_DETAILS) {
    if (touched) {
      // Back Button logic in Details screen
      if (p.x < 60 && p.y < 60) {
        if (millis() - _lastTouchTime < 200)
          return;
        _lastTouchTime = millis();

        if (millis() - _lastBackTapTime < 500) {
          // Back from details -> List (not Menu)
          _state = STATE_TRACK_LIST;
          _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                                  SCREEN_HEIGHT - STATUS_BAR_HEIGHT, COLOR_BG);
          drawTrackList();
          _lastBackTapTime = 0;
        } else {
          _lastBackTapTime = millis();
        }
        return;
      }

      // Select button in details screen?
      // Reuse existing logic or simplify since user moved options to Popup
      // User said "Select & Edit: View...and reposition".
      // So Details screen likely needs "Start/Finish" edit mode.
      // For now, I leave the old Details screen logic but only reachable via
      // "Select & Edit".

      // Let's allow the "left menu" in Details to still function or remove it?
      // "Select" button there is redundant but harmless.
      // I'll leave the existing `else if (_state == STATE_TRACK_DETAILS)` block
      // structure but I need to make sure I didn't overwrite it with my
      // replacement. Ah, I am replacing `STATE_TRACK_LIST` and
      // `STATE_TRACK_DETAILS` input logic here.

      // Wait, `STATE_TRACK_DETAILS` logic below needs to be preserved or
      // re-added. The snippet I'm replacing covers `STATE_TRACK_LIST` and
      // `STATE_TRACK_DETAILS` (from previous edits).

      // I need to include the `STATE_TRACK_DETAILS` logic in my replacement
      // content if I'm overwriting it. Yes, I will keep the simple Back logic
      // for Details for now.
    }
  } else if (_state == STATE_SUMMARY) {

    // --- LOGIKA STATUS RINGKASAN ---
    if (touched) {
      // 1. KEMBALI/MENU (Kiri Atas)
      if (p.x < 70 && p.y < 70) {
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
                                    COLOR_BG);
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
                                SCREEN_HEIGHT - STATUS_BAR_HEIGHT, COLOR_BG);
        drawTrackList();
        _ui->drawStatusBar();
        return;
      }
    }

  } else if (_state == STATE_RECORD_TRACK) {
    extern GPSManager gpsManager;

    if (touched) {
      // Back button (bottom left)
      if (p.x < 80 && p.y > SCREEN_HEIGHT - 30) {
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

      if (_recordingState == RECORD_IDLE) {
        // START button (bottom center)
        if (p.x > SCREEN_WIDTH / 2 - 50 && p.x < SCREEN_WIDTH / 2 + 50 &&
            p.y > SCREEN_HEIGHT - 60 && p.y < SCREEN_HEIGHT - 20) {

          if (gpsManager.isFixed() && gpsManager.getSatellites() >= 6) {
            // Start recording
            _recordingState = RECORD_ACTIVE;
            _recordStartLat = gpsManager.getLatitude();
            _recordStartLon = gpsManager.getLongitude();
            _recordingStartTime = millis();
            _lastPointTime = millis();
            _recordedPoints.clear();

            // Add first point
            GPSPoint firstPoint;
            firstPoint.lat = _recordStartLat;
            firstPoint.lon = _recordStartLon;
            firstPoint.timestamp = millis();
            _recordedPoints.push_back(firstPoint);

            drawRecordTrack();
          }
        }
      } else if (_recordingState == RECORD_ACTIVE) {
        // STOP button (bottom center)
        if (p.x > SCREEN_WIDTH / 2 - 50 && p.x < SCREEN_WIDTH / 2 + 50 &&
            p.y > SCREEN_HEIGHT - 60 && p.y < SCREEN_HEIGHT - 20) {

          _recordingState = RECORD_COMPLETE;
          drawRecordTrack();
        }
      } else if (_recordingState == RECORD_COMPLETE) {
        // SAVE button
        if (p.x > SCREEN_WIDTH / 2 - 50 && p.x < SCREEN_WIDTH / 2 + 50 &&
            p.y > SCREEN_HEIGHT - 100 && p.y < SCREEN_HEIGHT - 60) {

          // TODO: Implement track save to SD card
          // For now, just use the track as finish line
          _finishLat = _recordStartLat;
          _finishLon = _recordStartLon;
          _finishSet = true;
          _currentTrackName = "Recorded Track";

          // Go to racing screen
          _state = STATE_RACING;
          _lapCount = 0;
          _bestLapTime = 0;
          _lapTimes.clear();
          _currentLapStart = millis();
          _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                                  SCREEN_HEIGHT - STATUS_BAR_HEIGHT, COLOR_BG);
          drawRacingStatic();
          drawRacing();
          return;
        }

        // DISCARD button
        if (p.x > SCREEN_WIDTH / 2 - 50 && p.x < SCREEN_WIDTH / 2 + 50 &&
            p.y > SCREEN_HEIGHT - 50 && p.y < SCREEN_HEIGHT - 10) {

          _recordingState = RECORD_IDLE;
          _recordedPoints.clear();
          drawRecordTrack();
        }
      }
    }

    // GPS Recording Loop (when ACTIVE)
    if (_recordingState == RECORD_ACTIVE) {
      unsigned long now = millis();

      // Record point every 2 seconds OR every 10 meters
      if (now - _lastPointTime > 2000) {
        if (gpsManager.isFixed()) {
          double currentLat = gpsManager.getLatitude();
          double currentLon = gpsManager.getLongitude();

          // Check distance from last point
          if (_recordedPoints.size() > 0) {
            GPSPoint &lastPoint = _recordedPoints.back();
            double dist = gpsManager.distanceBetween(
                lastPoint.lat, lastPoint.lon, currentLat, currentLon);

            // Only record if moved at least 5 meters
            if (dist > 5) {
              GPSPoint newPoint;
              newPoint.lat = currentLat;
              newPoint.lon = currentLon;
              newPoint.timestamp = now;
              _recordedPoints.push_back(newPoint);

              // Check for finish line detection
              double distToStart = gpsManager.distanceBetween(
                  _recordStartLat, _recordStartLon, currentLat, currentLon);

              // Auto-finish if back at start (< 15m) and recorded enough points
              // (> 20)
              if (distToStart < 15 && _recordedPoints.size() > 20) {
                _recordingState = RECORD_COMPLETE;
              }

              _lastPointTime = now;
            }
          }
        }

        // Redraw to update stats
        if (now - _lastUpdate > 500) {
          drawRecordTrack();
          _lastUpdate = now;
        }
      }
    }

  } else if (_state == STATE_NO_GPS) {
    if (touched) {
      // Retry or Continue
      // Retry: Check GPS again
      if (p.x < SCREEN_WIDTH / 2) {
        if (gpsManager.isFixed()) {
          loadTracks(); // Refresh tracks with valid GPS
          _state = STATE_TRACK_LIST;
          // Clear background for Track List
          _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                                  SCREEN_HEIGHT - STATUS_BAR_HEIGHT, COLOR_BG);
          drawTrackList();
        } else {
          drawNoGPS(); // Redraws content area
        }
      } else {
        // Continue Anyway (Manual/Custom)
        _state = STATE_TRACK_LIST;
        _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                                SCREEN_HEIGHT - STATUS_BAR_HEIGHT, COLOR_BG);
        drawTrackList();
      }
    }
  } else {
    // --- LOGIKA STATUS BALAPAN ---
    // Sentuh: BERHENTI/SELESAI
    if (touched) {
      // 1. Back Button (Top Left)
      if (p.x < 60 && p.y < 60) {
        if (millis() - _lastTouchTime < 200)
          return;
        _lastTouchTime = millis();

        if (_menuSelectionIdx == -2) {
          if (millis() - _lastBackTapTime < 500) {
            // Execute Stop Logic
            if (sessionManager.isLogging()) {
              String dateStr =
                  gpsManager.getDateString() + " " + gpsManager.getTimeString();
              sessionManager.appendToHistoryIndex(
                  "Track Session", dateStr, _lapCount, _bestLapTime, "TRACK");
            }
            sessionManager.stopSession();
            _isRecording = false;

            // Return to Menu
            _state = STATE_MENU;
            _menuSelectionIdx = -1;
            _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                                    SCREEN_HEIGHT - STATUS_BAR_HEIGHT,
                                    COLOR_BG);
            drawMenu();
            _lastBackTapTime = 0;
          } else {
            _lastBackTapTime = millis();
          }
        } else {
          _menuSelectionIdx = -2;
          // Highlight Arrow
          TFT_eSPI *tft = _ui->getTft();
          tft->setTextColor(COLOR_HIGHLIGHT, TFT_BLACK);
          tft->setTextDatum(TL_DATUM);
          tft->setFreeFont(&Org_01);
          tft->setTextSize(2);
          tft->drawString("<", 10, 25);
          _lastBackTapTime = millis();
        }
        return;
      }
      // Tombol Berhenti (Bawah Tengah) - Tetap ada
      if (p.x > 80 && p.x < 240 && p.y > STOP_BTN_Y) {
        _state = STATE_SUMMARY;

        // Simpan Riwayat
        if (sessionManager.isLogging()) {
          // Dapatkan string Tanggal/Waktu?
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

      // Mode Toggle (Kiri < 80)
      if (p.x < 80 && p.y > 60 && p.y < 200) {
        // Double Tap Logic
        unsigned long now = millis();
        if (now - _lastModeTapTime < 400) {
          // Second tap! Toggle Mode.
          if (_raceMode == MODE_BEST)
            _raceMode = MODE_LAST;
          else if (_raceMode == MODE_LAST)
            _raceMode = MODE_PREDICTIVE;
          else
            _raceMode = MODE_BEST;

          // Trigger Redraw safely
          _needsStaticRedraw = true;
          _lastModeTapTime = 0; // Reset
        } else {
          // First tap
          _lastModeTapTime = now;
        }
        return;
      }

      // Set Selesai Manual (jika belum diset)
      if (!_finishSet && gpsManager.isFixed() && p.y < 200) {
        _finishLat = gpsManager.getLatitude();
        _finishLon = gpsManager.getLongitude();
        _finishSet = true;
        _ui->getTft()->fillCircle(SCREEN_WIDTH - 20, 35, 5, TFT_GREEN);
      }
    }

    // Logic
    if (_finishSet)
      checkFinishLine();

    // Pencatatan (1Hz atau 10Hz?)
    if (sessionManager.isLogging() && (millis() - _lastUpdate > 100)) {
      String data = String(millis()) + "," +
                    String(gpsManager.getLatitude(), 6) + "," +
                    String(gpsManager.getLongitude(), 6) + "," +
                    String(gpsManager.getSpeedKmph()) + "," +
                    String(gpsManager.getSatellites());
      sessionManager.logData(data);
    }

    // UI Update (RENDER LOOP)
    if (_needsStaticRedraw) {
      drawRacingStatic();
      // Forces dynamic update immediately
      _lastUpdate = 0;
      _needsStaticRedraw = false;
    }

    if (millis() - _lastUpdate > 100) {
      drawRacing();
      // _ui->drawStatusBar(); // Handled by UIManager
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
  tft->drawLine(cx - 20, mapY, cx + 20, mapY, TFT_WHITE);      // Top
  tft->drawLine(cx + 20, mapY, cx + 30, mapY + 25, TFT_WHITE); // Right Slope
  tft->drawLine(cx + 30, mapY + 25, cx - 30, mapY + 25, TFT_WHITE); // Bottom
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

  // Header
  tft->drawFastHLine(0, 20, SCREEN_WIDTH, COLOR_SECONDARY);

  // Title "Nearby Tracks" (Left)
  tft->setTextDatum(TL_DATUM);
  tft->setFreeFont(&Org_01);
  tft->setTextSize(2);
  tft->setTextColor(COLOR_TEXT, COLOR_BG);
  tft->drawString("Nearby Tracks", 10, 25);

  // "New Track" Button (Top Right)
  // Box: x=200, y=22, w=100, h=25 (Approx)
  int btnX = SCREEN_WIDTH - 110;
  int btnY = 22;
  int btnW = 100;
  int btnH = 20;
  tft->drawRoundRect(btnX, btnY, btnW, btnH, 5, TFT_WHITE);
  tft->setTextDatum(MC_DATUM);
  tft->setTextSize(1);
  tft->drawString("New Track", btnX + btnW / 2, btnY + btnH / 2 + 2);

  // List
  int startY = 60;
  int itemH = 30;

  tft->setTextDatum(TL_DATUM);
  tft->setTextSize(1);

  if (_tracks.empty()) {
    tft->drawString("No tracks found.", 20, 80);
    _ui->drawStatusBar();
    return;
  }

  for (size_t i = 0; i < _tracks.size(); i++) {
    int y = startY + (i * itemH);
    tft->setTextColor(COLOR_TEXT, COLOR_BG);
    tft->drawString(_tracks[i].name, 20, y);
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

  // Header
  tft->drawFastHLine(0, 20, SCREEN_WIDTH, COLOR_SECONDARY);

  // Back Arrow
  tft->setTextDatum(TL_DATUM);
  tft->setFreeFont(&Org_01);
  tft->setTextSize(2);
  tft->setTextColor(COLOR_TEXT, COLOR_BG);
  tft->drawString("<", 10, 25);

  // Menu Options (Left Side List)
  const char *options[] = {"Select", "Select & Edit", "Invert",
                           "Reinit best Lap", "Remove"};
  int menuY = 60; // Moved up slightly

  tft->setTextSize(1);
  for (int i = 0; i < 5; i++) {
    if (i == 0) { // Draw cursor box for 'Select'
      tft->fillRect(5, menuY + (i * 25) - 2, 110, 20, TFT_WHITE);
      tft->setTextColor(TFT_BLACK, TFT_WHITE);
    } else {
      tft->setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    }
    tft->drawString(options[i], 10, menuY + (i * 25));
  }

  // --- RIGHT PANEL INFO ---
  int panelX = 130;
  int panelY = 50;

  // 1. Map (Left side of Info Panel)
  // Draw a scaled Mock Map relative to panelX
  int mapX = panelX;
  int mapY = panelY + 10;

  tft->drawLine(mapX, mapY, mapX + 40, mapY + 10, TFT_WHITE);
  tft->drawLine(mapX + 40, mapY + 10, mapX + 30, mapY + 50, TFT_WHITE);
  tft->drawLine(mapX + 30, mapY + 50, mapX - 10, mapY + 40, TFT_WHITE);
  tft->drawLine(mapX - 10, mapY + 40, mapX, mapY, TFT_WHITE);
  // Add a "Start/Finish" dot
  tft->fillCircle(mapX + 15, mapY + 5, 3, TFT_RED);

  // 2. Info Text (Right side of Info Panel)
  int infoX = panelX + 55;
  int infoY = panelY;

  // Track Name Box
  tft->setTextSize(1);
  int nameW = tft->textWidth(t.name) + 40; // Extra padding
  tft->fillRect(infoX, infoY, nameW < 120 ? 120 : nameW, 18,
                TFT_WHITE);                // White Box
  tft->setTextColor(TFT_BLACK, TFT_WHITE); // Black Text
  tft->drawString(t.name, infoX + 5, infoY + 3);

  // Location / Subtitle
  tft->setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft->drawString("Location: Earth", infoX, infoY + 25);

  // Length
  tft->setTextColor(TFT_WHITE, TFT_BLACK);
  tft->setTextSize(2);
  tft->drawString("555.1m", infoX, infoY + 40);

  // Best Lap Label
  tft->setTextSize(1);
  tft->setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft->drawString("Best LAP:", infoX, infoY + 65);

  // Best Lap Time
  tft->setTextSize(2);
  tft->setTextColor(TFT_WHITE, TFT_BLACK);
  tft->drawString("01:10.501", infoX, infoY + 80);

  _ui->drawStatusBar();
}

void LapTimerScreen::drawRecordTrack() {
  TFT_eSPI *tft = _ui->getTft();
  extern GPSManager gpsManager;

  // Background Black
  tft->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                SCREEN_HEIGHT - STATUS_BAR_HEIGHT, TFT_BLACK);
  tft->drawFastHLine(0, 20, SCREEN_WIDTH, COLOR_SECONDARY); // Separator

  tft->setTextColor(TFT_WHITE, TFT_BLACK);
  tft->setFreeFont(&Org_01);
  tft->setTextSize(1);

  // Title
  tft->setTextDatum(TC_DATUM);
  tft->drawString("GPS TRACK RECORDING", SCREEN_WIDTH / 2, 30);

  // GPS Status Check
  if (!gpsManager.isFixed() || gpsManager.getSatellites() < 6) {
    tft->setTextColor(TFT_RED, TFT_BLACK);
    tft->setTextDatum(MC_DATUM);
    tft->drawString("GPS SIGNAL WEAK", SCREEN_WIDTH / 2, 100);
    tft->drawString("Need 6+ Satellites", SCREEN_WIDTH / 2, 120);
    tft->setTextColor(TFT_WHITE, TFT_BLACK);
    tft->setTextDatum(BL_DATUM);
    tft->drawString("< Back", 10, SCREEN_HEIGHT - 10);
    _ui->drawStatusBar();
    return;
  }

  // Recording State Display
  tft->setTextDatum(TL_DATUM);

  if (_recordingState == RECORD_IDLE) {
    // Instructions
    tft->drawString("Ready to Record", 10, 60);
    tft->drawString("Tap START to begin", 10, 80);
    tft->drawString("at finish line", 10, 100);

    // GPS Info
    tft->setTextColor(TFT_GREEN, TFT_BLACK);
    tft->drawString("GPS: " + String(gpsManager.getSatellites()) + " Sats", 10,
                    130);
    tft->drawString("HDOP: " + String(gpsManager.getHDOP(), 1), 10, 150);

    // START Button (Bottom Center)
    tft->setTextColor(TFT_BLACK, TFT_GREEN);
    tft->fillRect(SCREEN_WIDTH / 2 - 50, SCREEN_HEIGHT - 60, 100, 40,
                  TFT_GREEN);
    tft->setTextDatum(MC_DATUM);
    tft->drawString("START", SCREEN_WIDTH / 2, SCREEN_HEIGHT - 40);

  } else if (_recordingState == RECORD_ACTIVE) {
    // Recording in progress
    tft->setTextColor(TFT_RED, TFT_BLACK);
    tft->drawString("RECORDING...", 10, 60);

    // Stats
    tft->setTextColor(TFT_WHITE, TFT_BLACK);
    tft->drawString("Points: " + String(_recordedPoints.size()), 10, 90);

    unsigned long elapsed = (millis() - _recordingStartTime) / 1000;
    tft->drawString("Time: " + String(elapsed) + "s", 10, 110);

    // Distance from start
    double currentLat = gpsManager.getLatitude();
    double currentLon = gpsManager.getLongitude();
    double distToStart = gpsManager.distanceBetween(
        _recordStartLat, _recordStartLon, currentLat, currentLon);

    tft->drawString("Dist: " + String(distToStart, 0) + "m", 10, 130);

    // Finish detection hint
    if (distToStart < 50 && _recordedPoints.size() > 10) {
      tft->setTextColor(TFT_YELLOW, TFT_BLACK);
      tft->drawString("Near Start!", 10, 160);
      if (distToStart < 20) {
        tft->setTextColor(TFT_GREEN, TFT_BLACK);
        tft->drawString("FINISH DETECTED!", 10, 180);
      }
    }

    // STOP Button
    tft->setTextColor(TFT_BLACK, TFT_RED);
    tft->fillRect(SCREEN_WIDTH / 2 - 50, SCREEN_HEIGHT - 60, 100, 40, TFT_RED);
    tft->setTextDatum(MC_DATUM);
    tft->drawString("STOP", SCREEN_WIDTH / 2, SCREEN_HEIGHT - 40);

  } else if (_recordingState == RECORD_COMPLETE) {
    // Recording complete
    tft->setTextColor(TFT_GREEN, TFT_BLACK);
    tft->drawString("RECORDING COMPLETE!", 10, 60);

    tft->setTextColor(TFT_WHITE, TFT_BLACK);
    tft->drawString("Points: " + String(_recordedPoints.size()), 10, 90);

    unsigned long elapsed =
        (_recordedPoints.back().timestamp - _recordingStartTime) / 1000;
    tft->drawString("Duration: " + String(elapsed) + "s", 10, 110);

    // SAVE Button
    tft->setTextColor(TFT_BLACK, TFT_GREEN);
    tft->fillRect(SCREEN_WIDTH / 2 - 50, SCREEN_HEIGHT - 100, 100, 40,
                  TFT_GREEN);
    tft->setTextDatum(MC_DATUM);
    tft->drawString("SAVE", SCREEN_WIDTH / 2, SCREEN_HEIGHT - 80);

    // DISCARD Button
    tft->setTextColor(TFT_BLACK, TFT_RED);
    tft->fillRect(SCREEN_WIDTH / 2 - 50, SCREEN_HEIGHT - 50, 100, 40, TFT_RED);
    tft->setTextDatum(MC_DATUM);
    tft->drawString("DISCARD", SCREEN_WIDTH / 2, SCREEN_HEIGHT - 30);
  }

  // Back button
  tft->setTextColor(TFT_WHITE, TFT_BLACK);
  tft->setTextDatum(BL_DATUM);
  tft->drawString("< Back", 10, SCREEN_HEIGHT - 10);

  _ui->drawStatusBar();
}

void LapTimerScreen::drawNoGPS() {
  TFT_eSPI *tft = _ui->getTft();
  tft->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                SCREEN_HEIGHT - STATUS_BAR_HEIGHT, TFT_BLACK);
  tft->drawFastHLine(0, 20, SCREEN_WIDTH, COLOR_SECONDARY);

  tft->setTextColor(TFT_RED, TFT_BLACK);
  tft->setFreeFont(&Org_01);
  tft->setTextDatum(MC_DATUM);
  tft->setTextSize(2);
  tft->drawString("No tracks available", SCREEN_WIDTH / 2,
                  SCREEN_HEIGHT / 2 - 40);

  tft->setTextColor(TFT_WHITE, TFT_BLACK);
  tft->setTextSize(1);
  tft->drawString("- check GPS -", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
  tft->drawString("cannot enter the menu", SCREEN_WIDTH / 2,
                  SCREEN_HEIGHT / 2 + 20);

  // Buttons
  int btnY = SCREEN_HEIGHT - 60;

  // Retry (Left)
  tft->fillRoundRect(20, btnY, 130, 40, 5, TFT_DARKGREY);
  tft->setTextColor(TFT_WHITE, TFT_DARKGREY);
  tft->drawString("Retry", 85, btnY + 22);

  // Continue (Right)
  tft->fillRoundRect(170, btnY, 130, 40, 5, TFT_DARKGREY);
  tft->setTextColor(TFT_WHITE, TFT_DARKGREY);
  tft->drawString("Continue", 235, btnY + 22);

  _ui->drawStatusBar();
}

void LapTimerScreen::drawSummary() {
  TFT_eSPI *tft = _ui->getTft();
  tft->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                SCREEN_HEIGHT - STATUS_BAR_HEIGHT, TFT_BLACK);
  tft->drawFastHLine(0, 20, SCREEN_WIDTH, COLOR_SECONDARY);

  // 1. Back Arrow (Kiri Atas)
  tft->setTextColor(TFT_WHITE, TFT_BLACK);
  tft->setTextDatum(TL_DATUM);
  tft->setFreeFont(&Org_01); // Standard Font
  tft->setTextSize(2);       // Standard Standard Size
  tft->drawString("<", 10, 25);

  // 2. Best Lap (Kiri Atas - Green)
  int bestIdx = -1;
  unsigned long bestTime = 0;

  // Cari Best Lap Index
  if (!_lapTimes.empty()) {
    bestTime = _lapTimes[0];
    bestIdx = 0;
    for (int i = 1; i < _lapTimes.size(); i++) {
      if (_lapTimes[i] < bestTime) {
        bestTime = _lapTimes[i];
        bestIdx = i;
      }
    }
  }

  int leftX = 20;
  int topY = 60;

  tft->setTextDatum(TL_DATUM);
  tft->setTextColor(TFT_GREEN, TFT_BLACK); // Green Text
  tft->setTextFont(2);
  tft->setTextSize(1);
  String label = "Best lap #" + String(bestIdx + 1); // Lap Number (1-based)
  tft->drawString(label, leftX, topY);

  // Time Value (Large Green)
  char buf[32];
  if (bestIdx != -1) {
    int ms = bestTime % 1000;
    int s = (bestTime / 1000) % 60;
    int m = (bestTime / 60000);
    sprintf(buf, "%d:%02d.%02d", m, s, ms / 10);
  } else {
    sprintf(buf, "--:--.--");
  }

  tft->setTextFont(6); // Large Font
  tft->setTextSize(1);
  tft->drawString(buf, leftX, topY + 25);

  // 3. Theoretical Best (Kiri Bawah - White)
  int theoryY = 160;
  tft->setTextColor(TFT_WHITE, TFT_BLACK);
  tft->setTextFont(2);
  tft->drawString("Theoretical best", leftX, theoryY);

  // Mockup Theoretical: Best - 200ms
  unsigned long theoryTime = (bestTime > 200) ? (bestTime - 200) : bestTime;
  if (bestIdx != -1) {
    int ms = theoryTime % 1000;
    int s = (theoryTime / 1000) % 60;
    int m = (theoryTime / 60000);
    sprintf(buf, "%d:%02d.%02d", m, s, ms / 10);
  } else {
    sprintf(buf, "--:--.--");
  }

  tft->setTextFont(6);
  tft->drawString(buf, leftX, theoryY + 25);

  // 4. List (Kanan - Next Best Laps)
  // Sort indices by time (skipping actual best)
  std::vector<std::pair<unsigned long, int>> sortedLaps;
  for (int i = 0; i < _lapTimes.size(); i++) {
    if (i == bestIdx)
      continue;                                  // Skip the absolute best
    sortedLaps.push_back({_lapTimes[i], i + 1}); // Store time and Lap#
  }
  std::sort(sortedLaps.begin(), sortedLaps.end()); // Sort by time

  int listX = SCREEN_WIDTH / 2 + 40;
  int listY = 40;
  int itemsToShow = 3;

  tft->setTextFont(4);         // Medium Font
  tft->setTextDatum(TR_DATUM); // Align Right logic? Or TL of list item?
  // Let's draw TL based.

  for (int i = 0; i < itemsToShow && i < sortedLaps.size(); i++) {
    unsigned long t = sortedLaps[i].first;
    int lapNum = sortedLaps[i].second;

    int ms = t % 1000;
    int s = (t / 1000) % 60;
    int m = (t / 60000);
    sprintf(buf, "%02d. %d:%02d.%02d", lapNum, m, s, ms / 10);

    tft->setTextColor(TFT_WHITE, TFT_BLACK);
    tft->setTextDatum(TR_DATUM); // Rata Kanan Screen
    tft->drawString(buf, SCREEN_WIDTH - 10, listY + (i * 40));
  }

  _ui->drawStatusBar();
}

void LapTimerScreen::drawRacingStatic() {
  TFT_eSPI *tft = _ui->getTft();
  // Clear screen below status bar
  tft->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                SCREEN_HEIGHT - STATUS_BAR_HEIGHT, TFT_BLACK);

  // Restore Status Bar Separator (Line 20)
  tft->drawFastHLine(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH, TFT_DARKGREY);

  // --- TOP ROW (Info) y=25 to 70 ---
  int topRowY = 25; // Moved down from 20 to avoid status bar overlap
  int topRowH = 45;
  int lineY = topRowY + topRowH; // y=70

  // Horizontal Separator below Top Row
  tft->drawFastHLine(0, lineY, SCREEN_WIDTH, TFT_DARKGREY);

  // Vertical Separators
  // 3 Columns: 320 / 3 ~= 106
  tft->drawFastVLine(106, topRowY, topRowH, TFT_DARKGREY);
  tft->drawFastVLine(212, topRowY, topRowH, TFT_DARKGREY);

  // Static Small Labels (Superscript style) -> Now Subscript style (Below
  // values)
  tft->setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft->setFreeFont(&Org_01);
  tft->setTextSize(1);
  tft->setTextDatum(TC_DATUM); // Center Alignment

  // Column 1: LAP (Center of 0-106 = 53)
  tft->drawString("LAP", 53, 56);

  // Column 2: KPH (Center of 106-212 = 159)
  tft->drawString("KPH", 159, 56);

  // Column 3: MODE (Center of 212-320 = 266)
  tft->drawString("MODE", 266, 56);

  // --- BOTTOM ROW (Delta) y=190 to 240 ---
  int bottomY = 180;
  // Horizontal Separator above Bottom Row
  tft->drawFastHLine(0, bottomY, SCREEN_WIDTH, TFT_DARKGREY);

  // Label "Pred. Gap" (Stacked)
  tft->setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft->setTextDatum(TL_DATUM);
  tft->drawString("Pred.", 10, bottomY + 10);
  tft->drawString("Gap", 10, bottomY + 25);

  // Bar Graph Base Line / Ticks
  int barX = 130;
  int barW = 180; // 310 - 130
  int barCenter = barX + (barW / 2);
  int barY = bottomY + 15;
  int barH = 20;

  // Center Tick
  tft->drawFastVLine(barCenter, barY - 2, barH + 4, TFT_LIGHTGREY);

  // Range Ticks (e.g. +/- 2.0s or 5.0s)
  // Left Tick
  tft->drawFastVLine(barX, barY + 5, barH - 10, TFT_DARKGREY);
  // Right Tick
  tft->drawFastVLine(barX + barW, barY + 5, barH - 10, TFT_DARKGREY);

  // Small labels for ticks
  tft->setTextDatum(TC_DATUM);
  tft->drawString("-5.0", barX, barY + 22);
  tft->drawString("+5.0", barX + barW, barY + 22);

  _ui->drawStatusBar();
}

void LapTimerScreen::drawRacing() {
  TFT_eSPI *tft = _ui->getTft();

  // --- TOP ROW UPDATES ---
  // Font 6 is a good large condensed number font, or 7 for very large segments
  tft->setTextDatum(TC_DATUM); // Align Center

  // 1. Lap Count (Col 1 Center 53)
  tft->setTextColor(TFT_WHITE, TFT_BLACK);
  tft->setTextFont(4); // Medium Large
  tft->setTextSize(1);
  tft->setTextPadding(80); // Clear width
  tft->drawNumber(_lapCount, 53, 25);
  tft->setTextPadding(0);

  // 2. Speed (Col 2 Center 159)
  tft->setTextDatum(TC_DATUM);
  tft->setTextPadding(80);
  tft->drawFloat(gpsManager.getSpeedKmph(), 1, 159, 25);
  tft->setTextPadding(0);

  // 3. Mode (Col 3 Center 266)
  String modeStr = "UNK";
  if (_raceMode == MODE_BEST)
    modeStr = "BEST";
  else if (_raceMode == MODE_LAST)
    modeStr = "LAST";
  else if (_raceMode == MODE_PREDICTIVE)
    modeStr = "PRED.";

  tft->setTextDatum(TC_DATUM);
  tft->setTextPadding(80);
  tft->drawString(modeStr, 266, 25);
  tft->setTextPadding(0);

  // --- MAIN TIMER (Center) ---
  unsigned long currentLap = 0;
  if (_isRecording)
    currentLap = millis() - _currentLapStart;

  int ms = (currentLap % 1000) / 10; // 0-99
  int s = (currentLap / 1000) % 60;
  int m = (currentLap / 60000);

  char timeBuf[16];
  sprintf(timeBuf, "%02d:%02d.%02d", m, s, ms);

  // To center perfectly with large font
  tft->setTextDatum(MC_DATUM);
  tft->setTextColor(TFT_DARKGREY,
                    TFT_BLACK); // Ghost digits? No, just straightforward
  tft->setTextColor(TFT_WHITE, TFT_BLACK);

  // Font 7 is huge 7-seg. Font 6 is large numeric.
  // 6 Might be too small for "Giant Center". 7 is 7-seg style.
  tft->setTextFont(7);
  tft->setTextSize(2); // Double size for GIANT effect?
  // Check fit: 320px width. "00:00.00" is 8 chars.
  // Font 7 width is approx 32px? 8*32 = 256. Size 2 -> 512 (Too big).
  // Size 1 should be fine.
  tft->setTextSize(1);

  tft->setTextPadding(320); // Full width clear
  // Center Y = 70 + (120/2) = 130
  tft->drawString(timeBuf, SCREEN_WIDTH / 2, 130);
  tft->setTextPadding(0);

  // --- BOTTOM ROW (Delta Bar) ---
  int bottomY = 180;

  // Delta Value
  float delta = 0.0;
  // Calculate fake delta for now or use predictive logic
  // For now just mockup or realistic zero
  if (_raceMode == MODE_PREDICTIVE) {
    // TODO: Implement actual predictive math
    delta = -3.62; // Mock from image
  }

  char deltaBuf[16];
  if (delta >= 0)
    sprintf(deltaBuf, "+%02.2f", delta);
  else
    sprintf(deltaBuf, "%03.2f", delta); // includes -

  tft->setTextFont(4);
  tft->setTextSize(1);
  tft->setTextDatum(TL_DATUM);
  // Bar Graph
  int barX = 130;
  int barW = 180;
  int barCenter = barX + (barW / 2);
  int barY = bottomY + 15;
  int barH = 20;

  // Clear Bar Area first (important for variable length)
  tft->fillRect(barX, barY + 1, barW, barH - 2,
                TFT_BLACK); // +1/-2 to keep border area clean?
  // No, we didn't draw a box, just ticks. Clear fully.
  // But strictly, we want to clear defined area.

  // Normalize Delta to Bar Range (+/- 5.0s)
  // Max range 5.0 -> Width 90px (Half bar)
  // Px per sec = 90 / 5 = 18 px/s
  float pxPerSec = 90.0 / 5.0;
  int barLen = (int)(delta * pxPerSec);

  // Clamp
  if (barLen > 90)
    barLen = 90;
  if (barLen < -90)
    barLen = -90;

  // Draw Center Marker (Static) is drawn in drawRacingStatic, but we need
  // to ensure bar doesn't overwrite it improperly, or we redraw it.
  // Actually, standard bar graph logic: clear -> draw value.
  // We cleared box above.

  if (barLen < 0) {
    // Negative (Green, Left)
    int w = abs(barLen);
    tft->fillRect(barCenter - w, barY + 2, w, barH - 4, TFT_GREEN);
  } else if (barLen > 0) {
    // Positive (Red, Right)
    tft->fillRect(barCenter, barY + 2, barLen, barH - 4, TFT_RED);
  }

  // Re-draw center tick over the bar if needed?
  // Usually ticks are behind, but center marker is useful.
  // Let's leave it.
}

void LapTimerScreen::checkFinishLine() {
  double dist = gpsManager.distanceBetween(gpsManager.getLatitude(),
                                           gpsManager.getLongitude(),
                                           _finishLat, _finishLon);

  // Logika Deteksi Mulai/Lap
  static bool inside = false;
  static unsigned long lastCross = 0;

  if (dist < 20) {                                   // Radius 20m
    if (!inside && (millis() - lastCross > 10000)) { // Debounce 10s
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
      }
      _currentLapStart = millis();
      lastCross = millis();
      inside = true;
    }
  } else {
    if (dist > 25)
      inside = false;
  }
}
