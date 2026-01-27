#include "SpeedometerScreen.h"
#include "../../config.h"
#include "../../core/GPSManager.h"
#include "../fonts/Org_01.h"
#include <Preferences.h>

extern GPSManager gpsManager;

void SpeedometerScreen::onShow() {
  TFT_eSPI *tft = _ui->getTft();
  tft->fillScreen(_ui->getBackgroundColor()); // Latar belakang gelap

  _lastSpeed = -1;
  _lastRPM = -1;
  _lastTrip = -1;
  _lastTime = "";
  _lastGear = -1;
  _lastBat = -1;
  _maxSpeed = 0;
  _maxRPM = 0;
  _lastSats = -1;

  // SETUP RPM SENSOR
  // MOVED TO GPSManager for global access

  drawDashboard(true);
}

void SpeedometerScreen::update() {
  // 1. Tombol Kembali
  UIManager::TouchPoint p = _ui->getTouchPoint();
  if (p.x != -1 && p.x < 60 && p.y < 60) {
    static unsigned long lastBackTap = 0;
    if (millis() - lastBackTap < 500) {
      if (PIN_RPM_INPUT >= 0) {
        // detachInterrupt(digitalPinToInterrupt(PIN_RPM_INPUT)); // STOP SENSOR
        // - NO, IT'S GLOBAL NOW
      }
      _ui->switchScreen(SCREEN_MENU);
      lastBackTap = 0;
    } else {
      lastBackTap = millis();
    }
    return;
  }

  // 2. Pembaruan Data GPS
  float speed = gpsManager.getSpeedKmph();
  float trip = gpsManager.getTotalTrip();

  // Waktu Nyata dari GPS
  int h, m, s, d, mo, y;
  gpsManager.getLocalTime(h, m, s, d, mo, y);
  char timeBuf[6];
  sprintf(timeBuf, "%02d:%02d", h, m);
  String timeStr = String(timeBuf);

  // 3. HITUNG RPM (Real Time)
  // NOW HANDLED BY GPS MANAGER GLOBALLY
  int rpm = gpsManager.getRPM();

  // Push to Web API not needed as GPSManager has it already
  // gpsManager.setRPM(rpm);

  // Placeholder lainnya
  int gear = 0;
  int bat = 100;

  // Cek satuan (km/h atau mph)
  Preferences prefs;
  prefs.begin("laptimer", true);
  bool useMph = prefs.getInt("units", 0) == 1; // 0=km/h, 1=mph
  prefs.end();

  if (useMph) {
    speed *= 0.621371;
    trip *= 0.621371;
  }

  // Cek apakah ada perubahan data untuk digambar ulang
  int sats = gpsManager.getSatellites();
  if (rpm > _maxRPM)
    _maxRPM = rpm;
  if (speed > _maxSpeed)
    _maxSpeed = speed;

  if (speed != _lastSpeed || rpm != _lastRPM || useMph != _lastUnits ||
      timeStr != _lastTime || trip != _lastTrip || sats != _lastSats) {
    _lastSpeed = speed;
    _lastRPM = rpm;
    _lastUnits = useMph;
    _lastTime = timeStr;
    _lastTrip = trip;
    _lastSats = sats;
    drawDashboard(false);
  }
}

// Pembantu untuk menggambar segmen jajargenjang
void drawSegment(TFT_eSPI *tft, int x, int y, int w, int h, int angleOffset,
                 uint16_t color) {
  tft->fillTriangle(x, y + h, x + w, y + h, x + angleOffset, y, color);
  tft->fillTriangle(x + w, y + h, x + w + angleOffset, y, x + angleOffset, y,
                    color);
}

