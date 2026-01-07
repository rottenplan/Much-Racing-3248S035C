#include "UIManager.h"
#include "../../config.h"
#include <Preferences.h>
#include <WiFi.h>
#include "../core/WiFiManager.h"
#include "fonts/Org_01.h"

extern WiFiManager wifiManager;

// Sertakan layar (dibuat di langkah berikutnya)
#include "screens/DragMeterScreen.h"
#include "screens/HistoryScreen.h"
#include "screens/LapTimerScreen.h"
#include "screens/MenuScreen.h"
#include "screens/SettingsScreen.h"
#include "screens/SplashScreen.h"
#include "screens/TimeSettingScreen.h"
#include "screens/TimeSettingScreen.h"
// #include "screens/AutoOffScreen.h"
#include "screens/RpmSensorScreen.h"
#include "screens/RpmSensorScreen.h"

UIManager::UIManager(TFT_eSPI *tft) : _tft(tft), _touch(nullptr) {
  _currentScreen = nullptr;
  
  // Initialize State Trackers
  _lastTimeStr = "";
  _lastHdop = -1.0;
  _lastFix = false;
  _lastSignalStrength = -1;
  _lastBat = -1;
  _lastLogging = false;
  _lastWifiStatus = -1;
}

void UIManager::begin() {
  // Instansiasi Layar
  _splashScreen = new SplashScreen();
  _menuScreen = new MenuScreen();
  _lapTimerScreen = new LapTimerScreen();
  _dragMeterScreen = new DragMeterScreen();
  _historyScreen = new HistoryScreen();
  _settingsScreen = new SettingsScreen();
  _timeSettingScreen = new TimeSettingScreen();
  _timeSettingScreen = new TimeSettingScreen();
  // _autoOffScreen = new AutoOffScreen();
  _rpmSensorScreen = new RpmSensorScreen();
  _rpmSensorScreen = new RpmSensorScreen();

  // Mulai Layar
  _splashScreen->begin(this);
  _menuScreen->begin(this);
  _lapTimerScreen->begin(this);
  _dragMeterScreen->begin(this);
  _historyScreen->begin(this);
  _settingsScreen->begin(this);
  _timeSettingScreen->begin(this);
  _timeSettingScreen->begin(this);
  // _autoOffScreen->begin(this);
  _rpmSensorScreen->begin(this);
  _rpmSensorScreen->begin(this);

  // Initialize Sleep Logic (New System)
  Preferences prefs;
  prefs.begin("laptimer", true);
  bool autoOffEn = prefs.getBool("auto_off_en", false); // Default Off
  int autoOffSec = prefs.getInt("auto_off_val", 30);    // Default 30s
  prefs.end();
  
  // Disable Auto Off Logic
  /*
  if (autoOffEn) {
      setAutoOff(autoOffSec * 1000UL); 
  } else {
      setAutoOff(0);
  }
  */
  setAutoOff(0); // Force Disable

  _lastInteractionTime = millis();
  _isScreenOff = false;
  _currentBrightness = 255; // Default max, should load from prefs if we had a stored brightness variable
  
  _lastTapTime = 0;
  _wasTouched = false;

  // Mulai dengan Splash
  switchScreen(SCREEN_SPLASH);
}

void UIManager::update() {
  // Perbarui logika layar saat ini
  if (_currentScreen) {
    if (!_isScreenOff) {
        _currentScreen->update();
    } else {
        // Handle Wakeup from Off State (Double Tap)
        if (_touch) {
            _touch->read();
            bool touched = _touch->isTouched;
            
            // Detect Rising Edge (Touch Start)
            if (touched && !_wasTouched) {
                unsigned long now = millis();
                if (now - _lastTapTime < 500) {
                    // Double Tap Detected!
                    wakeUp();
                    _lastTapTime = 0; // Reset
                } else {
                    _lastTapTime = now;
                }
            }
            _wasTouched = touched;
        }
    }
  }

  checkSleep();

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
    updateInteraction(); // Reset timer on touch
    
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
  case SCREEN_TIME_SETTINGS:
    _currentScreen = _timeSettingScreen;
    _screenTitle = "CLOCK";
    break;
  // case SCREEN_AUTO_OFF:
  //   _currentScreen = _autoOffScreen;
  //   _screenTitle = "AUTO OFF";
  //   break;
  case SCREEN_RPM_SENSOR:
    _currentScreen = _rpmSensorScreen;
    _screenTitle = "RPM SENSOR";
    break;
  }

  if (_currentScreen) {
    _currentScreen->onShow();
  }
  
  // Gambar Bilah Status setelah onShow agar tidak tertimpa oleh fillScreen di layar
  if (_currentType != SCREEN_SPLASH) {
    drawStatusBar(true); // Force redraw on switch
  }
}

