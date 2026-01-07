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
           unsigned long now = millis();
           if (_backTapCount == 0 || (now - _lastBackTap > 2000)) { // 2 Seconds timeout
               _backTapCount = 1;
               _lastBackTap = now;
               
               // Visual Warning
               TFT_eSPI *tft = _ui->getTft();
               tft->setTextSize(1);
               tft->setTextColor(TFT_RED, COLOR_BG);
               tft->setTextDatum(TL_DATUM);
               tft->drawString("Tap again", 30, 25);
           } else {
               _ui->switchScreen(SCREEN_SETTINGS);
               _backTapCount = 0;
               return;
           }
       }

       // Layout Constants Adjusted to prevent overlap with Header (at y=60)
       int cY = 160;  // Moved down from 140 to 160
       int boxH = 50; // Slightly shorter box (was 60)
       int arrowGap = 8; 
       int arrowH = 15;
       int boxW = 60; // Narrower (was 70)
       int gap = 30;
       
       int hCX = (SCREEN_WIDTH / 2) - (boxW / 2) - (gap / 2);
       int mCX = (SCREEN_WIDTH / 2) + (boxW / 2) + (gap / 2);
       
       int upYBot = cY - (boxH/2) - arrowGap;
       int upYTop = upYBot - arrowH;
       int dnYTop = cY + (boxH/2) + arrowGap;
       int dnYBot = dnYTop + arrowH;
       
       int hitPadding = 20; // Hit box padding

       // Hour UP
       if (p.x > hCX - 40 && p.x < hCX + 40 && p.y > upYTop - hitPadding && p.y < upYBot + hitPadding) {
           g_manualHour++;
           if (g_manualHour > 23) g_manualHour = 0;
           g_lastTimeUpdate = millis();
           drawScreen();
           return;
       }
       
       // Hour DOWN
       if (p.x > hCX - 40 && p.x < hCX + 40 && p.y > dnYTop - hitPadding && p.y < dnYBot + hitPadding) {
           g_manualHour--;
           if (g_manualHour < 0) g_manualHour = 23;
           g_lastTimeUpdate = millis();
           drawScreen();
           return;
       }
       
       // Minute UP
       if (p.x > mCX - 40 && p.x < mCX + 40 && p.y > upYTop - hitPadding && p.y < upYBot + hitPadding) {
           g_manualMinute++;
           if (g_manualMinute > 59) g_manualMinute = 0;
           g_lastTimeUpdate = millis();
           drawScreen();
           return;
       }
       
       // Minute DOWN
       if (p.x > mCX - 40 && p.x < mCX + 40 && p.y > dnYTop - hitPadding && p.y < dnYBot + hitPadding) {
           g_manualMinute--;
           if (g_manualMinute < 0) g_manualMinute = 59;
           g_lastTimeUpdate = millis();
           drawScreen();
           return;
       }
    }
  }
}

void TimeSettingScreen::drawScreen() {
  TFT_eSPI *tft = _ui->getTft();

  // Draw background to clear previous state (Excluding Status Bar & Header)
  // Status bar (0-20), Header (21-60). Content starts at 61.
  tft->fillRect(0, 61, SCREEN_WIDTH, SCREEN_HEIGHT - 61, COLOR_BG);

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
  tft->drawString("SET CLOCK", SCREEN_WIDTH / 2, 40);

  tft->drawFastHLine(0, 60, SCREEN_WIDTH, COLOR_SECONDARY);

  // Layout Constants (Must match update())
  int cY = 160; 
  int boxH = 50; 
  int arrowGap = 8; 
  int arrowH = 15;
  int boxW = 60;
  int gap = 30; 
  
  int hCX = (SCREEN_WIDTH / 2) - (boxW / 2) - (gap / 2);
  int mCX = (SCREEN_WIDTH / 2) + (boxW / 2) + (gap / 2);

  // Arrow Params
  int arrowW = 15; // Half width
  
  int upYBot = cY - (boxH/2) - arrowGap;
  int upYTop = upYBot - arrowH;
  int dnYTop = cY + (boxH/2) + arrowGap;
  int dnYBot = dnYTop + arrowH;

  // Draw Hours Column
  // Label at top
  tft->setTextSize(1);
  tft->setTextColor(COLOR_TEXT, COLOR_BG);
  tft->setTextDatum(BC_DATUM); // Bottom Center
  tft->drawString("HOURS", hCX, upYTop - 5);
  
  // Up Arrow
  tft->fillTriangle(hCX, upYTop, hCX - arrowW, upYBot, hCX + arrowW, upYBot, TFT_LIGHTGREY);
  
  // Box
  tft->drawRoundRect(hCX - (boxW/2), cY - (boxH/2), boxW, boxH, 5, TFT_LIGHTGREY);
  
  // Number
  tft->setTextFont(4); 
  tft->setTextSize(1);
  tft->setTextColor(COLOR_TEXT, COLOR_BG);
  tft->setTextDatum(MC_DATUM); // Middle Center for perfect alignment
  tft->drawNumber(g_manualHour, hCX, cY); 
  
  // Down Arrow
  tft->fillTriangle(hCX, dnYBot, hCX - arrowW, dnYTop, hCX + arrowW, dnYTop, TFT_LIGHTGREY);

  // Colon
  tft->setTextFont(4);
  tft->setTextColor(COLOR_TEXT, COLOR_BG);
  tft->setTextDatum(MC_DATUM);
  tft->drawString(":", SCREEN_WIDTH / 2, cY);

  // Draw Minutes Column
  // Label
  tft->setTextFont(1); // Reset font
  tft->setFreeFont(&Org_01);
  tft->setTextSize(1);
  tft->setTextDatum(BC_DATUM);
  tft->drawString("MINUTES", mCX, upYTop - 5);

  // Up Arrow
  tft->fillTriangle(mCX, upYTop, mCX - arrowW, upYBot, mCX + arrowW, upYBot, TFT_LIGHTGREY);
  
  // Box
  tft->drawRoundRect(mCX - (boxW/2), cY - (boxH/2), boxW, boxH, 5, TFT_LIGHTGREY);

  // Number
  tft->setTextFont(4); 
  tft->setTextDatum(MC_DATUM);
  
  char buf[4]; sprintf(buf, "%02d", g_manualMinute);
  tft->drawString(buf, mCX, cY);

  // Down Arrow
  tft->fillTriangle(mCX, dnYBot, mCX - arrowW, dnYTop, mCX + arrowW, dnYTop, TFT_LIGHTGREY);
  
  _ui->drawStatusBar();
}
