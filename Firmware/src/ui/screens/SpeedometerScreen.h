#ifndef SPEEDOMETER_SCREEN_H
#define SPEEDOMETER_SCREEN_H

#include "../UIManager.h"
#include <Arduino.h>

class SpeedometerScreen : public UserScreen {
public:
  SpeedometerScreen(TFT_eSPI *tft); // Add Constructor
  void begin(UIManager *ui) override { _ui = ui; }
  void onShow() override;
  void onHide() override; // Add onHide
  void update() override;

private:
  UIManager *_ui;
  void drawDashboard(bool force = false);

  float _lastSpeed = -1;
  float _maxSpeed = 0;
  int _lastRPM = -1;
  int _maxRPM = 0;    // Added for new indicator
  int _lastSats = -1; // Added for GPS signal
  float _lastTrip = -1;
  String _lastTime = "";
  int _lastGear = -1;
  int _lastBat = -1;
  bool _lastUnits = false; // false = km/h, true = mph
};

#endif
