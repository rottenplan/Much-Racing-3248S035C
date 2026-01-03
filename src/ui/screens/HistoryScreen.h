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
  };

  std::vector<HistoryItem> _historyList;
  void scanHistory();
  void drawList(int scrollOffset);
  int _scrollOffset = 0;

  // Detail View State?
  bool _showingDetails;
  int _selectedIdx;
  void drawDetails(int idx);
  // void drawLapDetails(int idx); // Future expansion
};

#endif
