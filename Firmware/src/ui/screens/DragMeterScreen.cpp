#include "DragMeterScreen.h"
#include "../../core/GPSManager.h"
#include "../../core/SessionManager.h"
#include "../fonts/Org_01.h"
#include <Preferences.h>

extern GPSManager gpsManager;
extern SessionManager sessionManager;

// Tentukan subset disiplin untuk dilacak
// 0-60 km/h
// 0-100 km/h
// 100-200 km/h
// 402m (1/4 Mile)

// 0-100 km/h
// 100-200 km/h
// 402m (1/4 Mile)

void DragMeterScreen::onShow() {
  _state = STATE_MENU;
  _selectedBtn = -1;
  _selectedMenuIdx = -1;
  _selectedDragModeIdx = -1;
  _selectedPredictiveIdx = -1;
  _lastTapIdx = -1;
  _lastTapTime = 0;
  _menuItems = {"DRAG MODE", "DRAG SCREEN", "PREDICTIVE", "SUMMARY"};
  _dragModeItems = {"SPEED", "DISTANCE", "CUSTOM"};
  _predictiveItems = {"NORMAL MODE", "PREDICTIVE MODE"};

  _predictiveItems = {"NORMAL MODE", "PREDICTIVE MODE"};

  // Initialize Default Disciplines (Distance by default or empty?)
  // Let's default to Distance mode
  loadDisciplines(1); // 1 = Distance

  _currentSpeed = 0.0;
  _slope = 0.0;
  _highlightTitle = "400 m";
  _highlightValue = "--.--";

  TFT_eSPI *tft = _ui->getTft();
  // Clear only content area
  _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                          SCREEN_HEIGHT - STATUS_BAR_HEIGHT, COLOR_BG);
  _ui->setTitle("DRAG METER");
  drawDashboardStatic();

  // Reset Run State
  _runState = RUN_WAITING;
  _oneFootReached = false;
  _startPosition = 0;

  // Load Settings
  Preferences p;
  p.begin("laptimer", true);
  _rolloutEnabled = p.getBool("rollout", false); // Default false
  int treeIdx = p.getInt("tree_time", 0);
  int targetIdx = p.getInt("drag_target", 2); // Default index 2 (10.0s)
  // Options: 8.0, 9.0, 10.0, 10.5, 11.0, 11.5, 12.0, 13.0, 14.0, 15.0
  float targets[] = {8.0, 9.0, 10.0, 10.5, 11.0, 11.5, 12.0, 13.0, 14.0, 15.0};
  if (targetIdx >= 0 && targetIdx < 10)
    _targetTime = targets[targetIdx];
  else
    _targetTime = 10.0;

  _referenceTime = p.getFloat("drag_ref", 0.0);
  p.end();

  if (treeIdx <= 5)
    _treeInterval = (treeIdx + 1) * 500;
  else if (treeIdx == 6)
    _treeInterval = 4000;
  else
    _treeInterval = 5000;

  _displayMode = DISPLAY_NORMAL;
  _predictedFinalTime = 0;

  _summaryShowBest = false;
  _sessionBest.clear();
}

