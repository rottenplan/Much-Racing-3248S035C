#ifndef GPS_STATUS_SCREEN_H
#define GPS_STATUS_SCREEN_H

#include "../UIManager.h"

class GpsStatusScreen : public UserScreen {
public:
  void begin(UIManager *ui) override { _ui = ui; }
  void onShow() override;
  void update() override;

private:
  UIManager *_ui;
  void drawStatus();

  // Cache for optimized redraw
  int _lastSats = -1;
  double _lastHdop = -1.0;
  int _lastHz = -1;
  double _lastLat = 0.0;
  double _lastLng = 0.0;
  bool _lastFixed = false;
};

#endif
