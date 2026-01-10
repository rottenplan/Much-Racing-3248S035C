#include "HistoryScreen.h"
#include "../../config.h"
#include "../../core/SessionManager.h"
#include "../fonts/Org_01.h"

extern SessionManager sessionManager;

void HistoryScreen::onShow() {
  _scrollOffset = 0;
  _currentMode = MODE_MENU; // Start at Menu
  _selectedIdx = -1;
  scanHistory(); // Scan everything once

  TFT_eSPI *tft = _ui->getTft();
  _ui->drawStatusBar();
  drawMenu();
}

void HistoryScreen::update() {
  // Global Touch State Tracking
  static bool wasTouching = false;
  static int touchStartX = -1;
  static int touchStartY = -1;
  static unsigned long touchStartTime = 0;
  static unsigned long lastStatusUpdate = 0;

  // Status Bar Update
  // Status Bar Update - Handled by UIManager globally
  // if (millis() - lastStatusUpdate > 1000) {
  //   _ui->drawStatusBar();
  //   lastStatusUpdate = millis();
  // }

  UIManager::TouchPoint p = _ui->getTouchPoint();
  bool isTouching = (p.x != -1);

  // --- STATE MACHINE: START ---
  if (isTouching && !wasTouching) {
    touchStartX = p.x;
    touchStartY = p.y;
    touchStartTime = millis();
    _isDragging = false;
    _lastTouchY = p.y;
    wasTouching = true;
  }

  // --- STATE MACHINE: DRAGGING ---
  if (isTouching && wasTouching) {
    int dy = p.y - _lastTouchY;
    if (abs(p.y - touchStartY) > _dragThreshold) {
      _isDragging = true;
    }

    if (_isDragging) {
      if (_currentMode == MODE_GROUPS) {
        if (abs(dy) > 5) {
          if (dy > 0 && _scrollOffset > 0) {
            _scrollOffset--;
            drawGroups(_scrollOffset);
            _lastTouchY = p.y;
          } else if (dy < 0 && _scrollOffset < (int)_groups.size() - 5) {
            _scrollOffset++;
            drawGroups(_scrollOffset);
            _lastTouchY = p.y;
          }
        }
      } else if (_currentMode == MODE_LIST) {
        // Calculate filter count for bounds
        int filteredCount = 0;
        for (const auto &item : _historyList) {
          if (item.type == _selectedType && item.date.length() >= 10) {
            String g =
                item.date.substring(6, 10) + "-" + item.date.substring(3, 5);
            if (g == _selectedGroup)
              filteredCount++;
          }
        }
        int maxScroll = filteredCount - 5;

        if (abs(dy) > 5) {
          if (dy > 0 && _scrollOffset > 0) {
            _scrollOffset--;
            drawList(_scrollOffset);
            _lastTouchY = p.y;
          } else if (dy < 0 && _scrollOffset < maxScroll) {
            _scrollOffset++;
            drawList(_scrollOffset);
            _lastTouchY = p.y;
          }
        } else {
          // Not scrolling yet, treat as potential selection
          int listY = 50;
          int itemH = 25;
          if (p.y > listY) {
            int visIdx = (p.y - listY) / itemH;
            int clickedIdx = visIdx + _scrollOffset;

            // Bounds check against filtered count
            if (clickedIdx < filteredCount && clickedIdx >= 0) {
              if (_selectedIdx != clickedIdx) {
                _selectedIdx = clickedIdx;
                drawList(_scrollOffset);
              }
            }
          }
        }
      }
    }
  }

  // --- STATE MACHINE: RELEASE (TAP) ---
  if (!isTouching && wasTouching) {
    wasTouching = false;
    _lastTouchY = -1;

    // Only register tap if short duration and not dragged far
    if (!_isDragging && (millis() - touchStartTime < 500)) {
      int tx = touchStartX;
      int ty = touchStartY;

      // Global Back Button (Top Left < 60x60)
      // Check modes that have a back button
      if (tx < 60 && ty < 60) {
        if (_currentMode == MODE_MENU) {
          _ui->switchScreen(SCREEN_MENU);
          return;
        } else if (_currentMode == MODE_GROUPS) {
          _currentMode = MODE_MENU;
          // Clear only content area
          _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                                  SCREEN_HEIGHT - STATUS_BAR_HEIGHT, COLOR_BG);
          drawMenu();
          return;
        } else if (_currentMode == MODE_LIST) {
          _currentMode = MODE_GROUPS;
          _scrollOffset = 0;
          _selectedIdx = -1;
          // Clear only content area
          _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                                  SCREEN_HEIGHT - STATUS_BAR_HEIGHT, COLOR_BG);
          drawGroups(0);
          return;
        } else if (_currentMode == MODE_OPTIONS) {
          _currentMode = MODE_LIST;
          // Clear only content area
          _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                                  SCREEN_HEIGHT - STATUS_BAR_HEIGHT, COLOR_BG);
          drawList(_scrollOffset);
          return;
        } else if (_currentMode == MODE_VIEW_DATA) {
          _currentMode = MODE_OPTIONS;
          // Clear only content area
          _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                                  SCREEN_HEIGHT - STATUS_BAR_HEIGHT, COLOR_BG);
          _selectedIdx = 0;
          drawOptions();
          return;
        }
        // MODE_CONFIRM_DELETE doesn't have standard back, handled below
      }

      // Mode Specific Tap Logic
      if (_currentMode == MODE_MENU) {
        int startY = 80;
        int btnH = 40;
        int gap = 20;
        int btnW = 240;
        int x = (SCREEN_WIDTH - btnW) / 2;

        // Button 0
        if (tx > x && tx < x + btnW && ty > startY && ty < startY + btnH) {
          _selectedType = "TRACK";
          _currentMode = MODE_GROUPS;
          scanGroups();
          _scrollOffset = 0;
          _selectedIdx = -1;
          // Clear only content area
          _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                                  SCREEN_HEIGHT - STATUS_BAR_HEIGHT, COLOR_BG);
          drawGroups(0);
        }
        // Button 1
        int y1 = startY + btnH + gap;
        if (tx > x && tx < x + btnW && ty > y1 && ty < y1 + btnH) {
          _selectedType = "DRAG";
          _currentMode = MODE_GROUPS;
          scanGroups();
          _scrollOffset = 0;
          _selectedIdx = -1;
          // Clear only content area
          _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                                  SCREEN_HEIGHT - STATUS_BAR_HEIGHT, COLOR_BG);
          drawGroups(0);
        }

      } else if (_currentMode == MODE_GROUPS) {
        int listY = 50;
        int itemH = 30;
        if (ty > listY) {
          int visIdx = (ty - listY) / itemH;
          int actualIdx = visIdx + _scrollOffset;
          if (actualIdx >= 0 && actualIdx < _groups.size()) {
            _selectedGroup = _groups[actualIdx];
            _currentMode = MODE_LIST;
            _scrollOffset = 0;
            _selectedIdx = -1;
            // Clear only content area
            _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                                    SCREEN_HEIGHT - STATUS_BAR_HEIGHT,
                                    COLOR_BG);
            drawList(0);
          }
        }

      } else if (_currentMode == MODE_LIST) {
        int listY = 50;
        int itemH = 25;
        if (ty > listY) {
          int visIdx = (ty - listY) / itemH;
          int count = 0;
          int skip = 0;
          int targetIdx = -1;
          for (int i = 0; i < _historyList.size(); i++) {
            if (_historyList[i].type == _selectedType) {
              if (_historyList[i].date.length() >= 10) {
                String g = _historyList[i].date.substring(6, 10) + "-" +
                           _historyList[i].date.substring(3, 5);
                if (g == _selectedGroup) {
                  if (skip < _scrollOffset) {
                    skip++;
                    continue;
                  }
                  if (count == visIdx) {
                    targetIdx = i;
                    break;
                  }
                  count++;
                }
              }
            }
          }
          if (targetIdx != -1) {
            _lastTapIdx = targetIdx;
            _currentMode = MODE_OPTIONS;
            _selectedIdx = 0;
            // Clear only content area
            _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                                    SCREEN_HEIGHT - STATUS_BAR_HEIGHT,
                                    COLOR_BG);
            drawOptions();
          }
        }

      } else if (_currentMode == MODE_OPTIONS) {
        int startY = 60;
        int h = 50;
        int idx = (ty - startY) / h;
        if (idx >= 0 && idx < 3) {
          _selectedIdx = idx;
          drawOptions();
          if (idx == 0) { // View Data
            _currentMode = MODE_VIEW_DATA;
            _viewPage = 0;
            _ui->getTft()->fillScreen(TFT_BLACK);
            drawViewData();
          } else if (idx == 1) {
            // Sync placeholder
          } else if (idx == 2) { // Delete
            _currentMode = MODE_CONFIRM_DELETE;
            _selectedIdx = 1;
            drawConfirmDelete();
          }
        }

      } else if (_currentMode == MODE_VIEW_DATA) {
        // Tap anywhere (except back which is handled)
        _viewPage++;
        if (_viewPage > 4)
          _viewPage = 0;
        drawViewData();

      } else if (_currentMode == MODE_CONFIRM_DELETE) {
        int y = 160;
        int btnH = 40;
        if (ty > y && ty < y + btnH) {
          int btnW = 100;
          int gap = 20;
          int startX = (SCREEN_WIDTH - (btnW * 2 + gap)) / 2;
          int idx = -1;
          if (tx > startX && tx < startX + btnW)
            idx = 0; // Yes
          else if (tx > startX + btnW + gap && tx < startX + btnW + gap + btnW)
            idx = 1; // No

          if (idx != -1) {
            _selectedIdx = idx;
            drawConfirmDelete();
            if (idx == 0) { // YES
              if (_lastTapIdx >= 0 && _lastTapIdx < _historyList.size()) {
                sessionManager.deleteSession(
                    _historyList[_lastTapIdx].filename);
                scanHistory();
                scanGroups();
                _currentMode = MODE_LIST;
                _selectedIdx = -1;
                // Clear only content area
                _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                                        SCREEN_HEIGHT - STATUS_BAR_HEIGHT,
                                        COLOR_BG);
                drawList(0);
              }
            } else { // NO
              _currentMode = MODE_OPTIONS;
              _selectedIdx = 2;
              // Clear only content area
              _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                                      SCREEN_HEIGHT - STATUS_BAR_HEIGHT,
                                      COLOR_BG);
              drawOptions();
            }
          }
        }
      }
    }
  }
}

