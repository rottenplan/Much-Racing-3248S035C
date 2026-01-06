#include "DragMeterScreen.h"
#include "../../core/GPSManager.h"
#include "../../core/SessionManager.h"
#include "../fonts/Org_01.h"

extern GPSManager gpsManager;

// Tentukan subset disiplin untuk dilacak
// 0-60 km/h
// 0-100 km/h
// 100-200 km/h
// 402m (1/4 Mile)

// 0-100 km/h
// 100-200 km/h
// 402m (1/4 Mile)

void DragMeterScreen::onShow() {
  _state = STATE_MENU;
  _selectedBtn = -1;
  _selectedMenuIdx = -1;
  _selectedDragModeIdx = -1;
  _menuItems = {"DRAG MODE", "DRAG SCREEN", "PREDICTIVE", "SUMMARY"};
  _dragModeItems = {"SPEED", "DISTANCE", "CUSTOM"};
  
  TFT_eSPI *tft = _ui->getTft();
  tft->fillScreen(COLOR_BG);
  drawDashboardStatic();
}

void DragMeterScreen::update() {
  static unsigned long lastDragTouch = 0;
  UIManager::TouchPoint p = _ui->getTouchPoint();
  if (p.x != -1) {
     if (millis() - lastDragTouch > 200) {
        lastDragTouch = millis();

         // 1. Tombol Kembali (Top Left)
        if (p.x < 60 && p.y < 60) {
           if (_state == STATE_MENU) {
               _ui->switchScreen(SCREEN_MENU);
           } else if (_state == STATE_DRAG_MODE_MENU) {
               _state = STATE_MENU;
               _selectedDragModeIdx = -1; // Reset selection
               _ui->getTft()->fillScreen(COLOR_BG); // Clear entire screen
               drawMenu();
           } else {
               // If in running mode, go back to menu
               _state = STATE_MENU;
               _ui->getTft()->fillScreen(COLOR_BG);
               drawDashboardStatic();
           }
           return;
        }
        
        // 2. Menu Logic
        if (_state == STATE_MENU) {
            int startY = 55;
            int btnHeight = 35;
            int btnWidth = 240;
            int gap = 8;
            int x = (SCREEN_WIDTH - btnWidth) / 2;
            
            // Check if X is within button width (centered)
            if (p.x > x && p.x < x + btnWidth) {
                int touchedIdx = -1;
                // Check Y coordinates
                for (int i = 0; i < _menuItems.size(); i++) {
                    int btnY = startY + (i * (btnHeight + gap));
                    if (p.y > btnY && p.y < btnY + btnHeight) {
                        touchedIdx = i;
                        break;
                    }
                }
                
                if (touchedIdx != -1) {
                    if (_selectedMenuIdx == touchedIdx) {
                        // Second tap: Execute
                        handleMenuTouch(touchedIdx);
                    } else {
                        // First tap: Select/Highlight
                        _selectedMenuIdx = touchedIdx;
                        drawMenu();
                    }
                }
            }
        } else if (_state == STATE_DRAG_MODE_MENU) {
            // Drag Mode Menu Logic (Similar to Main Menu)
            int startY = 55;
            int btnHeight = 35;
            int btnWidth = 240;
            int gap = 8;
            int x = (SCREEN_WIDTH - btnWidth) / 2;
            
            if (p.x > x && p.x < x + btnWidth) {
                int touchedIdx = -1;
                for (int i = 0; i < _dragModeItems.size(); i++) {
                    int btnY = startY + (i * (btnHeight + gap));
                    if (p.y > btnY && p.y < btnY + btnHeight) {
                        touchedIdx = i;
                        break;
                    }
                }
                
                if (touchedIdx != -1) {
                    if (_selectedDragModeIdx == touchedIdx) {
                        handleDragModeTouch(touchedIdx);
                    } else {
                        _selectedDragModeIdx = touchedIdx;
                        drawDragModeMenu();
                    }
                }
            }
        }
     }
  }
}

void DragMeterScreen::checkStartCondition() {
}

void DragMeterScreen::checkStopCondition() {
}

void DragMeterScreen::updateDisciplines() {
}

void DragMeterScreen::handleMenuTouch(int idx) {
    if (idx < 0 || idx >= _menuItems.size()) return;
    
    String &item = _menuItems[idx];
    if (item == "DRAG SCREEN") {
        _state = STATE_RUNNING;
        _ui->getTft()->fillScreen(COLOR_BG);
        drawDashboardStatic();
    } else if (item == "DRAG MODE") {
        _state = STATE_DRAG_MODE_MENU;
        _selectedDragModeIdx = -1;
        _ui->getTft()->fillScreen(COLOR_BG);
        drawDragModeMenu();
    }
    // Other items are placeholders for now
}

