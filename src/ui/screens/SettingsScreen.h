#ifndef SETTINGS_SCREEN_H
#define SETTINGS_SCREEN_H

#include "../UIManager.h"
#include <Arduino.h>
#include <Preferences.h>
#include <vector>
#include <WiFi.h>
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
  
  enum ScreenMode { MODE_MAIN, MODE_GPS, MODE_SD_TEST, MODE_RPM, MODE_ENGINE, MODE_WIFI, MODE_WIFI_PASS };
  ScreenMode _currentMode;
  SessionManager::SDTestResult _sdResult;

  // WiFi Members
  int _scanCount = 0;
  int _selectedWiFiIdx = -1;
  String _targetSSID = "";
  String _enteredPass = "";
  bool _isScanning = false;
  bool _isUppercase = true;
  bool _showPassword = false;
  unsigned long _lastScanAnim = 0;
  int _scanAnimStep = 0;

  int _lastTapIdx = -1;
  unsigned long _lastTapTime;
  int _touchStartY = -1;
  unsigned long _lastWiFiTouch = 0;
  unsigned long _lastKeyboardTouch = 0;
  int _lastSelectedIdx = -3;
  int _lastWiFiSelectedIdx = -1;
  unsigned long _lastGPSUpdate = 0; // Rate limit GPS redraw

  void loadSettings();
  void saveSetting(int idx);
  void drawList(bool force = false);
  void drawHeader(String title, uint16_t backColor = COLOR_HIGHLIGHT);
  void drawGPSStatus();
  void drawSDTest();

  // WiFi Draw Functions
  void drawWiFiList(bool force = false);
  void drawKeyboard(bool fullRedraw = true);
  void connectWiFi();
  
  void handleTouch(int idx);


  

};

#endif