void SpeedometerScreen::drawDashboard(bool force) {
  TFT_eSPI *tft = _ui->getTft();

  // --- PENGATURAN WARNA ---
  uint16_t colTheme = TFT_GREEN;                 // Warna utama (Hijau)
  uint16_t colRed = TFT_RED;                     // Warna merah untuk Top Speed
  uint16_t colWhite = _ui->getTextColor();       // Warna putih (Teks Utama)
  uint16_t colBlack = _ui->getBackgroundColor(); // Warna hitam (Latar Belakang)

  // Special case: Inverted boxes (e.g. Center Box was White BG, Black Text)
  // If Dark Mode: BG=Black, colWhite=White. CenterBox=White, Text=Black. OK.
  // If Light Mode: BG=White, colWhite=Black. CenterBox=Black (should be?),
  // Text=White. We need distinct "Contrast Text" and "Contrast BG" or just
  // strictly use logic.

  // Let's redefine for clarity in this specific dashboard which uses "Boxes"
  uint16_t colBoxBg =
      _ui->getTextColor(); // Box Background (Opposite of Main BG)
  uint16_t colBoxText =
      _ui->getBackgroundColor(); // Text inside Box (Same as Main BG)

  // --- PENGATURAN POSISI (OFFSET) ---
  // --- PENGATURAN POSISI (OFFSET) ---
  int offTop = 15 + ((SCREEN_HEIGHT - 240) / 2); // Center vertically
  int offBot = 10; // Geser Bawah (Dikurangi dari 28)

  int cx = SCREEN_WIDTH / 2;
  int boxTotalW = 280;
  int startX = (SCREEN_WIDTH - boxTotalW) / 2;

  // Dynamic X:
  int x1 = startX;
  int x2 = startX + 80 + 10;
  int x3 = startX + 80 + 10 + 100 + 10;

  if (force) {
    // Clear only content area
    _ui->drawCarbonBackground(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                              SCREEN_HEIGHT - STATUS_BAR_HEIGHT);
    _ui->drawStatusBar(true);

    // --- 0. INDIKATOR KECIL DI ATAS (Revised: Bigger & Fitted) ---
    // Status Bar assumed ~20px. Header starts at 39+offTop (54px).
    // Available Y: 22 to 52 (30px Height).
    int boxY = 22;
    int boxH = 30;

    // Layout: Left(20, width 80) - Gap(10) - Center(110, width 100) - Gap(10) -
    // Right(220, width 80)

    tft->fillRect(x1, boxY, 80, boxH, colTheme);  // Hijau
    tft->fillRect(x2, boxY, 100, boxH, colBoxBg); // Putih (Contrast)
    tft->fillRect(x3, boxY, 80, boxH, colRed);    // Merah

    // --- TEXT INDIKATOR ---
    char buf[10];
    tft->setFreeFont(&Org_01);
    tft->setTextSize(2); // Perbesar teks agar sesuai box
    tft->setTextDatum(MC_DATUM);

    int yTextMid = boxY + (boxH / 2) + 2; // +2 adjustment for font baseline

    // Box 1 (Green/Kiri): Max RPM
    tft->setTextColor(colBoxText,
                      colTheme); // Text=BGColor (Black in Dark), BG=Green
    sprintf(buf, "%d", _maxRPM);
    tft->drawString(buf, x1 + 40, yTextMid); // Center of Box 1

    // Box 2 (White/Tengah): Max Speed
    tft->setTextColor(colBoxText, colBoxBg); // Text=BGColor, BG=Contrast
    sprintf(buf, "%.0f", _maxSpeed);
    tft->drawString(buf, x2 + 50, yTextMid); // Center of Box 2

    // Box 3 (Red/Kanan): GPS Signal
    tft->setTextColor(colBoxText, colRed); // Text=BGColor, BG=Red
    sprintf(buf, "%d", _lastSats);
    tft->drawString(buf, x3 + 40, yTextMid); // Center of Box 3

    tft->setTextDatum(TL_DATUM); // Reset

    // --- 1. HEADER BAR (SPEED) ---
    tft->fillRect(2, 39 + offTop, SCREEN_WIDTH - 4, 19, colBoxBg);

    // Tulisan "SPEED" - DI CENTERKAN
    tft->setTextColor(colBoxText, colBoxBg);
    tft->setTextSize(2);
    tft->setTextDatum(MC_DATUM);
    tft->drawString("SPEED", cx, 39 + offTop + 10);

    // Requested Titles "RPM" (Left) and "MAX" (Right)
    // Use x1 and x3 centers
    tft->drawString("RPM", x1 + 40, 39 + offTop + 10);
    tft->drawString("MAX", x3 + 40, 39 + offTop + 10);

    tft->setTextDatum(TL_DATUM);

    // --- 2. ANGKA KECEPATAN BESAR ---
    tft->setTextColor(colTheme, _ui->getBackgroundColor());
    tft->setTextSize(7);
    tft->setTextDatum(MC_DATUM);

    int yCenterSpeed = 86 + offTop;

    char speedBuf[10];
    sprintf(speedBuf, "%.0f", _lastSpeed);
    tft->drawString(speedBuf, cx, yCenterSpeed);
    tft->setTextDatum(TL_DATUM);

    // Satuan "Km/H" (Naik dikit ke 115)
    tft->setTextSize(1);
    tft->drawCentreString("Km/H", cx, 115 + offTop, 1);

    // --- 3. DATA TRIP ---
    // Geser naik lagi ke 122 (User request: "0000 naikan lagi")
    tft->setTextColor(colTheme, _ui->getBackgroundColor());
    tft->setTextSize(1);
    tft->drawCentreString("LONG TRIP", cx, 122 + offTop, 1);

    // Angka Trip (Geser naik ke 142)
    tft->setTextSize(3);
    char tripBuf[10];
    sprintf(tripBuf, "%04.0f", _lastTrip);
    tft->drawCentreString(tripBuf, cx, 142 + offTop, 1);

    // --- 5. SKALA & GARIS ---
    int yScaleVal = 176 + offBot;

    tft->setTextColor(colWhite, _ui->getBackgroundColor());
    tft->setTextSize(1);

    // Angka Skala
    tft->drawString("0", 45, yScaleVal);
    tft->drawString("40", 83, yScaleVal);
    tft->drawString("80", 126, yScaleVal);
    tft->drawString("100", 170, yScaleVal);
    tft->drawString("120", 217, yScaleVal);
    tft->drawString("150", 269, yScaleVal);

    // Garis Horizontal
    uint16_t c = colWhite;
    int yLine = 179 + offBot;
    tft->drawLine(54, yLine, 77, yLine, c);
    tft->drawLine(97, yLine, 120, yLine, c);
    tft->drawLine(141, yLine, 164, yLine, c);
    tft->drawLine(187, yLine, 210, yLine, c);
    tft->drawLine(235, yLine, 258, yLine, c);

    // Garis Vertikal (Ticks)
    tft->drawLine(78, yLine, 78, yLine - 3, c);
    tft->drawLine(121, yLine, 121, yLine - 3, c);
    tft->drawLine(165, yLine, 165, yLine - 3, c);
    tft->drawLine(211, yLine, 211, yLine - 3, c);
    tft->drawLine(259, yLine, 259, yLine - 3, c);

    // --- 6. BAR RPM ---
    int yRPM = 185 + offBot + offTop - 15; // Re-adjust based on offTop shift?
    // Wait, everything above shifted by offTop.
    // yRPM was 185 + offBot (fixed). offTop is dynamic now.
    // We should probably shift this too to maintain relative layout?
    // Let's rely on offTop logic implicitly if we want to move WHOLE dashboard.
    // Actually, offTop was for "Top Part". This is bottom part.
    // If we want to Center Vertically, we should add offTop offset to
    // EVERYTHING. Original code: yRPM = 185 + offBot. (Independent of offTop).
    // Let's add the ((H-240)/2) offset here too.
    int vShift = (SCREEN_HEIGHT - 240) / 2;
    yRPM = 185 + offBot + vShift;

    // Scale Bar to Screen Width? Or Keep 254?
    // Let's keep 254 centered.
    int barW = 254;
    int barX = (SCREEN_WIDTH - barW) / 2;

    tft->drawRect(barX, yRPM, barW, 17, colWhite);

    // ANGKA LIVE RPM (Y=205 Absolute, Rendered around 215?)
    // Note: offBot=10. Y=205.
    tft->setTextSize(2);
    tft->setTextColor(colWhite, _ui->getBackgroundColor());
    char rpmBuf[10];
    int dispRpm = (_lastRPM < 0) ? 0 : _lastRPM;
    sprintf(rpmBuf, "%d", dispRpm);
    tft->drawCentreString(rpmBuf, cx, 205 + offBot + vShift, 1);

    // Isi Bar RPM
    int maxRpmWidth = barW - 2;
    int curRpmWidth =
        map(constrain(_lastRPM, 0, 8000), 0, 8000, 0, maxRpmWidth);

    tft->fillRect(barX + 1, yRPM + 1, curRpmWidth, 15, colTheme);
    tft->fillRect(barX + 1 + curRpmWidth, yRPM + 1, maxRpmWidth - curRpmWidth,
                  15, COLOR_BG);
  }

  // --- UPDATE DINAMIS ---
  if (!force) {
    char buf[10];
    tft->setFreeFont(&Org_01);

    // Update Indikator Atas
    // Update Indikator Atas
    tft->setTextSize(2); // Match size defined in static draw
    tft->setTextDatum(MC_DATUM);

    int boxY = 22;
    int boxH = 30;
    int yTextMid = boxY + (boxH / 2) + 2;

    // Box 1: Max RPM
    tft->setTextColor(colBoxText, colTheme);
    tft->setTextPadding(70); // Width 80 -> Padding 70 safe
    sprintf(buf, "%d", _maxRPM);
    tft->drawString(buf, x1 + 40, yTextMid);
    tft->setTextPadding(0);

    // Box 2: Max Speed
    tft->setTextColor(colBoxText, colBoxBg);
    tft->setTextPadding(90); // Width 100 -> Padding 90 safe
    sprintf(buf, "%.0f", _maxSpeed);
    tft->drawString(buf, x2 + 50, yTextMid);
    tft->setTextPadding(0);

    // Box 3: GPS Satellites
    tft->setTextColor(colBoxText, colRed);
    tft->setTextPadding(70); // Width 80 -> Padding 70 safe
    sprintf(buf, "%d", _lastSats);
    tft->drawString(buf, x3 + 40, yTextMid);
    tft->setTextPadding(0);

    tft->setTextDatum(TL_DATUM);

    // 2. Update Speed Utama
    tft->setTextColor(colTheme, _ui->getBackgroundColor());
    tft->setTextSize(7);
    tft->setTextDatum(MC_DATUM);
    tft->setTextPadding(240); // PADDING untuk hapus sisa angka lama
    int yCenterSpeed = 86 + offTop;
    char speedBuf[10];
    sprintf(speedBuf, "%.0f", _lastSpeed);
    tft->drawString(speedBuf, cx, yCenterSpeed);
    tft->setTextPadding(0);
    tft->setTextDatum(TL_DATUM);

    // 3. Update Trip
    tft->setTextColor(colTheme, _ui->getBackgroundColor());
    tft->setTextSize(3);
    tft->setTextPadding(120); // PADDING
    char tripBuf[10];
    sprintf(tripBuf, "%04.0f", _lastTrip);
    tft->drawCentreString(tripBuf, cx, 142 + offTop, 1);
    tft->setTextPadding(0);

    // 4. Update Bar RPM & ANGKA RPM LIVE
    int vShift = (SCREEN_HEIGHT - 240) / 2;
    int yRPM = 185 + offBot + vShift;
    int barW = 254;
    int maxRpmWidth = barW - 2;
    int barX = (SCREEN_WIDTH - barW) / 2;

    int curRpmWidth =
        map(constrain(_lastRPM, 0, 8000), 0, 8000, 0, maxRpmWidth);

    // Update Bar
    tft->fillRect(barX + 1, yRPM + 1, curRpmWidth, 15, colTheme);
    tft->fillRect(barX + 1 + curRpmWidth, yRPM + 1, maxRpmWidth - curRpmWidth,
                  15, COLOR_BG);

    // Update Angka RPM
    tft->setTextSize(2);
    tft->setTextColor(colWhite, _ui->getBackgroundColor());
    tft->setTextPadding(100); // PADDING untuk mencegah ghosting
    char rpmBuf[10];
    int dispRpm = (_lastRPM < 0) ? 0 : _lastRPM;
    sprintf(rpmBuf, "%d", dispRpm);
    tft->drawCentreString(rpmBuf, cx, 205 + offBot + vShift, 1);
    tft->setTextPadding(0);
  }
}
