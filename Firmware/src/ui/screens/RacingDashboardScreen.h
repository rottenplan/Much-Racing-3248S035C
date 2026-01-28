#ifndef RACING_DASHBOARD_SCREEN_H
#define RACING_DASHBOARD_SCREEN_H

#include "../UIManager.h"
#include "TrackData.h"

class RacingDashboardScreen : public UserScreen {
public:
  void begin(UIManager *ui) override { _ui = ui; }
  void onShow() override;
  void onHide() override;
  void update() override;

private:
  UIManager *_ui;

  // Data
  Track _currentTrack;
  unsigned long _currentLapStart;
  long _lastLapTime;
  long _bestLapTime;
  int _lapCount;
  std::vector<unsigned long> _lapTimes;
  unsigned long _maxRpmSession;

  // Logic
  bool _finishLineInside;
  unsigned long _lastFinishCross;

  // Flicker Reduction
  float _lastSpeed = -1.0;
  int _lastSats = -1;
  int _lastRpmRender = -1;
  unsigned long _lastUpdate = 0;
  bool _needsStaticRedraw = true;

  void drawStatic();
  void drawDynamic();
  void checkFinishLine();
  void drawRPMBar(int rpm, int maxRpm);
  void drawTrackMap(int x, int y, int w, int h);
};

#endif
