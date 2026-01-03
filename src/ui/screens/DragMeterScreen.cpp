#include "DragMeterScreen.h"
#include "../../core/GPSManager.h"
#include "../../core/SessionManager.h"

extern GPSManager gpsManager;

// Define subset of disciplines to track
// 0-60 km/h
// 0-100 km/h
// 100-200 km/h
// 402m (1/4 Mile)

void DragMeterScreen::onShow() {
  _state = STATE_READY;
  _startTime = 0;
  _lastUpdate = 0;

  // Initialize Disciplines
  _disciplines.clear();
  _disciplines.push_back({"0-60", false, 60.0, 0, false, 0});
  _disciplines.push_back({"0-100", false, 100.0, 0, false, 0});
  // _disciplines.push_back({"100-200", false, 200.0, 0, false, 0}); // Complex
  // start logic needed? For now just 0-X
  _disciplines.push_back({"1/4 Mi", true, 402.34, 0, false, 0});

  _ui->getTft()->fillScreen(COLOR_BG);
  drawDashboard();
}

void DragMeterScreen::update() {
  // Handle Touch (Back Button)
  UIManager::TouchPoint p = _ui->getTouchPoint();
  // DEBUG: Visual Touch Feedback
  if (p.x != -1) {
      _ui->getTft()->fillCircle(p.x, p.y, 3, TFT_YELLOW);
  }
  if (p.x != -1) {
    if (p.x < 60 && p.y < 40) {
      _ui->switchScreen(SCREEN_MENU);
      return;
    }

    // Reset on touch in Summary
    if (_state == STATE_SUMMARY && p.y > 200) {
      onShow(); // Reset
      return;
    }
  }

  // State Logic
  switch (_state) {
  case STATE_READY:
    checkStartCondition();
    break;
  case STATE_RUNNING:
    updateDisciplines();
    checkStopCondition();
    break;
  case STATE_SUMMARY:
    // Just waiting for reset
    break;
  }

  // Refresh UI (10Hz)
  if (millis() - _lastUpdate > 100) {
    if (_state != STATE_SUMMARY) {
      drawDashboard();
    }
    _ui->drawStatusBar();
    _lastUpdate = millis();
  }
}

void DragMeterScreen::checkStartCondition() {
  // Start if speed > 2 km/h (Basic movement detection)
  if (gpsManager.getSpeedKmph() > 2.0 && gpsManager.isFixed()) {
    _state = STATE_RUNNING;
    _startTime = millis();
    // Reset discipline start times if needed?
    // For 0-X, start is now. For 100-200, start is when 100 is hit.
    // Simplified: Assume standing start for all for now.
  }
}

void DragMeterScreen::checkStopCondition() {
  // Auto-stop if speed < 1 km/h for > 2 seconds?
  // Or if all disciplines done?
  // Let's stop if speed drops near zero after being high.

  // Simple: If speed < 1.0, switch to Summary
  if (gpsManager.getSpeedKmph() < 1.0 && (millis() - _startTime > 2000)) {
    _state = STATE_SUMMARY;
    _ui->getTft()->fillScreen(COLOR_BG);
    drawResults();
  }
}

void DragMeterScreen::updateDisciplines() {
  unsigned long currentRunTime = millis() - _startTime;
  float currentSpeed = gpsManager.getSpeedKmph();
  // Distance? GPSManager needs to track distance from start point.
  // For now estimating distance: dist += speed * time_delta
  // Better: use start lat/lon
  // TODO: Implement distance tracking in GPSManager or locally.

  // Mocking distance logic or just using simple integration
  static unsigned long lastCalc = 0;
  static float distMeters = 0;
  if (currentRunTime == 0) {
    distMeters = 0;
    lastCalc = millis();
  }

  unsigned long delta = millis() - lastCalc;
  if (delta > 0) {
    distMeters += (currentSpeed / 3.6) * (delta / 1000.0);
    lastCalc = millis();
  }

  for (auto &d : _disciplines) {
    if (!d.completed) {
      if (d.isDistance) {
        if (distMeters >= d.target) {
          d.completed = true;
          d.resultTime = currentRunTime;
          d.endSpeed = currentSpeed;
        }
      } else {
        // Speed Mode (0-X)
        if (currentSpeed >= d.target) {
          d.completed = true;
          d.resultTime = currentRunTime;
          d.endSpeed = currentSpeed;
        }
      }
    }
  }
}

