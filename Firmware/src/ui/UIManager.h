#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include "../config.h"
#include "screens/TrackData.h"
#include <TFT_eSPI.h>

// Forward declaration
class UIManager;

enum ScreenType {
  SCREEN_SPLASH,
  SCREEN_SETUP,
  SCREEN_MENU,
  SCREEN_LAP_TIMER,
  SCREEN_DRAG_METER,
  SCREEN_HISTORY,
  SCREEN_SETTINGS,
  SCREEN_TIME_SETTINGS,
  SCREEN_AUTO_OFF,
  SCREEN_RPM_SENSOR,
  SCREEN_SPEEDOMETER,
  SCREEN_GPS_STATUS,
  SCREEN_SYNCHRONIZE,
  SCREEN_GNSS_LOG,
  SCREEN_WEB_SERVER,
  SCREEN_TRACK_RECORDER,
  SCREEN_SESSION_SUMMARY,
  SCREEN_RACING_DASHBOARD
};

// Global Layout Constants for 480x320
#define UI_CARD_W (SCREEN_WIDTH - 40)
#define UI_BTN_W 240
#define UI_POPUP_W 320
#define UI_MARGIN 20

class UserScreen {
public:
  virtual void begin(UIManager *ui) = 0;
  virtual void onShow() = 0;
  virtual void onHide() {}
  virtual void update() = 0;
};

#include "TAMC_GT911.h"

class UIManager {
public:
  struct TouchPoint {
    int x;
    int y;
    TouchPoint(int _x = -1, int _y = -1) : x(_x), y(_y) {}
  };

  UIManager(TFT_eSPI *tft);
  void begin();
  void update();
  void switchScreen(ScreenType type);
  void drawStatusBar(bool force = false);
  void setTitle(String title) { _screenTitle = title; }
  void setAutoOff(unsigned long ms);
  void setBrightness(int level);
  void wakeUp();
  void checkSleep();
  void updateInteraction();

  void showToast(String message, int duration = 2000);
  void drawCarbonBackground(int x, int y, int w, int h);
  void drawBackButton();
  bool isBackButtonTouched(TouchPoint p);

  void setTouch(TAMC_GT911 *touch) { _touch = touch; }
  TAMC_GT911 *getTouch() { return _touch; }
  TouchPoint getTouchPoint();
  TFT_eSPI *getTft() { return _tft; }

  void setDarkMode(bool enable);
  bool isDarkMode() { return _isDarkMode; }
  uint16_t getBackgroundColor();
  uint16_t getTextColor();
  uint16_t getSecondaryColor();

  // Session Data Sharing
  void setLastSession(ActiveSessionData data) { _lastSession = data; }
  ActiveSessionData &getLastSession() { return _lastSession; }
  void setSelectedTrack(Track t) { _selectedTrack = t; }
  Track &getSelectedTrack() { return _selectedTrack; }

private:
  TFT_eSPI *_tft;
  TAMC_GT911 *_touch;
  UserScreen *_currentScreen;
  ScreenType _currentType;
  bool _isDarkMode;

  ActiveSessionData _lastSession;
  Track _selectedTrack;

  // Screens
  UserScreen *_splashScreen;
  UserScreen *_setupScreen;
  UserScreen *_menuScreen;
  UserScreen *_lapTimerScreen;
  UserScreen *_dragMeterScreen;
  UserScreen *_historyScreen;
  UserScreen *_settingsScreen;
  UserScreen *_timeSettingScreen;
  UserScreen *_rpmSensorScreen;
  UserScreen *_speedometerScreen;
  UserScreen *_gpsStatusScreen;
  UserScreen *_synchronizeScreen;
  UserScreen *_gnssLogScreen;
  UserScreen *_webServerScreen;
  UserScreen *_trackRecorderScreen;
  UserScreen *_sessionSummaryScreen;
  UserScreen *_racingDashboardScreen;

  String _screenTitle;

  // Status Bar State Trackers
  String _lastTimeStr;
  double _lastHdop;
  bool _lastFix;
  int _lastSignalStrength;
  int _lastBat;
  bool _lastLogging;
  int _lastWifiStatus;

  // Sleep / Auto-Off
  unsigned long _lastInteractionTime;
  unsigned long _autoOffMs;
  bool _isScreenOff;
  int _currentBrightness;

  // Wakeup
  unsigned long _lastTapTime;
  bool _wasTouched;
  unsigned long _lastTouchProcessedTime;
};

#endif