void DragMeterScreen::handleDragModeTouch(int idx) {
    // Placeholder for Drag Mode logic
    // For now, just maybe highlight or go back?
    // Let's assume we select it and go back to menu with selection?
    // Or just stay here. User asked for sub-menu presence.
    
    // Example: Go back to menu after selection
    // _state = STATE_MENU;
    // drawMenu();
}

void DragMeterScreen::drawDashboardStatic() {
  TFT_eSPI *tft = _ui->getTft();

  // Common Header (Back Arrow)
  tft->setTextColor(COLOR_HIGHLIGHT, COLOR_BG);
  tft->setTextDatum(TL_DATUM);
  tft->setFreeFont(&Org_01);
  tft->setTextSize(2);
  tft->drawString("<", 10, 25);
  
  if (_state == STATE_MENU) {
      drawMenu();
  } else if (_state == STATE_DRAG_MODE_MENU) {
      drawDragModeMenu();
  } else {
      // Draw Running View (Back arrow already drawn)
      // Placeholder for actual gauges
      tft->setTextSize(1);
      tft->setTextFont(2);
      tft->drawString("RACE MODE", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
  }
  _ui->drawStatusBar();
}

void DragMeterScreen::drawMenu() {
    TFT_eSPI *tft = _ui->getTft();
    
    // Title
    tft->setTextColor(COLOR_TEXT, COLOR_BG);
    tft->setTextDatum(TC_DATUM);
    tft->setFreeFont(&Org_01);
    tft->setTextSize(2);
    tft->drawString("DRAG MENU", SCREEN_WIDTH / 2, 25);

    // Back Button
    tft->setTextColor(COLOR_HIGHLIGHT, COLOR_BG);
    tft->setTextDatum(TL_DATUM);
    tft->drawString("<", 10, 25);
    
    tft->drawFastHLine(0, 45, SCREEN_WIDTH, COLOR_SECONDARY);
    
    int startY = 55;
    int btnHeight = 35; 
    int btnWidth = 240;
    int gap = 8;
    int x = (SCREEN_WIDTH - btnWidth) / 2;
    
    for (int i = 0; i < _menuItems.size(); i++) {
        int y = startY + (i * (btnHeight + gap));
        
        // Determine Color based on selection
        uint16_t btnColor = (i == _selectedMenuIdx) ? TFT_RED : TFT_DARKGREY;
        
        tft->fillRoundRect(x, y, btnWidth, btnHeight, 5, btnColor);
        tft->setTextColor(TFT_WHITE, btnColor);
        tft->setTextDatum(MC_DATUM);
        tft->setFreeFont(&Org_01);
        tft->setTextSize(2); // Using Org_01 size 2 to match LapTimer
        tft->drawString(_menuItems[i], SCREEN_WIDTH / 2, y + btnHeight/2 + 2);
    }
    _ui->drawStatusBar();
}

void DragMeterScreen::drawDragModeMenu() {
    TFT_eSPI *tft = _ui->getTft();
    
    // Title
    tft->setTextColor(COLOR_TEXT, COLOR_BG);
    tft->setTextDatum(TC_DATUM);
    tft->setFreeFont(&Org_01);
    tft->setTextSize(2);
    tft->drawString("DRAG MODE", SCREEN_WIDTH / 2, 25);

    // Back Button
    tft->setTextColor(COLOR_HIGHLIGHT, COLOR_BG);
    tft->setTextDatum(TL_DATUM);
    tft->drawString("<", 10, 25);
    
    tft->drawFastHLine(0, 45, SCREEN_WIDTH, COLOR_SECONDARY);
    
    int startY = 55;
    int btnHeight = 35; 
    int btnWidth = 240;
    int gap = 8;
    int x = (SCREEN_WIDTH - btnWidth) / 2;
    
    for (int i = 0; i < _dragModeItems.size(); i++) {
        int y = startY + (i * (btnHeight + gap));
        
        // Determine Color based on selection
        uint16_t btnColor = (i == _selectedDragModeIdx) ? TFT_RED : TFT_DARKGREY;
        
        tft->fillRoundRect(x, y, btnWidth, btnHeight, 5, btnColor);
        tft->setTextColor(TFT_WHITE, btnColor);
        tft->setTextDatum(MC_DATUM);
        tft->setFreeFont(&Org_01);
        tft->setTextSize(2);
        tft->drawString(_dragModeItems[i], SCREEN_WIDTH / 2, y + btnHeight/2 + 2);
    }
    _ui->drawStatusBar();
}