void HistoryScreen::scanHistory() {
  _historyList.clear();
  String content = sessionManager.loadHistoryIndex();

  // Uraikan CSV: nama file,tanggal,lap,lap terbaik,TIPE(opt)
  int start = 0;
  while (start < content.length()) {
    int end = content.indexOf('\n', start);
    if (end == -1)
      end = content.length();

    String line = content.substring(start, end);
    line.trim();

    if (line.length() > 0) {
      // Pisahkan koma
      int c1 = line.indexOf(',');
      int c2 = line.indexOf(',', c1 + 1);
      int c3 = line.indexOf(',', c2 + 1);
      int c4 = line.indexOf(',', c3 + 1); // Type separator

      if (c1 > 0 && c2 > 0 && c3 > 0) {
        HistoryItem item;
        item.filename = line.substring(0, c1);
        item.date = line.substring(c1 + 1, c2);
        item.laps = line.substring(c2 + 1, c3).toInt();
        item.bestLap =
            line.substring(c3 + 1, (c4 > 0) ? c4 : line.length()).toInt();

        if (c4 > 0) {
          item.type = line.substring(c4 + 1);
          item.type.trim();
        } else {
          item.type = "TRACK"; // Default backward compatibility
        }

        _historyList.insert(_historyList.begin(),
                            item); // Tambahkan di awal (Terbaru dulu)
      }
    }
    start = end + 1;
  }
}

