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

  enum HistoryMode { MODE_MENU, MODE_LIST, MODE_DETAILS };
  HistoryMode _currentMode;
  
  String _selectedType; // "TRACK" or "DRAG"

  int _selectedIdx;
  
  int _lastTapIdx;
  unsigned long _lastTapTime;  void drawMenu();
  void drawDetails(int idx);
};

#endif
