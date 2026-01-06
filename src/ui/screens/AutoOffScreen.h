#ifndef AUTO_OFF_SCREEN_H
#define AUTO_OFF_SCREEN_H

#include "../UIManager.h"
#include <Arduino.h>
#include <vector>

class AutoOffScreen : public UserScreen {
public:
  void begin(UIManager *ui) override { _ui = ui; }
  void onShow() override;
  void update() override;

private:
  UIManager *_ui;
  std::vector<String> _options;
  int _selectedIdx; // Index in the list (0-5)
  
  void drawScreen();
  void handleTouch(int idx);
};

#endif
