#include "SynchronizeScreen.h"
#include "../../config.h"
#include "../../core/SyncManager.h"
#include "../../core/WiFiManager.h"
#include "../fonts/Org_01.h"

extern SyncManager syncManager;
extern WiFiManager wifiManager;

void SynchronizeScreen::begin(UIManager *ui) { _ui = ui; }

void SynchronizeScreen::onShow() {
  _statusMessage = "READY TO SYNC";
  _detailMessage = "Tap below to update";
  _isSyncing = false;
  _lastSyncSuccess = false;
  _lastTouchTime = 0;

  // Static Draw (Background & Header)
  TFT_eSPI *tft = _ui->getTft();
  tft->fillRect(0, STATUS_BAR_HEIGHT, SCREEN_WIDTH,
                SCREEN_HEIGHT - STATUS_BAR_HEIGHT, COLOR_BG);

  // Header using standard font
  tft->setFreeFont(&Org_01);
  tft->setTextSize(2);
  tft->setTextColor(COLOR_HIGHLIGHT, COLOR_BG);
  tft->setTextDatum(TC_DATUM);
  tft->drawString("SYNCHRONIZE", SCREEN_WIDTH / 2, 35);

  // Back Arrow
  tft->setTextSize(2);
  tft->setTextDatum(TL_DATUM);
  tft->setTextColor(COLOR_HIGHLIGHT, COLOR_BG);
  tft->drawString("<", 10, 25);

  drawScreen(true);
}

void SynchronizeScreen::update() {
  UIManager::TouchPoint p = _ui->getTouchPoint();
  if (p.x != -1 && p.y != -1) {
    if (millis() - _lastTouchTime > 150) { // Reduced debounce for double tap
      _lastTouchTime = millis();
      handleTouch(p.x, p.y);
    }
  }
}

void SynchronizeScreen::drawScreen(bool fullRedraw) {
  TFT_eSPI *tft = _ui->getTft();

  // Clear dynamic area (below header)
  // Header ends approx y=40. Start clearing at 50.
  tft->fillRect(0, 50, SCREEN_WIDTH, SCREEN_HEIGHT - 50, COLOR_BG);

  // Ensure font is set
  tft->setFreeFont(&Org_01);

  // Status Area
  tft->setTextSize(1);
  tft->setTextColor(COLOR_TEXT, COLOR_BG);
  tft->setTextDatum(MC_DATUM);

  // Main Status
  if (_isSyncing) {
    tft->setTextColor(TFT_YELLOW, COLOR_BG);
  } else if (_lastSyncSuccess) {
    tft->setTextColor(TFT_GREEN, COLOR_BG);
  } else if (_statusMessage.indexOf("FAILED") != -1) {
    tft->setTextColor(TFT_RED, COLOR_BG);
  }
  tft->drawString(_statusMessage, SCREEN_WIDTH / 2, 80);

  // Details
  tft->setTextColor(TFT_WHITE, COLOR_BG);
  tft->drawString(_detailMessage, SCREEN_WIDTH / 2, 110);

  // Sync Button
  if (!_isSyncing) {
    int btnW = 200;
    int btnH = 50;
    int btnX = (SCREEN_WIDTH - btnW) / 2;
    int btnY = 160;

    uint16_t btnColor = COLOR_PRIMARY;
    tft->fillRect(btnX, btnY, btnW, btnH, btnColor);
    tft->drawRect(btnX, btnY, btnW, btnH, TFT_WHITE);

    tft->setTextColor(COLOR_BG, btnColor);
    tft->setTextDatum(MC_DATUM);
    tft->drawString("START SYNC", SCREEN_WIDTH / 2, btnY + btnH / 2);
  } else {
    // Show spinner placeholder or "Please Wait"
    tft->setTextColor(TFT_BROWN, COLOR_BG); // Orange-ish
    tft->drawString("PLEASE WAIT...", SCREEN_WIDTH / 2, 180);
  }

  // Footer info
  tft->setTextColor(TFT_DARKGREY, COLOR_BG);
  tft->setTextDatum(BC_DATUM);
  String lastSync = "Last Sync: " + syncManager.getLastSyncTime();
  tft->drawString(lastSync, SCREEN_WIDTH / 2, SCREEN_HEIGHT - 10);
}

void SynchronizeScreen::handleTouch(int x, int y) {
  // Back button area
  if (x < 50 && y < 50) {
    static unsigned long lastBackTap = 0;
    if (millis() - lastBackTap < 500) {
      _ui->switchScreen(SCREEN_MENU);
      lastBackTap = 0;
    } else {
      lastBackTap = millis();
    }
    return;
  }

  // Sync Button Area (approx)
  if (!_isSyncing && y >= 160 && y <= 210) {
    _isSyncing = true;
    _statusMessage = "CONNECTING...";
    _detailMessage = "Checking WiFi...";
    drawScreen(false); // Partial redraw

    // Use a small delay/yield to let UI update before blocking work
    delay(100);
    performSync();
  }
}

void SynchronizeScreen::performSync() {
  // 1. Check/Connect WiFi
  if (WiFi.status() != WL_CONNECTED) {
    // Try to auto-connect with saved credentials
    if (!wifiManager.tryAutoConnect()) {
      _statusMessage = "WIFI DISCONNECTED";
      _detailMessage = "Setup WiFi in Settings";
      _isSyncing = false;
      drawScreen(false);
      return;
    }
  }

  // 2. Get Credentials
  Preferences prefs;
  prefs.begin("muchrace", true);
  String username = prefs.getString("username", "");
  String password = prefs.getString("password", ""); // Account password
  prefs.end();

  if (username.length() == 0 || password.length() == 0) {
    _statusMessage = "AUTH ERROR";
    _detailMessage = "Update Setup > Account";
    _isSyncing = false;
    drawScreen(false);
    return;
  }

  // 3. Perform Settings Sync
  _statusMessage = "SYNCING SETTINGS...";
  drawScreen(false);
  bool settingsSuccess =
      syncManager.syncSettings(API_URL, username.c_str(), password.c_str());

  // 4. Perform Session Upload
  _statusMessage = "UPLOADING SESSIONS...";
  drawScreen(false);
  bool uploadSuccess =
      syncManager.uploadSessions(API_URL, username.c_str(), password.c_str());

  // 5. Perform GPX Track Upload
  _statusMessage = "UPLOADING TRACKS...";
  drawScreen(false);
  bool gpxSuccess =
      syncManager.uploadGPXTracks(API_URL, username.c_str(), password.c_str());

  if (settingsSuccess && uploadSuccess && gpxSuccess) {
    _statusMessage = "SYNC COMPLETE";
    _detailMessage = "All Data Updated";
    _lastSyncSuccess = true;
  } else if (settingsSuccess && (uploadSuccess || gpxSuccess)) {
    _statusMessage = "PARTIAL SYNC";
    _detailMessage = "Upload Incomplete";
    _lastSyncSuccess = true;
  } else if (settingsSuccess) {
    _statusMessage = "SETTINGS SYNCED";
    _detailMessage = "Upload Failed";
    _lastSyncSuccess = true;
  } else {
  }

  _isSyncing = false;
  drawScreen(false);
}