void DragMeterScreen::update() {
  static unsigned long lastDragTouch = 0;
  UIManager::TouchPoint p = _ui->getTouchPoint();
  if (p.x != -1) {
    if (millis() - lastDragTouch > 200) {
      lastDragTouch = millis();

      // 1. Tombol Kembali (Top Left)
      if (p.x < 60 && p.y < 60) {
        static unsigned long lastBackTap = 0;
        if (millis() - lastBackTap < 500) {
          lastBackTap = 0;
          if (_state == STATE_MENU) {
            _ui->switchScreen(SCREEN_MENU);
          } else if (_state == STATE_DRAG_MODE_MENU) {
            _state = STATE_MENU;
            _ui->setTitle("DRAG METER");
            _selectedDragModeIdx = -1;           // Reset selection
            _ui->getTft()->fillScreen(COLOR_BG); // Clear entire screen
            drawMenu();
          } else if (_state == STATE_PREDICTIVE_MENU) {
            _state = STATE_MENU;
            _ui->setTitle("DRAG METER");
            _selectedPredictiveIdx = -1;
            _ui->getTft()->fillScreen(COLOR_BG);
            drawMenu();
          } else if (_state == STATE_SUMMARY_VIEW) {
            _state = STATE_MENU;
            _ui->setTitle("DRAG METER");
            _ui->getTft()->fillScreen(COLOR_BG);
            drawMenu();
          } else {
            // If in running mode, go back to menu
            _state = STATE_MENU;
            _ui->setTitle("DRAG METER");
            _ui->getTft()->fillScreen(COLOR_BG);
            drawDashboardStatic();
          }
        } else {
          lastBackTap = millis();
        }
        return;
      }

      // 2. Menu Logic
      if (_state == STATE_MENU) {
        int startY = 55;
        int btnHeight = 35;
        int btnWidth = 240;
        int gap = 8;
        int x = (SCREEN_WIDTH - btnWidth) / 2;

        // Check if X is within button width (centered)
        if (p.x > x && p.x < x + btnWidth) {
          int touchedIdx = -1;
          // Check Y coordinates
          for (int i = 0; i < _menuItems.size(); i++) {
            int btnY = startY + (i * (btnHeight + gap));
            if (p.y > btnY && p.y < btnY + btnHeight) {
              touchedIdx = i;
              break;
            }
          }

          if (touchedIdx != -1) {
            unsigned long now = millis();
            if (_lastTapIdx == touchedIdx && (now - _lastTapTime < 500)) {
              // Second tap: Execute
              _lastTapIdx = -1;
              handleMenuTouch(touchedIdx);
            } else {
              // First tap: Select/Highlight
              _lastTapIdx = touchedIdx;
              _lastTapTime = now;

              if (_selectedMenuIdx != touchedIdx) {
                _selectedMenuIdx = touchedIdx;
                drawMenu();
              }
            }
          }
        }
      } else if (_state == STATE_DRAG_MODE_MENU) {
        // Drag Mode Menu Logic (Similar to Main Menu)
        int startY = 55;
        int btnHeight = 35;
        int btnWidth = 240;
        int gap = 8;
        int x = (SCREEN_WIDTH - btnWidth) / 2;

        if (p.x > x && p.x < x + btnWidth) {
          int touchedIdx = -1;
          for (int i = 0; i < _dragModeItems.size(); i++) {
            int btnY = startY + (i * (btnHeight + gap));
            if (p.y > btnY && p.y < btnY + btnHeight) {
              touchedIdx = i;
              break;
            }
          }

          if (touchedIdx != -1) {
            unsigned long now = millis();
            if (_lastTapIdx == touchedIdx && (now - _lastTapTime < 500)) {
              // Confirmed
              _lastTapIdx = -1;
              handleDragModeTouch(touchedIdx);
            } else {
              // First tap
              _lastTapIdx = touchedIdx;
              _lastTapTime = now;

              if (_selectedDragModeIdx != touchedIdx) {
                _selectedDragModeIdx = touchedIdx;
                drawDragModeMenu();
              }
            }
          }
        }
      }
    }
  }

  // Handle Summary View Touch
  if (_state == STATE_SUMMARY_VIEW && p.x != -1) {
    if (millis() - lastDragTouch > 200) {
      lastDragTouch = millis();

      // Toggle Area (Header center/right)
      if (p.y < 50 && p.x > 100) {
        _summaryShowBest = !_summaryShowBest;
        _ui->getTft()->fillScreen(COLOR_BG);
        drawSummary();
      }
    }
  }

  if (_state == STATE_PREDICTIVE_MENU) {
    int touchedIdx = -1;
    int listY = 60;
    int itemH = 50;
    int gap = 10;

    for (int i = 0; i < _predictiveItems.size(); i++) {
      int y = listY + i * (itemH + gap);
      if (p.y >= y && p.y < y + itemH && p.x >= 10 && p.x < SCREEN_WIDTH - 10) {
        touchedIdx = i;
        break;
      }
    }

    if (touchedIdx != -1) {
      unsigned long now = millis();
      if (_lastTapIdx == touchedIdx && (now - _lastTapTime < 500)) {
        // Confirmed
        _lastTapIdx = -1;
        handlePredictiveTouch(touchedIdx);
      } else {
        // First tap
        _lastTapIdx = touchedIdx;
        _lastTapTime = now;

        if (_selectedPredictiveIdx != touchedIdx) {
          _selectedPredictiveIdx = touchedIdx;
          drawPredictiveMenu();
        }
      }
    }
  }

  if (_state == STATE_RUNNING) {
    if (_runState == RUN_WAITING) {
      checkStartCondition();
      // Check for Tree Button (Bottom Right)
      if (p.x != -1 && p.x > SCREEN_WIDTH - 60 && p.y > SCREEN_HEIGHT - 60) {
        if (millis() - lastDragTouch > 500) { // Debounce
          lastDragTouch = millis();
          startChristmasTree();
        }
      }
    } else if (_runState == RUN_COUNTDOWN) {
      // Christmas Tree Logic
      unsigned long elapsed = millis() - _startTime;
      if (elapsed >= _treeInterval) {
        // GO!
        _runState = RUN_RUNNING;
        _runStartTime = millis();            // Start timer
        _ui->getTft()->fillScreen(COLOR_BG); // Clear tree
        drawDashboardStatic();
      } else {
        drawChristmasTreeOverlay();
      }
    } else if (_runState == RUN_RUNNING) {
      checkStopCondition();
      updateDisciplines();
    }
    drawDashboardDynamic();
  }
}

