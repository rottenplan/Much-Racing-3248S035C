#include "DragMeterScreen.h"
#include "../../core/GPSManager.h"
#include "../../core/SessionManager.h"

extern GPSManager gpsManager;

// Tentukan subset disiplin untuk dilacak
// 0-60 km/h
// 0-100 km/h
// 100-200 km/h
// 402m (1/4 Mile)

void DragMeterScreen::onShow() {
  _state = STATE_READY;
  _startTime = 0;
  _lastUpdate = 0;

  // Inisialisasi Disiplin
  _disciplines.clear();
  _disciplines.push_back({"0-60", false, 60.0, 0, false, 0});
  _disciplines.push_back({"0-100", false, 100.0, 0, false, 0});
  // _disciplines.push_back({"100-200", false, 200.0, 0, false, 0}); // Kompleks
  // logika mulai diperlukan? Untuk saat ini hanya 0-X
  _disciplines.push_back({"1/4 Mi", true, 402.34, 0, false, 0});

  // _ui->getTft()->fillScreen(COLOR_BG); // Sudah dibersihkan oleh UIManager
  drawDashboard();
}

void DragMeterScreen::update() {
  // Tangani Sentuh (Tombol Kembali)
  UIManager::TouchPoint p = _ui->getTouchPoint();
  if (p.x != -1) {
    if (p.x < 60 && p.y < 40) {
      _ui->switchScreen(SCREEN_MENU);
      return;
    }

    // Reset saat sentuh di Ringkasan
    if (_state == STATE_SUMMARY && p.y > 200) {
      onShow(); // Reset
      return;
    }
  }

  // Logika Status
  switch (_state) {
  case STATE_READY:
    checkStartCondition();
    break;
  case STATE_RUNNING:
    updateDisciplines();
    checkStopCondition();
    break;
  case STATE_SUMMARY:
    // Hanya menunggu reset
    break;
  }

  // Segarkan UI (10Hz)
  if (millis() - _lastUpdate > 100) {
    if (_state != STATE_SUMMARY) {
      drawDashboard();
    }
    _ui->drawStatusBar();
    _lastUpdate = millis();
  }
}

void DragMeterScreen::checkStartCondition() {
  // Mulai jika kecepatan > 2 km/jam (Deteksi gerakan dasar)
  if (gpsManager.getSpeedKmph() > 2.0 && gpsManager.isFixed()) {
    _state = STATE_RUNNING;
    _startTime = millis();
    // Reset waktu mulai disiplin jika diperlukan?
    // Untuk 0-X, mulai sekarang. Untuk 100-200, mulai saat 100 tercapai.
    // Disederhanakan: Asumsikan mulai berdiri untuk semua saat ini.
  }
}

void DragMeterScreen::checkStopCondition() {
  // Berhenti otomatis jika kecepatan < 1 km/jam selama > 2 detik?
  // Atau jika semua disiplin selesai?
  // Mari berhenti jika kecepatan turun mendekati nol setelah tinggi.

  // Sederhana: Jika kecepatan < 1.0, beralih ke Ringkasan
  if (gpsManager.getSpeedKmph() < 1.0 && (millis() - _startTime > 2000)) {
    _state = STATE_SUMMARY;
    _ui->getTft()->fillScreen(COLOR_BG);
    drawResults();
  }
}

void DragMeterScreen::updateDisciplines() {
  unsigned long currentRunTime = millis() - _startTime;
  float currentSpeed = gpsManager.getSpeedKmph();
  // Jarak? GPSManager perlu melacak jarak dari titik awal.
  // Untuk saat ini memperkirakan jarak: dist += speed * time_delta
  // Lebih baik: gunakan lat/lon awal
  // TODO: Implementasikan pelacakan jarak di GPSManager atau secara lokal.

  // Mocking logika jarak atau hanya menggunakan integrasi sederhana
  static unsigned long lastCalc = 0;
  static float distMeters = 0;
  if (currentRunTime == 0) {
    distMeters = 0;
    lastCalc = millis();
  }

  unsigned long delta = millis() - lastCalc;
  if (delta > 0) {
    distMeters += (currentSpeed / 3.6) * (delta / 1000.0);
    lastCalc = millis();
  }

  for (auto &d : _disciplines) {
    if (!d.completed) {
      if (d.isDistance) {
        if (distMeters >= d.target) {
          d.completed = true;
          d.resultTime = currentRunTime;
          d.endSpeed = currentSpeed;
        }
      } else {
        // Mode Kecepatan (0-X)
        if (currentSpeed >= d.target) {
          d.completed = true;
          d.resultTime = currentRunTime;
          d.endSpeed = currentSpeed;
        }
      }
    }
  }
}

