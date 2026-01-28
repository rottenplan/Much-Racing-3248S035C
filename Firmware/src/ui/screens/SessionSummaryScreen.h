#ifndef SESSION_SUMMARY_SCREEN_H
#define SESSION_SUMMARY_SCREEN_H

#include "../UIManager.h"
#include "TrackData.h"

class SessionSummaryScreen : public UserScreen {
public:
  void begin(UIManager *ui) override { _ui = ui; }
  void onShow() override;
  void onHide() override {}
  void update() override;

private:
  UIManager *_ui;

  void drawSummary();
  void drawTrackMap(int x, int y, int w, int h,
                    const std::vector<GPSPoint> &points);
};

#endif
