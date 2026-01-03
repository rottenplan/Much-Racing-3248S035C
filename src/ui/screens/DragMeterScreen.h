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
  enum State { STATE_READY, STATE_RUNNING, STATE_SUMMARY };
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

  // Logic
  void checkStartCondition();
  void checkStopCondition();
  void updateDisciplines();

  // Drawing
  void drawDashboard(); // Ready/Running
  void drawResults();   // Summary
};

#endif