void DragMeterScreen::checkStartCondition() {
  float speed = gpsManager.getSpeedKmph();
  if (speed > 1.0) { // Moving (> 1 km/h)
    if (_rolloutEnabled) {
      // Integrate distance: dist += speed (m/s) * dt
      // Simple approx: speed in m/s * (millis - lastUpdate) / 1000
      float speedMs = speed / 3.6;
      unsigned long now = millis();
      float dt = (now - _lastUpdate) / 1000.0;
      if (dt > 1.0)
        dt = 0.1; // Cap dt if weird jump
      _startPosition += speedMs * dt;
      _lastUpdate = now;

      if (_startPosition >= 0.3048) { // 1 ft
        _runState = RUN_RUNNING;
        _runStartTime = now;
        // Reset disciplines
        for (auto &d : _disciplines) {
          d.completed = false;
          d.resultTime = 0;
        }
      }

    } else {
      // Immediate Start
      _runState = RUN_RUNNING;
      _runStartTime = millis();
      // Reset disciplines
      for (auto &d : _disciplines) {
        d.completed = false;
        d.resultTime = 0;
      }
    }

    if (_runState == RUN_RUNNING) {
      sessionManager.startSession();
    }
  } else {
    _lastUpdate = millis(); // Keep updating time while stationary
    _startPosition = 0;     // Reset rollout if stopped
  }
}

// ... functions ...

void DragMeterScreen::drawPredictiveMode() {
  TFT_eSPI *tft = _ui->getTft();

  // Draw Big Predicted Time
  tft->setTextColor(TFT_WHITE, COLOR_BG);
  tft->setTextDatum(MC_DATUM);
  tft->setFreeFont(&Org_01);
  tft->setTextSize(6); // Very big

  calculatePrediction();

  String timeStr;
  if (_predictedFinalTime > 0) {
    timeStr = String(_predictedFinalTime, 2);
  } else {
    timeStr = "--.--";
  }
  tft->drawString(timeStr, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);

  // Delta to Target
  tft->setTextSize(2);
  if (_targetTime > 0 && _predictedFinalTime > 0) {
    float delta = _predictedFinalTime - _targetTime;
    uint16_t color = TFT_WHITE;
    if (abs(delta) <= 0.1)
      color = TFT_GREEN;
    else
      color = TFT_RED;

    tft->setTextColor(color, COLOR_BG);
    String deltaStr = "Target: " + String(_targetTime, 1) + "s";
    tft->drawString(deltaStr, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 50);
  }
}

void DragMeterScreen::calculatePrediction() {
  // Basic prediction: Use reference time
  _predictedFinalTime = _referenceTime;
}

void DragMeterScreen::saveReferenceRun() {
  if (_disciplines.empty())
    return;

  // Calculate final time of the longest discipline?
  // We iterate to find the longest target discipline.
  // Simplifying: The last one is usually the longest or the "Result".
  Discipline *best = nullptr;
  for (auto &d : _disciplines) {
    if (d.completed)
      best = &d;
  }

  if (best && best->completed) {
    float runTime = best->resultTime / 1000.0;

    // Save if faster (runTime < _referenceTime) or if no reference exists
    // Note: Faster means LOWER time.
    if (_referenceTime <= 0.0 || runTime < _referenceTime) {
      _referenceTime = runTime;

      Preferences p;
      p.begin("laptimer", false);
      p.putFloat("drag_ref", _referenceTime);
      p.end();
    }
  }
}

void DragMeterScreen::checkStopCondition() {
  // If speed drops to 0 and we have been running for a bit?
  // Or simply if speed < 1.0
  if (gpsManager.getSpeedKmph() < 1.0) {
    // Only stop if we actually started (which we did if we are here)

    saveReferenceRun(); // Save if good run

    // Save to History
    // Use the first discipline as the primary result
    unsigned long resultTime = 0;
    String runName = "Drag Run";
    if (!_disciplines.empty()) {
      runName = _disciplines[0].name;
      if (_disciplines[0].completed) {
        resultTime = _disciplines[0].resultTime;
      }
    }

    sessionManager.stopSession();
    // Use the actual filename if we were logging
    String actualFilename = sessionManager.getCurrentFilename();
    if (actualFilename.length() == 0)
      actualFilename = "DragRun"; // Fallback

    String dateStr =
        gpsManager.getDateString() + " " + gpsManager.getTimeString();
    // Use '1' for run count, and resultTime for bestLap (repurposed field)
    sessionManager.appendToHistoryIndex(actualFilename, dateStr, 1, resultTime,
                                        "DRAG");

    _runState = RUN_FINISHED;

    // Go to Summary
    _state = STATE_SUMMARY_VIEW;
    _ui->getTft()->fillScreen(COLOR_BG);
    drawSummary();
  }
}

