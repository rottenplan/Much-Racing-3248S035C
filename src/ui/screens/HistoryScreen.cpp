#include "HistoryScreen.h"
#include "../../config.h"
#include "../../core/SessionManager.h"
#include "../fonts/Org_01.h"

extern SessionManager sessionManager;

void HistoryScreen::onShow() {
  _scrollOffset = 0;
  _lastScrollOffset = -1;
  _currentMode = MODE_MENU; 
  _selectedIdx = -1;
  _lastSelectedIdx = -3;
  _lastTapIdx = -1;
  scanHistory(); 

  TFT_eSPI *tft = _ui->getTft();
  tft->fillScreen(COLOR_BG);
  _ui->drawStatusBar(true);
  drawMenu(true);
}

void HistoryScreen::update() {
  static unsigned long lastHistoryTouch = 0;
  UIManager::TouchPoint p = _ui->getTouchPoint();
  if (p.x == -1)
    return;
    
  // Debounce global
  if (millis() - lastHistoryTouch < 200) return;

  // --- MENU MODE ---
  if (_currentMode == MODE_MENU) {
      if (p.x < 60 && p.y < 60) {
          // Back to Main Menu
          lastHistoryTouch = millis();
          _ui->switchScreen(SCREEN_MENU);
          return;
      }
      
      // Menu Items (Simple Layout)
      // Button Layout (Match LapTimer)
      int startY = 80;
      int btnHeight = 40;
      int gap = 20;
      int btnWidth = 240;
      int x = (SCREEN_WIDTH - btnWidth) / 2;
      
      int touchedIdx = -1;
      for(int i=0; i<2; i++) {
          int y = startY + i * (btnHeight + gap);
          if (p.x > x && p.x < x + btnWidth && p.y > y && p.y < y + btnHeight) {
              touchedIdx = i;
              break;
          }
      }
      
      if (touchedIdx != -1) {
          // Debounce
          if (millis() - lastHistoryTouch < 200) return;
          lastHistoryTouch = millis();
          
          unsigned long now = millis();
          if (_lastTapIdx == touchedIdx && (now - _lastTapTime < 500)) {
              // Confirmed Double Tap
              _lastTapIdx = -1;
              
              if (touchedIdx == 0) {
                  _selectedType = "TRACK";
              } else {
                  _selectedType = "DRAG";
              }
              _currentMode = MODE_LIST;
              _scrollOffset = 0;
              _selectedIdx = -1; 
              _lastSelectedIdx = -3;
              _ui->getTft()->fillScreen(COLOR_BG);
              _ui->drawStatusBar(true);
              drawList(true);
          } else {
              // First Tap
              _lastTapIdx = touchedIdx;
              _lastTapTime = now;
              
              if (_selectedIdx != touchedIdx) {
                  _lastSelectedIdx = _selectedIdx;
                  _selectedIdx = touchedIdx;
                  drawMenu(false);
              }
          }
      }
      return;
  }
  
  // --- LIST MODE ---
  if (_currentMode == MODE_LIST) {
      // Back Button
      if (p.x < 60 && p.y < 60) {
          _selectedIdx = -1;
          _lastSelectedIdx = -3;
          _ui->getTft()->fillScreen(COLOR_BG);
          _ui->drawStatusBar(true);
          drawMenu(true);
          lastHistoryTouch = millis();
          return;
      }
      
      // List Items (Y > 70)
      int listY = 70;
      int itemH = 40;
      
      if (p.y > listY) {
          int visibleIdx = (p.y - listY) / itemH;
          // We need to map visible index to actual index in filtered list
          // This matches how we draw it
          
          int actualIdx = -1;
          int count = 0;
          for(int i = 0; i < _historyList.size(); i++) {
              if (_historyList[i].type == _selectedType) {
                  if (count == visibleIdx + _scrollOffset) {
                      actualIdx = i;
                      break;
                  }
                  count++;
              }
          }
          
          if (actualIdx != -1) {
             unsigned long now = millis();
             if (_lastTapIdx == actualIdx && (now - _lastTapTime < 500)) {
                 // Confirmed Double Tap
                 _lastTapIdx = -1;
                 
                  // Open Details
                  _currentMode = MODE_DETAILS;
                  _ui->getTft()->fillScreen(COLOR_BG);
                  drawDetails(_selectedIdx);
             } else {
                 // First Tap
                 _lastTapIdx = actualIdx;
                 _lastTapTime = now;
                 
                 if (_selectedIdx != actualIdx) {
                     _lastSelectedIdx = _selectedIdx; _selectedIdx = actualIdx;
                     drawList(false);
                 }
             }
             lastHistoryTouch = now;
          }
      }
      return;
  }

  // --- DETAILS MODE ---
  if (_currentMode == MODE_DETAILS) {
      if (p.y < 40) { // Back header
          _currentMode = MODE_LIST;
          _ui->getTft()->fillScreen(COLOR_BG);
          drawList(true);
          lastHistoryTouch = millis();
          return;
      }
  }

  // Status Bar Update loop (if needed)
  static unsigned long lastStatusUpdate = 0;
  if (millis() - lastStatusUpdate > 1000) {
    _ui->drawStatusBar();
    lastStatusUpdate = millis();
  }
}

