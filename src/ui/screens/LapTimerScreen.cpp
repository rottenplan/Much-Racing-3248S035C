#include "LapTimerScreen.h"
#include "../../core/GPSManager.h"
#include "../../core/SessionManager.h"
#include "../fonts/Org_01.h"
#include <algorithm> // Untuk min_element

extern GPSManager gpsManager;
extern SessionManager sessionManager;

// Konstanta untuk Tata Letak UI
#define STATUS_BAR_HEIGHT 20
#define LIST_ITEM_HEIGHT 30

// Tentukan Area Tombol
#define STOP_BTN_Y 200 // Dipindahkan KE BAWAH untuk jarak yang lebih baik
#define STOP_BTN_H 35

void LapTimerScreen::onShow() {
  _lastUpdate = 0;
  _isRecording = false; // Mulai tidak merekam
  _finishSet = false;
  _lapCount = 0;
  _state = STATE_SUMMARY; // Mulai dalam Tampilan Ringkasan/Menu
  _bestLapTime = 0;
  _lapTimes.clear();
  _listScroll = 0;

  TFT_eSPI *tft = _ui->getTft();
  // tft->fillScreen(COLOR_BG); // Sudah dibersihkan oleh UIManager
  drawSummary();
}

void LapTimerScreen::update() {
  UIManager::TouchPoint p = _ui->getTouchPoint();
  bool touched = (p.x != -1);



  if (_state == STATE_SUMMARY) {
    // --- LOGIKA STATUS RINGKASAN ---
    if (touched) {
      // 1. TOMBOL MULAI (Bawah)
      // Rect: y > 200
      if (p.y > 200) {
        // Mulai Balapan
        _state = STATE_RACING;
        _ui->getTft()->fillScreen(COLOR_BG);
        drawRacingStatic(); // Gambar elemen statis sekali
        drawRacing(); // Gambar awal elemen dinamis
        return;
      }

      // 2. KEMBALI/MENU (Kiri Atas)
      // Tombol Kembali (Area Header 20-60)
      if (p.x < 70 && p.y < 70) { // Distandarisasi ke 70x70
        _ui->switchScreen(SCREEN_MENU);
        return;
      }

      // 3. PENGGULIRAN (Tengah)
      // Daftar ada di y=70 hingga ~190
      if (p.y > 70 && p.y < 200) {
        if (p.y < 135) { // Setengah atas
          if (_listScroll > 0)
            _listScroll--;
        } else { // Setengah bawah
          if (_listScroll < _lapTimes.size())
            _listScroll++;
        }
        drawLapList(_listScroll);
      }
    }

  } else {
    // --- LOGIKA STATUS BALAPAN ---
    // Sentuh: BERHENTI/SELESAI
    if (touched) {
      // Tombol Berhenti (Bawah)
      if (p.y > STOP_BTN_Y) {
        _state = STATE_SUMMARY;

        // Simpan Riwayat
        if (sessionManager.isLogging()) {
          // Dapatkan string Tanggal/Waktu?
          String dateStr =
              gpsManager.getDateString() + " " + gpsManager.getTimeString();
          sessionManager.appendToHistoryIndex("Track Session", dateStr,
                                              _lapCount, _bestLapTime);
        }

        sessionManager.stopSession();
        _isRecording = false;
        _ui->getTft()->fillScreen(COLOR_BG);
        drawSummary();
        return;
      }

      // Set Selesai Manual (jika belum diset)
      if (!_finishSet && gpsManager.isFixed() && p.y < 200) {
        _finishLat = gpsManager.getLatitude();
        _finishLon = gpsManager.getLongitude();
        _finishSet = true;
        _ui->getTft()->fillCircle(SCREEN_WIDTH - 20, 35, 5, TFT_GREEN);
      }
    }

    // Logic
    if (_finishSet)
      checkFinishLine();

    // Pencatatan (1Hz atau 10Hz?)
    if (sessionManager.isLogging() && (millis() - _lastUpdate > 100)) {
      String data = String(millis()) + "," +
                    String(gpsManager.getLatitude(), 6) + "," +
                    String(gpsManager.getLongitude(), 6) + "," +
                    String(gpsManager.getSpeedKmph()) + "," +
                    String(gpsManager.getSatellites());
      sessionManager.logData(data);
    }

    // UI Update (10Hz)
    if (millis() - _lastUpdate > 100) {
      drawRacing();
      _ui->drawStatusBar();
      _lastUpdate = millis();
    }
  }
}