void DragMeterScreen::updateDisciplines() {
  float speed = gpsManager.getSpeedKmph();
  unsigned long now = millis();
  unsigned long runTime = now - _runStartTime;
  // We definitely need distance tracking here for distance disciplines
  // _currentDistance += speed * dt...
  // Let's rely on simple integration for now as we don't have DistanceManager
  static unsigned long lastUpdate = 0;
  if (lastUpdate == 0)
    lastUpdate = now;
  float dt = (now - lastUpdate) / 1000.0;
  if (dt > 0.5)
    dt = 0.1; // Cap
  lastUpdate = now;

  if (_runState == RUN_RUNNING && runTime < 100)
    _totalRunDistance = 0; // Reset at start

  _totalRunDistance += (speed / 3.6) * dt;
  _currentSpeed = speed; // Update display var
  _slope = 0;

  // LOG DATA
  if (_runState == RUN_RUNNING && sessionManager.isLogging()) {
    // Time,Lat,Lon,Speed,Sats
    String data = String(millis()) + "," + String(gpsManager.getLatitude(), 6) +
                  "," + String(gpsManager.getLongitude(), 6) + "," +
                  String(speed, 2) + "," + String(gpsManager.getSatellites());
    sessionManager.logData(data);
  }

  // Check disciplines
  bool allComplete = true;
  for (auto &d : _disciplines) {
    if (!d.completed) {
      allComplete = false;
      // Capture slope if available (mocking logic or real if GPSManager has it)
      // d.slope = gpsManager.getSlope(); // Assuming fake for now
      d.slope = (random(-20, 20) / 10.0); // Dummy: -2.0 to 2.0%

      // Peak Speed
      if (speed > d.peakSpeed)
        d.peakSpeed = speed;

      if (d.isDistance) {
        if (_totalRunDistance >= d.target) {
          d.completed = true;
          d.resultTime = runTime;
          d.endSpeed = speed;
          // Validate slope: < -1.5% invalid
          if (d.slope < -1.5)
            d.valid = false;
          else
            d.valid = true;
        }
      } else {
        if (speed >= d.target) {
          d.completed = true;
          d.resultTime = runTime;
          // Validate slope
          if (d.slope < -1.5)
            d.valid = false;
          else
            d.valid = true;
        }
      }
    }
  }

  // Braking Logic
  if (allComplete) {
    if (!_brakingMeasurable) {
      _brakingMeasurable = true;
      _brakingStartDistance = _totalRunDistance;
    }
  } else {
    _brakingMeasurable = false;
  }

  // Update highlight
  if (!_disciplines.empty()) {
    _highlightTitle = _disciplines[0].name;
    if (_disciplines[0].completed) {
      _highlightValue = String(_disciplines[0].resultTime / 1000.0, 2) + "s";
    } else {
      _highlightValue = String(runTime / 1000.0, 1) + "s";
    }
  }
}

void DragMeterScreen::startChristmasTree() {
  _runState = RUN_COUNTDOWN;
  _startTime = millis();
  _ui->getTft()->fillScreen(COLOR_BG);
  // Draw Tree Background
}

void DragMeterScreen::drawChristmasTreeOverlay() {
  // Simple Lights Logic
  TFT_eSPI *tft = _ui->getTft();
  unsigned long elapsed = millis() - _startTime;
  int phase = elapsed / (_treeInterval / 4); // 4 phases: Yellow 1, 2, 3, Green

  int cx = SCREEN_WIDTH / 2;
  int cy = SCREEN_HEIGHT / 2;

  // Draw Traffic Light
  // Pre-Stage (White)
  tft->fillCircle(cx, cy - 60, 20, TFT_WHITE);

  // Stage (White)
  tft->fillCircle(cx, cy - 20, 20, TFT_WHITE);

  // Yellows
  if (phase >= 1)
    tft->fillCircle(cx, cy + 20, 20, TFT_YELLOW);
  if (phase >= 2)
    tft->fillCircle(cx, cy + 60, 20, TFT_YELLOW);
  if (phase >= 3)
    tft->fillCircle(cx, cy + 100, 20, TFT_YELLOW);

  // Green (Handled by state switch, but last frame)
}

void DragMeterScreen::handleMenuTouch(int idx) {
  if (idx < 0 || idx >= _menuItems.size())
    return;

  String &item = _menuItems[idx];
  if (item == "DRAG SCREEN") {
    _state = STATE_RUNNING;
    _ui->setTitle("DRAG METER");
    _ui->getTft()->fillScreen(COLOR_BG);
    drawDashboardStatic();
  } else if (item == "DRAG MODE") {
    _state = STATE_DRAG_MODE_MENU;
    _ui->setTitle("DRAG MODE");
    _selectedDragModeIdx = -1;
    _ui->getTft()->fillScreen(COLOR_BG);
    drawDragModeMenu();
  } else if (item == "PREDICTIVE") {
    _state = STATE_PREDICTIVE_MENU;
    _ui->setTitle("PREDICTIVE");
    _ui->getTft()->fillScreen(COLOR_BG);
    drawPredictiveMenu();
  } else if (item == "SUMMARY") {
    _state = STATE_SUMMARY_VIEW;
    _ui->setTitle("RUN SUMMARY");
    _ui->getTft()->fillScreen(COLOR_BG); // Clear for summary
    drawSummary();
  }
}