void HistoryScreen::drawMenu() {
  TFT_eSPI *tft = _ui->getTft();

  // Header
  tft->setTextColor(COLOR_TEXT, COLOR_BG);
  tft->setTextDatum(TL_DATUM);
  tft->setFreeFont(&Org_01);
  tft->setTextSize(2);
  tft->drawString("<", 10, 25);

  tft->setTextDatum(TC_DATUM);
  tft->setFreeFont(&Org_01);
  tft->setTextSize(2);
  tft->drawString("HISTORY MENU", SCREEN_WIDTH / 2, 40);

  tft->drawFastHLine(0, 60, SCREEN_WIDTH, COLOR_SECONDARY);

  // Buttons
  int startY = 80;
  int btnHeight = 40;
  int gap = 20;
  int btnWidth = 240;
  int x = (SCREEN_WIDTH - btnWidth) / 2;

  const char *items[] = {"TRACK HISTORY", "DRAG HISTORY"};

  for (int i = 0; i < 2; i++) {
    int y = startY + i * (btnHeight + gap);

    uint16_t btnColor = (i == _selectedIdx) ? TFT_RED : TFT_DARKGREY;

    tft->fillRoundRect(x, y, btnWidth, btnHeight, 5, btnColor);
    tft->setTextColor(TFT_WHITE, btnColor);
    tft->setTextDatum(MC_DATUM);
    tft->drawString(items[i], SCREEN_WIDTH / 2, y + btnHeight / 2 + 2);
  }
}

