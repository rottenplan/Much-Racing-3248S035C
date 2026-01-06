#include "SplashScreen.h"
#include "../fonts/Org_01.h"
#include "SplashScreenAssets.h"

void SplashScreen::onShow() {
  TFT_eSPI *tft = _ui->getTft();
  // tft->setRotation(1); // Already set in main.cpp
  tft->fillScreen(0x0000); // Hitam

  // Gambar Bitmap (320x240)
  tft->drawBitmap(0, 0, image_BOLONG_bits, 320, 240, 0xFFFF); // Putih

  // Gambar Teks "ENGINE STARTING" menggunakan Org_01
  tft->setTextColor(0xFFFF);
  tft->setTextSize(FONT_SIZE_SPLASH_TEXT);
  tft->setFreeFont(&Org_01);
  tft->drawString("ENGINE STARTING", 115, 214);

  // Inisialisasi Kemajuan
  _progress = 0;
  // Gambar batang kosong awal atau hanya mulai dari 0
  tft->fillRect(38, 198, _progress, 11, 0xFFFF);

  _lastUpdate = millis();
}

void SplashScreen::update() {
  // Logika Animasi
  // Lebar target adalah ~246 berdasarkan kode pengguna
  if (_progress < 246) {
    // Pembaruan non-pemblokiran setiap 10ms (lebih cepat dari 100ms untuk kelancaran)
    if (millis() - _lastUpdate > 10) {
      _progress += 2; // Penambahan lebih cepat

      TFT_eSPI *tft = _ui->getTft();
      tft->fillRect(38, 198, _progress, 11, 0xFFFF);

      _lastUpdate = millis();
    }
  } else {
    // Animasi selesai, tunggu sebentar lalu beralih
    if (millis() - _lastUpdate > 1000) {
      _ui->switchScreen(SCREEN_MENU);
    }
  }
}