void HistoryScreen::scanHistory() {
  _historyList.clear();
  String content = sessionManager.loadHistoryIndex();

  // Uraikan CSV: nama file,tanggal,lap,lap terbaik,TIPE(opt)
  int start = 0;
  while (start < content.length()) {
    int end = content.indexOf('\n', start);
    if (end == -1)
      end = content.length();

    String line = content.substring(start, end);
    line.trim();

    if (line.length() > 0) {
      // Pisahkan koma
      int c1 = line.indexOf(',');
      int c2 = line.indexOf(',', c1 + 1);
      int c3 = line.indexOf(',', c2 + 1);
      int c4 = line.indexOf(',', c3 + 1); // Type separator

      if (c1 > 0 && c2 > 0 && c3 > 0) {
        HistoryItem item;
        item.filename = line.substring(0, c1);
        item.date = line.substring(c1 + 1, c2);
        item.laps = line.substring(c2 + 1, c3).toInt();
        item.bestLap = line.substring(c3 + 1, (c4 > 0) ? c4 : line.length()).toInt(); 
        
        if (c4 > 0) {
            item.type = line.substring(c4 + 1);
            item.type.trim();
        } else {
            item.type = "TRACK"; // Default backward compatibility
        }
        
        _historyList.insert(_historyList.begin(),
                            item); // Tambahkan di awal (Terbaru dulu)
      }
    }
    start = end + 1;
  }
}

void HistoryScreen::drawMenu(bool force) {
    TFT_eSPI *tft = _ui->getTft();
    
    if (force) {
        // Header
        tft->setTextColor(COLOR_TEXT, COLOR_BG);
        tft->setTextDatum(TL_DATUM);
        tft->setFreeFont(&Org_01);
        tft->setTextSize(2);
        tft->drawString("<", 10, 25);
        
        tft->setTextDatum(TC_DATUM);
        tft->setFreeFont(&Org_01); 
        tft->setTextSize(2);       
        tft->drawString("HISTORY MENU", SCREEN_WIDTH/2, 40);
        
        tft->drawFastHLine(0, 60, SCREEN_WIDTH, COLOR_SECONDARY);
    }
    
    int startY = 80;
    int btnHeight = 40;
    int gap = 20;
    int btnWidth = 240;
    int x = (SCREEN_WIDTH - btnWidth) / 2;
    
    const char* items[] = {"TRACK HISTORY", "DRAG HISTORY"};
    
    for (int i = 0; i < 2; i++) {
        if (force || i == _selectedIdx || i == _lastSelectedIdx) {
            int y = startY + i * (btnHeight + gap);
            uint16_t btnColor = (i == _selectedIdx) ? TFT_RED : TFT_DARKGREY;
            tft->fillRoundRect(x, y, btnWidth, btnHeight, 5, btnColor);
            tft->setTextColor(TFT_WHITE, btnColor);
            tft->setTextDatum(MC_DATUM);
            tft->drawString(items[i], SCREEN_WIDTH / 2, y + btnHeight/2 + 2);
        }
    }
    _lastSelectedIdx = _selectedIdx;
}