void UIManager::drawStatusBar(bool force) {
  // 1. Time Update Logic (Manual Clock)
  if (g_lastTimeUpdate == 0) g_lastTimeUpdate = millis();
  
  if (millis() - g_lastTimeUpdate >= 60000) {
      g_manualMinute++;
      if (g_manualMinute > 59) {
          g_manualMinute = 0;
          g_manualHour++;
          if (g_manualHour > 23) g_manualHour = 0;
      }
      g_lastTimeUpdate = millis();
      force = true; // Redraw to update time
  }

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
      _tft->drawString("GPS", 5, 10); // Centered at 10

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

  // --- WiFi Section ---
  int wifiStatus = wifiManager.isConnected() ? 1 : 0;
  if (force || wifiStatus != _lastWifiStatus) {
      // Clear WiFi Area (60 to 80)
      _tft->fillRect(60, 0, 20, 20, COLOR_BG);
      
      uint16_t color = (wifiStatus == 1) ? TFT_GREEN : TFT_RED;
      int wx = 70;
      int wy = 17;
      
      // WiFi Icon (Refined Pixel Art to match user image)
      // Bottom Dot
      _tft->fillCircle(wx, wy - 1, 1, color);
      
      // Arc 1 (Small)
      _tft->drawFastHLine(wx - 1, wy - 4, 3, color);
      _tft->drawPixel(wx - 2, wy - 3, color);
      _tft->drawPixel(wx + 2, wy - 3, color);
      
      // Arc 2 (Medium)
      _tft->drawFastHLine(wx - 3, wy - 7, 7, color);
      _tft->drawPixel(wx - 4, wy - 6, color);
      _tft->drawPixel(wx + 4, wy - 6, color);
      _tft->drawPixel(wx - 5, wy - 5, color);
      _tft->drawPixel(wx + 5, wy - 5, color);

      // Arc 3 (Large/Top)
      _tft->drawFastHLine(wx - 5, wy - 10, 11, color);
      _tft->drawPixel(wx - 6, wy - 9, color);
      _tft->drawPixel(wx + 6, wy - 9, color);
      _tft->drawPixel(wx - 7, wy - 8, color);
      _tft->drawPixel(wx + 7, wy - 8, color);
      
      _lastWifiStatus = wifiStatus;
  }

  // --- Bagian Waktu / Judul ---
  // Jika _screenTitle diatur, tampilkan. Jika tidak tampilkan Waktu Manual.
  String centerText;
  if (_screenTitle.length() > 0) {
      centerText = _screenTitle;
  } else {
      // Manual Time Format HH:MM
      char buf[16];
      sprintf(buf, "%02d:%02d", g_manualHour, g_manualMinute);
      centerText = String(buf);
  }

  if (force || centerText != _lastTimeStr) { // Menggunakan kembali _lastTimeStr untuk cache teks tengah
      // Hapus Area Waktu / Judul (Tengah)
      // Diasumsikan lebar maksimal 160px (menyisakan 80px di setiap sisi)
      int areaW = 160;
      
      _tft->setTextPadding(areaW);
      _tft->setTextDatum(MC_DATUM); // Middle Center
      _tft->setTextColor(COLOR_TEXT, COLOR_BG);
      _tft->drawString(centerText, SCREEN_WIDTH / 2, 10); // Y=10 Centered
      _tft->setTextPadding(0);
      
      _lastTimeStr = centerText;
  }

  // --- Bagian Baterai ---
  // Mock value (ADC value 0-4095)
  // Misal Voltage divider: 4.2V = 4095 (atau disesuaikan)
  // Sederhana: 0-100% linear dari 3.0V(0%) - 4.2V(100%)?
  // Mock: Selalu 100% untuk sekarang atau simulasi fluktuasi
  int rawBat = 4095; 
  
  if (force || rawBat != _lastBat) {
      // Hitung Persentase
      // Anggap 4095 = 100%
      int pct = (rawBat * 100) / 4096;
      if (pct > 100) pct = 100;
  
      // Hapus Area Bar + Teks
      // Area Kanan: 60px lebar?
      _tft->fillRect(SCREEN_WIDTH - 60, 0, 60, 20, COLOR_BG);
      
      // Gambar Teks Persentase
      _tft->setTextDatum(MR_DATUM); // Rata Kanan Tengah
      _tft->setTextSize(1);
      _tft->setTextColor(COLOR_TEXT, COLOR_BG);
      String pctStr = String(pct) + "%";
      _tft->drawString(pctStr, SCREEN_WIDTH - 32, 10); // Centered at 10

      // Gambar Ikon Baterai
      int batX = SCREEN_WIDTH - 28; // Geser sedikit ke kiri
      int batY = 5; // Centered (5 to 15)
      int batW = 20;
      int batH = 10;
      
      _tft->drawRect(batX, batY, batW, batH, COLOR_TEXT);
      _tft->fillRect(batX + batW, batY + 2, 2, 6, COLOR_TEXT);
      
      // Isi Level
      int innerW = batW - 4;
      int fillW = (innerW * pct) / 100;
      
      if (pct > 20) _tft->fillRect(batX + 2, batY + 2, fillW, batH - 4, TFT_GREEN);
      else _tft->fillRect(batX + 2, batY + 2, fillW, batH - 4, TFT_RED);
      
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

// --- Auto Off Logic ---

void UIManager::setAutoOff(unsigned long ms) {
    _autoOffMs = ms;
    updateInteraction();
}

void UIManager::updateInteraction() {
    _lastInteractionTime = millis();
    // if (_isScreenOff) {
    //     wakeUp();
    // }
}

void UIManager::checkSleep() {
    if (_autoOffMs > 0 && !_isScreenOff) {
        if (millis() - _lastInteractionTime > _autoOffMs) {
            _isScreenOff = true;
            ledcWrite(0, 0);
        }
    }
}

void UIManager::wakeUp() {
    _isScreenOff = false;
    _lastInteractionTime = millis();
    ledcWrite(0, _currentBrightness);
}
