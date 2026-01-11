#include "RpmSensorScreen.h"
#include "../../config.h"
#include "../fonts/Org_01.h"
#include <Preferences.h>

// Define colors if not in config
#define COLOR_ORANGE 0xFDA0
#define COLOR_GREEN 0x07E0

// Initialize static members
// volatile unsigned long RpmSensorScreen::_rpmPulses = 0;
// volatile unsigned long RpmSensorScreen::_lastPulseMicros = 0;

// void IRAM_ATTR RpmSensorScreen::onPulse() {
//   unsigned long now = micros();
//   // Debounce: 1ms (1000us) dead time -> Max 60,000 RPM
//   // Filters out high-frequency ringing from spark
//   // if (now - _lastPulseMicros > 1000) {
//   //   _rpmPulses++;
//   //   _lastPulseMicros = now;
//   // }
// }

// External reference
#include "../../core/GPSManager.h"
extern GPSManager gpsManager;

void RpmSensorScreen::onShow() {
  _currentRpm = 0;
  _maxRpm = 0;
  _currentLvl = 0;
  _currentSpeed = 0;
  _graphIndex = 0;
  _lastUpdate = 0;

  // Calculate RPM
  // _rpmPulses = 0;
  // _lastRpmCalcTime = millis();

  // Setup Interrupt for Inductive Clamp
  // MOVED TO GLOBAL GPS MANAGER
  // if (PIN_RPM_INPUT >= 0) {
  //   pinMode(PIN_RPM_INPUT, INPUT);
  //   attachInterrupt(digitalPinToInterrupt(PIN_RPM_INPUT), onPulse, FALLING);
  // }

  // Clear graph history
  for (int i = 0; i < GRAPH_WIDTH; i++) {
    _graphHistory[i] = 0;
    _speedHistory[i] = 0;
  }

  // Create Sprite for Flicker-Free Graph
  if (_graphSprite == nullptr) {
    _graphSprite = new TFT_eSprite(_ui->getTft());
    _graphSprite->createSprite(GRAPH_WIDTH, GRAPH_HEIGHT); // 280x125
  }

  drawScreen();
}

void RpmSensorScreen::update() {
  // 1. Back Button
  UIManager::TouchPoint p = _ui->getTouchPoint();
  if (p.x != -1 && p.x < 60 && p.y < 60) {
    // detachInterrupt(digitalPinToInterrupt(PIN_RPM_INPUT)); // Cleanup - NO,
    // GLOBAL
    _ui->switchScreen(SCREEN_MENU);
    return;
  }

  // 2. Real RPM Calculation (Get from Global)
  unsigned long now = millis();
  if (now - _lastRpmCalcTime > 100) {
    _lastRpmCalcTime = now;

    // Use Global Manager
    _currentRpm = gpsManager.getRPM();

    if (_currentRpm > _maxRpm)
      _maxRpm = _currentRpm;

    // RPM Level (0-12000 scaling)
    int maxScale = 12000;
    _currentLvl = map(constrain(_currentRpm, 0, maxScale), 0, maxScale, 0, 100);

    // Calculate Speed Level (0-150 km/h scaling)
    float rawSpeed = gpsManager.getSpeedKmph();
    _currentSpeed = (int)rawSpeed;
    int speedLvl = map(constrain(_currentSpeed, 0, MAX_SPEED_SCALE), 0,
                       MAX_SPEED_SCALE, 0, 100);

    // Update Graph Buffers
    _graphHistory[_graphIndex] = _currentLvl;
    _speedHistory[_graphIndex] = speedLvl;

    _graphIndex = (_graphIndex + 1) % GRAPH_WIDTH; // 280

    // Redraw Dynamic Parts
    updateValues();
    drawGraphLine();
  }
}

void RpmSensorScreen::drawScreen() {
  TFT_eSPI *tft = _ui->getTft();
  // Clear only content area
  tft->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                SCREEN_HEIGHT - STATUS_BAR_HEIGHT, COLOR_BG);

  // Back Button
  tft->setTextColor(COLOR_HIGHLIGHT, COLOR_BG);
  tft->setTextDatum(TL_DATUM);
  tft->setFreeFont(&Org_01);
  tft->setTextSize(2);
  tft->drawString("<", 10, 25);

  // --- INFO BOX (Resized for Font 4) ---
  int boxY = 25;
  int boxH = 60; // Increased to 60 (30px per row)
  int lblW = 80;

  // Border
  tft->drawRect(10, boxY, SCREEN_WIDTH - 20, boxH, COLOR_TEXT);
  tft->drawFastHLine(10, boxY + 30, SCREEN_WIDTH - 20,
                     COLOR_TEXT); // Split at 30
  tft->drawFastVLine(10 + lblW, boxY, boxH, COLOR_TEXT);

  // Labels (White BG, Black Text)
  // Row 1: Y+1 to Y+29
  // Row 2: Y+31 to Y+59
  tft->fillRect(11, boxY + 1, lblW, 29, COLOR_TEXT);
  tft->fillRect(11, boxY + 31, lblW, 28, COLOR_TEXT);

  tft->setTextColor(COLOR_BG);
  tft->setTextSize(2);
  tft->setTextDatum(ML_DATUM);
  // Centered in 30px heights
  tft->drawString("MAX", 20, boxY + 15);
  tft->drawString("LVL", 20, boxY + 45);

  // --- GRAPH AREA ---
  drawGraphGrid();

  // --- BOTTOM BAR ---
  tft->drawRect(10, 215, SCREEN_WIDTH - 20, 18, COLOR_TEXT);
}