void DragMeterScreen::handleDragModeTouch(int idx) {
  if (idx >= 0) {
    _selectedDragModeIdx = idx;
    loadDisciplines(idx);

    // Go to Running View
    _state = STATE_RUNNING;
    _ui->setTitle("DRAG METER");
    _ui->getTft()->fillScreen(COLOR_BG);
    drawDashboardStatic();
  }
}

void DragMeterScreen::loadDisciplines(int modeIdx) {
  _disciplines.clear();

  if (modeIdx == 0) {
    // SPEED MODE
    // 0-60 kph, 0-100 kph, 100-200 kph, 0-200 kph
    // Fields: name, isDist, start, target, resTime, compl, endSpd, slope,
    // peakSpd, brakeDist, valid
    _disciplines.push_back({"0-60", false, 0, 60, 0, false, 0, 0, 0, 0, true});
    _disciplines.push_back(
        {"0-100", false, 0, 100, 0, false, 0, 0, 0, 0, true});
    _disciplines.push_back(
        {"100-200", false, 100, 200, 0, false, 0, 0, 0, 0, true});
    _disciplines.push_back(
        {"0-200", false, 0, 200, 0, false, 0, 0, 0, 0, true});

    _highlightTitle = "0-100";
    _highlightValue = "--.--";

  } else if (modeIdx == 1) {
    // DISTANCE MODE
    // 60 ft, 100 m, 200 m, 400 m
    _disciplines.push_back(
        {"60 ft", true, 0, 18.288, 0, false, 0, 0, 0, 0, true});
    _disciplines.push_back({"100 m", true, 0, 100, 0, false, 0, 0, 0, 0, true});
    _disciplines.push_back({"200 m", true, 0, 200, 0, false, 0, 0, 0, 0, true});
    _disciplines.push_back({"400 m", true, 0, 400, 0, false, 0, 0, 0, 0, true});

    _highlightTitle = "400 m";
    _highlightValue = "--.--";

  } else {
    // CUSTOM (Placeholder)
    _disciplines.push_back(
        {"Custom 1", true, 0, 100, 0, false, 0, 0, 0, 0, true});
    _highlightTitle = "Custom";
    _highlightValue = "--.--";
  }

  // Reset session best if structure changed?
  // Ideally we match by name. For simplicitly, clear session best on mode
  // change.
  _sessionBest.clear();
}

void DragMeterScreen::drawDashboardStatic() {
  TFT_eSPI *tft = _ui->getTft();

  // Common Header (Back Arrow)
  tft->setTextColor(COLOR_HIGHLIGHT, COLOR_BG);
  tft->setTextDatum(TL_DATUM);
  tft->setFreeFont(&Org_01);
  tft->setTextSize(2);
  tft->drawString("<", 10, 25);

  if (_state == STATE_MENU) {
    _ui->setTitle("DRAG METER");
    drawMenu();
  } else if (_state == STATE_DRAG_MODE_MENU) {
    _ui->setTitle("DRAG MODE");
    drawDragModeMenu();
  } else if (_state == STATE_PREDICTIVE_MENU) {
    _ui->setTitle("PREDICTIVE");
    drawPredictiveMenu();
  } else if (_state == STATE_SUMMARY_VIEW) {
    _ui->setTitle("RUN SUMMARY");
    drawSummary();
  } else {
    _ui->setTitle("DRAG METER");
    // Draw Running View (Back arrow already drawn)
    tft->drawFastHLine(0, 45, SCREEN_WIDTH, TFT_WHITE);

    // Speed Area
    // "KPH" Label
    tft->setTextColor(TFT_WHITE, COLOR_BG);
    tft->setTextDatum(TR_DATUM); // Top Right
    tft->setFreeFont(&Org_01);
    tft->setTextSize(2);
    tft->drawString("KPH", SCREEN_WIDTH - 10, 80);

    // Horizontal Line below Speed
    tft->drawFastHLine(0, 110, SCREEN_WIDTH, TFT_WHITE);

    // Draw Mode Toggle Arrows
    tft->setTextColor(TFT_WHITE, COLOR_BG);
    tft->setTextDatum(TC_DATUM);
    tft->setTextSize(1);
    if (_displayMode == DISPLAY_NORMAL) {
      tft->drawString("v", SCREEN_WIDTH / 2, 220);
    } else {
      tft->drawString("^", SCREEN_WIDTH / 2, 120);
    }

    // Vertical Line splitting List and Highlight
    tft->drawFastVLine(SCREEN_WIDTH / 2, 110, SCREEN_HEIGHT - 110 - 20,
                       TFT_WHITE); // -20 for footer

    // List Area (Left)
    // Labels updated in Dynamic or Static? Labels are static.
    tft->setTextDatum(TL_DATUM);
    tft->setTextSize(2);
    int listStartY = 120;
    int gap = 25;
    for (int i = 0; i < _disciplines.size(); i++) {
      tft->drawString(_disciplines[i].name, 10, listStartY + (i * gap));
    }

    // Highlight Box (Right) - Background White
    // We will draw this in Dynamic to avoid flickering or redraw it here and
    // update text in Dynamic? Better to draw static bg here.
    tft->fillRect(SCREEN_WIDTH / 2 + 1, 111, SCREEN_WIDTH / 2 - 1,
                  SCREEN_HEIGHT - 110 - 21, TFT_WHITE);

    // Footer Area (Slope)
    tft->fillRect(0, SCREEN_HEIGHT - 20, SCREEN_WIDTH, 20,
                  COLOR_BG); // Clear footer
    tft->setTextColor(TFT_WHITE, COLOR_BG);
    tft->setTextDatum(TL_DATUM);
    tft->setTextSize(1); // Smaller font for footer
    tft->drawString("SL:", 10, SCREEN_HEIGHT - 15);
  }
  _ui->drawStatusBar(true);
}

