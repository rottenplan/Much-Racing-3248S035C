#ifndef LAP_TIMER_SCREEN_H
#define LAP_TIMER_SCREEN_H

#include "../UIManager.h"

#include <vector>
#include <Arduino.h>

struct TrackConfig {
  String name;
  // TODO: Add sectors/finish line coordinates here
};

struct Track {
  String name;
  std::vector<TrackConfig> configs;
  double lat;
  double lon;
};


enum LapTimerState { STATE_MENU, STATE_TRACK_SELECT, STATE_SUMMARY, STATE_RACING, STATE_CREATE_TRACK, STATE_NO_GPS };
enum RaceMode { MODE_BEST, MODE_LAST, MODE_PREDICTIVE };

class LapTimerScreen : public UserScreen {
public:
  void begin(UIManager *ui) override { _ui = ui; }
  void onShow() override;
  void update() override;

private:
  UIManager *_ui;
  LapTimerState _state;
  RaceMode _raceMode;
  int _menuSelectionIdx = -1;
  unsigned long _lastUpdate;
  unsigned long _lastTouchTime = 0;
  bool _isRecording;

  // Custom Track Creation
  double _tempStartLat, _tempStartLon;
  int _tempSplitCount;

  // UI Methods
  void drawSummary();
  void drawRacingStatic();
  void drawRacing();
  void drawLapList(int scrollOffset);
  void drawMenu(); // Sub-menu
  void drawTrackSelect();
  void drawCreateTrack(); // Custom Track UI
  void drawNoGPS();


  // Lap Data
  double _finishLat, _finishLon;
  bool _finishSet;
  unsigned long _currentLapStart;
  long _lastLapTime;
  long _bestLapTime;
  int _lapCount;
  std::vector<unsigned long> _lapTimes; // Store lap times in ms
  int _listScroll;                      // Scroll offset for list

  // Track Data
  std::vector<Track> _tracks;
  int _selectedTrackIdx;
  int _selectedConfigIdx;
  String _currentTrackName; // Store selected track name
  void loadTracks(); // Load dummy tracks


  void checkFinishLine();
};

#endif
