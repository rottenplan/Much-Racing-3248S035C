#include "SplashScreen.h"
#include "../fonts/Org_01.h"
#include "SplashScreenAssets.h"
#include <Preferences.h>

void SplashScreen::onShow() {
  TFT_eSPI *tft = _ui->getTft();
  // tft->setRotation(1); // Already set in main.cpp
  // Screen already cleared in main.cpp before backlight turns on

  // Gambar Bitmap (320x240) - Centered
  int bmpX = (SCREEN_WIDTH - 320) / 2;
  int bmpY = (SCREEN_HEIGHT - 240) / 2;
  tft->drawBitmap(bmpX, bmpY, image_BOLONG_bits, 320, 240, 0xFFFF); // Putih

  // Gambar Teks "ENGINE STARTING" menggunakan Org_01
  tft->setTextColor(0xFFFF);
  tft->setTextSize(FONT_SIZE_SPLASH_TEXT);
  tft->setFreeFont(&Org_01);
  // Center text horizontally, and position vertically relative to image or
  // screen
  tft->setTextDatum(TC_DATUM); // Ensure text is centered
  tft->drawString("ENGINE STARTING", SCREEN_WIDTH / 2, bmpY + 214);

  // Inisialisasi Kemajuan
  _progress = 0;
  // Gambar batang kosong awal atau hanya mulai dari 0
  int barX = bmpX + 38;
  int barY = bmpY + 198;
  tft->fillRect(barX, barY, _progress, 11, 0xFFFF);

  _lastUpdate = millis();
}

void SplashScreen::update() {
  TFT_eSPI *tft = _ui->getTft();
  // Logika Animasi
  // Lebar target adalah ~246 berdasarkan kode pengguna
  if (_progress < 246) {
    // Pembaruan non-pemblokiran setiap 10ms (lebih cepat dari 100ms untuk
    // kelancaran)
    if (millis() - _lastUpdate > 10) {
      _progress += 2; // Penambahan lebih cepat

      int bmpX = (SCREEN_WIDTH - 320) / 2;
      int bmpY = (SCREEN_HEIGHT - 240) / 2;
      int barX = bmpX + 38;
      int barY = bmpY + 198;
      tft->fillRect(barX, barY, _progress, 11, 0xFFFF);

      _lastUpdate = millis();
    }
  } else {
    // Animasi selesai, tunggu sebentar lalu beralih
    if (millis() - _lastUpdate > 1000) {
      // Check if this is first launch
      Preferences prefs;
      prefs.begin("muchrace", true); // Read-only
      bool setupDone = prefs.getBool("setup_done", false);
      prefs.end();

      if (!setupDone) {
        // First launch - go to setup
        Serial.println("First launch detected, showing setup screen");
        _ui->switchScreen(SCREEN_SETUP);
      } else {
        // Normal launch - go to menu
        _ui->switchScreen(SCREEN_MENU);
      }
    }
  }
}