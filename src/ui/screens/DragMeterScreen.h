#ifndef DRAG_METER_SCREEN_H
#define DRAG_METER_SCREEN_H

#include "../UIManager.h"
#include <Arduino.h>
#include <vector>


class DragMeterScreen : public UserScreen {
public:
  void begin(UIManager *ui) override { _ui = ui; }
  void onShow() override;
  void update() override;

private:
  UIManager *_ui;
  enum State { STATE_MENU, STATE_DRAG_MODE_MENU, STATE_RUNNING, STATE_SUMMARY_VIEW };
  State _state;

  struct Discipline {
    String name;
    bool isDistance;          // true=distance (m), false=speed (km/h)
    float target;             // meters or km/h
    unsigned long resultTime; // ms
    bool completed;
    float endSpeed; // speed at completion (for distance modes)
  };

  std::vector<Discipline> _disciplines;

  unsigned long _startTime;
  unsigned long _lastUpdate;
  int _selectedBtn; // -1:None, 0:Back, 1:Reset
  
  // Menu
  // Menu
  std::vector<String> _menuItems;
  std::vector<String> _dragModeItems;
  int _selectedMenuIdx;
  int _selectedDragModeIdx;
  void drawMenu();
  void drawDragModeMenu();
  void handleMenuTouch(int idx);
  void handleDragModeTouch(int idx);

  // Logic
  void checkStartCondition();
  void checkStopCondition();
  void updateDisciplines();

  // Drawing
  void drawDashboardStatic();  // Dib panggil sekali di onShow
  void drawDashboardDynamic(); // Dipanggil di loop update
  void drawResults();   // Summary
};

#endif
