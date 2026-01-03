#include "UIManager.h"
#include "../../config.h"

// Include screens (created in next steps)
#include "screens/DragMeterScreen.h"
#include "screens/HistoryScreen.h"
#include "screens/LapTimerScreen.h"
#include "screens/MenuScreen.h"
#include "screens/SettingsScreen.h"
#include "screens/SplashScreen.h"

UIManager::UIManager(TFT_eSPI *tft) : _tft(tft), _touch(nullptr) {
  _currentScreen = nullptr;
}

void UIManager::begin() {
  // Instantiate Screens
  _splashScreen = new SplashScreen();
  _menuScreen = new MenuScreen();
  _lapTimerScreen = new LapTimerScreen();
  _dragMeterScreen = new DragMeterScreen();
  _historyScreen = new HistoryScreen();
  _settingsScreen = new SettingsScreen();

  // Begin Screens
  _splashScreen->begin(this);
  _menuScreen->begin(this);
  _lapTimerScreen->begin(this);
  _dragMeterScreen->begin(this);
  _historyScreen->begin(this);
  _settingsScreen->begin(this);

  // Start with Splash
  switchScreen(SCREEN_SPLASH);
}

void UIManager::update() {
  // Update current screen logic
  if (_currentScreen) {
    _currentScreen->update();
  }

  // --- TOUCH HANDLING ---
  // (Optional) We can handle specific global touches here, but for now
  // letting screens handle it provides more context control.
}

UIManager::TouchPoint UIManager::getTouchPoint() {
  TouchPoint p = {-1, -1};
  if (!_touch)
    return p;

  _touch->read();
  if (_touch->isTouched) {
    int rawX = _touch->points[0].x;
    int rawY = _touch->points[0].y;

    // Calibration for 320x240 Screen
    // Raw Y typically comes in 0-320 but screen is 0-240.
    // Raw X is usually 0-320 matching screen width.
    // Standard 1:1 Mapping (GT911 usually auto-scales)
    // If inaccurate, check Serial Monitor for "Touch: Raw..." output
    // Manual Calibration / Mapping from config.h
    int pX = rawX;
    int pY = rawY;

    // 1. Swap XY
    if (TOUCH_SWAP_XY) {
      int temp = pX;
      pX = pY; // pX now holds RawY (0-320 approx)
      pY = temp; // pY now holds RawX (0-240 approx)
    }

    // 2. Invert X (Screen Coordinates)
    if (TOUCH_INVERT_X) {
      pX = SCREEN_WIDTH - 1 - pX;
    }

    // 3. Invert Y (Screen Coordinates)
    if (TOUCH_INVERT_Y) {
      pY = SCREEN_HEIGHT - 1 - pY;
    }

    p.x = pX;
    p.y = pY;

    // Constrain
    if (p.x < 0)
      p.x = 0;
    if (p.x > 320)
      p.x = 320;
    if (p.y < 0)
      p.y = 0;
    if (p.y > 240)
      p.y = 240;

    // Debug: Trace touch coordinates
    Serial.printf("Touch: Raw[%d,%d] -> Screen[%d,%d]\n", rawX, rawY, p.x, p.y);
  }
  return p;
}

#include "../core/GPSManager.h"
#include "../core/SessionManager.h"

extern GPSManager gpsManager;
extern SessionManager sessionManager;

void UIManager::switchScreen(ScreenType type) {
  // Add 1-second delay for smooth transition (except from Splash)
  if (_currentType != SCREEN_SPLASH) {
    delay(500);
  }

  _currentType = type;

  _tft->fillScreen(COLOR_BG);

  switch (type) {
  case SCREEN_SPLASH:
    _currentScreen = _splashScreen;
    break;
  case SCREEN_MENU:
    _currentScreen = _menuScreen;
    break;
  case SCREEN_LAP_TIMER:
    _currentScreen = _lapTimerScreen;
    break;
  case SCREEN_DRAG_METER:
    _currentScreen = _dragMeterScreen;
    break;
  case SCREEN_HISTORY:
    _currentScreen = _historyScreen;
    break;
  case SCREEN_SETTINGS:
    _currentScreen = _settingsScreen;
    break;
  }

  // Draw Status Bar immediately on switch (background already cleared)
  if (_currentType != SCREEN_SPLASH) {
    drawStatusBar();
  }

  if (_currentScreen) {
    _currentScreen->onShow();
  }
}