void DragMeterScreen::drawDashboardDynamic() {
  if (_state != STATE_RUNNING)
    return;

  TFT_eSPI *tft = _ui->getTft();

  if (_displayMode == DISPLAY_PREDICTIVE) {
    drawPredictiveMode();
    return;
  }

  // NORMAL MODE DRAWING
  // 1. Update Speed
  // Big Font
  tft->setTextColor(TFT_WHITE, COLOR_BG);
  tft->setTextDatum(TC_DATUM);
  tft->setTextFont(7); // Big 7-segment like font
  tft->setTextSize(1); // Standard size for font 7
  // We need to verify if Font 7 is available or usage. Org_01 is tiny.
  // Use FreeFont if needed or standard font numbers.
  // Let's try font 6 or 7 if available, otherwise large scaled FreeFont.
  // Assuming Font 6 (large numeric) is available in TFT_eSPI default.
  tft->drawString(String(_currentSpeed, 1), SCREEN_WIDTH / 2 - 20, 50);

  // 2. Update List Values (Left)
  tft->setTextColor(TFT_WHITE, COLOR_BG);
  tft->setTextDatum(TR_DATUM); // Align Right for values
  tft->setFreeFont(&Org_01);   // Back to Org_01
  tft->setTextSize(2);
  int listStartY = 120;
  int gap = 25;
  for (int i = 0; i < _disciplines.size(); i++) {
    String valText = "--.--";
    if (_disciplines[i].completed) {
      valText = String(_disciplines[i].resultTime / 1000.0, 2);
    }
    // Draw value at fixed X (half width - padding)
    tft->drawString(valText, (SCREEN_WIDTH / 2) - 10, listStartY + (i * gap));
  }

  // 3. Highlight Box (Right) - White BG, Black Text
  tft->setTextColor(TFT_BLACK, TFT_WHITE);

  // Title (e.g., "400 m")
  tft->setTextDatum(TC_DATUM);
  tft->setTextSize(2); // Smaller for title
  tft->drawString(_highlightTitle, (SCREEN_WIDTH * 3) / 4, 125);

  // Value (e.g., "11.37")
  tft->setTextFont(6); // Large numeric
  tft->setTextSize(1);
  tft->drawString(_highlightValue, (SCREEN_WIDTH * 3) / 4, 160);

  // 4. Slope (Footer)
  tft->setTextColor(TFT_WHITE, COLOR_BG);
  tft->setFreeFont(&Org_01); // Back to Org_01
  tft->setTextSize(1);
  tft->setTextDatum(TL_DATUM);
  tft->drawString(String(_slope, 1) + "%", 40, SCREEN_HEIGHT - 15);
}

