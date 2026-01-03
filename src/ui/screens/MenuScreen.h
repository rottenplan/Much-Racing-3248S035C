#ifndef MENU_SCREEN_H
#define MENU_SCREEN_H

#include "../UIManager.h"

class MenuScreen : public UserScreen {
public:
  void begin(UIManager *ui) override { _ui = ui; }
  void onShow() override;
  void update() override;

private:
  UIManager *_ui;
  int _selectedIndex;
  void drawMenu();
};

#endif
