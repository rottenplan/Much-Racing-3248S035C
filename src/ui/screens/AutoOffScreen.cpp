#include "AutoOffScreen.h"
#include "../../config.h"
#include <Preferences.h>
#include "../fonts/Org_01.h"

void AutoOffScreen::onShow() {
  _options = {"NEVER", "15 SEC", "30 SEC", "1 MIN", "3 MIN", "5 MIN"};
  
  // Get current setting to highlight
  // We need a way to get the current index from UIManager or Prefs.
  // Ideally UIManager exposes this or we load it ourselves.
  // Since UIManager manages the state, we can ask it or just read prefs.
  // Reading prefs is safer/easier here to decouple slightly.
  Preferences prefs;
  prefs.begin("laptimer", true);
  _selectedIdx = prefs.getInt("auto_off", 0);
  prefs.end();

  TFT_eSPI *tft = _ui->getTft();
  tft->fillScreen(COLOR_BG);
  drawScreen();
}

void AutoOffScreen::update() {
  static unsigned long lastTouch = 0;
  UIManager::TouchPoint p = _ui->getTouchPoint();
  
  if (p.x == -1) return;

  if (millis() - lastTouch > 200) {
      // 1. Back Button (Top Left)
      if (p.x < 60 && p.y < 60) {
          _ui->switchScreen(SCREEN_SETTINGS);
          lastTouch = millis();
          return;
      }

      // 2. List Items
      int listY = 65;
      int itemH = 28; // 6 items * 28 = 168. 65 + 168 = 233. Safe for 240 height.
      
      int idx = (p.y - listY) / itemH;
      if (idx >= 0 && idx < _options.size()) {
          handleTouch(idx);
      }
      
      lastTouch = millis();
  }
}

void AutoOffScreen::handleTouch(int idx) {
    if (idx < 0 || idx >= _options.size()) return;

    _selectedIdx = idx;
    
    // Save and Apply
    Preferences prefs;
    prefs.begin("laptimer", false);
    prefs.putInt("auto_off", idx);
    prefs.end();
    
    _ui->setAutoOff(idx); // Apply immediately
    
    drawScreen();
    // Optional: Return to settings immediately?
    // _ui->switchScreen(SCREEN_SETTINGS);
}

void AutoOffScreen::drawScreen() {
  TFT_eSPI *tft = _ui->getTft();

  // Header
  // Back Arrow
  tft->setTextColor(COLOR_HIGHLIGHT, COLOR_BG);
  tft->setTextDatum(TL_DATUM);
  tft->setFreeFont(&Org_01);
  tft->setTextSize(2);
  tft->drawString("<", 10, 25);

  // Title
  tft->setTextColor(COLOR_TEXT, COLOR_BG);
  tft->setTextDatum(TC_DATUM);
  tft->setTextSize(1);
  tft->setTextSize(1);
  tft->setTextFont(2);
  tft->drawString("AUTO OFF", SCREEN_WIDTH / 2, 30); // Title at 30

  tft->drawFastHLine(0, 60, SCREEN_WIDTH, COLOR_SECONDARY); // Line at 60

  // List
  int listY = 65; // List starts at 65
  int itemH = 28;

  for (int i = 0; i < _options.size(); i++) {
      int y = listY + (i * itemH);
      bool isSelected = (i == _selectedIdx);

      // Background
      if (isSelected) {
          tft->fillRoundRect(5, y, SCREEN_WIDTH - 10, itemH - 2, 5, COLOR_HIGHLIGHT);
      } else {
          tft->fillRect(5, y, SCREEN_WIDTH - 10, itemH - 2, COLOR_BG); // Clear previous if switching
      }

      // Text
      tft->setTextDatum(ML_DATUM); // Middle Left
      tft->setTextFont(2);
      tft->setTextSize(1);
      
      uint16_t txtColor = isSelected ? COLOR_BG : COLOR_TEXT;
      tft->setTextColor(txtColor, isSelected ? COLOR_HIGHLIGHT : COLOR_BG);
      tft->drawString(_options[i], 20, y + (itemH/2));

      // Checkmark for selected?
      if (isSelected) {
          tft->setTextDatum(MR_DATUM);
          tft->drawString("OK", SCREEN_WIDTH - 20, y + (itemH/2));
      }
  }
}
