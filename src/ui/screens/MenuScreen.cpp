#include "MenuScreen.h"
#include "../../core/GPSManager.h"

// extern GPSManager gpsManager; // If needed

#include <stdlib.h>

#define MENU_ITEMS 8
const char *menuLabels[MENU_ITEMS] = {"LAP TIMER",   "DRAG METER", "RPM SENSOR",
                                      "SPEEDOMETER", "HISTORY",    "GPS STATUS",
                                      "SETTINGS",    "SYNCHRONIZE"};

void MenuScreen::onShow() {
  _selectedIndex = -1;
  _lastSelectedIndex = -1;
  _lastTouchTime = 0;
  _currentPage = 0; // Start at Page 0
  _touchStartY = -1;
  _lastTapIdx = -1;
  _lastTapTime = 0;
  drawMenu(true);
}

void MenuScreen::update() {
  bool enter = false;
  UIManager::TouchPoint p = _ui->getTouchPoint();

  // Layout Constants
  int startY = 80;
  int gap = 38;
  int itemHeight = 30;

  if (p.x != -1) { // Touched
    if (_touchStartY == -1) {
      _touchStartY = p.y;
    } else {
      // Just tracking for swipe on release or continuous?
      // Let's do "Swipe to Switch" - Trigger on Release or Threshold
    }

    // Check for Swipe vs Tap
    // We handle logic slightly differently:
    // If finger moves significantly -> Swipe Page
    // If finger stays -> Tap Item
  } else {
    // Touch Released
    _touchStartY = -1;
  }

  // Re-implementing Swipe/Tap Logic inside the "Touched" block properly
  if (p.x != -1) {
    if (_touchStartY == -1)
      _touchStartY = p.y;

    // Calculate Delta from Start
    int deltaY = _touchStartY - p.y;

    if (abs(deltaY) > 40) { // Significant Swipe
      // Debounce Page Switch
      static unsigned long lastPageSwitch = 0;
      if (millis() - lastPageSwitch > 300) {
        if (deltaY > 0) { // Swipe Up -> Next Page
          int maxPage = (MENU_ITEMS - 1) / ITEMS_PER_PAGE;
          if (_currentPage < maxPage) {
            _currentPage++;
            drawMenu(true);
            lastPageSwitch = millis();
            _touchStartY = p.y; // Reset anchor
          }
        } else { // Swipe Down -> Prev Page
          if (_currentPage > 0) {
            _currentPage--;
            drawMenu(true);
            lastPageSwitch = millis();
            _touchStartY = p.y;
          }
        }
      }
    }
    // Tap Detection
    else {
      static unsigned long lastPageSwitch = 0;

      // 1. Check Navigation Arrow (Bottom Center)
      int maxPage = (MENU_ITEMS - 1) / ITEMS_PER_PAGE;

      // Toggle Button Area (Y > 210)
      if (p.y > 210) {
        if (millis() - lastPageSwitch > 300) {
          if (_currentPage < maxPage) {
            _currentPage++; // Go Down/Next
          } else if (_currentPage > 0) {
            _currentPage--; // Go Up/Prev
          }
          drawMenu(true);
          lastPageSwitch = millis();
        }
        return; // Skip item tap
      }

      // Removed Top Arrow Logic as we now use a single toggle button at the
      // bottom

      // 2. Item Tap Detection
      int localIndex = -1;
      for (int i = 0; i < ITEMS_PER_PAGE; i++) {
        int yTop = startY + (i * gap) - 5;
        int yBot = yTop + itemHeight;
        if (p.y >= yTop && p.y <= yBot) {
          localIndex = i;
          break;
        }
      }

      if (localIndex != -1) {
        int actualIndex = (_currentPage * ITEMS_PER_PAGE) + localIndex;
        if (actualIndex < MENU_ITEMS) {
          if (millis() - _lastTouchTime > 200) { // Debounce
            // Check Double Tap
            if (_lastTapIdx == actualIndex && (millis() - _lastTapTime < 500)) {
              enter = true;
              _lastTapIdx = -1;
            } else {
              _lastTapIdx = actualIndex;
              _lastTapTime = millis();
              if (_selectedIndex != actualIndex) {
                _selectedIndex = actualIndex;
                drawMenu(false);
              }
            }
            _lastTouchTime = millis();
          }
        }
      }
    }
  } else {
    _touchStartY = -1;
  }

  if (enter) {
    switch (_selectedIndex) {
    case 0:
      _ui->switchScreen(SCREEN_LAP_TIMER);
      break;
    case 1:
      _ui->switchScreen(SCREEN_DRAG_METER);
      break;
    case 2:
      _ui->switchScreen(SCREEN_RPM_SENSOR);
      break;
    case 3:
      _ui->switchScreen(SCREEN_SPEEDOMETER);
      break;
    case 4:
      _ui->switchScreen(SCREEN_HISTORY);
      break;
    case 5:
      _ui->switchScreen(SCREEN_GPS_STATUS);
      break;
    case 6:
      _ui->switchScreen(SCREEN_SETTINGS);
      break;
    case 7:
      _ui->switchScreen(SCREEN_SYNCHRONIZE);
      break;
    }
  }

  // static unsigned long lastStatusUpdate = 0;
  // if (millis() - lastStatusUpdate > 1000) {
  //   _ui->drawStatusBar();
  //   lastStatusUpdate = millis();
  // }
}