void HistoryScreen::drawGroups(int scrollOffset) {
  TFT_eSPI *tft = _ui->getTft();
  _ui->drawStatusBar();

  // Header
  tft->fillRect(0, 0, SCREEN_WIDTH, 20, TFT_BLACK);

  // Custom Header Bar "My sessions"
  tft->fillRect(0, 20, SCREEN_WIDTH, 25, TFT_BLACK);
  tft->drawFastHLine(0, 45, SCREEN_WIDTH, TFT_WHITE);

  tft->setTextColor(TFT_WHITE, TFT_BLACK);
  tft->setTextDatum(MC_DATUM); // Centered
  tft->setFreeFont(&Org_01);
  tft->setTextSize(1); // Standard
  tft->drawString("My sessions", SCREEN_WIDTH / 2, 32);

  int startY = 50;
  int itemH = 30;
  int count = 0;
  int skip = 0;

  for (int i = 0; i < _groups.size(); i++) {
    if (count >= 5)
      break;
    if (skip < scrollOffset) {
      skip++;
      continue;
    }

    int y = startY + (count * itemH);
    bool selected = (i == _selectedIdx);

    uint16_t bg = selected ? TFT_WHITE : TFT_BLACK;
    uint16_t fg = selected ? TFT_BLACK : TFT_WHITE;

    tft->fillRect(0, y, SCREEN_WIDTH, itemH, bg);
    tft->setTextColor(fg, bg);
    tft->setTextDatum(TL_DATUM);
    tft->setTextFont(2); // Larger blocky font

    tft->drawString(_groups[i], 10, y + 8);

    count++;
  }

  if (_groups.size() == 0) {
    tft->setTextDatum(MC_DATUM);
    tft->setTextColor(TFT_WHITE, TFT_BLACK);
    tft->drawString("No Groups Found", SCREEN_WIDTH / 2, 80);
  }
}