void UIManager::drawStatusBar() {
  // Only draw every second or so to minimize flicker?
  // Or just draw transparent text.

  // Background Strip
  _tft->fillRect(0, 0, SCREEN_WIDTH, 20, COLOR_BG);
  _tft->drawFastHLine(0, 20, SCREEN_WIDTH, COLOR_SECONDARY);

  _tft->setTextSize(FONT_SIZE_STATUS_BAR);
  _tft->setTextColor(COLOR_TEXT, COLOR_BG);

  // 1. GPS Signal Icon (Left)
  double hdop = gpsManager.getHDOP();
  bool fix = gpsManager.isFixed();

  // Draw "GPS" Label
  _tft->setTextDatum(ML_DATUM);
  _tft->setTextColor(COLOR_TEXT, COLOR_BG);
  _tft->setTextSize(1); // Small
  _tft->drawString(
      "GPS", 5,
      11); // Vertically centered (y=11 is approx visual middle for this font)

  // Calculate signal strength (0-4 bars) based on HDOP/Accuracy
  int signalStrength = 0;
  if (fix) {
    if (hdop <= 0.8)
      signalStrength = 4; // Excellent
    else if (hdop <= 1.0)
      signalStrength = 3; // Good
    else if (hdop <= 1.5)
      signalStrength = 2; // Fair
    else
      signalStrength = 1; // Poor
  } else {
    signalStrength = 0;
  }

  int barX = 30; // Shifted right to make room for "GPS" text
  int barY = 16; // Bottom align
  int barW = 3;
  int barGap = 2;

  for (int i = 0; i < 4; i++) {
    int h = (i + 1) * 3; // Heights: 3, 6, 9, 12
    int x = barX + (i * (barW + barGap));
    int y = barY - h;

    if (i < signalStrength) {
      // Filled bar
      uint16_t color = fix ? TFT_GREEN : TFT_RED;
      _tft->fillRect(x, y, barW, h, color);
    } else {
      // Empty bar (outline or gray)
      _tft->drawRect(x, y, barW, h, COLOR_TEXT); // Outline
    }
  }

  // Optional: Draw 'GPS' text small next to it if needed, or just rely on icon.
  // Let's keep it clean with just the icon as requested.

  // 2. Time (Center)
  _tft->setTextDatum(TC_DATUM);
  _tft->setTextColor(COLOR_TEXT, COLOR_BG);
  String timeStr = gpsManager.getTimeString();
  _tft->drawString(timeStr, SCREEN_WIDTH / 2, 2);

  // 3. Battery (Right)
#ifdef PIN_BATTERY
  int rawBat = analogRead(PIN_BATTERY);
  // Estimate percentage (very rough) - just mocking for now
  // Real implementation would map voltage
#else
  int rawBat = 4095; // Mock Full Battery
#endif

  int batX = SCREEN_WIDTH - 25;
  int batY = 5;
  int batW = 20;
  int batH = 10;

  // Battery Body
  _tft->drawRect(batX, batY, batW, batH, COLOR_TEXT);
  // Battery Nipple
  _tft->fillRect(batX + batW, batY + 2, 2, 6, COLOR_TEXT);
  // Battery Level (Green)
  // Mock level 75%
  _tft->fillRect(batX + 2, batY + 2, batW - 4, batH - 4, TFT_GREEN);

  // Recording Indicator (Next to Time?)
  if (sessionManager.isLogging()) {
    _tft->fillCircle((SCREEN_WIDTH / 2) + 40, 10, 3, TFT_RED);
  }
}
