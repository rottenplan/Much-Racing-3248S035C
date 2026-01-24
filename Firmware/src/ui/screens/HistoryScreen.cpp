#include "HistoryScreen.h"
#include "../../config.h"
#include "../../core/SessionManager.h"
#include "../fonts/Org_01.h"

extern SessionManager sessionManager;

void HistoryScreen::onShow() {
  _scrollOffset = 0;
  _currentMode = MODE_MENU; // Start at Menu
  _selectedIdx = -1;
  scanHistory();

  // Reset Variables
  _wasTouching = false;
  _touchStartX = -1;
  _touchStartY = -1;
  _touchStartTime = 0;
  _lastBackTapTime = 0;
  _isDragging = false;
  _ignoreInitialTouch = true;

  TFT_eSPI *tft = _ui->getTft();
  // Safe Clear (Keep Status Bar)
  tft->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                SCREEN_HEIGHT - STATUS_BAR_HEIGHT, TFT_BLACK);
  _ui->drawStatusBar(true);

  // --- STATIC HEADER ---
  tft->drawFastHLine(0, 20, SCREEN_WIDTH, COLOR_SECONDARY);

  tft->setTextColor(TFT_WHITE, TFT_BLACK);
  tft->setTextDatum(TC_DATUM);
  tft->setFreeFont(&Org_01);
  tft->setTextSize(2);
  tft->drawString("HISTORY", SCREEN_WIDTH / 2, 28);

  // Back Button (Blue Triangle) - Bottom Left
  tft->fillTriangle(10, 220, 22, 214, 22, 226, TFT_BLUE);

  drawMenu();
}

