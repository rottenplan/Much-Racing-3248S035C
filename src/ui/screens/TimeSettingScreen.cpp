#include "TimeSettingScreen.h"
#include "../../config.h"
#include "../fonts/Org_01.h"

// Define specific global variables for Time
int g_manualHour = 12;
int g_manualMinute = 0;
unsigned long g_lastTimeUpdate = 0;

void TimeSettingScreen::onShow() {
  _selectedIdx = -1;
  TFT_eSPI *tft = _ui->getTft();
  // tft->fillScreen(COLOR_BG); // Already cleared by UIManager
  drawScreen();
}

void TimeSettingScreen::update() {
  static unsigned long lastTouch = 0;
  UIManager::TouchPoint p = _ui->getTouchPoint();

  // Time Increment Logic (Keep running here too to show live updates)
  if (g_lastTimeUpdate == 0) g_lastTimeUpdate = millis();
  if (millis() - g_lastTimeUpdate >= 60000) {
      g_manualMinute++;
      if (g_manualMinute > 59) {
          g_manualMinute = 0;
          g_manualHour++;
          if (g_manualHour > 23) g_manualHour = 0;
      }
      g_lastTimeUpdate = millis();
      drawScreen(); // Redraw automatically
  }

  if (p.x != -1) {
    if (millis() - lastTouch > 200) {
       lastTouch = millis();

       // 1. Back Button (Top Left)
       if (p.x < 60 && p.y < 60) {
           if (_selectedIdx == 0) {
               _ui->switchScreen(SCREEN_SETTINGS);
           } else {
               _selectedIdx = 0;
               drawScreen();
           }
           return;
       }

       // 2. Hour Button (Left Center)
       // Area approx: x=20-140, y=100-160 ?
       // Let's define zones based on drawing.
       // Hour: x=60, y=120 radius 40?
       if (p.x > 20 && p.x < 140 && p.y > 80 && p.y < 160) {
           g_manualHour++;
           if (g_manualHour > 23) g_manualHour = 0;
           g_lastTimeUpdate = millis(); // Reset seconds
           _selectedIdx = 1; // Highlight Hour?
           drawScreen();
           return;
       }

       // 3. Minute Button (Right Center)
       // Area approx: x=180-300, y=80-160
       if (p.x > 180 && p.x < 300 && p.y > 80 && p.y < 160) {
           g_manualMinute++;
           if (g_manualMinute > 59) g_manualMinute = 0;
           g_lastTimeUpdate = millis();
           _selectedIdx = 2; // Highlight Minute?
           drawScreen();
           return;
       }
       
       // Click outside -> Deselect
       if (_selectedIdx != -1) {
           _selectedIdx = -1;
           drawScreen();
       }
    }
  }
}

void TimeSettingScreen::drawScreen() {
  TFT_eSPI *tft = _ui->getTft();

  // Header
  // Back Arrow
  uint16_t backColor = (_selectedIdx == 0) ? COLOR_HIGHLIGHT : COLOR_TEXT;
  tft->setTextColor(backColor, COLOR_BG);
  tft->setTextDatum(TL_DATUM);
  tft->setFreeFont(&Org_01);
  tft->setTextSize(2);
  tft->drawString("<", 10, 25);

  // Title
  tft->setTextColor(COLOR_TEXT, COLOR_BG);
  tft->setTextDatum(TC_DATUM);
  tft->setTextSize(1);
  tft->setTextFont(2);
  tft->drawString("SET CLOCK", SCREEN_WIDTH / 2, 40);

  tft->drawFastHLine(0, 70, SCREEN_WIDTH, COLOR_SECONDARY);

  // Time Display (Big)
  int yCenter = 130;
  int xHour = 80;
  int xMin = 240;
  int radius = 45;

  // Hour Circle
  uint16_t hColor = (_selectedIdx == 1) ? COLOR_HIGHLIGHT : COLOR_SECONDARY;
  tft->fillCircle(xHour, yCenter, radius, hColor);
  
  tft->setTextDatum(MC_DATUM);
  tft->setTextSize(2);
  tft->setTextColor(COLOR_TEXT, hColor);
  tft->drawNumber(g_manualHour, xHour, yCenter);
  
  // Note
  tft->setTextSize(1);
  tft->drawString("HOUR", xHour, yCenter + radius + 15);

  // Separator
  tft->setTextColor(COLOR_TEXT, COLOR_BG);
  tft->setTextSize(2);
  tft->drawString(":", SCREEN_WIDTH / 2, yCenter);

  // Minute Circle
  uint16_t mColor = (_selectedIdx == 2) ? COLOR_HIGHLIGHT : COLOR_SECONDARY;
  tft->fillCircle(xMin, yCenter, radius, mColor);
  
  tft->setTextDatum(MC_DATUM);
  tft->setTextSize(2);
  tft->setTextColor(COLOR_TEXT, mColor);
  
  // Draw Minute with leading zero
  char buf[4]; sprintf(buf, "%02d", g_manualMinute);
  tft->drawString(buf, xMin, yCenter);

  tft->setTextSize(1);
  tft->drawString("MINUTE", xMin, yCenter + radius + 15);
}