#include "../fonts/Org_01.h"

void MenuScreen::drawMenu(bool force) {
  TFT_eSPI *tft = _ui->getTft();

  if (force) {
    tft->fillRect(0, 21, SCREEN_WIDTH, SCREEN_HEIGHT - 21, COLOR_BG);
    tft->drawFastHLine(0, 20, SCREEN_WIDTH, COLOR_SECONDARY);

    tft->setTextDatum(TC_DATUM);
    tft->setFreeFont(&Org_01);
    tft->setTextSize(FONT_SIZE_MENU_TITLE);
    tft->setTextColor(COLOR_ACCENT);
    tft->drawString("MAIN MENU", SCREEN_WIDTH / 2, 45);
  }

  tft->setTextSize(FONT_SIZE_MENU_ITEM);

  int startY = 80;
  int gap = 38;
  int rectWidth = SCREEN_WIDTH - 10;
  int rectX = 5;

  // Draw Items for CURRENT PAGE
  int startIndex = _currentPage * ITEMS_PER_PAGE;
  int endIndex = startIndex + ITEMS_PER_PAGE;
  if (endIndex > MENU_ITEMS)
    endIndex = MENU_ITEMS;

  for (int i = 0; i < (endIndex - startIndex); i++) {
    int actualIndex = startIndex + i;
    int yPos = startY + (i * gap);

    // Only draw if forced OR if this item's selection state changed
    bool stateChanged =
        (actualIndex == _selectedIndex || actualIndex == _lastSelectedIndex);

    if (force || stateChanged) {
      // Clear item area
      tft->fillRect(rectX, yPos - 5, rectWidth, 30, COLOR_BG);

      if (actualIndex == _selectedIndex) {
        tft->setTextColor(COLOR_BG, COLOR_HIGHLIGHT);
        tft->fillRoundRect(rectX, yPos - 5, rectWidth, 30, 5, COLOR_HIGHLIGHT);
      } else {
        tft->setTextColor(COLOR_TEXT, COLOR_BG);
      }

      tft->setTextDatum(MC_DATUM);
      tft->setFreeFont(&Org_01);
      tft->setTextSize(FONT_SIZE_MENU_ITEM);
      tft->drawString(menuLabels[actualIndex], SCREEN_WIDTH / 2, yPos + 10);
    }
  }

  _lastSelectedIndex = _selectedIndex;

  if (force) {
    // Page Indicators (only on full redraw/page change)
    tft->setTextDatum(BC_DATUM);
    tft->setTextSize(1);
    tft->setTextColor(COLOR_SECONDARY);

    int maxPage = (MENU_ITEMS - 1) / ITEMS_PER_PAGE;

    if (maxPage > 0) {
      if (_currentPage < maxPage) {
        tft->fillTriangle(SCREEN_WIDTH / 2 - 10, 230, SCREEN_WIDTH / 2 + 10,
                          230, SCREEN_WIDTH / 2, 238, COLOR_ACCENT);
      } else if (_currentPage > 0) {
        tft->fillTriangle(SCREEN_WIDTH / 2 - 10, 238, SCREEN_WIDTH / 2 + 10,
                          238, SCREEN_WIDTH / 2, 230, COLOR_ACCENT);
      }
    }
  }
}
