#include "UIManager.h"
#include "../../config.h"
#include "fonts/Org_01.h"

// Sertakan layar (dibuat di langkah berikutnya)
#include "screens/DragMeterScreen.h"
#include "screens/HistoryScreen.h"
#include "screens/LapTimerScreen.h"
#include "screens/MenuScreen.h"
#include "screens/SettingsScreen.h"
#include "screens/SplashScreen.h"

UIManager::UIManager(TFT_eSPI *tft) : _tft(tft), _touch(nullptr) {
  _currentScreen = nullptr;
  
  // Initialize State Trackers
  _lastTimeStr = "";
  _lastHdop = -1.0;
  _lastFix = false;
  _lastSignalStrength = -1;
  _lastBat = -1;
  _lastLogging = false;
}

void UIManager::begin() {
  // Instansiasi Layar
  _splashScreen = new SplashScreen();
  _menuScreen = new MenuScreen();
  _lapTimerScreen = new LapTimerScreen();
  _dragMeterScreen = new DragMeterScreen();
  _historyScreen = new HistoryScreen();
  _settingsScreen = new SettingsScreen();

  // Mulai Layar
  _splashScreen->begin(this);
  _menuScreen->begin(this);
  _lapTimerScreen->begin(this);
  _dragMeterScreen->begin(this);
  _historyScreen->begin(this);
  _settingsScreen->begin(this);

  // Mulai dengan Splash
  switchScreen(SCREEN_SPLASH);
}

void UIManager::update() {
  // Perbarui logika layar saat ini
  if (_currentScreen) {
    _currentScreen->update();
  }

  // --- PENANGANAN SENTUH ---
  // (Opsional) Kita dapat menangani sentuhan global tertentu di sini, tetapi untuk saat ini
  // membiarkan layar menanganinya memberikan kontrol konteks yang lebih banyak.
}

UIManager::TouchPoint UIManager::getTouchPoint() {
  TouchPoint p = {-1, -1};
  if (!_touch)
    return p;

  _touch->read();
  if (_touch->isTouched) {
    int rawX = _touch->points[0].x;
    int rawY = _touch->points[0].y;

    // Kalibrasi untuk Layar 320x240
    // Y Mentah biasanya masuk 0-320 tetapi layar adalah 0-240.
    // X Mentah biasanya 0-320 sesuai lebar layar.
    // Pemetaan Standar 1:1 (GT911 biasanya skala otomatis)
    // Jika tidak akurat, periksa Monitor Serial untuk output "Touch: Raw..."
    // Kalibrasi Manual / Pemetaan dari config.h
    int pX = rawX;
    int pY = rawY;

    // 1. Tukar XY
    if (TOUCH_SWAP_XY) {
      int temp = pX;
      pX = pY; // pX sekarang menampung RawY (0-320 kira-kira)
      pY = temp; // pY sekarang menampung RawX (0-240 kira-kira)
    }

    // 2. Balik X (Koordinat Layar)
    if (TOUCH_INVERT_X) {
      pX = SCREEN_WIDTH - 1 - pX;
    }

    // 3. Balik Y (Koordinat Layar)
    if (TOUCH_INVERT_Y) {
      pY = SCREEN_HEIGHT - 1 - pY;
    }

    p.x = pX;
    p.y = pY;

    // Batasi
    if (p.x < 0)
      p.x = 0;
    if (p.x > 320)
      p.x = 320;
    if (p.y < 0)
      p.y = 0;
    if (p.y > 240)
      p.y = 240;

    // Debug: Lacak koordinat sentuh
    Serial.printf("Touch: Raw[%d,%d] -> Screen[%d,%d]\n", rawX, rawY, p.x, p.y);
  }
  return p;
}

#include "../core/GPSManager.h"
#include "../core/SessionManager.h"

extern GPSManager gpsManager;
extern SessionManager sessionManager;

void UIManager::switchScreen(ScreenType type) {
  // Tambahkan penundaan 1 detik untuk transisi yang mulus (kecuali dari Splash)
  // Tidak ada penundaan untuk peralihan instan

  _currentType = type;

  _tft->fillScreen(COLOR_BG);

  switch (type) {
  case SCREEN_SPLASH:
    _currentScreen = _splashScreen;
    _screenTitle = "";
    break;
  case SCREEN_MENU:
    _currentScreen = _menuScreen;
    _screenTitle = ""; // Kosong = Tampilkan Waktu
    break;
  case SCREEN_LAP_TIMER:
    _currentScreen = _lapTimerScreen;
    _screenTitle = "LAP TIMER";
    break;
  case SCREEN_DRAG_METER:
    _currentScreen = _dragMeterScreen;
    _screenTitle = "DRAG METER";
    break;
  case SCREEN_HISTORY:
    _currentScreen = _historyScreen;
    _screenTitle = "HISTORY";
    break;
  case SCREEN_SETTINGS:
    _currentScreen = _settingsScreen;
    _screenTitle = "SETTINGS";
    break;
  }

  // Gambar Bilah Status segera saat beralih (latar belakang sudah dihapus)
  if (_currentType != SCREEN_SPLASH) {
    drawStatusBar(true); // Force redraw on switch
  }

  if (_currentScreen) {
    _currentScreen->onShow();
  }
}

