#include "SplashScreen.h"
#include "../fonts/Org_01.h"
#include "SplashScreenAssets.h"

void SplashScreen::onShow() {
  TFT_eSPI *tft = _ui->getTft();
  tft->setRotation(1);
  tft->fillScreen(0x0000); // Black

  // Draw Bitmap (320x240)
  tft->drawBitmap(0, 0, image_BOLONG_bits, 320, 240, 0xFFFF); // White

  // Draw Text "ENGINE STARTING" using Org_01
  tft->setTextColor(0xFFFF);
  tft->setTextSize(FONT_SIZE_SPLASH_TEXT);
  tft->setFreeFont(&Org_01);
  tft->drawString("ENGINE STARTING", 115, 214);

  // Initialize Progress
  _progress = 0;
  // Draw initial empty bar or just start at 0
  tft->fillRect(38, 198, _progress, 11, 0xFFFF);

  _lastUpdate = millis();
}

void SplashScreen::update() {
  // Animation Logic
  // Target width is ~246 based on user code
  if (_progress < 246) {
    // Non-blocking update every 10ms (faster than 100ms for smoothness)
    if (millis() - _lastUpdate > 10) {
      _progress += 2; // Increment faster

      TFT_eSPI *tft = _ui->getTft();
      tft->fillRect(38, 198, _progress, 11, 0xFFFF);

      _lastUpdate = millis();
    }
  } else {
    // Animation complete, wait a moment then switch
    if (millis() - _lastUpdate > 1000) {
      _ui->switchScreen(SCREEN_MENU);
    }
  }
}