#include "SettingsScreen.h"
#include "../../config.h"
#include "../../core/SessionManager.h"
#include "../fonts/Org_01.h"
#include "../fonts/Picopixel.h"

extern SessionManager sessionManager;

void SettingsScreen::onShow() {
  _selectedIdx = -1; // Reset selection logic
  loadSettings(); // Reload to ensure sync
  
  TFT_eSPI *tft = _ui->getTft();
  tft->fillScreen(COLOR_BG);
  drawList(0);
}

void SettingsScreen::loadSettings() {
  _settings.clear();
  _settings.push_back({"CLOCK SETTING", TYPE_ACTION});
  
  
  // Auto Off
  SettingItem autoOff = {"AUTO SCREEN OFF", TYPE_ACTION, "auto_off"};
  // Options not needed for action type here, handled in sub-screen
  _settings.push_back(autoOff);
}

void SettingsScreen::saveSetting(int idx) {
  if (idx < 0 || idx >= _settings.size())
    return;

  _prefs.begin("laptimer", false);
  SettingItem &item = _settings[idx];

  if (item.type == TYPE_VALUE) {
    _prefs.putInt(item.key.c_str(), item.currentOptionIdx);

    // Terapkan efek langsung
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
      ledcWrite(0, duty); // Saluran 0
    }
    // Untuk saat ini hanya menyimpan.
    /*
    if (item.name == "AUTO SCREEN OFF") {
        _ui->setAutoOff(item.currentOptionIdx);
    }
    */
  } else if (item.type == TYPE_TOGGLE) {
    _prefs.putBool(item.key.c_str(), item.checkState);
  }
  _prefs.end();
}

void SettingsScreen::update() {
  static unsigned long lastSettingTouch = 0;
  UIManager::TouchPoint p = _ui->getTouchPoint();
  if (p.x == -1)
    return;

  // Tombol Kembali (Area Header 20-60)
  if (p.x < 60 && p.y < 60) {
    if (millis() - lastSettingTouch < 200) return;
    lastSettingTouch = millis();

    if (_selectedIdx == -2) {
       _ui->switchScreen(SCREEN_MENU);
    } else {
       _selectedIdx = -2;
       drawList(_scrollOffset);
    }
    return;
  }

  // Daftar Sentuh
  int listY = 60; // Dipindahkan ke atas (sebelumnya 60)
  int itemH = 35; // Dikurangi agar muat 5 item (5 * 35 = 175 + 60 = 235 < 240)

    // Debounce Check
    if (millis() - lastSettingTouch > 200) {
        int idx = _scrollOffset + ((p.y - listY) / itemH);
        
        if (idx >= 0 && idx < _settings.size()) {
             if (_selectedIdx == idx) {
                 // Second tap -> Execute
                 handleTouch(idx);
             } else {
                 // First tap -> Highlight
                 _selectedIdx = idx;
                 drawList(_scrollOffset);
             }
        }
        
        lastSettingTouch = millis();
    }
}

void SettingsScreen::handleTouch(int idx) {
  if (idx < 0 || idx >= _settings.size())
    return;
  
  SettingItem &item = _settings[idx];
  if (item.name == "CLOCK SETTING") {
      _ui->switchScreen(SCREEN_TIME_SETTINGS);
  } else if (item.name == "AUTO SCREEN OFF") {
      _ui->switchScreen(SCREEN_AUTO_OFF);
  }
}

void SettingsScreen::drawList(int scrollOffset) {
  TFT_eSPI *tft = _ui->getTft();

  // Header
  uint16_t backColor = (_selectedIdx == -2) ? COLOR_HIGHLIGHT : COLOR_TEXT;
  tft->setTextColor(backColor, COLOR_BG);
  tft->setTextDatum(TL_DATUM);
  tft->setFreeFont(&Org_01);
  tft->setTextSize(2); 
  tft->drawString("<", 10, 25);
  
  tft->setTextColor(COLOR_TEXT, COLOR_BG);
  tft->setTextDatum(TC_DATUM);
  tft->setTextSize(1);
  tft->setTextFont(2);
  tft->drawString("SETTINGS", SCREEN_WIDTH / 2, 40);
  
  tft->drawFastHLine(0, 60, SCREEN_WIDTH, COLOR_SECONDARY);

  // List
  int listY = 60;
  int itemH = 35;
  
  for (int i = 0; i < _settings.size(); i++) {
    SettingItem &item = _settings[i];
    int y = listY + (i * itemH);
    int sIdx = i;

    // Background
    uint16_t bgColor = (sIdx == _selectedIdx) ? COLOR_HIGHLIGHT : COLOR_BG;
    uint16_t txtColor = (sIdx == _selectedIdx) ? COLOR_BG : COLOR_TEXT;
    // uint16_t valColor = (sIdx == _selectedIdx) ? COLOR_BG : COLOR_ACCENT;

    if (sIdx == _selectedIdx) {
        tft->fillRoundRect(5, y, SCREEN_WIDTH - 10, itemH, 5, COLOR_HIGHLIGHT);
    } else {
        tft->drawFastHLine(0, y + itemH - 1, SCREEN_WIDTH, COLOR_SECONDARY);
    }

    // Name
    tft->setTextDatum(TL_DATUM);
    tft->setTextFont(2); 
    tft->setTextSize(1);
    tft->setTextColor(txtColor, bgColor);
    tft->drawString(item.name, 10, y + 10);
  }
}
