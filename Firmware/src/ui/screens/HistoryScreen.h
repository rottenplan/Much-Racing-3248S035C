#ifndef HISTORY_SCREEN_H
#define HISTORY_SCREEN_H

#include "../UIManager.h"
#include <Arduino.h>
#include <vector>

class HistoryScreen : public UserScreen {
public:
  void begin(UIManager *ui) override { _ui = ui; }
  void onShow() override;
  void update() override;

private:
  UIManager *_ui;

  struct HistoryItem {
    String filename;
    String date;
    int laps;              // or runs for drag
    unsigned long bestLap; // ms
    String type;           // "TRACK" or "DRAG"
  };

  std::vector<HistoryItem> _historyList;
  void scanHistory();
  void drawList(int scrollOffset);

  int _scrollOffset = 0;

  enum HistoryMode {
    MODE_MENU,          // Track vs Drag
    MODE_GROUPS,        // Year-Month Folders
    MODE_LIST,          // Session List (filtered by Group)
    MODE_OPTIONS,       // View/Sync/Delete
    MODE_VIEW_DATA,     // 5 Pages of Data
    MODE_CONFIRM_DELETE // Safety check
  };
  HistoryMode _currentMode;

  String _selectedType;  // "TRACK" or "DRAG"
  String _selectedGroup; // "YYYY-MM"
  int _selectedIdx;      // Index in filtered list (or group list)

  // Data View
  int _viewPage; // 0=Speed, 1=RPM, 2=Temp, 3=Sector, 4=Replay

  int _lastTapIdx;
  unsigned long _lastTapTime;

  std::vector<String> _groups; // Unique Year-Month list

  void scanGroups(); // Populate _groups based on _selectedType

  // Scroll Logic
  int _lastTouchY = -1;
  bool _isDragging = false;
  int _dragThreshold = 10;

  // Touch State
  bool _wasTouching = false;
  int _touchStartX = -1;
  int _touchStartY = -1;
  unsigned long _touchStartTime = 0;
  unsigned long _lastBackTapTime = 0;
  bool _ignoreInitialTouch = true;

  void drawMenu();
  void drawGroups(int scrollOffset);
  // drawList is already declared above
  void drawOptions();
  void drawViewData();
  void drawConfirmDelete();
};

#endif