void HistoryScreen::drawList(int scrollOffset) {
  TFT_eSPI *tft = _ui->getTft();
  _ui->drawStatusBar();

  // Header "My sessions - [Count]"
  // Calculate total in this group
  int totalInGroup = 0;
  for (const auto &h : _historyList) {
    if (h.type == _selectedType && h.date.length() >= 10) {
      String g = h.date.substring(6, 10) + "-" + h.date.substring(3, 5);
      if (g == _selectedGroup)
        totalInGroup++;
    }
  }

  tft->fillRect(0, 20, SCREEN_WIDTH, 25, TFT_BLACK);
  tft->drawFastHLine(0, 45, SCREEN_WIDTH, TFT_WHITE);

  tft->setTextColor(TFT_WHITE, TFT_BLACK);
  tft->setTextDatum(MC_DATUM);
  tft->setFreeFont(&Org_01);
  tft->setTextSize(1);
  tft->drawString("My sessions - " + String(totalInGroup), SCREEN_WIDTH / 2,
                  32);

  int startY = 50;
  int itemH =
      20; // 30->20 tighter list? Image 2 has 6 items active. 320/20 = 16 lines?
  // Image 2 has 6 items taking up most of screen. 6*30 = 180 + 50 = 230. 240
  // height.
  itemH = 25; // Good balance

  int count = 0;
  int skip = 0;

  // We need to iterate _historyList to find items in this group
  // And we need to assign IDs. Let's assume ID = TotalItems - GlobalIndex
  // (Latest = MaxID) Or just TotalInGroup - IndexInGroup
  int currentGroupIdx = 0;

  for (int i = 0; i < _historyList.size(); i++) {
    if (_historyList[i].type != _selectedType)
      continue;

    // Filter by Group
    String g = "";
    if (_historyList[i].date.length() >= 10) {
      g = _historyList[i].date.substring(6, 10) + "-" +
          _historyList[i].date.substring(3, 5);
    }
    if (g != _selectedGroup)
      continue;

    // Valid item in group
    // ID logic: If list is sorted Newest First (index 0), then ID should be
    // descending? Image 2 shows 023, 022, 021 (Newest at top). So ID =
    // totalInGroup - currentGroupIdx
    int idVal = totalInGroup - currentGroupIdx;
    currentGroupIdx++;

    // Pagination
    if (count >= 6) { // Show 6 items
      continue; // Don't break, need to continue logic if we want total? No we
                // calculated total.
                // break;
    }

    if (skip < scrollOffset) {
      skip++;
      continue;
    }
    if (count >= 6)
      break; // Optimization

    // Draw Item
    // Format: [ID] [YYYY-MM-DD] [HH:MM:SS]
    // Parse "DD/MM/YYYY HH:MM:SS" -> "YYYY-MM-DD"
    // ID: 3 digits %03d

    int y = startY + (count * itemH);

    // Selection Highlight? Image 2 doesn't show explicit selection bar, but
    // typical UI does. Assuming we can select rows. But wait, Image 2 has no
    // Highlight. It's just a list. Wait, Image 1 has highlight. Image 2 must
    // support selection to Open Options. We'll mimic Image 1 highlight style
    // (Inverted Colors). But which one is selected? _selectedIdx tracks
    // relative visible index or group index? In update(), we used visible
    // index.

    // Wait, in update() logic: _selectedIdx tracks VISIBLE index?
    // No, scanGroups logic: _selectedIdx is bound to "actualIdx".
    // drawList logic needs to match update() logic.
    // In update() for MODE_LIST:
    // int visIdx = (p.y - listY) / itemH;
    // We map click to item.
    // We don't necessarily maintain a persistent highlight unless using
    // buttons. For touch, usually checking "active" tap. But let's support
    // highlighting if we use physical buttons later or just for feedback.

    // Let's just draw white on black.

    tft->fillRect(0, y, SCREEN_WIDTH, itemH, TFT_BLACK);
    tft->setTextColor(TFT_WHITE, TFT_BLACK);
    tft->setTextDatum(TL_DATUM);
    tft->setTextFont(1); // Small font to fit line? Or Font 2?
    // Font 2 might be too wide for "023 2025-05-18 14:40:45" (24 chars)
    // 24 * ~10px = 240px. might fit exactly.

    // Parse Date
    String dRaw = _historyList[i].date.substring(0, 10); // DD/MM/YYYY
    String tRaw = (_historyList[i].date.length() > 11)
                      ? _historyList[i].date.substring(11)
                      : "";

    String yyyy = dRaw.substring(6, 10);
    String mm = dRaw.substring(3, 5);
    String dd = dRaw.substring(0, 2);
    String dDisp = yyyy + "-" + mm + "-" + dd;

    char bufID[16];
    sprintf(bufID, "%03d", idVal);

    // X Positions
    // ID: 5
    // Date: 40
    // Time: 140

    tft->drawString(bufID, 5, y + 4, 2); // Font 2
    tft->drawString(dDisp, 45, y + 4, 2);
    tft->drawString(tRaw, 155, y + 4, 2);

    count++;
  }
}

void HistoryScreen::scanGroups() {
  _groups.clear();
  for (const auto &item : _historyList) {
    // Filter by type first
    if (item.type != _selectedType)
      continue;

    // Date format: "DD/MM/YYYY" -> "YYYY-MM"
    if (item.date.length() >= 10) {
      String yyyy = item.date.substring(6, 10);
      String mm = item.date.substring(3, 5);
      String group = yyyy + "-" + mm;

      bool exists = false;
      for (const auto &g : _groups) {
        if (g == group) {
          exists = true;
          break;
        }
      }
      if (!exists)
        _groups.push_back(group);
    }
  }
}

void HistoryScreen::drawOptions() {
  TFT_eSPI *tft = _ui->getTft();
  _ui->drawStatusBar();

  // Header
  tft->drawFastHLine(0, 20, SCREEN_WIDTH, TFT_WHITE);
  tft->setTextColor(TFT_WHITE, TFT_BLACK);
  tft->setTextDatum(TC_DATUM);
  tft->setFreeFont(&Org_01);
  tft->setTextSize(1);
  tft->drawString("SESSION OPTIONS", SCREEN_WIDTH / 2, 25);
  tft->drawFastHLine(0, 45, SCREEN_WIDTH, TFT_WHITE);

  // Options
  const char *options[] = {"1. View Data", "2. Synchronize",
                           "3. Delete Session"};
  int startY = 60;
  int h = 40;

  for (int i = 0; i < 3; i++) {
    int y = startY + (i * 50);
    bool sel = (i == _selectedIdx);

    if (sel) {
      tft->fillRoundRect(20, y, SCREEN_WIDTH - 40, h, 5, TFT_WHITE);
      tft->setTextColor(TFT_BLACK, TFT_WHITE);
    } else {
      tft->fillRoundRect(20, y, SCREEN_WIDTH - 40, h, 5, TFT_DARKGREY);
      tft->setTextColor(TFT_WHITE, TFT_DARKGREY);
    }

    tft->setTextDatum(MC_DATUM);
    tft->setTextFont(2);
    tft->drawString(options[i], SCREEN_WIDTH / 2, y + h / 2);
  }
}

