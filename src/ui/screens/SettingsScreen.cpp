#include "SettingsScreen.h"
#include "../../config.h"
#include "../../core/SessionManager.h"
#include "../fonts/Picopixel.h"

extern SessionManager sessionManager;

void SettingsScreen::onShow() {
  _scrollOffset = 0;
  loadSettings();

  TFT_eSPI *tft = _ui->getTft();
  _ui->drawStatusBar(); // Gambar Bilah Status
  drawList(0);
}

void SettingsScreen::loadSettings() {
  _settings.clear();
  _prefs.begin("laptimer", false); // Namespace "laptimer"

  // 1. Kecerahan (Nilai: Rendah, Sedap, Tinggi)
  SettingItem br;
  br.name = "Brightness";
  br.type = TYPE_VALUE;
  br.key = "bright";
  br.options = {"25%", "50%", "75%", "100%"};
  br.currentOptionIdx = _prefs.getInt("bright", 3); // Default 100% (Indeks 3)
  _settings.push_back(br);

  // 2. Unit (Nilai: Metrik, Imperial)
  SettingItem unit;
  unit.name = "Units";
  unit.type = TYPE_VALUE;
  unit.key = "units";
  unit.options = {"Metric", "Imperial"};
  unit.currentOptionIdx = _prefs.getInt("units", 0); // Default Metrik
  _settings.push_back(unit);

  // 3. Tes Kartu SD (Tindakan) - Dipindahkan ke atas untuk visibilitas
  SettingItem sdTest;
  sdTest.name = "SD Card Test";
  sdTest.type = TYPE_ACTION;
  _settings.push_back(sdTest);

  // 4. Zona Waktu (Nilai)
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

  // 5. Reset Pabrik (Tindakan)
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
  } else if (item.type == TYPE_TOGGLE) {
    _prefs.putBool(item.key.c_str(), item.checkState);
  }
  _prefs.end();
}

void SettingsScreen::update() {
  UIManager::TouchPoint p = _ui->getTouchPoint();
  if (p.x == -1)
    return;

  // Tombol Kembali (Area Header 20-60)
  if (p.x < 60 && p.y < 60) {
    _ui->switchScreen(SCREEN_MENU);
    return;
  }

  // Daftar Sentuh
  int listY = 60; // Dipindahkan ke atas (sebelumnya 60)
  int itemH = 35; // Dikurangi agar muat 5 item (5 * 35 = 175 + 60 = 235 < 240)

    // Debounce Check
    static unsigned long lastSettingTouch = 0;
    if (millis() - lastSettingTouch > 200) {
        int idx = _scrollOffset + ((p.y - listY) / itemH);
        handleTouch(idx);
        lastSettingTouch = millis();
        
        // Redraw
        drawList(_scrollOffset);
    }

  // Perbarui Bilah Status
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

      // Pemblokiran tunggu sentuhan
      delay(500); // Debounce
      while (_ui->getTouchPoint().x == -1) {
        delay(50);
      }

      // Gambar ulang daftar setelah kembali
      tft->fillScreen(COLOR_BG);
      drawList(_scrollOffset);
    }
  }
}

void SettingsScreen::drawList(int scrollOffset) {
  TFT_eSPI *tft = _ui->getTft();

  // Header
  // Header (Di Bawah Bilah Status)
  // Panah Kembali
  tft->setTextColor(COLOR_TEXT, COLOR_BG);
  tft->setTextDatum(TL_DATUM);
  tft->setTextSize(2);
  tft->drawString("<", 10, 35);

  // Daftar
  int startY = 60; // Dimulai lebih tinggi (sebelumnya 60)
  int itemH = 35;  // 5 item * 35 = 175. + 60 header = 235 (Muat dalam 240)

  for (int i = 0; i < 5; i++) { // Tampilkan 5 item
    int sIdx = scrollOffset + i;
    if (sIdx >= _settings.size())
      break;

    SettingItem &item = _settings[sIdx];
    int y = startY + (i * itemH);

    // Pemisah
    tft->drawFastHLine(0, y + itemH - 1, SCREEN_WIDTH, COLOR_SECONDARY);

    // Nama
    tft->setTextColor(COLOR_TEXT, COLOR_BG);

    // ... di dalam drawList ...
    tft->setTextDatum(TL_DATUM);
    tft->setTextFont(2); // Font Standar
    tft->setTextSize(1);
    tft->drawString(item.name, 10, y + 12);

    // Nilai/Elemen
    tft->setTextDatum(TR_DATUM);
    tft->setTextColor(COLOR_ACCENT, COLOR_BG);

    if (item.type == TYPE_VALUE) {
      String val = item.options[item.currentOptionIdx];
      tft->drawString(val, SCREEN_WIDTH - 20, y + 12);
      tft->drawString(">", SCREEN_WIDTH - 10, y + 12); // Panah
    } else if (item.type == TYPE_TOGGLE) {
      // Gambar Sakelar Toggle
      int swX = SCREEN_WIDTH - 40;
      int swY = y + 12;
      int swW = 30;
      int swH = 15;

      uint16_t color = item.checkState ? TFT_GREEN : TFT_RED;
      tft->fillRoundRect(swX, swY, swW, swH, 7, color);
      // Tombol
      int knobX = item.checkState ? (swX + swW - 14) : (swX + 2);
      tft->fillCircle(knobX + 6, swY + 7, 5, TFT_WHITE);
    } else if (item.type == TYPE_ACTION) {
      tft->drawString("EXEC", SCREEN_WIDTH - 20, y + 12);
    }
  }
}
