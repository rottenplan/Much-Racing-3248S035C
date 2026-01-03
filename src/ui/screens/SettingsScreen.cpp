#include "SettingsScreen.h"
#include "../../config.h"
#include "../../core/SessionManager.h"
#include "../fonts/Picopixel.h"
#include "SettingsScreen.h"

extern SessionManager sessionManager;

void SettingsScreen::onShow() {
  _scrollOffset = 0;
  loadSettings();

  TFT_eSPI *tft = _ui->getTft();
  tft->fillScreen(COLOR_BG);
  _ui->drawStatusBar(); // Draw Status Bar
  drawList(0);
}

void SettingsScreen::loadSettings() {
  _settings.clear();
  _prefs.begin("laptimer", false); // Namespace "laptimer"

  // 1. Brightness (Value: Low, Med, High)
  SettingItem br;
  br.name = "Brightness";
  br.type = TYPE_VALUE;
  br.key = "bright";
  br.options = {"25%", "50%", "75%", "100%"};
  br.currentOptionIdx = _prefs.getInt("bright", 3); // Default 100% (Index 3)
  _settings.push_back(br);

  // 2. Units (Value: Metric, Imperial)
  SettingItem unit;
  unit.name = "Units";
  unit.type = TYPE_VALUE;
  unit.key = "units";
  unit.options = {"Metric", "Imperial"};
  unit.currentOptionIdx = _prefs.getInt("units", 0); // Default Metric
  _settings.push_back(unit);

  // 3. SD Card Test (Action) - Moved up for visibility
  SettingItem sdTest;
  sdTest.name = "SD Card Test";
  sdTest.type = TYPE_ACTION;
  _settings.push_back(sdTest);

  // 4. Time Zone (Value)
  SettingItem tz;
  tz.name = "Time Zone";
  tz.type = TYPE_VALUE;
  tz.key = "timezone";
  for (int i = -12; i <= 14; i++) {
    String s = "UTC";
    if (i >= 0)
      s += "+";
    s += String(i);
    tz.options.push_back(s);
  }
  tz.currentOptionIdx = _prefs.getInt("timezone", 19); // Default UTC+7
  _settings.push_back(tz);

  // 5. Factory Reset (Action)
  SettingItem reset;
  reset.name = "Factory Reset";
  reset.type = TYPE_ACTION;
  _settings.push_back(reset);

  // 7. Factory Reset (Action)

  // 7. Factory Reset (Action)

  _prefs.end();
}

void SettingsScreen::saveSetting(int idx) {
  if (idx < 0 || idx >= _settings.size())
    return;

  _prefs.begin("laptimer", false);
  SettingItem &item = _settings[idx];

  if (item.type == TYPE_VALUE) {
    _prefs.putInt(item.key.c_str(), item.currentOptionIdx);

    // Apply immediate effects
    if (item.name == "Brightness") {
      int duty = 255;
      switch (item.currentOptionIdx) {
      case 0:
        duty = 64;
        break; // 25%
      case 1:
        duty = 128;
        break; // 50%
      case 2:
        duty = 192;
        break; // 75%
      case 3:
        duty = 255;
        break; // 100%
      }
      ledcWrite(0, duty); // Channel 0
    }
    // For now just storing.
  } else if (item.type == TYPE_TOGGLE) {
    _prefs.putBool(item.key.c_str(), item.checkState);
  }
  _prefs.end();
}

void SettingsScreen::update() {
  UIManager::TouchPoint p = _ui->getTouchPoint();
  if (p.x == -1)
    return;

  // Back Button (Header Area 20-60)
  if (p.x < 60 && p.y < 60) {
    _ui->switchScreen(SCREEN_MENU);
    return;
  }

  // List Touch
  int listY = 60; // Should match visual startY
  int itemH = 35; // Reduced to fit 5 items (5 * 35 = 175 + 60 = 235 < 240)

    // Debounce Check
    static unsigned long lastSettingTouch = 0;
    if (millis() - lastSettingTouch > 200) {
        int idx = _scrollOffset + ((p.y - listY) / itemH);
        handleTouch(idx);
        lastSettingTouch = millis();
        
        // Redraw
        drawList(_scrollOffset);
    }

  // Update Status Bar
  static unsigned long lastStatusUpdate = 0;
  if (millis() - lastStatusUpdate > 1000) {
    _ui->drawStatusBar();
    lastStatusUpdate = millis();
  }
}