void DragMeterScreen::drawSummary() {
  TFT_eSPI *tft = _ui->getTft();

  // Header
  tft->setTextColor(TFT_WHITE, COLOR_BG);
  tft->setTextDatum(TC_DATUM);
  tft->setFreeFont(&Org_01);
  tft->setTextSize(2);
  String header = _summaryShowBest ? "SESSION BEST" : "LAST RUN";
  tft->drawString(header, SCREEN_WIDTH / 2, 25);

  // Toggle Arrows
  tft->setTextSize(1);
  if (_summaryShowBest) {
    tft->drawString("v", SCREEN_WIDTH / 2 + 80, 25); // Down to go to Last
  } else {
    tft->drawString("^", SCREEN_WIDTH / 2 + 80, 25); // Up to go to Best
  }

  // Back Button
  tft->setTextColor(COLOR_HIGHLIGHT, COLOR_BG);
  tft->setTextDatum(TL_DATUM);
  tft->setTextSize(2); // Org_01 size 2
  tft->drawString("<", 10, 25);

  tft->drawFastHLine(0, 45, SCREEN_WIDTH, TFT_WHITE);

  // Peak Speed (Top Right)
  // Find peak speed of longest discipline or just max speed seen?
  // We captured peakSpeed in each discipline. Let's show the peak of the
  // last/longest one.
  float peak = 0;
  const std::vector<Discipline> *data =
      _summaryShowBest ? &_sessionBest : &_disciplines;

  if (!data->empty()) {
    peak = data->back().peakSpeed;
  }

  tft->setTextColor(TFT_WHITE, COLOR_BG);
  tft->setTextDatum(TR_DATUM);
  tft->setTextFont(4); // Medium numeric
  tft->setTextSize(1);
  tft->drawString(String(peak, 1), SCREEN_WIDTH - 40, 50);

  tft->setFreeFont(&Org_01);
  tft->setTextSize(1);
  tft->drawString("PK KPH", SCREEN_WIDTH - 5, 65);

  tft->drawFastHLine(0, 85, SCREEN_WIDTH, TFT_WHITE);

  // Table Headers
  int y = 90;
  tft->setTextColor(TFT_LIGHTGREY, COLOR_BG); // Grey headers
  tft->setTextDatum(TL_DATUM);
  tft->drawString("DISC.", 5, y);
  tft->setTextDatum(TC_DATUM);
  tft->drawString("TIME", 110, y);
  tft->setTextDatum(TR_DATUM);
  tft->drawString("@KPH", SCREEN_WIDTH - 45, y);
  tft->drawString("SL%", SCREEN_WIDTH - 5, y);

  tft->drawFastHLine(0, 105, SCREEN_WIDTH, TFT_LIGHTGREY);

  // Rows
  int startY = 110;
  int gap = 20; // compact

  if (data->empty()) {
    tft->setTextColor(TFT_WHITE, COLOR_BG);
    tft->setTextDatum(TC_DATUM);
    tft->drawString("NO DATA", SCREEN_WIDTH / 2, 150);
    return;
  }

  for (int i = 0; i < data->size(); i++) {
    const Discipline &d = (*data)[i];
    int rowY = startY + (i * gap);

    // Color: Red if invalid, Green/White otherwise
    // Use flag d.valid
    uint16_t color = d.valid ? TFT_GREEN : TFT_RED;
    if (!_summaryShowBest && !d.valid) {
      // In last run, if invalid show red.
    }
    // Actually, let's stick to Green for valid, Red for invalid
    tft->setTextColor(color, COLOR_BG);
    tft->setTextSize(1);

    // Name
    tft->setTextDatum(TL_DATUM);
    tft->drawString(d.name, 5, rowY);

    // Time
    tft->setTextDatum(TC_DATUM);
    if (d.completed) {
      tft->drawString(String(d.resultTime / 1000.0, 2), 110, rowY);
    } else {
      tft->drawString("-", 110, rowY);
    }

    // End Speed
    tft->setTextDatum(TR_DATUM);
    if (d.completed) {
      tft->drawString(String(d.endSpeed, 1), SCREEN_WIDTH - 45, rowY);
    } else {
      tft->drawString("-", SCREEN_WIDTH - 45, rowY);
    }

    // Slope
    tft->drawString(String(d.slope, 1), SCREEN_WIDTH - 5, rowY);

    // Invalid Icon? If red, maybe draw a small X?
    // Rely on color for now.
  }

  // Footer: Braking Distance
  tft->drawFastHLine(0, SCREEN_HEIGHT - 25, SCREEN_WIDTH, TFT_WHITE);
  tft->setTextColor(TFT_WHITE, COLOR_BG);
  tft->setTextDatum(TL_DATUM);
  tft->drawString("Brk Dist:", 10, SCREEN_HEIGHT - 20);

  float brakeDist = 0;
  if (!data->empty())
    brakeDist = data->back().brakingDistance;

  tft->setTextDatum(TR_DATUM);
  tft->drawString(String(brakeDist, 1) + " m", SCREEN_WIDTH - 10,
                  SCREEN_HEIGHT - 20);
}