void RpmSensorScreen::updateValues() {
  TFT_eSPI *tft = _ui->getTft();

  // Use Font 4 (LCD/Digital style, 26px)
  tft->setTextFont(4);
  tft->setTextSize(1);
  tft->setTextDatum(MR_DATUM);
  tft->setTextColor(COLOR_TEXT, COLOR_BG);

  int valX = 10 + 80;
  int w = SCREEN_WIDTH - 20 - 80 - 2; // ~218px
  int boxY = 25;

  // Padding to erase previous text without flicker
  tft->setTextPadding(w - 10);

  char buf[10];

  // MAX
  // Row 1 center: boxY + 15
  sprintf(buf, "%05d", _maxRpm);
  // Draw string automatically clears background due to Padding + TextColor
  tft->drawString(buf, SCREEN_WIDTH - 20, boxY + 15);

  // LVL
  // Row 2 center: boxY + 45
  sprintf(buf, "%05d", _currentRpm);
  tft->drawString(buf, SCREEN_WIDTH - 20, boxY + 45);

  tft->setTextPadding(0); // Reset padding

  // Update Bottom Bar
  int barW = map(_currentLvl, 0, 100, 0, SCREEN_WIDTH - 24);
  if (barW < 0)
    barW = 0;

  int barY = 217;
  int barH = 14;

  // Clear remaining Part
  tft->fillRect(12 + barW, barY, (SCREEN_WIDTH - 24) - barW, barH, COLOR_BG);
  // Draw Bar
  tft->fillRect(12, barY, barW, barH, COLOR_GREEN);
}

void RpmSensorScreen::drawGraphGrid() {
  TFT_eSPI *tft = _ui->getTft();
  // Adjusted Position: boxY(25)+boxH(60)+Gap(10) = 95
  int gY = 95;
  int gH = GRAPH_HEIGHT; // 115

  // Box
  tft->drawRect(10, gY, SCREEN_WIDTH - 20, gH, COLOR_TEXT);

  // Grid - Dyno Style
  // Vertical Grid (every 74px ~ 25%)
  // Note: On-Screen Grid (Static) - optional if sprite covers it?
  // Sprite covers INSIDE (1px border). So we don't strictly need to draw inner
  // grid here if sprite is always pushed. But effectively this draws the Frame.

  // Axis Labels
  tft->setTextFont(1);
  tft->setTextSize(1);
  tft->setTextColor(TFT_LIGHTGREY);

  // Left (RPM)
  tft->setTextDatum(MR_DATUM);
  tft->drawString("12k", 8, gY);
  tft->drawString("6k", 8, gY + gH / 2);
  tft->drawString("0", 8, gY + gH);

  // Right (Speed)
  tft->setTextDatum(ML_DATUM);
  tft->drawString(String(MAX_SPEED_SCALE), SCREEN_WIDTH - 8, gY);
  tft->drawString(String(MAX_SPEED_SCALE / 2), SCREEN_WIDTH - 8, gY + gH / 2);
  tft->drawString("0", SCREEN_WIDTH - 8, gY + gH);
}

void RpmSensorScreen::drawGraphLine() {
  if (_graphSprite == nullptr)
    return;

  // 1. Clear Sprite Background
  _graphSprite->fillSprite(COLOR_BG);

  // 2. Redraw Grid on Sprite
  int sW = GRAPH_WIDTH;
  int sH = GRAPH_HEIGHT;

  // Vertical Grid (every 74px)
  for (int x = 74; x < sW; x += 74) {
    _graphSprite->drawFastVLine(x, 0, sH, 0x39E7);
  }

  // Horizontal Grid
  for (int i = 1; i < 4; i++) {
    int y = sH * i / 4;
    _graphSprite->drawFastHLine(0, y, sW, 0x39E7);
  }

  // 3. Draw Lines
  int start = _graphIndex;
  for (int i = 0; i < GRAPH_WIDTH - 1; i++) {
    int idx1 = (start + i) % GRAPH_WIDTH;
    int idx2 = (start + i + 1) % GRAPH_WIDTH;

    // RPM (Red)
    int yR1 = sH - (int)((_graphHistory[idx1] / 100.0) * sH);
    int yR2 = sH - (int)((_graphHistory[idx2] / 100.0) * sH);

    // Clamp
    if (yR1 < 0)
      yR1 = 0;
    if (yR1 >= sH)
      yR1 = sH - 1;
    if (yR2 < 0)
      yR2 = 0;
    if (yR2 >= sH)
      yR2 = sH - 1;

    _graphSprite->drawLine(i, yR1, i + 1, yR2, TFT_RED);

    // Speed (Cyan)
    int yS1 = sH - (int)((_speedHistory[idx1] / 100.0) * sH);
    int yS2 = sH - (int)((_speedHistory[idx2] / 100.0) * sH);

    if (yS1 < 0)
      yS1 = 0;
    if (yS1 >= sH)
      yS1 = sH - 1;
    if (yS2 < 0)
      yS2 = 0;
    if (yS2 >= sH)
      yS2 = sH - 1;

    _graphSprite->drawLine(i, yS1, i + 1, yS2, TFT_CYAN);
  }

  // 4. Push Sprite to Screen
  // BoxRect: (10, 95). Inner: (11, 96).
  _graphSprite->pushSprite(11, 96);
}
