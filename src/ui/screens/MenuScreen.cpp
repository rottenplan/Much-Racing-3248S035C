#include "MenuScreen.h"
#include "../../core/GPSManager.h"

// extern GPSManager gpsManager; // If needed

#define MENU_ITEMS 4
const char *menuLabels[MENU_ITEMS] = {"LAP TIMER", "DRAG METER", "HISTORY",
                                      "SETTINGS"};

void MenuScreen::onShow() {
  _selectedIndex = -1;
  _lastTouchTime = 0; // Reset debounce
  drawMenu();
}

void MenuScreen::update() {
  bool enter = false;

  // --- PENANGANAN SENTUH ---
  UIManager::TouchPoint p = _ui->getTouchPoint();
  if (p.x != -1) { // Disentuh

    // Debug Touch
    // Serial.printf("Touch: x=%d, y=%d\n", p.x, p.y);

    // Deteksi Hit Sederhana untuk Item Menu
    // Kita tahu item mulai di Y=80 (sebelumnya 65)
    // Assuming MENU_ITEMS = 4
    int startY = 80;
    int gap = 38;
    int itemHeight = 30;

    for (int i = 0; i < MENU_ITEMS; i++) {
        int yTop = startY + (i * gap) - 5;
        // ...
      int yBot =
          yTop + 30; // Cocokkan tinggi visual dengan tepat (30px)
                     // sebelumnya menggunakan 'gap' (38px) yang menyebabkan tumpang tindih/tidak ada celah.
                     // Cocokan ketat untuk menyorot "acuan".

      // Periksa apakah sentuhan berada dalam kotak sorotan visual
      if (p.x >= 5 && p.x <= (SCREEN_WIDTH - 5) && p.y >= yTop && p.y <= yBot) {
        
        // Cek Debounce
        if (millis() - _lastTouchTime < 200) {
            return; // Abaikan sentuhan cepat
        }
        _lastTouchTime = millis();
        
        // Ketuk Valid pada Item i
        if (_selectedIndex != i) {
          // Ketukan pertama: Hanya Sorot
          _selectedIndex = i;
          drawMenu(); // Perbarui visual
          enter = false;
        } else {
          // Ketukan kedua (pada yang dipilih): Masuk
          enter = true;
        }
        // delay(200); // Menghapus penundaan pemblokiran
        break;
      }
    }
  }
  // ----------------------

  if (enter) {
    switch (_selectedIndex) {
    case 0:
      _ui->switchScreen(SCREEN_LAP_TIMER);
      break;
    case 1:
      _ui->switchScreen(SCREEN_DRAG_METER);
      break;
    case 2:
      _ui->switchScreen(SCREEN_HISTORY);
      break;
    case 3:
      _ui->switchScreen(SCREEN_SETTINGS);
      break;
    }
  }

  // Perbarui Bilah Status secara berkala
  static unsigned long lastStatusUpdate = 0;
  if (millis() - lastStatusUpdate > 1000) {
    _ui->drawStatusBar();
    lastStatusUpdate = millis();
  }
}

#include "../fonts/Org_01.h"

// ... existing code ...

void MenuScreen::drawMenu() {
  TFT_eSPI *tft = _ui->getTft();

  // Hapus area DI BAWAH bilah status (Mulai dari 21)
  tft->fillRect(0, 21, SCREEN_WIDTH, SCREEN_HEIGHT - 21, COLOR_BG);
  
  // Gambar ulang garis pemisah untuk memastikan batas yang bersih
  tft->drawFastHLine(0, 20, SCREEN_WIDTH, COLOR_SECONDARY);

  tft->setTextDatum(TC_DATUM);
  tft->setFreeFont(&Org_01);
  tft->setTextSize(FONT_SIZE_MENU_TITLE); // Org_01 adalah 6px, jadi 3x adalah 18px (Judul)
  tft->setTextColor(COLOR_ACCENT);
  tft->drawString("MAIN MENU", SCREEN_WIDTH / 2, 45); // Dipindahkan ke atas ke 45

  tft->setTextSize(FONT_SIZE_MENU_ITEM); // Item pada 2x (12px)
  int startY = 80;     // Dimulai lebih awal (sebelumnya 65)
  int gap = 38;        // Celah berkurang (sebelumnya 40)

  int rectWidth = SCREEN_WIDTH - 10; // Panjang "Penuh" (margin 5px)
  int rectX = 5;

  for (int i = 0; i < MENU_ITEMS; i++) {
    if (i == _selectedIndex) {
      tft->setTextColor(COLOR_BG, COLOR_HIGHLIGHT);
      tft->fillRoundRect(rectX, startY + (i * gap) - 5, rectWidth, 30, 5,
                         COLOR_HIGHLIGHT);
    } else {
      tft->setTextColor(COLOR_TEXT, COLOR_BG);
    }
    // Atur Datum Teks ke Tengah Tengah untuk integrasi vertikal yang lebih baik
    tft->setTextDatum(MC_DATUM);
    tft->setFreeFont(&Org_01);
    tft->setTextSize(FONT_SIZE_MENU_ITEM); // Manual pengguna diatur ke 3
    tft->drawString(menuLabels[i], SCREEN_WIDTH / 2,
                    startY + (i * gap) +
                        10); // +10 adalah pusat vertikal (Kotak adalah -5 hingga +25)
  }
}