void DragMeterScreen::drawMenu() {
  TFT_eSPI *tft = _ui->getTft();

  // Back Button
  tft->setTextColor(COLOR_HIGHLIGHT, COLOR_BG);
  tft->setTextDatum(TL_DATUM);
  tft->setFreeFont(&Org_01);
  tft->setTextSize(2);
  tft->drawString("<", 10, 25);

  tft->drawFastHLine(0, 45, SCREEN_WIDTH, COLOR_SECONDARY);

  int startY = 55;
  int btnHeight = 35;
  int btnWidth = 240;
  int gap = 8;
  int x = (SCREEN_WIDTH - btnWidth) / 2;

  for (int i = 0; i < _menuItems.size(); i++) {
    int y = startY + (i * (btnHeight + gap));

    // Determine Color based on selection
    uint16_t btnColor = (i == _selectedMenuIdx) ? TFT_RED : TFT_DARKGREY;

    tft->fillRoundRect(x, y, btnWidth, btnHeight, 5, btnColor);
    tft->setTextColor(TFT_WHITE, btnColor);
    tft->setTextDatum(MC_DATUM);
    tft->setFreeFont(&Org_01);
    tft->setTextSize(2); // Using Org_01 size 2 to match LapTimer
    tft->drawString(_menuItems[i], SCREEN_WIDTH / 2, y + btnHeight / 2 + 2);
  }
  _ui->drawStatusBar(true);
}

void DragMeterScreen::drawDragModeMenu() {
  TFT_eSPI *tft = _ui->getTft();

  // Back Button
  tft->setTextColor(COLOR_HIGHLIGHT, COLOR_BG);
  tft->setTextDatum(TL_DATUM);
  tft->setFreeFont(&Org_01);
  tft->setTextSize(2);
  tft->drawString("<", 10, 25);

  tft->drawFastHLine(0, 45, SCREEN_WIDTH, COLOR_SECONDARY);

  int startY = 55;
  int btnHeight = 35;
  int btnWidth = 240;
  int gap = 8;
  int x = (SCREEN_WIDTH - btnWidth) / 2;

  for (int i = 0; i < _dragModeItems.size(); i++) {
    int y = startY + (i * (btnHeight + gap));

    // Determine Color based on selection
    uint16_t btnColor = (i == _selectedDragModeIdx) ? TFT_RED : TFT_DARKGREY;

    tft->fillRoundRect(x, y, btnWidth, btnHeight, 5, btnColor);
    tft->setTextColor(TFT_WHITE, btnColor);
    tft->setTextDatum(MC_DATUM);
    tft->setFreeFont(&Org_01);
    tft->setTextSize(2);
    tft->drawString(_dragModeItems[i], SCREEN_WIDTH / 2, y + btnHeight / 2 + 2);
  }
  _ui->drawStatusBar(true);
}

void DragMeterScreen::drawPredictiveMenu() {
  TFT_eSPI *tft = _ui->getTft();

  // Back Button
  tft->setTextColor(COLOR_HIGHLIGHT, COLOR_BG);
  tft->setTextDatum(TL_DATUM);
  tft->setFreeFont(&Org_01);
  tft->setTextSize(2);
  tft->drawString("<", 10, 25);

  tft->drawFastHLine(0, 45, SCREEN_WIDTH, COLOR_SECONDARY);

  int startY = 55;
  int btnHeight = 35;
  int btnWidth = 240;
  int gap = 8;
  int x = (SCREEN_WIDTH - btnWidth) / 2;

  for (int i = 0; i < _predictiveItems.size(); i++) {
    int y = startY + (i * (btnHeight + gap));

    // Determine Color based on selection
    uint16_t btnColor = (i == _selectedPredictiveIdx) ? TFT_RED : TFT_DARKGREY;

    tft->fillRoundRect(x, y, btnWidth, btnHeight, 5, btnColor);
    tft->setTextColor(TFT_WHITE, btnColor);
    tft->setTextDatum(MC_DATUM);
    tft->setFreeFont(&Org_01);
    tft->setTextSize(2); // Using Org_01 size 2 to match LapTimer
    tft->drawString(_predictiveItems[i], SCREEN_WIDTH / 2,
                    y + btnHeight / 2 + 2);
  }
  _ui->drawStatusBar(true);
}

void DragMeterScreen::handlePredictiveTouch(int idx) {
  // If Preductive Mode selected (idx 1), maybe we go to Summary View?
  // User didn't specify what Normal Mode does.
  // For now, let's just go to Summary View regardless, or stay in menu with
  // selection indicator? Let's assume selecting it goes to the view for now, as
  // that's typical. Or maybe it toggles a mode and goes back? Given the
  // previous "Predictive" went to "Summary View", let's make "Predictive Mode"
  // go there. "Normal Mode" might just go back to Drag Mode or similar? Let's
  // make "Predictive Mode" go to SummaryView.

  if (idx == 1) { // PREDICTIVE MODE
    _state = STATE_SUMMARY_VIEW;
    _ui->setTitle("RUN SUMMARY");
    _ui->getTft()->fillScreen(COLOR_BG);
    drawSummary();
  } else {
    // Normal Mode - maybe back to drag screen?
    _state = STATE_RUNNING;
    _ui->getTft()->fillScreen(COLOR_BG);
    drawDashboardStatic();
  }
}
