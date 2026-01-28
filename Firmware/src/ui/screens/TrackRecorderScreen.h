#ifndef TRACK_RECORDER_SCREEN_H
#define TRACK_RECORDER_SCREEN_H

#include "../UIManager.h"
#include "TrackData.h"
#include <vector>

enum RecorderState { REC_IDLE, REC_ACTIVE, REC_COMPLETE };

class TrackRecorderScreen : public UserScreen {
public:
  void begin(UIManager *ui) override { _ui = ui; }
  void onShow() override;
  void onHide() override;
  void update() override;

private:
  UIManager *_ui;
  RecorderState _state;
  std::vector<GPSPoint> _recordedPoints;
  double _recordStartLat, _recordStartLon;
  unsigned long _recordingStartTime;
  unsigned long _lastPointTime;
  unsigned long _lastTouchTime = 0;

  // Flicker Reduction
  RecorderState _lastStateRender = (RecorderState)-1;
  bool _lastGpsFixed = false;
  int _lastSats = -1;

  void drawStatic();
  void drawDynamic();
  void saveTrackToGPX(String filename);
};

#endif
