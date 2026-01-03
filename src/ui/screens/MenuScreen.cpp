#include "MenuScreen.h"
#include "../../core/GPSManager.h"

// extern GPSManager gpsManager; // If needed

#define MENU_ITEMS 4
const char *menuLabels[MENU_ITEMS] = {"LAP TIMER", "DRAG METER", "HISTORY",
                                      "SETTINGS"};

void MenuScreen::onShow() {
  _selectedIndex = 0;
  drawMenu();
}

void MenuScreen::update() {
  bool enter = false;

  // --- TOUCH HANDLING ---
  UIManager::TouchPoint p = _ui->getTouchPoint();
  if (p.x != -1) { // Touched

    // Debug Touch
    // Serial.printf("Touch: x=%d, y=%d\n", p.x, p.y);

    // Simple Hit Detection for Menu Items
    // We know items start at Y=65 and have gap=38
    // Assuming MENU_ITEMS = 4
    int startY = 65;
    int gap = 38;
    int itemHeight = 30;

    for (int i = 0; i < MENU_ITEMS; i++) {
      int yTop = startY + (i * gap) - 5;
      int yBot =
          yTop + 30; // Match visual height exactly (30px)
                     // formerly used 'gap' (38px) which caused overlap/no-gap.
                     // Strict match to highlight "acuan".

      // Check if touch is within the visual highlight box
      if (p.x >= 5 && p.x <= (SCREEN_WIDTH - 5) && p.y >= yTop && p.y <= yBot) {
        // Valid Tap on Item i
        if (_selectedIndex != i) {
          // First tap: Just Highlight
          _selectedIndex = i;
          drawMenu(); // Update visual
          enter = false;
        } else {
          // Second tap (on selected): Enter
          enter = true;
        }
        delay(200); // Simple debounce
        break;
      }
    }
  }
  // ----------------------

  if (enter) {
    switch (_selectedIndex) {
    case 0:
      _ui->switchScreen(SCREEN_LAP_TIMER);
      break;
    case 1:
      _ui->switchScreen(SCREEN_DRAG_METER);
      break;
    case 2:
      _ui->switchScreen(SCREEN_HISTORY);
      break;
    case 3:
      _ui->switchScreen(SCREEN_SETTINGS);
      break;
    }
  }

  // Update Status Bar periodically
  static unsigned long lastStatusUpdate = 0;
  if (millis() - lastStatusUpdate > 1000) {
    _ui->drawStatusBar();
    lastStatusUpdate = millis();
  }
}

#include "../fonts/Org_01.h"

// ... existing code ...

void MenuScreen::drawMenu() {
  TFT_eSPI *tft = _ui->getTft();

  // Clear area BELOW status bar
  tft->fillRect(0, 21, SCREEN_WIDTH, SCREEN_HEIGHT - 21, COLOR_BG);

  tft->setTextDatum(TC_DATUM);
  tft->setFreeFont(&Org_01);
  tft->setTextSize(3); // Org_01 is 6px, so 3x is 18px (Title)
  tft->setTextColor(COLOR_ACCENT);
  tft->drawString("MAIN MENU", SCREEN_WIDTH / 2, 30); // Moved up to 30

  tft->setTextSize(2); // Items at 2x (12px)
  int startY = 65;     // Started earlier (was 80)
  int gap = 38;        // Reduced gap (was 40)

  int rectWidth = SCREEN_WIDTH - 10; // "Full" length (5px margin)
  int rectX = 5;

  for (int i = 0; i < MENU_ITEMS; i++) {
    if (i == _selectedIndex) {
      tft->setTextColor(COLOR_BG, COLOR_HIGHLIGHT);
      tft->fillRoundRect(rectX, startY + (i * gap) - 5, rectWidth, 30, 5,
                         COLOR_HIGHLIGHT);
    } else {
      tft->setTextColor(COLOR_TEXT, COLOR_BG);
    }
    // Set Text Datum to Middle Center for better vertical integration
    tft->setTextDatum(MC_DATUM);
    tft->setFreeFont(&Org_01);
    tft->setTextSize(3); // User manual set to 3
    tft->drawString(menuLabels[i], SCREEN_WIDTH / 2,
                    startY + (i * gap) +
                        10); // +10 is vertical center (Box is -5 to +25)
  }
}
