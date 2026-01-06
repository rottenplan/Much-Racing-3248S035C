#include "HistoryScreen.h"
#include "../../config.h"
#include "../../core/SessionManager.h"
#include "../fonts/Org_01.h"

extern SessionManager sessionManager;

void HistoryScreen::onShow() {
  _scrollOffset = 0;
  _showingDetails = false;
  _selectedIdx = -1;
  scanHistory();

  TFT_eSPI *tft = _ui->getTft();
  _ui->drawStatusBar(); // Gambar Bilah Status
  drawList(0);
}

void HistoryScreen::update() {
  static unsigned long lastHistoryTouch = 0;
  UIManager::TouchPoint p = _ui->getTouchPoint();
  if (p.x == -1)
    return;

  if (_showingDetails) {
    // Kembali dari Detail (Area Header 0-40 biasanya, tapi mari kita periksa drawDetails)
    // drawDetails header adalah 0-40. Jadi <40 benar di sana.
    if (p.y < 40) {
      _showingDetails = false;
      _ui->getTft()->fillScreen(COLOR_BG);
      drawList(_scrollOffset);
      return;
    }
  } else {
  // Tampilan Daftar (Area Header 20-60)
    // Tombol Kembali
    if (p.x < 60 && p.y < 60) {
      if (millis() - lastHistoryTouch < 200) return;
      lastHistoryTouch = millis();

      if (_selectedIdx == -2) {
         _ui->switchScreen(SCREEN_MENU);
      } else {
         _selectedIdx = -2;
         drawList(_scrollOffset);
      }
      return;
    }

    // Gulir/Pilih
    // Area daftar: Y=70 hingga 240
    // Item setinggi ~40px.
    int listY = 70;
    int itemH = 40;

    if (p.y > listY) {
      // Debounce checks
      if (millis() - lastHistoryTouch < 200) return;
      lastHistoryTouch = millis();

      int clickedIdx = _scrollOffset + ((p.y - listY) / itemH);
      if (clickedIdx < _historyList.size()) {
        if (_selectedIdx == clickedIdx) {
            // Second Tap -> Open Details
            _showingDetails = true;
            _ui->getTft()->fillScreen(COLOR_BG);
            drawDetails(_selectedIdx);
        } else {
            // First Tap -> Highlight
            _selectedIdx = clickedIdx;
            drawList(_scrollOffset);
        }
      }
    }
  }

  // Perbarui Bilah Status
  static unsigned long lastStatusUpdate = 0;
  if (millis() - lastStatusUpdate > 1000) {
    _ui->drawStatusBar();
    lastStatusUpdate = millis();
  }
}

void HistoryScreen::scanHistory() {
  _historyList.clear();
  String content = sessionManager.loadHistoryIndex();

  // Uraikan CSV: nama file,tanggal,lap,lap terbaik
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

      if (c1 > 0 && c2 > 0 && c3 > 0) {
        HistoryItem item;
        item.filename = line.substring(0, c1);
        item.date = line.substring(c1 + 1, c2);
        item.laps = line.substring(c2 + 1, c3).toInt();
        item.bestLap = line.substring(c3 + 1).toInt(); // Asumsi millis
        _historyList.insert(_historyList.begin(),
                            item); // Tambahkan di awal (Terbaru dulu)
      }
    }
    start = end + 1;
  }
}

void HistoryScreen::drawList(int scrollOffset) {
  TFT_eSPI *tft = _ui->getTft();

  // Header
  // Header
  // Bilah Status 0-20. 
  
  // Panah Kembali
  tft->setTextColor(COLOR_TEXT, COLOR_BG);
  tft->setTextDatum(TL_DATUM);
  tft->setFreeFont(&Org_01); // Atau standar
  tft->setTextSize(2); // Besar untuk sentuhan mudah
  tft->drawString("<", 10, 25);

  // Daftar
  int startY = 70; // Dipindahkan ke atas (sebelumnya 70)
  int itemH = 40;
  int count = 0;

  for (int i = scrollOffset; i < _historyList.size(); i++) {
    if (count >= 4)
      break; // Tampilkan 4 item

    HistoryItem &item = _historyList[i];
    int y = startY + (count * itemH);

    // Latar belakang untuk item (bergantian?)
    if (count % 2 == 0)
      tft->fillRect(0, y, SCREEN_WIDTH, itemH, 0x10A2); // Abu-abu Gelap

    // Tanggal (Kiri Atas)
    tft->setTextColor(COLOR_TEXT); // Latar belakang transparan
    tft->setTextDatum(TL_DATUM);
    tft->setTextFont(2);
    tft->setTextSize(1);
    tft->drawString(item.date, 10, y + 2);

    // Statistik (Kiri/Kanan Bawah)
    tft->setTextSize(1);
    tft->setTextFont(2); // Tetap font 2
    tft->setTextColor(COLOR_SECONDARY);
    String sub = "Laps: " + String(item.laps);
    tft->drawString(sub, 10, y + 22);

    // Lap Terbaik (Kanan)
    tft->setTextDatum(TR_DATUM);
    tft->setTextColor(COLOR_ACCENT); // Hijau/Teal
    tft->setTextFont(2);
    tft->setTextSize(1);

    int ms = item.bestLap % 1000;
    int s = (item.bestLap / 1000) % 60;
    int m = (item.bestLap / 60000);
    char buf[16];
    sprintf(buf, "%02d:%02d.%02d", m, s, ms / 10);

    tft->drawString(buf, SCREEN_WIDTH - 10, y + 8);

    count++;
  }

  if (_historyList.size() == 0) {
    tft->setTextDatum(MC_DATUM);
    tft->setTextColor(COLOR_SECONDARY, COLOR_BG);
    tft->drawString("No Sessions Found", SCREEN_WIDTH / 2, 120);
  }
}

void HistoryScreen::drawDetails(int idx) {
  if (idx < 0 || idx >= _historyList.size())
    return;
  HistoryItem &item = _historyList[idx];

  TFT_eSPI *tft = _ui->getTft();

  // Header
  // Detail Header
  // Panah Kembali
  tft->setTextColor(COLOR_TEXT, COLOR_BG);
  tft->setTextDatum(TL_DATUM);
  tft->setFreeFont(&Org_01);
  tft->setTextSize(2);
  tft->drawString("<", 10, 25);

  // Info
  int y = 70;
  tft->setTextDatum(TL_DATUM);
  tft->setTextColor(COLOR_TEXT, COLOR_BG);

  tft->drawString("Date: " + item.date, 20, y);
  y += 30;
  tft->drawString("Total Laps: " + String(item.laps), 20, y);
  y += 30;

  // Lap Terbaik
  tft->drawString("Best Lap:", 20, y);
  tft->setTextColor(TFT_GREEN, COLOR_BG);

  int ms = item.bestLap % 1000;
  int s = (item.bestLap / 1000) % 60;
  int m = (item.bestLap / 60000);
  char buf[16];
  sprintf(buf, "%02d:%02d.%02d", m, s, ms / 10);
  tft->setTextSize(3);
  tft->drawString(buf, 20, y + 25);

  // Catatan: Mengurai CSV sebenarnya untuk daftar lap lengkap itu rumit.
  // Untuk saat ini, tampilan ringkasan ini cocok dengan permintaan pengguna "seperti RaceBox" daftar
  // riwayat.
}
