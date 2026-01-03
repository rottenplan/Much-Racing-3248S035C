#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include "../config.h"
#include <TFT_eSPI.h>

// Forward declaration
class UIManager;

enum ScreenType {
  SCREEN_SPLASH,
  SCREEN_MENU,
  SCREEN_LAP_TIMER,
  SCREEN_DRAG_METER,
  SCREEN_HISTORY,
  SCREEN_SETTINGS
};

class UserScreen {
public:
  virtual void begin(UIManager *ui) = 0;
  virtual void onShow() = 0;
  virtual void update() = 0;
};

// Include the Touch class declaration (using forward decl if possible to avoid
// circular dep)
#include "TAMC_GT911.h"

// ... existing code ...

class UIManager {
public:
  UIManager(TFT_eSPI *tft);
  void begin();
  void update();
  void switchScreen(ScreenType type);
  void drawStatusBar();

  // Touch Handling
  void setTouch(TAMC_GT911 *touch) { _touch = touch; }
  TAMC_GT911 *getTouch() { return _touch; }

  struct TouchPoint {
    int x;
    int y;
  };
  TouchPoint getTouchPoint();

  TFT_eSPI *getTft() { return _tft; }

private:
  TFT_eSPI *_tft;
  TAMC_GT911 *_touch; // Added touch pointer
  UserScreen *_currentScreen;
  ScreenType _currentType;
  // ... existing code ...

  // Screens
  UserScreen *_splashScreen;
  UserScreen *_menuScreen;
  UserScreen *_lapTimerScreen;
  UserScreen *_dragMeterScreen;
  UserScreen *_historyScreen;
  UserScreen *_settingsScreen;
};

#endif