void SettingsScreen::handleTouch(int idx) {
  if (idx < 0 || idx >= _settings.size())
    return;
  SettingItem &item = _settings[idx];

  if (item.type == TYPE_VALUE) {
    item.currentOptionIdx++;
    if (item.currentOptionIdx >= item.options.size()) {
      item.currentOptionIdx = 0;
    }
    saveSetting(idx);
  } else if (item.type == TYPE_TOGGLE) {
    item.checkState = !item.checkState;
    saveSetting(idx);
  } else if (item.type == TYPE_ACTION) {
    if (item.name == "Factory Reset") {
      _prefs.begin("laptimer", false);
      _prefs.clear();
      _prefs.end();
      loadSettings(); // Reload defaults
    } else if (item.name == "SD Card Test") {
      TFT_eSPI *tft = _ui->getTft();
      tft->fillScreen(COLOR_BG);
      tft->setTextColor(COLOR_TEXT, COLOR_BG);
      tft->setTextDatum(MC_DATUM);
      tft->setTextFont(2);
      tft->setTextSize(1);
      tft->drawString("Testing SD Card...", SCREEN_WIDTH / 2,
                      SCREEN_HEIGHT / 2);

      uint64_t total = 0, used = 0;
      bool ok = sessionManager.getSDStatus(total, used);

      tft->fillScreen(COLOR_BG);
      if (ok) {
        float totalGB = total / (1024.0 * 1024.0 * 1024.0);
        float usedMB = used / (1024.0 * 1024.0);

        tft->setTextColor(TFT_GREEN, COLOR_BG);
        tft->drawString("SD CARD OK", SCREEN_WIDTH / 2, 60);

        tft->setTextColor(COLOR_TEXT, COLOR_BG);
        String s1 = "Total: " + String(totalGB, 2) + " GB";
        String s2 = "Used: " + String(usedMB, 2) + " MB";
        tft->drawString(s1, SCREEN_WIDTH / 2, 100);
        tft->drawString(s2, SCREEN_WIDTH / 2, 130);
      } else {
        tft->setTextColor(TFT_RED, COLOR_BG);
        tft->drawString("SD CARD ERROR", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
        tft->setTextColor(COLOR_TEXT, COLOR_BG);
        tft->drawString("Check Card / Insert", SCREEN_WIDTH / 2,
                        SCREEN_HEIGHT / 2 + 30);
      }

      tft->setTextColor(COLOR_SECONDARY, COLOR_BG);
      tft->drawString("Touch to Return", SCREEN_WIDTH / 2, SCREEN_HEIGHT - 30);

      // Blocking wait for touch
      delay(500); // Debounce
      while (_ui->getTouchPoint().x == -1) {
        delay(50);
      }

      // Redraw list after return
      tft->fillScreen(COLOR_BG);
      drawList(_scrollOffset);
    }
  }
}

void SettingsScreen::drawList(int scrollOffset) {
  TFT_eSPI *tft = _ui->getTft();

  // Header
  // Header (Below Status Bar)
  // Status Bar is 0-20. Header 20-60 (Height 40).
  tft->fillRect(0, 20, SCREEN_WIDTH, 40, COLOR_SECONDARY);
  tft->setTextColor(COLOR_TEXT, COLOR_SECONDARY);
  tft->setTextDatum(MC_DATUM); // Ensure Centering
  tft->setTextSize(1);         // Standard Font Size
  tft->drawString("SETTINGS", SCREEN_WIDTH / 2,
                  40); // Y centered in 20-60 (which is 40)
  tft->drawString("<", 15, 40);
  tft->drawFastHLine(0, 60, SCREEN_WIDTH, COLOR_SECONDARY); // Header Separator

  // List
  int startY = 60; // Starts below header (20+40)
  int itemH = 35;  // 5 items * 35 = 175. + 60 header = 235 (Fits in 240)

  for (int i = 0; i < 5; i++) { // Show 5 items
    int sIdx = scrollOffset + i;
    if (sIdx >= _settings.size())
      break;

    SettingItem &item = _settings[sIdx];
    int y = startY + (i * itemH);

    // Separator
    tft->drawFastHLine(0, y + itemH - 1, SCREEN_WIDTH, COLOR_SECONDARY);

    // Name
    tft->setTextColor(COLOR_TEXT, COLOR_BG);

    // ... inside drawList ...
    tft->setTextDatum(TL_DATUM);
    tft->setTextFont(2); // Standard Font
    tft->setTextSize(1);
    tft->drawString(item.name, 10, y + 12);

    // Value/Element
    tft->setTextDatum(TR_DATUM);
    tft->setTextColor(COLOR_ACCENT, COLOR_BG);

    if (item.type == TYPE_VALUE) {
      String val = item.options[item.currentOptionIdx];
      tft->drawString(val, SCREEN_WIDTH - 20, y + 12);
      tft->drawString(">", SCREEN_WIDTH - 10, y + 12); // Arrow
    } else if (item.type == TYPE_TOGGLE) {
      // Draw Toggle Switch
      int swX = SCREEN_WIDTH - 40;
      int swY = y + 12;
      int swW = 30;
      int swH = 15;

      uint16_t color = item.checkState ? TFT_GREEN : TFT_RED;
      tft->fillRoundRect(swX, swY, swW, swH, 7, color);
      // Knob
      int knobX = item.checkState ? (swX + swW - 14) : (swX + 2);
      tft->fillCircle(knobX + 6, swY + 7, 5, TFT_WHITE);
    } else if (item.type == TYPE_ACTION) {
      tft->drawString("EXEC", SCREEN_WIDTH - 20, y + 12);
    }
  }
}
