#ifndef LAP_TIMER_SCREEN_H
#define LAP_TIMER_SCREEN_H

#include "../UIManager.h"

#include <vector>

enum LapTimerState { STATE_SUMMARY, STATE_RACING };

class LapTimerScreen : public UserScreen {
public:
  void begin(UIManager *ui) override { _ui = ui; }
  void onShow() override;
  void update() override;

private:
  UIManager *_ui;
  LapTimerState _state;
  unsigned long _lastUpdate;
  bool _isRecording;

  // UI Methods
  void drawSummary();
  void drawRacingStatic();
  void drawRacing();
  void drawLapList(int scrollOffset);

  // Lap Data
  double _finishLat, _finishLon;
  bool _finishSet;
  unsigned long _currentLapStart;
  long _lastLapTime;
  long _bestLapTime;
  int _lapCount;
  std::vector<unsigned long> _lapTimes; // Store lap times in ms
  int _listScroll;                      // Scroll offset for list

  void checkFinishLine();
};

#endif