// --- PEMBANTU PENGGAMBARAN ---

void LapTimerScreen::drawSummary() {
  TFT_eSPI *tft = _ui->getTft();

  // Header
  // Header (Di Bawah Bilah Status)
  int headerY = STATUS_BAR_HEIGHT; // 20

  // Panah Kembali (Minimal seperti DragMeter)
  tft->setTextColor(COLOR_TEXT, COLOR_BG);
  tft->setTextDatum(TL_DATUM);
  tft->setTextSize(2);
  tft->drawString("<", 10, 35); 

  // Kotak Lap Terbaik (Atas)
  int bestLapY = 60; // Dipindahkan ke atas (sebelumnya 65)
  tft->setTextColor(COLOR_TEXT, COLOR_BG);
  tft->setTextDatum(TC_DATUM);
  tft->setTextFont(2);
  tft->setTextSize(1);
  tft->drawString("BEST LAP", SCREEN_WIDTH / 2, bestLapY);

  char buf[32];
  if (_bestLapTime > 0) {
    int ms = _bestLapTime % 1000;
    int s = (_bestLapTime / 1000) % 60;
    int m = (_bestLapTime / 60000);
    sprintf(buf, "%02d:%02d.%02d", m, s, ms / 10);
  } else {
    sprintf(buf, "--:--.--");
  }

  tft->setTextFont(4); // Gunakan Font 4 untuk angka besar (tinggi sekitar 26px)
  tft->setTextSize(1); // Ukuran standar
  tft->drawString(buf, SCREEN_WIDTH / 2,
                  bestLapY + 25); // Y disesuaikan untuk font 4

  // Daftar (Tengah)
  // Geser daftar sedikit ke atas
  drawLapList(_listScroll);

  // Tombol Mulai (Bawah)
  // Layar H=240. Status=20. Header=40. Daftar=~150.
  // Tombol harus berada di ~200.
  int btnY = 200;
  int btnH = 35;

  tft->fillRoundRect(40, btnY, SCREEN_WIDTH - 80, btnH, 5, TFT_GREEN);
  tft->setTextColor(TFT_BLACK, TFT_GREEN);
  tft->setFreeFont(&Org_01);
  tft->setTextSize(2);
  tft->setTextDatum(MC_DATUM);
  tft->drawString("START", SCREEN_WIDTH / 2,
                  btnY + (btnH / 2) + 2); // +2 pemusatan
}

void LapTimerScreen::drawLapList(int scrollOffset) {
  TFT_eSPI *tft = _ui->getTft();
  int startY = 90;     // Dipindahkan ke atas (sebelumnya 90)
  int itemsToShow = 4; // Tampilkan lebih sedikit item agar muat

  // Hapus Area Daftar
  tft->fillRect(0, startY, SCREEN_WIDTH, itemsToShow * LIST_ITEM_HEIGHT,
                COLOR_BG);

  tft->setTextFont(2);
  tft->setTextSize(1);
  tft->setTextDatum(TL_DATUM);
  tft->setTextColor(COLOR_TEXT, COLOR_BG);

  for (int i = 0; i < itemsToShow; i++) {
    int idx = scrollOffset + i;
    if (idx >= _lapTimes.size())
      break;

    unsigned long t = _lapTimes[idx];
    int ms = t % 1000;
    int s = (t / 1000) % 60;
    int m = (t / 60000);
    char buf[32];
    sprintf(buf, "%d. %02d:%02d.%02d", idx + 1, m, s, ms / 10);

    int y = startY + (i * LIST_ITEM_HEIGHT);
    tft->drawString(buf, 20, y);

    // Sorot Lap Terbaik?
    if (t == _bestLapTime && t > 0) {
      tft->drawRect(15, y - 2, SCREEN_WIDTH - 30, LIST_ITEM_HEIGHT - 2,
                    TFT_GOLD);
    }
  }
}