void HistoryScreen::update() {
  // Global Touch State Tracking
  // Removed static variables to avoid stale state
  // static bool wasTouching = false; ...

  // Status Bar Update - Handled by UIManager globally
  // if (millis() - lastStatusUpdate > 1000) { ... }

  UIManager::TouchPoint p = _ui->getTouchPoint();
  bool isTouching = (p.x != -1);

  // Anti-Ghosting / Debounce Logic
  if (_ignoreInitialTouch) {
    if (!isTouching) {
      _ignoreInitialTouch = false; // Finger released, ready for input
    } else {
      return; // Ignore lingering touch
    }
  }

  // --- STATE MACHINE: START ---
  if (isTouching && !_wasTouching) {
    _touchStartX = p.x;
    _touchStartY = p.y;
    _touchStartTime = millis();
    _isDragging = false;
    _lastTouchY = p.y;
    _wasTouching = true;
  }

  // --- STATE MACHINE: DRAGGING ---
  if (isTouching && _wasTouching) {
    int dy = p.y - _lastTouchY;
    if (abs(p.y - _touchStartY) > _dragThreshold) {
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
  if (!isTouching && _wasTouching) {
    _wasTouching = false;
    _lastTouchY = -1;

    // Only register tap if short duration and not dragged far
    if (!_isDragging && (millis() - _touchStartTime < 500)) {
      int tx = _touchStartX;
      int ty = _touchStartY;

      // Global Back Button (Bottom Left < 60, > 180) - Matching GNSS Log
      // style
      // Global Back Button (Bottom Left < 60, > 180) - Matching GNSS Log style
      if (tx < 60 && ty > 180) {
        // Single Tap Logic
        if (_currentMode == MODE_MENU) {
          _ui->switchScreen(SCREEN_MENU);
          return;
        } else if (_currentMode == MODE_GROUPS) {
          _currentMode = MODE_MENU;
          // Clear only content area (Below Header Y=40)
          _ui->getTft()->fillRect(0, 40, SCREEN_WIDTH, SCREEN_HEIGHT - 40,
                                  COLOR_BG);
          drawMenu();
          return;
        } else if (_currentMode == MODE_LIST) {
          _currentMode = MODE_GROUPS;
          _scrollOffset = 0;
          _selectedIdx = -1;
          // Clear only content area
          _ui->getTft()->fillRect(0, 40, SCREEN_WIDTH, SCREEN_HEIGHT - 40,
                                  COLOR_BG);
          drawGroups(0);
          return;
        } else if (_currentMode == MODE_OPTIONS) {
          _currentMode = MODE_LIST;
          // Clear only content area
          _ui->getTft()->fillRect(0, 40, SCREEN_WIDTH, SCREEN_HEIGHT - 40,
                                  COLOR_BG);
          drawList(_scrollOffset);
          return;
        } else if (_currentMode == MODE_VIEW_DATA) {
          _currentMode = MODE_OPTIONS;
          // Clear only content area
          _ui->getTft()->fillRect(0, 40, SCREEN_WIDTH, SCREEN_HEIGHT - 40,
                                  COLOR_BG);
          _selectedIdx = 0;
          drawOptions();
          return;
        }
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
            _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                                    SCREEN_HEIGHT - STATUS_BAR_HEIGHT,
                                    TFT_BLACK);
            drawViewData();
          } else if (idx == 1) {
            // Sync placeholder
          } else if (idx == 2) { // Delete
            _currentMode = MODE_CONFIRM_DELETE;
            _selectedIdx = 1;
            _ui->getTft()->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                                    SCREEN_HEIGHT - STATUS_BAR_HEIGHT,
                                    TFT_BLACK);
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

  // Clear Content Area (Below Header line Y=20, down to footer Y=210)
  tft->fillRect(0, 50, SCREEN_WIDTH, 160, TFT_BLACK);

  tft->setTextDatum(MC_DATUM);
  tft->setFreeFont(&Org_01);
  tft->setTextSize(1);
  tft->setTextColor(TFT_WHITE, TFT_BLACK);

  // Menu Options (Drag / Lap)
  int startY = 60;
  int btnHeight = 45;
  int gap = 15;
  int btnWidth = 240;
  int x = (SCREEN_WIDTH - btnWidth) / 2;

  const char *items[] = {"TRACK HISTORY", "DRAG HISTORY"};

  for (int i = 0; i < 2; i++) {
    int y = startY + i * (btnHeight + gap);
    // Draw Button
    tft->drawRect(x, y, btnWidth, btnHeight, TFT_DARKGREY);
    tft->drawString(items[i], SCREEN_WIDTH / 2, y + (btnHeight / 2));
  }
}

void HistoryScreen::drawGroups(int scrollOffset) {
  TFT_eSPI *tft = _ui->getTft();

  // Clear Content Area (Below Header)
  // Clear Content Area (Below Header)
  tft->fillRect(0, 40, SCREEN_WIDTH, 170, TFT_BLACK);

  // Sub-Header "- SESSIONS -"
  tft->setTextColor(TFT_SILVER, TFT_BLACK);
  tft->setTextDatum(MC_DATUM);
  tft->setFreeFont(NULL);
  tft->setTextSize(1);
  tft->drawString("- SESSIONS -", SCREEN_WIDTH / 2, 50);

  int startY = 70;
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

  // Clear any remaining space at the bottom to prevent ghosting
  int nextY = startY + (count * itemH);
  if (nextY < SCREEN_HEIGHT) {
    tft->fillRect(0, nextY, SCREEN_WIDTH, SCREEN_HEIGHT - nextY, TFT_BLACK);
  }

  if (_groups.empty()) {
    tft->setTextColor(TFT_WHITE, TFT_BLACK);
    tft->drawString("No Groups Found", SCREEN_WIDTH / 2, 120);
  }
}

void HistoryScreen::drawList(int scrollOffset) {
  TFT_eSPI *tft = _ui->getTft();

  // Calculate total in this group
  int totalInGroup = 0;
  for (const auto &h : _historyList) {
    if (h.type == _selectedType && h.date.length() >= 10) {
      String g = h.date.substring(6, 10) + "-" + h.date.substring(3, 5);
      if (g == _selectedGroup)
        totalInGroup++;
    }
  }

  // Clear Content Area
  tft->fillRect(0, 40, SCREEN_WIDTH, 170, TFT_BLACK);

  // Sub-Header
  tft->setTextColor(TFT_SILVER, TFT_BLACK);
  tft->setTextDatum(MC_DATUM);
  tft->setFreeFont(NULL);
  tft->setTextSize(1);
  String sub = "Total: " + String(totalInGroup);
  tft->drawString(sub, SCREEN_WIDTH / 2, 50);

  int startY = 70; // Lowered to Y=70
  int itemH = 25;

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

  // Clear any remaining space at the bottom to prevent ghosting
  int nextY = startY + (count * itemH);
  if (nextY < SCREEN_HEIGHT) {
    tft->fillRect(0, nextY, SCREEN_WIDTH, SCREEN_HEIGHT - nextY, TFT_BLACK);
  }

  // Back Triangle
  tft->fillTriangle(10, 220, 22, 214, 22, 226, TFT_BLUE);
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
  // _ui->drawStatusBar();

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

  // Back Triangle
  tft->fillTriangle(10, 220, 22, 214, 22, 226, TFT_BLUE);
}

void HistoryScreen::drawViewData() {
  TFT_eSPI *tft = _ui->getTft();
  // Clear only content area
  tft->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                SCREEN_HEIGHT - STATUS_BAR_HEIGHT, TFT_BLACK);
  // _ui->drawStatusBar(); // Removed to prevent flicker

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

  // Retrieve Analysis
  static SessionManager::SessionAnalysis analysis;
  static String lastLoadedFile = "";

  String currentFile = _historyList[_lastTapIdx].filename;
  if (currentFile != lastLoadedFile) {
    // Show Loading...
    tft->setTextColor(TFT_WHITE, TFT_BLACK);
    tft->setTextDatum(MC_DATUM);
    tft->drawString("Loading...", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
    analysis = sessionManager.analyzeSession(currentFile);
    lastLoadedFile = currentFile;
    // Clear Loading
    tft->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                  SCREEN_HEIGHT - STATUS_BAR_HEIGHT, TFT_BLACK);
  }

  int startY = 50;

  if (_viewPage == 0) {
    // SUMMARY PAGE
    // 4 Grid Box
    int boxW = (SCREEN_WIDTH - 25) / 2;
    int boxH = 50;
    int gap = 5;

    // Box 1: Total Time
    tft->fillRoundRect(10, startY, boxW, boxH, 5, 0x18E3);
    tft->setTextColor(TFT_SILVER, 0x18E3);
    tft->setTextDatum(TL_DATUM);
    tft->drawString("TOTAL TIME", 15, startY + 5);
    // Fmt
    unsigned long tt = analysis.totalTime;
    int ms = tt % 1000;
    int s = (tt / 1000) % 60;
    int m = (tt / 60000) % 60;
    int h = (tt / 3600000);
    char buf[16];
    sprintf(buf, "%02d:%02d:%02d", h, m, s);
    tft->setTextColor(TFT_WHITE, 0x18E3);
    tft->setTextDatum(MC_DATUM);
    tft->setTextFont(4);
    tft->drawString(buf, 10 + boxW / 2, startY + 30);

    // Box 2: Valid Laps (Count)
    tft->fillRoundRect(15 + boxW, startY, boxW, boxH, 5, 0x18E3);
    tft->setTextColor(TFT_SILVER, 0x18E3);
    tft->setTextDatum(TL_DATUM);
    tft->drawString("VALID LAPS", 20 + boxW, startY + 5);
    tft->setTextColor(TFT_SKYBLUE, 0x18E3);
    tft->setTextDatum(MC_DATUM);
    tft->drawString(String(analysis.validLaps), 15 + boxW + boxW / 2,
                    startY + 30);

    // Row 2
    int Y2 = startY + boxH + gap;

    // Box 3: Distance
    tft->fillRoundRect(10, Y2, boxW, boxH, 5, 0x18E3);
    tft->setTextColor(TFT_SILVER, 0x18E3);
    tft->setTextDatum(TL_DATUM);
    tft->drawString("DISTANCE (km)", 15, Y2 + 5);
    tft->setTextColor(TFT_ORANGE, 0x18E3);
    tft->setTextDatum(MC_DATUM);
    tft->drawFloat(analysis.totalDistance, 2, 10 + boxW / 2, Y2 + 30);

    // Box 4: Max Speed
    tft->fillRoundRect(15 + boxW, Y2, boxW, boxH, 5, 0x18E3);
    tft->setTextColor(TFT_SILVER, 0x18E3);
    tft->setTextDatum(TL_DATUM);
    tft->drawString("MAX SPEED", 20 + boxW, Y2 + 5);
    tft->setTextColor(TFT_RED, 0x18E3);
    tft->setTextDatum(MC_DATUM);
    tft->drawFloat(analysis.maxSpeed, 1, 15 + boxW + boxW / 2, Y2 + 30);

    // Best Lap Highlight
    int Y3 = Y2 + boxH + 10;
    tft->drawRect(10, Y3, SCREEN_WIDTH - 20, 50, TFT_DARKGREY);
    tft->setTextColor(TFT_GOLD, TFT_BLACK);
    tft->setTextDatum(MC_DATUM);
    if (analysis.bestLap > 0) {
      unsigned long b = analysis.bestLap;
      int bs = (b / 1000) % 60;
      int bm = (b / 60000);
      int bms = b % 1000;
      sprintf(buf, "BEST: %d:%02d.%02d", bm, bs, bms / 10);
      tft->drawString(buf, SCREEN_WIDTH / 2, Y3 + 25);
    } else {
      tft->drawString("NO LAP DATA", SCREEN_WIDTH / 2, Y3 + 25);
    }

  } else if (_viewPage == 1) {
    // LAP LIST
    // Simple scrollable list?
    // For now, just show first 6 laps or "Coming Soon" for full scroll
    tft->setTextDatum(TL_DATUM);
    tft->setTextColor(TFT_SILVER, TFT_BLACK);
    tft->drawString("LAP TIMES:", 20, startY);

    if (analysis.lapTimes.empty()) {
      tft->setTextColor(TFT_DARKGREY, TFT_BLACK);
      tft->drawString("No Laps Recorded", 20, startY + 30);
    } else {
      int y = startY + 25;
      int count = 0;
      for (unsigned long t : analysis.lapTimes) {
        if (count > 5)
          break; // Show max 6
        int ms = t % 1000;
        int s = (t / 1000) % 60;
        int m = (t / 60000);
        char buf[32];
        sprintf(buf, "%d.  %d:%02d.%02d", count + 1, m, s, ms / 10);

        // Highlight best?
        if (t == analysis.bestLap)
          tft->setTextColor(TFT_GREEN, TFT_BLACK);
        else
          tft->setTextColor(TFT_WHITE, TFT_BLACK);

        tft->drawString(buf, 30, y);
        y += 20;
        count++;
      }
      if (analysis.lapTimes.size() > 6) {
        tft->setTextColor(TFT_DARKGREY, TFT_BLACK);
        tft->drawString("... more laps ...", 30, y);
      }
    }
  } else {
    // Placeholders for Sector/Graphs
    tft->setTextDatum(MC_DATUM);
    tft->setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft->drawString("Analysis Module", SCREEN_WIDTH / 2, 100);
    tft->drawString("Under Construction", SCREEN_WIDTH / 2, 130);
  }

  // Back Triangle
  tft->fillTriangle(10, 220, 22, 214, 22, 226, TFT_BLUE);
}

void HistoryScreen::drawConfirmDelete() {
  TFT_eSPI *tft = _ui->getTft();
  // Clear only content area - MOVED TO CALLER to prevent flicker on update
  // tft->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
  //               SCREEN_HEIGHT - STATUS_BAR_HEIGHT, TFT_BLACK);
  // _ui->drawStatusBar(); // Removed to prevent flicker

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
