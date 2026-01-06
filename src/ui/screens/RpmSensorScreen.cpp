#include "RpmSensorScreen.h"
#include "../../config.h"
#include "../fonts/Org_01.h"
#include <Preferences.h>

// Define colors if not in config
#define COLOR_ORANGE 0xFDA0 
#define COLOR_GREEN  0x07E0

// Initialize static members
volatile unsigned long RpmSensorScreen::_rpmPulses = 0;
volatile unsigned long RpmSensorScreen::_lastPulseMicros = 0;

void IRAM_ATTR RpmSensorScreen::onPulse() {
    unsigned long now = micros();
    // Debounce: 1ms (1000us) dead time -> Max 60,000 RPM
    // Filters out high-frequency ringing from spark
    if (now - _lastPulseMicros > 1000) {
        _rpmPulses++;
        _lastPulseMicros = now;
    }
}

void RpmSensorScreen::onShow() {
  _currentRpm = 0;
  _maxRpm = 0;
  _currentLvl = 0;
  _graphIndex = 0;
  _lastUpdate = 0;
  
  // Initialize Pulse Counter
  _rpmPulses = 0;
  _lastRpmCalcTime = millis();
  
  // Setup Interrupt for Inductive Clamp
  // GPIO 35 is Input Only.
  pinMode(PIN_RPM_INPUT, INPUT); 
  attachInterrupt(digitalPinToInterrupt(PIN_RPM_INPUT), onPulse, FALLING);
  
  // Clear graph history
  for(int i=0; i<GRAPH_WIDTH; i++) _graphHistory[i] = 0;
  
  drawScreen();
}

void RpmSensorScreen::update() {
  // 1. Back Button
  UIManager::TouchPoint p = _ui->getTouchPoint();
  if (p.x != -1 && p.x < 60 && p.y < 60) {
      detachInterrupt(digitalPinToInterrupt(PIN_RPM_INPUT)); // Cleanup
      _ui->switchScreen(SCREEN_MENU);
      return;
  }
  
  // 2. Real RPM Calculation (Every 100ms)
  unsigned long now = millis();
  if (now - _lastRpmCalcTime > 100) {
      // safely read and reset pulses
      noInterrupts();
      unsigned long pulses = _rpmPulses;
      _rpmPulses = 0;
      interrupts();
      
      unsigned long dt = now - _lastRpmCalcTime;
      _lastRpmCalcTime = now;
      
      // Get Calibration from Preferences
      Preferences prefs;
      prefs.begin("laptimer", true);
      int pprIdx = prefs.getInt("rpm_ppr", 0);
      prefs.end();
      
      float ppr = 1.0;
      switch(pprIdx) {
          case 0: ppr = 1.0; break; // 1.0 Default
          case 1: ppr = 0.5; break; // 0.5 (1 spark / 2 revs)
          case 2: ppr = 2.0; break; // 2.0 (2 sparks / rev)
          case 3: ppr = 3.0; break;
          case 4: ppr = 4.0; break;
      }
      
      // Calculate RPM
      // Formula: RPM = (Pulses / PPR) * (60000 / dt)
      if (dt > 0) {
          // Use float for ppr division, cast back to ulong
          _currentRpm = (unsigned long)((pulses * 60000.0) / (dt * ppr));
      } else {
          _currentRpm = 0;
      }
      
      if (_currentRpm > _maxRpm) _maxRpm = _currentRpm;
      
      // Map to Bar Level (0-12000 RPM range?) Assuming 12k max for Kart/Bike
      int maxScale = 12000;
      _currentLvl = map(constrain(_currentRpm, 0, maxScale), 0, maxScale, 0, 100);
      
      // Update Graph Buffer
      _graphHistory[_graphIndex] = _currentLvl;
      _graphIndex = (_graphIndex + 1) % GRAPH_WIDTH; // 280
      
      // Redraw Dynamic Parts
      updateValues();
      drawGraphLine(); 
  }
}