void UIManager::drawStatusBar(bool force) {
  // 1. Elemen Statis (Hanya gambar sekali atau jika dipaksa? Untuk saat ini, kita asumsikan latar belakang dihapus hanya pada peralihan layar)
  // Jika kita ingin menghindari kedipan, kita TIDAK boleh menghapus seluruh bilah setiap bingkai.
  // Kita mengandalkan warna latar belakang teks untuk menimpa teks lama.
  
  // Gambar garis pemisah (aman untuk digambar ulang, cepat)
  if (force) {
      _tft->drawFastHLine(0, 20, SCREEN_WIDTH, COLOR_SECONDARY);
  }

  _tft->setFreeFont(&Org_01);
  _tft->setTextSize(FONT_SIZE_STATUS_BAR);
  _tft->setTextColor(COLOR_TEXT, COLOR_BG);

  // --- Bagian GPS ---
  double hdop = gpsManager.getHDOP();
  bool fix = gpsManager.isFixed();
  
  // Logika untuk menghitung kekuatan
  int signalStrength = 0;
  if (fix) {
    if (hdop <= 0.8) signalStrength = 4;
    else if (hdop <= 1.0) signalStrength = 3;
    else if (hdop <= 1.5) signalStrength = 2;
    else signalStrength = 1;
  }

  // Gambar ulang Ikon GPS hanya jika status berubah (Status perbaikan atau Kekuatan)
  if (force || fix != _lastFix || signalStrength != _lastSignalStrength) {
      // Hapus Area GPS
      _tft->fillRect(0, 0, 80, 20, COLOR_BG);
      
      // Gambar Label "GPS"
      _tft->setTextDatum(ML_DATUM);
      _tft->setTextColor(COLOR_TEXT, COLOR_BG);
      _tft->setTextSize(1);
      _tft->drawString("GPS", 5, 11);

      int barX = 30;
      int barY = 16;
      int barW = 3;
      int barGap = 2;

      for (int i = 0; i < 4; i++) {
        int h = (i + 1) * 3;
        int x = barX + (i * (barW + barGap));
        int y = barY - h;

        if (i < signalStrength) {
          uint16_t color = fix ? TFT_GREEN : TFT_RED;
          _tft->fillRect(x, y, barW, h, color);
        } else {
          _tft->drawRect(x, y, barW, h, COLOR_TEXT);
        }
      }
      _lastFix = fix;
      _lastSignalStrength = signalStrength;
  }

  // --- Bagian Waktu / Judul ---
  // Jika _screenTitle diatur, tampilkan. Jika tidak tampilkan Waktu.
  String centerText;
  if (_screenTitle.length() > 0) {
      centerText = _screenTitle;
  } else {
      centerText = ""; // Jangan tampilkan waktu
  }

  if (force || centerText != _lastTimeStr) { // Menggunakan kembali _lastTimeStr untuk cache teks tengah
      // Hapus Area Waktu / Judul (Tengah)
      // Diasumsikan lebar maksimal 160px (menyisakan 80px di setiap sisi)
      int areaW = 160;
      
      _tft->setTextPadding(areaW);
      _tft->setTextDatum(TC_DATUM);
      _tft->setTextColor(COLOR_TEXT, COLOR_BG);
      _tft->drawString(centerText, SCREEN_WIDTH / 2, 5); // Y disesuaikan ke 5
      _tft->setTextPadding(0);
      
      _lastTimeStr = centerText;
  }

  // --- Bagian Baterai (Tiruan) ---
  // Cukup gambar ulang baterai sederhana setiap saat? Atau periksa perubahan?
  // Memeriksa mock perubahan
  int rawBat = 4095; // Mock
  if (force || rawBat != _lastBat) {
      // Hapus Area Bar
      _tft->fillRect(SCREEN_WIDTH - 30, 0, 30, 20, COLOR_BG);
      
      int batX = SCREEN_WIDTH - 25;
      int batY = 5;
      int batW = 20;
      int batH = 10;
      
      _tft->drawRect(batX, batY, batW, batH, COLOR_TEXT);
      _tft->fillRect(batX + batW, batY + 2, 2, 6, COLOR_TEXT);
      
      // Level Mock
      _tft->fillRect(batX + 2, batY + 2, batW - 4, batH - 4, TFT_GREEN);
      
      _lastBat = rawBat;
  }

  // --- Indikator Perekaman ---
  bool isLogging = sessionManager.isLogging();
  if (force || isLogging != _lastLogging) {
       // Hapus Area Titik
       int dotX = (SCREEN_WIDTH / 2) + 40;
       _tft->fillRect(dotX - 5, 0, 10, 20, COLOR_BG);
       
       if (isLogging) {
         _tft->fillCircle(dotX, 10, 3, TFT_RED);
       }
       _lastLogging = isLogging;
  }
}