void HistoryScreen::drawList(bool force) {
  TFT_eSPI *tft = _ui->getTft();
  
  if (force || _scrollOffset != _lastScrollOffset) {
      _ui->drawStatusBar(true); 
      tft->drawFastHLine(0, 20, SCREEN_WIDTH, TFT_WHITE);
      
      tft->setTextColor(TFT_WHITE, TFT_BLACK);
      tft->setTextDatum(TC_DATUM);
      tft->setFreeFont(&Org_01);
      tft->setTextSize(1);
      tft->drawString(_selectedType + " History", SCREEN_WIDTH/2, 25);
      
      tft->drawFastHLine(0, 45, SCREEN_WIDTH, TFT_WHITE);
  }

  int startY = 50;
  int itemH = 30; 
  int count = 0;
  int skip = 0;

  for (int i = 0; i < _historyList.size(); i++) {
     if (count >= 5) break; 
     if (_historyList[i].type != _selectedType) continue;
     
     if (skip < _scrollOffset) {
         skip++;
         continue;
     }

    if (force || _scrollOffset != _lastScrollOffset || i == _selectedIdx || i == _lastSelectedIdx) {
        HistoryItem &item = _historyList[i];
        int y = startY + (count * itemH);

        uint16_t bg = (i == _selectedIdx) ? TFT_WHITE : TFT_BLACK;
        uint16_t fg = (i == _selectedIdx) ? TFT_BLACK : TFT_WHITE;
        
        tft->fillRect(0, y, SCREEN_WIDTH, itemH, bg);
        tft->setTextColor(fg, bg);
        tft->setTextDatum(TL_DATUM);
        tft->setTextFont(2);
        tft->setTextSize(1);
        
        String leftText = item.date.substring(0, 10) + " - " + String(item.laps) + " Laps";
        tft->drawString(leftText, 5, y + 8);

        tft->setTextDatum(TR_DATUM);
        int ms = item.bestLap % 1000;
        int s = (item.bestLap / 1000) % 60;
        int m = (item.bestLap / 60000);
        char buf[16];
        sprintf(buf, "%d:%02d.%02d", m, s, ms / 10);
        tft->drawString(buf, SCREEN_WIDTH - 5, y + 8);
    }
    count++;
  }

  if (force && count == 0 && skip == 0) {
    tft->setTextDatum(MC_DATUM);
    tft->setTextColor(TFT_WHITE, TFT_BLACK);
    tft->drawString("No Sessions Found", SCREEN_WIDTH / 2, 120);
  }
  _lastSelectedIdx = _selectedIdx;
  _lastScrollOffset = _scrollOffset;
}

void HistoryScreen::drawDetails(int idx) {
  if (idx < 0 || idx >= _historyList.size())
    return;
  HistoryItem &item = _historyList[idx];

  TFT_eSPI *tft = _ui->getTft();
  tft->fillScreen(TFT_BLACK);
  
  // 1. Header Line (Track Name + Session ID)
  // Use Filename as Track Name proxy for now
  String trackName = item.filename;
  if(trackName.length() == 0) trackName = "Track Session";
  
  tft->setTextColor(TFT_WHITE, TFT_BLACK);
  tft->setTextDatum(TL_DATUM);
  tft->setFreeFont(&Org_01);
  tft->setTextSize(1); 
  tft->drawString(trackName, 5, 25);
  
  tft->setTextDatum(TR_DATUM);
  tft->drawString("Session " + String(idx + 1), SCREEN_WIDTH - 5, 25);
  
  // 2. Sub Header (Laps + Date)
  tft->setTextDatum(TL_DATUM);
  tft->drawString(String(item.laps) + " Laps", 5, 45);
  
  tft->setTextDatum(TR_DATUM);
  tft->drawString(item.date, SCREEN_WIDTH - 5, 45);
  
  tft->drawFastHLine(0, 60, SCREEN_WIDTH, TFT_WHITE);
  
  // 3. Stats Grid
  int y = 70;
  int rowH = 30;
  
  // Helper to draw row
  auto drawRow = [&](String label, String value, int yPos) {
      tft->setTextDatum(TL_DATUM);
      tft->setTextColor(TFT_WHITE, TFT_BLACK); // White for label? Image has white labels
      tft->drawString(label, 5, yPos);
      
      tft->setTextDatum(TR_DATUM);
      tft->setFreeFont(&Org_01); // Or bold if available
      tft->drawString(value, SCREEN_WIDTH - 5, yPos);
  };
  
  // Best Lap
  int ms = item.bestLap % 1000;
  int s = (item.bestLap / 1000) % 60;
  int m = (item.bestLap / 60000);
  char buf[16];
  sprintf(buf, "%d:%02d.%02d", m, s, ms / 10);
  drawRow("Best Lap", String(buf), y);
  
  // Max Speed (Placeholder)
  y += rowH;
  drawRow("Max Speed", "-- km/h", y); // Need full log parsing for this
  
  // Max G (Placeholder)
  y += rowH;
  drawRow("Max G-Force", "-- G", y);
  
  // Theoretical (Placeholder)
  y += rowH;
  drawRow("Theoretical", String(buf), y); // Mock as best for now
  
  _ui->drawStatusBar();
}