void HistoryScreen::drawViewData() {
  TFT_eSPI *tft = _ui->getTft();
  tft->fillScreen(TFT_BLACK);
  _ui->drawStatusBar();

  // Page Header
  tft->setTextColor(TFT_WHITE, TFT_BLACK);
  tft->setTextDatum(TC_DATUM);
  tft->setFreeFont(&Org_01);
  tft->setTextSize(1);

  String title = "";
  switch (_viewPage) {
  case 0:
    title = "SPEED DETAILS";
    break;
  case 1:
    title = "RPM DETAILS";
    break;
  case 2:
    title = "TEMP DETAILS";
    break;
  case 3:
    title = "SECTOR ANALYSIS";
    break;
  case 4:
    title = "LAP REPLAY";
    break;
  }
  tft->drawString(title, SCREEN_WIDTH / 2, 25);

  // Valid data check
  if (_lastTapIdx < 0 || _lastTapIdx >= _historyList.size())
    return;
  // Note: _lastTapIdx here needs to track the ACTUAL selected session index
  // from LIST mode. We should store the selected session index in a separate
  // variable if _lastTapIdx is transient using tap logic. Let's assume
  // _selectedIdx in MODE_LIST pointed to the filtered index, we need to map it.
  // However, HistoryScreen::update currently maps visible index to actual list
  // index. We'll fix this in update() by storing `_activeSessionIdx`.

  // Placeholder Content
  tft->setTextDatum(MC_DATUM);
  tft->setTextFont(2);
  tft->drawString("Page " + String(_viewPage + 1) + "/5", SCREEN_WIDTH / 2,
                  100);
  tft->drawString("Content Placeholder", SCREEN_WIDTH / 2, 130);

  if (_viewPage == 4) {
    tft->drawString("(Replay Animation)", SCREEN_WIDTH / 2, 160);
  }

  // Navigation hint
  tft->setTextSize(1);
  tft->drawString("Tap OK for Next", SCREEN_WIDTH / 2, 220);
}

void HistoryScreen::drawConfirmDelete() {
  TFT_eSPI *tft = _ui->getTft();
  tft->fillScreen(TFT_BLACK);
  _ui->drawStatusBar();

  tft->setTextColor(TFT_RED, TFT_BLACK);
  tft->setTextDatum(MC_DATUM);
  tft->setFreeFont(&Org_01);
  tft->setTextSize(2); // Large warning
  tft->drawString("DELETE?", SCREEN_WIDTH / 2, 80);

  tft->setTextSize(1);
  tft->setTextColor(TFT_WHITE, TFT_BLACK);
  tft->drawString("Confirm Permanent Delete", SCREEN_WIDTH / 2, 120);

  // Yes / No options
  int btnW = 100;
  int btnH = 40;
  int gap = 20;
  int startX = (SCREEN_WIDTH - (btnW * 2 + gap)) / 2;
  int y = 160;

  // YES
  bool selYes = (_selectedIdx == 0);
  tft->fillRoundRect(startX, y, btnW, btnH, 5, selYes ? TFT_RED : TFT_DARKGREY);
  tft->setTextColor(TFT_WHITE, selYes ? TFT_RED : TFT_DARKGREY);
  tft->drawString("YES", startX + btnW / 2, y + btnH / 2);

  // NO
  bool selNo = (_selectedIdx == 1);
  tft->fillRoundRect(startX + btnW + gap, y, btnW, btnH, 5,
                     selNo ? TFT_GREEN : TFT_DARKGREY);
  tft->setTextColor(TFT_BLACK, selNo ? TFT_GREEN : TFT_DARKGREY);
  tft->drawString("NO", startX + btnW + gap + btnW / 2, y + btnH / 2);
}
