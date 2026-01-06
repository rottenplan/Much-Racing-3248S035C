#ifndef SETTINGS_SCREEN_H
#define SETTINGS_SCREEN_H

#include "../UIManager.h"
#include <Arduino.h>
#include <Preferences.h>
#include <vector>
#include "../../core/SessionManager.h" // Added for SDTestResult

class SettingsScreen : public UserScreen {
public:
  void begin(UIManager *ui) override { _ui = ui; }
  void onShow() override;
  void update() override;

private:
  UIManager *_ui;
  Preferences _prefs;

  enum SettingType { TYPE_TOGGLE, TYPE_VALUE, TYPE_ACTION };

  struct SettingItem {
    String name;
    SettingType type;
    String key; // Preference key

    // For Values
    std::vector<String> options;
    int currentOptionIdx;

    // For Toggle
    bool checkState;

    // For Action
    // (Simplified: check logic in update)
  };

  std::vector<SettingItem> _settings;
  int _scrollOffset;
  int _selectedIdx;
  
  enum ScreenMode { MODE_MAIN, MODE_DRAG, MODE_GPS, MODE_SD_TEST, MODE_RPM, MODE_WIFI, MODE_WIFI_PASS };
  ScreenMode _currentMode;
  SessionManager::SDTestResult _sdResult;

  void loadSettings();
  void saveSetting(int idx);
  void drawList(int scrollOffset);
  void drawHeader(String title, uint16_t backColor = COLOR_HIGHLIGHT);
  void drawGPSStatus();
  void drawSDTest();
  void drawWiFiScan();
  void drawKeyboard(); // New Method
  void handleTouch(int idx);
  
  // WiFi State
  int _scanCount = 0;
  String _targetSSID = "";
  String _enteredPass = "";
  
  int _lastTapIdx;
  unsigned long _lastTapTime;
  int _touchStartY = -1;
};

#endif
