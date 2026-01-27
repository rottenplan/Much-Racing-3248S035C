#include "WebServerScreen.h"
#include "../fonts/Org_01.h"

extern WiFiManager wifiManager;

void WebServerScreen::onShow() {
  _lastUpdate = 0;
  _lastTouchTime = millis();

  // Ensure WiFi is ON
  if (!wifiManager.isEnabled()) {
    wifiManager.setEnabled(true);
  }

  TFT_eSPI *tft = _ui->getTft();
  tft->fillScreen(TFT_BLACK);

  drawStatic();
  drawStatus();
}

void WebServerScreen::drawStatic() {
  TFT_eSPI *tft = _ui->getTft();

  // Header
  tft->drawFastHLine(0, 20, SCREEN_WIDTH,
                     _ui->getSecondaryColor()); // Dynamic width
  tft->setTextDatum(TC_DATUM);
  tft->setFreeFont(&Org_01);
  tft->setTextSize(2);
  tft->setTextColor(TFT_WHITE, TFT_BLACK);
  tft->drawString("OFFLINE SERVER", SCREEN_WIDTH / 2, 25);

  // Back
  tft->setTextDatum(TL_DATUM);
  tft->setTextSize(1);
  tft->drawString("<", 10, 25);

  // Instructions
  tft->setTextDatum(MC_DATUM);
  tft->setTextColor(TFT_SILVER, TFT_BLACK);
  tft->setTextFont(1);
  tft->drawString("Connect via Phone/Laptop to:", SCREEN_WIDTH / 2, 60);
}

void WebServerScreen::drawStatus() {
  TFT_eSPI *tft = _ui->getTft();

  int boxY = 80;
  int boxW = 400; // Wider for 480px
  int boxH = 120;
  int boxX = (SCREEN_WIDTH - boxW) / 2;

  tft->fillRoundRect(boxX, boxY, boxW, boxH, 8, 0x18E3); // Charcoal
  tft->drawRoundRect(boxX, boxY, boxW, boxH, 8, TFT_DARKGREY);

  // Network Info
  tft->setTextDatum(ML_DATUM);

  // SSID
  tft->setTextColor(TFT_SILVER, 0x18E3);
  tft->drawString("SSID:", boxX + 20, boxY + 30);
  tft->setTextColor(TFT_GREEN, 0x18E3);
  tft->setTextFont(2);
  tft->drawString("MuchRacing-GPS", boxX + 80,
                  boxY + 30); // Hardcoded in WiFiManager.cpp

  // PASS
  tft->setTextFont(1);
  tft->setTextColor(TFT_SILVER, 0x18E3);
  tft->drawString("PASS:", boxX + 20, boxY + 60);
  tft->setTextColor(TFT_WHITE, 0x18E3);
  tft->setTextFont(2);
  tft->drawString("12345678", boxX + 80, boxY + 60);

  // IP Address (Big)
  tft->setTextFont(1);
  tft->setTextColor(TFT_SILVER, 0x18E3);
  tft->drawString("IP ADDR:", boxX + 20, boxY + 90);
  tft->setTextColor(TFT_CYAN, 0x18E3);
  tft->setTextFont(4);
  tft->drawString("192.168.4.1", boxX + 80, boxY + 90);

  // Footer Hint
  tft->setTextDatum(MC_DATUM);
  tft->setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft->setTextFont(1);
  tft->drawString("Open IP in Browser to download data", SCREEN_WIDTH / 2, 220);

  _ui->drawStatusBar();
}

void WebServerScreen::update() {
  UIManager::TouchPoint p = _ui->getTouchPoint();

  if (p.x != -1) {
    if (millis() - _lastTouchTime < 200)
      return;
    _lastTouchTime = millis();

    // Back Button
    if (p.x < 60 && p.y < 60) {
      _ui->switchScreen(SCREEN_SETTINGS); // Return to Settings
      return;
    }
  }

  // Refresh Status? (Connected clients count is not easily avail in basic
  // WiFiAP) So static draw is mostly fine. We re-draw Status Bar periodically
  if (millis() - _lastUpdate > 1000) {
    _ui->drawStatusBar();
    _lastUpdate = millis();
  }
}
