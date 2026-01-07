#ifndef TIME_SETTING_SCREEN_H
#define TIME_SETTING_SCREEN_H

#include "../UIManager.h"
#include <Arduino.h>

extern int g_manualHour;
extern int g_manualMinute;
extern unsigned long g_lastTimeUpdate;

class TimeSettingScreen : public UserScreen {
public:
  void begin(UIManager *ui) override { _ui = ui; }
  void onShow() override;
  void update() override;

private:
  UIManager *_ui;
  int _selectedIdx; // -1:None, 0:Back, 1:Hour, 2:Minute
  
  // Double Tap Logic
  unsigned long _lastBackTap = 0;
  int _backTapCount = 0;

  void drawScreen();
};

#endif