void RpmSensorScreen::drawScreen() {
  TFT_eSPI *tft = _ui->getTft();
  tft->fillScreen(COLOR_BG);
  
  // Standard Header
  tft->setTextColor(COLOR_HIGHLIGHT, COLOR_BG);
  tft->setTextDatum(TL_DATUM);
  tft->setFreeFont(&Org_01);
  tft->setTextSize(2);
  tft->drawString("<", 10, 25);

  tft->setTextColor(COLOR_TEXT, COLOR_BG);
  tft->setTextDatum(TC_DATUM);
  tft->setTextSize(1);
  tft->setTextFont(2);
  tft->drawString("RPM SENSOR", SCREEN_WIDTH / 2, 40);

  tft->drawFastHLine(0, 60, SCREEN_WIDTH, COLOR_SECONDARY);
  
  // --- INFO BOX (Top) ---
  int boxY = 60; // Shifted down
  int boxH = 50; 
  int lblW = 80;
  
  // Border
  tft->drawRect(10, boxY, SCREEN_WIDTH - 20, boxH, COLOR_TEXT);
  tft->drawFastHLine(10, boxY + 25, SCREEN_WIDTH - 20, COLOR_TEXT); 
  
  // Labels (White BG, Black Text)
  tft->fillRect(11, boxY + 1, lblW, 24, COLOR_TEXT);      
  tft->fillRect(11, boxY + 26, lblW, 23, COLOR_TEXT);     
  
  tft->setTextColor(COLOR_BG); 
  tft->setTextSize(2);
  tft->setTextDatum(ML_DATUM);
  tft->drawString("MAX", 20, boxY + 13);
  tft->drawString("LVL", 20, boxY + 38);
  
  // --- GRAPH AREA ---
  drawGraphGrid();
  
  // --- BOTTOM BAR ---
  // Border
  tft->drawRect(10, 215, SCREEN_WIDTH - 20, 18, COLOR_TEXT);
}

void RpmSensorScreen::updateValues() {
    TFT_eSPI *tft = _ui->getTft();
    tft->setTextSize(2);
    tft->setTextDatum(MR_DATUM); 
    tft->setTextColor(COLOR_TEXT, COLOR_BG); 
    
    int valX = 10 + 80; 
    int w = SCREEN_WIDTH - 20 - 80 - 2;
    int boxY = 60; // Matched new boxY
    
    char buf[10];
    
    // MAX
    sprintf(buf, "%05d", _maxRpm);
    tft->fillRect(valX, boxY + 2, w, 22, COLOR_BG); 
    tft->drawString(buf, SCREEN_WIDTH - 20, boxY + 13);
    
    // LVL
    sprintf(buf, "%05d", _currentRpm);
    tft->fillRect(valX, boxY + 27, w, 22, COLOR_BG);
    tft->drawString(buf, SCREEN_WIDTH - 20, boxY + 38);
    
    // Update Bottom Bar
    int barW = map(_currentLvl, 0, 100, 0, SCREEN_WIDTH - 24);
    if (barW < 0) barW = 0;
    
    int barY = 217; 
    int barH = 14;
    
    // Clear remaining Part
    tft->fillRect(12 + barW, barY, (SCREEN_WIDTH - 24) - barW, barH, COLOR_BG);
    // Draw Bar
    tft->fillRect(12, barY, barW, barH, COLOR_GREEN);
}

void RpmSensorScreen::drawGraphGrid() {
     TFT_eSPI *tft = _ui->getTft();
     int gY = 120; // Shifted Down
     int gH = 85;  // Restore height
     
     // Box
     tft->drawRect(10, gY, SCREEN_WIDTH - 20, gH, COLOR_TEXT);
     
     // Grid Lines (Dotted)
     // Changed to 20% steps, skipping 100 as requested
     int levels[] = {80, 60, 40, 20};
     int numLevels = 4;
     
     tft->setFreeFont(NULL); // Use Standard GLCD Font for cleaner small text
     tft->setTextSize(1);
     tft->setTextColor(COLOR_TEXT);
     tft->setTextDatum(MR_DATUM); // Middle Right
     
     for (int i=0; i<numLevels; i++) {
         int lvl = levels[i];
         int lineY = gY + gH - (int)((lvl / 100.0) * gH);
         
         // Draw Label (Right aligned at x=35)
         tft->drawString(String(lvl), 35, lineY);
         
         // Draw Dotted Line (Skip if it's the top border 100%)
         if (lvl < 100) {
             for (int x = 40; x < SCREEN_WIDTH - 20; x += 4) {
                 tft->drawPixel(x, lineY, COLOR_SECONDARY);
             }
         }
     }
}

void RpmSensorScreen::drawGraphLine() {
    TFT_eSPI *tft = _ui->getTft();
    int gX = 40;
    int gY = 120; // Match Grid
    int gW = SCREEN_WIDTH - 20 - 30; 
    int gH = 85;
    
    tft->fillRect(gX, gY + 1, gW - 1, gH - 2, COLOR_BG); 
    
    int startIdx = _graphIndex; 
    
    for (int i = 0; i < gW; i++) {
        int idx = (startIdx + i) % 280;
        int val = _graphHistory[idx];
        if (i >= 280) break;
        
        int py = gY + gH - (int)((val / 100.0) * gH);
        if (py < gY + 1) py = gY + 1;
        if (py > gY + gH - 2) py = gY + gH - 2;
        
        tft->drawPixel(gX + i, py, COLOR_GREEN); 
    }
}