void DragMeterScreen::drawDashboard() {
  TFT_eSPI *tft = _ui->getTft();

  // Panah Kembali Header
  tft->setTextColor(COLOR_TEXT, COLOR_BG);
  tft->setTextDatum(TL_DATUM);
  tft->setTextSize(2);
  tft->drawString("<", 10, 30); // Di bawah bilah status

  // 1. Kecepatan Besar (Tengah Atas)
  tft->setTextDatum(TC_DATUM);
  tft->setTextFont(7); // Numerik seperti 7-segmen
  tft->setTextSize(1);
  tft->drawFloat(gpsManager.getSpeedKmph(), 0, SCREEN_WIDTH / 2, 40);
  tft->setTextFont(2);
  tft->setTextSize(1);
  tft->drawString("km/h", SCREEN_WIDTH / 2, 90);

  // 2. Daftar Disiplin (Kiri)
  int listY = 120;
  tft->setTextDatum(TL_DATUM);
  tft->setTextFont(2);
  tft->setTextSize(1);

  for (int i = 0; i < _disciplines.size(); i++) {
    Discipline &d = _disciplines[i];
    int y = listY + (i * 30);

    tft->setTextColor(COLOR_SECONDARY, COLOR_BG);
    tft->drawString(d.name, 20, y);

    // 3. Status/Waktu (Kanan)
    tft->setTextDatum(TR_DATUM);
    if (d.completed) {
      tft->setTextColor(TFT_GREEN, COLOR_BG);
      char buf[16];
      sprintf(buf, "%.2fs", d.resultTime / 1000.0);
      tft->drawString(buf, SCREEN_WIDTH - 20, y);
    } else {
      tft->setTextColor(COLOR_SECONDARY, COLOR_BG);
      tft->drawString("--.--", SCREEN_WIDTH - 20, y);
    }

    tft->setTextDatum(TL_DATUM); // Reset
  }

  // Teks Status
  tft->setTextDatum(BC_DATUM);
  tft->setTextSize(1);
  if (_state == STATE_READY) {
    tft->setTextColor(TFT_ORANGE, COLOR_BG);
    tft->drawString(gpsManager.isFixed() ? "READY" : "WAIT GPS",
                    SCREEN_WIDTH / 2, 230);
  } else {
    tft->setTextColor(TFT_GREEN, COLOR_BG);
    tft->drawString("RUNNING", SCREEN_WIDTH / 2, 230);
  }
}

void DragMeterScreen::drawResults() {
  TFT_eSPI *tft = _ui->getTft();

  // Header
  tft->fillRect(0, 20, SCREEN_WIDTH, 40, COLOR_SECONDARY);
  tft->setTextColor(COLOR_TEXT, COLOR_SECONDARY);
  tft->setTextDatum(MC_DATUM);
  tft->setTextFont(2);
  tft->setTextSize(1);
  tft->drawString("RUN SUMMARY", SCREEN_WIDTH / 2, 40);

  tft->drawString("<", 15, 40); // Back

  // Daftar Hasil
  int listY = 80;
  tft->setTextSize(2);

  for (int i = 0; i < _disciplines.size(); i++) {
    Discipline &d = _disciplines[i];
    int y = listY + (i * 40);

    // Nama
    tft->setTextDatum(TL_DATUM);
    tft->setTextColor(COLOR_TEXT, COLOR_BG);
    tft->drawString(d.name, 20, y);

    // Waktu
    tft->setTextDatum(TR_DATUM);
    if (d.completed) {
      tft->setTextColor(TFT_GREEN, COLOR_BG);
      char buf[16];
      sprintf(buf, "%.2fs", d.resultTime / 1000.0);
      tft->drawString(buf, SCREEN_WIDTH - 20, y);

      // Kecepatan Akhir (Kecil di bawah)
      if (d.isDistance) {
        tft->setTextSize(1);
        tft->setTextColor(COLOR_SECONDARY, COLOR_BG);
        sprintf(buf, "@ %.0f km/h", d.endSpeed);
        tft->drawString(buf, SCREEN_WIDTH - 20, y + 20);
        tft->setTextSize(2);
      }
    } else {
      tft->setTextColor(TFT_RED, COLOR_BG);
      tft->drawString("---", SCREEN_WIDTH - 20, y);
    }
  }

  // Tombol Reset
  tft->fillRoundRect(80, 200, 160, 30, 5, COLOR_SECONDARY);
  tft->setTextColor(COLOR_TEXT, COLOR_SECONDARY);
  tft->setTextDatum(MC_DATUM);
  tft->drawString("RESET", SCREEN_WIDTH / 2, 215);
}