void LapTimerScreen::drawRacingStatic() {
  TFT_eSPI *tft = _ui->getTft();

  // Gambar Label Statis
  tft->setTextColor(COLOR_TEXT, COLOR_BG);
  
  // 1. Label Kecepatan (Kiri Atas)
  tft->setTextDatum(TL_DATUM); // Kiri Atas
  tft->setTextFont(2);
  tft->setTextSize(1);
  tft->drawString("km/h", 10, 50); // Label di 50

  // 2. Label Lap (Kanan Atas)
  tft->setTextDatum(TR_DATUM); // Kanan Atas
  tft->setTextFont(2);
  tft->drawString("LAP", SCREEN_WIDTH - 10, 50); // Label di 50

  // 3. Label Lap Terbaik (Tengah Bawah, di atas STOP)
  tft->setTextDatum(TC_DATUM);
  tft->drawString("BEST LAP", SCREEN_WIDTH / 2, 165); // Dipindahkan ke ATAS ke 165

  // Tombol Berhenti (Dipindahkan ke sini, digambar SEKALI)
  tft->fillRoundRect(40, STOP_BTN_Y, SCREEN_WIDTH - 80, STOP_BTN_H, 5, TFT_RED);
  tft->setTextColor(TFT_WHITE, TFT_RED);
  tft->setFreeFont(&Org_01);
  tft->setTextSize(2);
  tft->setTextDatum(MC_DATUM);
  tft->drawString("STOP", SCREEN_WIDTH / 2,
                  STOP_BTN_Y + (STOP_BTN_H / 2) + 2); // Cocokkan offset
}

void LapTimerScreen::drawRacing() {
  TFT_eSPI *tft = _ui->getTft();
  tft->setTextColor(COLOR_TEXT, COLOR_BG);

  // 1. Kecepatan (Kiri Atas)
  tft->setTextDatum(TL_DATUM);
  tft->setTextFont(4); // Font Sedang
  tft->setTextSize(1);
  tft->setTextPadding(80); 
  tft->drawFloat(gpsManager.getSpeedKmph(), 0, 10, 20); // Nilai di 20
  tft->setTextPadding(0);

  // 2. Hitungan Lap (Kanan Atas)
  tft->setTextDatum(TR_DATUM);
  tft->setTextFont(4); 
  tft->setTextSize(1);
  tft->setTextPadding(50);
  tft->drawNumber(_lapCount, SCREEN_WIDTH - 10, 20); // Nilai di 20
  tft->setTextPadding(0);

  // 3. Waktu Lap Saat Ini (TENGAH - Fokus Utama)
  unsigned long currentLap = 0;
  if (_isRecording)
    currentLap = millis() - _currentLapStart;
  int ms = currentLap % 1000;
  int s = (currentLap / 1000) % 60;
  int m = (currentLap / 60000);
  char buf[32];
  sprintf(buf, "%02d:%02d.%01d", m, s, ms / 100); // Format MM:SS.d

  tft->setTextDatum(MC_DATUM); // Layar Tengah
  tft->setTextFont(6); // Font Besar
  tft->setTextSize(1);
  tft->setTextPadding(SCREEN_WIDTH); // Hapus lebar penuh
  tft->drawString(buf, SCREEN_WIDTH / 2, 115); // Tengah di 115
  tft->setTextPadding(0);

  // 4. Waktu Lap Terbaik (Tengah Bawah - Kecil)
  if (_bestLapTime > 0) {
      int bms = _bestLapTime % 1000;
      int bs = (_bestLapTime / 1000) % 60;
      int bm = (_bestLapTime / 60000);
      sprintf(buf, "%02d:%02d.%02d", bm, bs, bms / 10);
  } else {
      sprintf(buf, "--:--.--");
  }
  
  tft->setTextFont(2);
  tft->setTextDatum(TC_DATUM);
  tft->setTextPadding(100);
  tft->drawString(buf, SCREEN_WIDTH / 2, 185); // Nilai di 185 (Hapus celah ke 200)
  tft->setTextPadding(0);
}

void LapTimerScreen::checkFinishLine() {
  double dist = gpsManager.distanceBetween(gpsManager.getLatitude(),
                                           gpsManager.getLongitude(),
                                           _finishLat, _finishLon);

  // Logika Deteksi Mulai/Lap
  static bool inside = false;
  static unsigned long lastCross = 0;

  if (dist < 20) {                                   // Radius 20m
    if (!inside && (millis() - lastCross > 10000)) { // Debounce 10s
      // Lap Baru / Mulai
      if (!_isRecording) {
        _isRecording = true;
        sessionManager.startSession();
        _lapCount = 1;
      } else {
        unsigned long lapTime = millis() - _currentLapStart;
        _lastLapTime = lapTime;
        _lapTimes.push_back(lapTime); // Tambahkan ke riwayat
        if (_bestLapTime == 0 || lapTime < _bestLapTime)
          _bestLapTime = lapTime;
        _lapCount++;
      }
      _currentLapStart = millis();
      lastCross = millis();
      inside = true;
    }
  } else {
    if (dist > 25)
      inside = false;
  }
}
