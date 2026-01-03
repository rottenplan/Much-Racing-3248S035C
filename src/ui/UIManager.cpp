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
  // No delay for instant switching

  _currentType = type;

  _tft->fillScreen(COLOR_BG);

  switch (type) {
  case SCREEN_SPLASH:
    _currentScreen = _splashScreen;
    _screenTitle = "";
    break;
  case SCREEN_MENU:
    _currentScreen = _menuScreen;
    _screenTitle = ""; // Empty = Show Time
    break;
  case SCREEN_LAP_TIMER:
    _currentScreen = _lapTimerScreen;
    _screenTitle = "LAP TIMER";
    break;
  case SCREEN_DRAG_METER:
    _currentScreen = _dragMeterScreen;
    _screenTitle = "DRAG METER";
    break;
  case SCREEN_HISTORY:
    _currentScreen = _historyScreen;
    _screenTitle = "HISTORY";
    break;
  case SCREEN_SETTINGS:
    _currentScreen = _settingsScreen;
    _screenTitle = "SETTINGS";
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

// Variables to track last state for conditional redrawing
static String _lastTimeStr = "";
static double _lastHdop = -1.0;
static bool _lastFix = false;
static int _lastSignalStrength = -1;
static int _lastBat = -1;
static bool _lastLogging = false;

void UIManager::drawStatusBar() {
  // 1. Static Elements (Only draw once or if forced? For now, we assume background is cleared only on screen switch)
  // If we want to avoid flickering, we must NOT clear the whole bar every frame.
  // We rely on text background colors to overwrite old text.
  
  // Draw separator line (safe to redraw, fast)
  _tft->drawFastHLine(0, 20, SCREEN_WIDTH, COLOR_SECONDARY);

  _tft->setTextSize(FONT_SIZE_STATUS_BAR);
  _tft->setTextColor(COLOR_TEXT, COLOR_BG);

  // --- GPS Section ---
  double hdop = gpsManager.getHDOP();
  bool fix = gpsManager.isFixed();
  
  // Logic to calculate strength
  int signalStrength = 0;
  if (fix) {
    if (hdop <= 0.8) signalStrength = 4;
    else if (hdop <= 1.0) signalStrength = 3;
    else if (hdop <= 1.5) signalStrength = 2;
    else signalStrength = 1;
  }

  // Redraw GPS Icon only if state changed (Fix status or Strength)
  if (fix != _lastFix || signalStrength != _lastSignalStrength) {
      // Clear GPS Area
      _tft->fillRect(0, 0, 80, 20, COLOR_BG);
      
      // Draw "GPS" Label
      _tft->setTextDatum(ML_DATUM);
      _tft->setTextColor(COLOR_TEXT, COLOR_BG);
      _tft->setTextSize(1);
      _tft->drawString("GPS", 5, 11);

      int barX = 30;
      int barY = 16;
      int barW = 3;
      int barGap = 2;

      for (int i = 0; i < 4; i++) {
        int h = (i + 1) * 3;
        int x = barX + (i * (barW + barGap));
        int y = barY - h;

        if (i < signalStrength) {
          uint16_t color = fix ? TFT_GREEN : TFT_RED;
          _tft->fillRect(x, y, barW, h, color);
        } else {
          _tft->drawRect(x, y, barW, h, COLOR_TEXT);
        }
      }
      _lastFix = fix;
      _lastSignalStrength = signalStrength;
  }

  // --- Time / Title Section ---
  // If _screenTitle is set, show it. Otherwise show Time.
  String centerText;
  if (_screenTitle.length() > 0) {
      centerText = _screenTitle;
  } else {
      centerText = gpsManager.getTimeString();
  }

  if (centerText != _lastTimeStr) { // Reusing _lastTimeStr for center text cache
      // Clear Time/Title Area (Center)
      // Assuming max width 160px (leaving 80px on each side)
      int areaW = 160;
      
      _tft->setTextPadding(areaW);
      _tft->setTextDatum(TC_DATUM);
      _tft->setTextColor(COLOR_TEXT, COLOR_BG);
      _tft->drawString(centerText, SCREEN_WIDTH / 2, 5); // Adjusted Y to 5
      _tft->setTextPadding(0);
      
      _lastTimeStr = centerText;
  }

  // --- Battery Section (Mock) ---
  // Just redraw simple battery every time? Or check change?
  // Mocking change check
  int rawBat = 4095; // Mock
  if (rawBat != _lastBat) {
      // Clear Bat Area
      _tft->fillRect(SCREEN_WIDTH - 30, 0, 30, 20, COLOR_BG);
      
      int batX = SCREEN_WIDTH - 25;
      int batY = 5;
      int batW = 20;
      int batH = 10;
      
      _tft->drawRect(batX, batY, batW, batH, COLOR_TEXT);
      _tft->fillRect(batX + batW, batY + 2, 2, 6, COLOR_TEXT);
      
      // Mock Level
      _tft->fillRect(batX + 2, batY + 2, batW - 4, batH - 4, TFT_GREEN);
      
      _lastBat = rawBat;
  }

  // --- Recording Indicator ---
  bool isLogging = sessionManager.isLogging();
  if (isLogging != _lastLogging) {
       // Clear Dot Area
       int dotX = (SCREEN_WIDTH / 2) + 40;
       _tft->fillRect(dotX - 5, 0, 10, 20, COLOR_BG);
       
       if (isLogging) {
         _tft->fillCircle(dotX, 10, 3, TFT_RED);
       }
       _lastLogging = isLogging;
  }
}
