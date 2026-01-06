#ifndef SETTINGS_SCREEN_H
#define SETTINGS_SCREEN_H

#include "../UIManager.h"
#include <Arduino.h>
#include <Preferences.h>
#include <vector>

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

  void loadSettings();
  void saveSetting(int idx);
  void drawList(int scrollOffset);
  void handleTouch(int idx);
};

#endif
