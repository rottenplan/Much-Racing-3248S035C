#include "HistoryScreen.h"
#include "../../config.h"
#include "../../core/SessionManager.h"

extern SessionManager sessionManager;

void HistoryScreen::onShow() {
  _scrollOffset = 0;
  _showingDetails = false;
  _selectedIdx = -1;
  scanHistory();

  TFT_eSPI *tft = _ui->getTft();
  tft->fillScreen(COLOR_BG);
  _ui->drawStatusBar(); // Draw Status Bar
  drawList(0);
}

void HistoryScreen::update() {
  UIManager::TouchPoint p = _ui->getTouchPoint();
  if (p.x == -1)
    return;

  if (_showingDetails) {
    // Back from Details (Header Area 0-40 usually, but let's check drawDetails)
    // drawDetails header is 0-40. So <40 is correct there.
    if (p.y < 40) {
      _showingDetails = false;
      _ui->getTft()->fillScreen(COLOR_BG);
      drawList(_scrollOffset);
      return;
    }
  } else {
    // List View (Header Area 20-60)
    // Back Button
    if (p.x < 60 && p.y < 60) {
      _ui->switchScreen(SCREEN_MENU);
      return;
    }

    // Scroll/Select
    // List area: Y=70 to 240
    // Items are ~40px high.
    int listY = 70;
    int itemH = 40;

    // Check scroll buttons (if we add them) or simple touch zones
    // Simple Touch-to-Select for now
    if (p.y > listY) {
      int clickedIdx = _scrollOffset + ((p.y - listY) / itemH);
      if (clickedIdx < _historyList.size()) {
        _selectedIdx = clickedIdx;
        _showingDetails = true;
        _ui->getTft()->fillScreen(COLOR_BG);
        drawDetails(_selectedIdx);
      }
    }
  }

  // Update Status Bar
  static unsigned long lastStatusUpdate = 0;
  if (millis() - lastStatusUpdate > 1000) {
    _ui->drawStatusBar();
    lastStatusUpdate = millis();
  }
}

void HistoryScreen::scanHistory() {
  _historyList.clear();
  String content = sessionManager.loadHistoryIndex();

  // Parse CSV: filename,date,laps,bestLap
  int start = 0;
  while (start < content.length()) {
    int end = content.indexOf('\n', start);
    if (end == -1)
      end = content.length();

    String line = content.substring(start, end);
    line.trim();

    if (line.length() > 0) {
      // Split commas
      int c1 = line.indexOf(',');
      int c2 = line.indexOf(',', c1 + 1);
      int c3 = line.indexOf(',', c2 + 1);

      if (c1 > 0 && c2 > 0 && c3 > 0) {
        HistoryItem item;
        item.filename = line.substring(0, c1);
        item.date = line.substring(c1 + 1, c2);
        item.laps = line.substring(c2 + 1, c3).toInt();
        item.bestLap = line.substring(c3 + 1).toInt(); // Assuming millis
        _historyList.insert(_historyList.begin(),
                            item); // Prepend (Newest first)
      }
    }
    start = end + 1;
  }
}

void HistoryScreen::drawList(int scrollOffset) {
  TFT_eSPI *tft = _ui->getTft();

  // Header
  // Status Bar 0-20, Header 20-60
  tft->fillRect(0, 20, SCREEN_WIDTH, 40, COLOR_SECONDARY);
  tft->setTextColor(COLOR_TEXT, COLOR_SECONDARY);
  tft->setTextDatum(MC_DATUM);
  tft->setTextFont(2);
  tft->setTextSize(1);
  tft->drawString("HISTORY", SCREEN_WIDTH / 2, 40); // Centered at Y=40
  tft->drawString("<", 15, 40);
  tft->drawFastHLine(0, 60, SCREEN_WIDTH, COLOR_SECONDARY); // Header Separator

  // List
  int startY = 70; // 60 (Header End) + 10 margin
  int itemH = 40;
  int count = 0;

  for (int i = scrollOffset; i < _historyList.size(); i++) {
    if (count >= 4)
      break; // Show 4 items

    HistoryItem &item = _historyList[i];
    int y = startY + (count * itemH);

    // Background for item (alternating?)
    if (count % 2 == 0)
      tft->fillRect(0, y, SCREEN_WIDTH, itemH, 0x10A2); // Dark Grey

    // Date (Top Left)
    tft->setTextColor(COLOR_TEXT); // Transparent bg
    tft->setTextDatum(TL_DATUM);
    tft->setTextFont(2);
    tft->setTextSize(1);
    tft->drawString(item.date, 10, y + 2);

    // Stats (Bottom Left/Right)
    tft->setTextSize(1);
    tft->setTextFont(2); // Keep font 2
    tft->setTextColor(COLOR_SECONDARY);
    String sub = "Laps: " + String(item.laps);
    tft->drawString(sub, 10, y + 22);

    // Best Lap (Right)
    tft->setTextDatum(TR_DATUM);
    tft->setTextColor(COLOR_ACCENT); // Green/Teal
    tft->setTextFont(2);
    tft->setTextSize(1);

    int ms = item.bestLap % 1000;
    int s = (item.bestLap / 1000) % 60;
    int m = (item.bestLap / 60000);
    char buf[16];
    sprintf(buf, "%02d:%02d.%02d", m, s, ms / 10);

    tft->drawString(buf, SCREEN_WIDTH - 10, y + 8);

    count++;
  }

  if (_historyList.size() == 0) {
    tft->setTextDatum(MC_DATUM);
    tft->setTextColor(COLOR_SECONDARY, COLOR_BG);
    tft->drawString("No Sessions Found", SCREEN_WIDTH / 2, 120);
  }
}

void HistoryScreen::drawDetails(int idx) {
  if (idx < 0 || idx >= _historyList.size())
    return;
  HistoryItem &item = _historyList[idx];

  TFT_eSPI *tft = _ui->getTft();

  // Header
  tft->fillRect(0, 0, SCREEN_WIDTH, 40, COLOR_SECONDARY);
  tft->setTextColor(COLOR_TEXT, COLOR_SECONDARY);
  tft->setTextDatum(MC_DATUM);
  tft->setTextFont(2);
  tft->setTextSize(1);
  tft->drawString("SESSION DETAILS", SCREEN_WIDTH / 2, 20);
  tft->drawString("<", 15, 20);

  // Info
  int y = 60;
  tft->setTextDatum(TL_DATUM);
  tft->setTextColor(COLOR_TEXT, COLOR_BG);

  tft->drawString("Date: " + item.date, 20, y);
  y += 30;
  tft->drawString("Total Laps: " + String(item.laps), 20, y);
  y += 30;

  // Best Lap
  tft->drawString("Best Lap:", 20, y);
  tft->setTextColor(TFT_GREEN, COLOR_BG);

  int ms = item.bestLap % 1000;
  int s = (item.bestLap / 1000) % 60;
  int m = (item.bestLap / 60000);
  char buf[16];
  sprintf(buf, "%02d:%02d.%02d", m, s, ms / 10);
  tft->setTextSize(3);
  tft->drawString(buf, 20, y + 25);

  // Note: Parsing the actual CSV for full lap list is complex.
  // For now, this summary view matches user request "like RaceBox" history
  // list.
}