void DragMeterScreen::drawDashboard() {
  TFT_eSPI *tft = _ui->getTft();

  // Header Back Arrow
  tft->setTextColor(COLOR_TEXT, COLOR_BG);
  tft->setTextDatum(TL_DATUM);
  tft->setTextSize(2);
  tft->drawString("<", 10, 30); // Below status bar

  // 1. Big Speed (Top Center)
  tft->setTextDatum(TC_DATUM);
  tft->setTextFont(7); // Numeric 7-seg like
  tft->setTextSize(1);
  tft->drawFloat(gpsManager.getSpeedKmph(), 0, SCREEN_WIDTH / 2, 40);
  tft->setTextFont(2);
  tft->setTextSize(1);
  tft->drawString("km/h", SCREEN_WIDTH / 2, 90);

  // 2. Disciplines List (Left)
  int listY = 120;
  tft->setTextDatum(TL_DATUM);
  tft->setTextFont(2);
  tft->setTextSize(1);

  for (int i = 0; i < _disciplines.size(); i++) {
    Discipline &d = _disciplines[i];
    int y = listY + (i * 30);

    tft->setTextColor(COLOR_SECONDARY, COLOR_BG);
    tft->drawString(d.name, 20, y);

    // 3. Status/Time (Right)
    tft->setTextDatum(TR_DATUM);
    if (d.completed) {
      tft->setTextColor(TFT_GREEN, COLOR_BG);
      char buf[16];
      sprintf(buf, "%.2fs", d.resultTime / 1000.0);
      tft->drawString(buf, SCREEN_WIDTH - 20, y);
    } else {
      tft->setTextColor(COLOR_SECONDARY, COLOR_BG);
      tft->drawString("--.--", SCREEN_WIDTH - 20, y);
    }

    tft->setTextDatum(TL_DATUM); // Reset
  }

  // Status Text
  tft->setTextDatum(BC_DATUM);
  tft->setTextSize(1);
  if (_state == STATE_READY) {
    tft->setTextColor(TFT_ORANGE, COLOR_BG);
    tft->drawString(gpsManager.isFixed() ? "READY" : "WAIT GPS",
                    SCREEN_WIDTH / 2, 230);
  } else {
    tft->setTextColor(TFT_GREEN, COLOR_BG);
    tft->drawString("RUNNING", SCREEN_WIDTH / 2, 230);
  }
}

void DragMeterScreen::drawResults() {
  TFT_eSPI *tft = _ui->getTft();

  // Header
  tft->fillRect(0, 20, SCREEN_WIDTH, 40, COLOR_SECONDARY);
  tft->setTextColor(COLOR_TEXT, COLOR_SECONDARY);
  tft->setTextDatum(MC_DATUM);
  tft->setTextFont(2);
  tft->setTextSize(1);
  tft->drawString("RUN SUMMARY", SCREEN_WIDTH / 2, 40);

  tft->drawString("<", 15, 40); // Back

  // List Results
  int listY = 80;
  tft->setTextSize(2);

  for (int i = 0; i < _disciplines.size(); i++) {
    Discipline &d = _disciplines[i];
    int y = listY + (i * 40);

    // Name
    tft->setTextDatum(TL_DATUM);
    tft->setTextColor(COLOR_TEXT, COLOR_BG);
    tft->drawString(d.name, 20, y);

    // Time
    tft->setTextDatum(TR_DATUM);
    if (d.completed) {
      tft->setTextColor(TFT_GREEN, COLOR_BG);
      char buf[16];
      sprintf(buf, "%.2fs", d.resultTime / 1000.0);
      tft->drawString(buf, SCREEN_WIDTH - 20, y);

      // End Speed (Small below)
      if (d.isDistance) {
        tft->setTextSize(1);
        tft->setTextColor(COLOR_SECONDARY, COLOR_BG);
        sprintf(buf, "@ %.0f km/h", d.endSpeed);
        tft->drawString(buf, SCREEN_WIDTH - 20, y + 20);
        tft->setTextSize(2);
      }
    } else {
      tft->setTextColor(TFT_RED, COLOR_BG);
      tft->drawString("---", SCREEN_WIDTH - 20, y);
    }
  }

  // Reset Button
  tft->fillRoundRect(80, 200, 160, 30, 5, COLOR_SECONDARY);
  tft->setTextColor(COLOR_TEXT, COLOR_SECONDARY);
  tft->setTextDatum(MC_DATUM);
  tft->drawString("RESET", SCREEN_WIDTH / 2, 215);
}
