#ifndef LAP_TIMER_SCREEN_H
#define LAP_TIMER_SCREEN_H

#include "../UIManager.h"
#include "../components/KeyboardComponent.h"
#include "TrackData.h"
#include <Arduino.h>
#include <vector>

enum LapTimerState {
  STATE_MENU,
  STATE_TRACK_LIST,
  STATE_TRACK_MENU,
  STATE_TRACK_DETAILS,
  STATE_SEARCHING,
  STATE_CREATE_TRACK,
  STATE_RENAME_TRACK,
  STATE_NO_GPS
};

class LapTimerScreen : public UserScreen {
public:
  void begin(UIManager *ui) override { _ui = ui; }
  void onShow() override;
  void update() override;

private:
  UIManager *_ui;
  LapTimerState _state;
  int _menuSelectionIdx = -1;
  unsigned long _lastUpdate;
  unsigned long _lastTouchTime = 0;
  unsigned long _lastBackTapTime = 0;
  unsigned long _searchStartTime = 0;
  bool _needsStaticRedraw;

  // Track Management
  std::vector<Track> _tracks;
  int _selectedTrackIdx = -1;
  String _currentTrackName;

  // Session results (legacy bridge)
  long _bestLapTime = 0;
  int _lapCount = 0;
  std::vector<unsigned long> _lapTimes;
  unsigned long _maxRpmSession = 0;

  // UI Methods
  void drawMenu();
  void drawTrackList();
  void drawTrackOptionsPopup();
  void drawTrackDetails();
  void drawSearching();
  void drawCreateTrack();
  void drawRenameTrack();
  void drawNoGPS();
  void transitionToSummary();

  // Data Methods
  void loadTracks();
  void loadTrackPath(String filename);
  void saveNewTrack(String name, double sLat, double sLon, double fLat,
                    double fLon);
  void renameTrack(int index, String newName);
  void checkFinishLine(); // Bridge if needed

  // Flicker Tracking (General)
  float _lastSpeed = -1.0;
  int _lastSats = -1;
  int _lastRpmRender = -1;
  unsigned long _lastMaxRpmRender = 0;
  int _lastLapCountRender = -1;

  // Track Creator State
  int _createStep = 0;
  double _createStartLat, _createStartLon;
  double _createFinishLat, _createFinishLon;

  // Renaming State
  KeyboardComponent _keyboard;
  String _renamingName;
  bool _keyboardShift = true;
};

#endif
