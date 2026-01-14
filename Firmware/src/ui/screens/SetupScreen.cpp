#include "SetupScreen.h"
#include "../../core/WiFiManager.h"
#include "../fonts/Org_01.h"

extern WiFiManager wifiManager;

// Update onShow to reset new variables
void SetupScreen::onShow() {
  _currentStep = STEP_WELCOME;
  _username = "";
  _wifiSSID = "";
  _wifiPassword = "";
  _isEditingUsername = false;
  _isEditingSSID = false;
  _isEditingPassword = false;
  _cursorVisible = true;
  _lastTouchTime = 0;
  _lastTapY = -1;
  _isUppercase = true;
  _showPassword = false;
  _scanCount = 0;    // Reset
  _scrollOffset = 0; // Reset
  _hasScanned = false;
  _lastWiFiTapIndex = -1;
  _lastWiFiTapTime = 0;

  drawWelcome();
}

// Update update() to handle new step with Compact Coordinates
void SetupScreen::update() {
  TFT_eSPI *tft = _ui->getTft();
  UIManager::TouchPoint tp = _ui->getTouchPoint();

  // Handle cursor blink
  if (millis() - _lastBlinkTime > 500) {
    _cursorVisible = !_cursorVisible;
    _lastBlinkTime = millis();

    switch (_currentStep) {
    case STEP_ACCOUNT:
      if (_isEditingUsername)
        drawTextField("USERNAME", _username, 30, _isEditingUsername, false);
      else if (_isEditingAccountPassword)
        drawTextField("PASSWORD", _password, 70, _isEditingAccountPassword,
                      true);
      break;
    case STEP_WIFI:
      if (_isEditingSSID)
        drawTextField("SSID", _wifiSSID, 30, _isEditingSSID, false);
      else if (_isEditingPassword)
        drawTextField("PASSWORD", _wifiPassword, 70, _isEditingPassword, true);
      break;
    default:
      break;
    }
  }

  // Handle touch
  if (tp.x >= 0 && tp.y >= 0) {
    if (millis() - _lastTouchTime < 200)
      return;
    _lastTouchTime = millis();

    switch (_currentStep) {
    case STEP_WELCOME:
      handleWelcomeTouch(tp.x, tp.y);
      break;
    case STEP_ACCOUNT:
      handleAccountTouch(tp.x, tp.y);
      break;
    case STEP_WIFI_SCAN:
      handleWiFiScanTouch(tp.x, tp.y);
      break;
    case STEP_WIFI:
      handleWiFiTouch(tp.x, tp.y);
      break;
    case STEP_COMPLETE:
      handleCompleteTouch(tp.x, tp.y);
      break;
    }
  }
}

// ===== DRAWING METHODS =====

void SetupScreen::drawWelcome() {
  TFT_eSPI *tft = _ui->getTft();
  // Clear entire screen
  tft->fillScreen(COLOR_BG);

  tft->setFreeFont(&Org_01);
  tft->setTextSize(2);
  tft->setTextColor(COLOR_PRIMARY, COLOR_BG);
  tft->setTextDatum(MC_DATUM);

  // Title
  tft->setTextSize(1);
  tft->setTextColor(TFT_WHITE, COLOR_BG);
  tft->drawString("WELCOME TO", SCREEN_WIDTH / 2, 75);

  tft->setTextSize(2);
  tft->setTextColor(COLOR_PRIMARY, COLOR_BG);
  tft->drawString("MUCH RACING", SCREEN_WIDTH / 2, 90);

  tft->setTextSize(1);
  tft->setTextColor(COLOR_TEXT, COLOR_BG);
  tft->drawString("LET'S GET STARTED", SCREEN_WIDTH / 2, 130);

  // Continue button
  drawButton("TAP TO BEGIN", SCREEN_WIDTH / 2 - 80, 180, 160, 40, false);
}

void SetupScreen::drawComplete() {
  TFT_eSPI *tft = _ui->getTft();
  tft->fillScreen(COLOR_BG);

  tft->setFreeFont(&Org_01);
  tft->setTextSize(2);
  tft->setTextColor(TFT_GREEN, COLOR_BG);
  tft->setTextDatum(MC_DATUM);

  // Success message
  tft->drawString("SETUP", SCREEN_WIDTH / 2, 80);
  tft->drawString("COMPLETE!", SCREEN_WIDTH / 2, 110);

  tft->setTextSize(1);
  tft->setTextColor(COLOR_TEXT, COLOR_BG);

  if (_username.length() > 0) {
    String msg = "WELCOME " + _username + "!";
    tft->drawString(msg, SCREEN_WIDTH / 2, 150);
  }

  // Continue button
  drawButton("START RACING", SCREEN_WIDTH / 2 - 80, 180, 160, 40, false);
}

// Updated Account Setup (Compact Layout)
void SetupScreen::drawAccountSetup(bool fullRedraw) {
  TFT_eSPI *tft = _ui->getTft();

  if (fullRedraw) {
    tft->fillScreen(COLOR_BG);

    // Header (Use Small Font 1 to avoid overlap with buttons)
    tft->setTextFont(1);
    tft->setTextSize(1);
    tft->setTextColor(COLOR_PRIMARY, COLOR_BG);
    tft->setTextDatum(TC_DATUM);
    tft->drawString("ACCOUNT SETUP", SCREEN_WIDTH / 2, 5);

    // Nav Buttons
    // Skip button removed to enforce registration
    drawButton("NEXT", SCREEN_WIDTH - 55, 2, 50, 20,
               _username.length() > 0 && _password.length() > 0);
  }

  // Fields (Compact: Y=30, Y=70)
  drawTextField("USERNAME", _username, 30, _isEditingUsername, false);
  drawTextField("PASSWORD", _password, 70, _isEditingAccountPassword, true);

  // Keyboard (Y=110)
  if (_isEditingUsername || _isEditingAccountPassword) {
    drawKeyboard(110, _isEditingAccountPassword);
  }
}

void SetupScreen::drawTextField(const char *label, String value, int y,
                                bool isActive, bool isPassword) {
  TFT_eSPI *tft = _ui->getTft();

  // Use Org_01 (Tiny Font) for the Label
  tft->setFreeFont(&Org_01);
  tft->setTextSize(1);
  tft->setTextDatum(BL_DATUM);
  tft->setTextColor(COLOR_TEXT, COLOR_BG);
  tft->drawString(label, 10, y);

  // Field background (Start box at y+2 to give gap)
  // Box: y+2 to y+27 (Height 25)
  uint16_t borderColor = isActive ? COLOR_PRIMARY : COLOR_SECONDARY;
  tft->drawRect(10, y + 2, SCREEN_WIDTH - 20, 25, borderColor);
  tft->fillRect(11, y + 3, SCREEN_WIDTH - 22, 23, TFT_DARKGREY);

  // Switch back to Standard Font 1 for Value
  tft->setTextFont(1);
  tft->setTextSize(1);

  // Field value
  String displayValue = value;
  if (isPassword && !_showPassword && value.length() > 0) {
    displayValue = "";
    for (int i = 0; i < value.length(); i++) {
      displayValue += "*";
    }
  }

  // Draw Value centered in box (y + 2 + 12.5 = ~y+15)
  tft->setTextColor(TFT_WHITE, TFT_DARKGREY);
  tft->setTextDatum(ML_DATUM);
  tft->drawString(displayValue, 15, y + 15);

  // Password Toggle Button for WiFi (Right aligned in box)
  if (isPassword) {
    tft->setTextDatum(MR_DATUM);
    tft->setTextColor(COLOR_HIGHLIGHT, TFT_DARKGREY);
    tft->drawString(_showPassword ? "HIDE" : "SHOW", SCREEN_WIDTH - 15, y + 15);
  }

  // Cursor
  if (isActive && _cursorVisible) {
    int cursorX = 15 + tft->textWidth(displayValue);
    // Cursor Line: y+7 to y+23 (16px tall centered)
    tft->drawFastVLine(cursorX, y + 7, 16, COLOR_PRIMARY);
  }
}

void SetupScreen::drawButton(const char *label, int x, int y, int w, int h,
                             bool isHighlighted) {
  TFT_eSPI *tft = _ui->getTft();

  uint16_t bgColor = isHighlighted ? COLOR_PRIMARY : COLOR_BG;
  uint16_t borderColor = isHighlighted ? COLOR_PRIMARY : COLOR_SECONDARY;
  uint16_t textColor = isHighlighted ? COLOR_BG : COLOR_TEXT;

  tft->fillRect(x, y, w, h, bgColor);
  tft->drawRect(x, y, w, h, borderColor);

  tft->setTextSize(1);
  tft->setTextDatum(MC_DATUM);
  tft->setTextColor(textColor, bgColor);
  tft->drawString(label, x + w / 2, y + h / 2);
}

void SetupScreen::drawKeyboard(int y, bool isPassword) {
  _keyboard.draw(_ui->getTft(), y, _isUppercase);
}

// New WiFi Scan Screen
void SetupScreen::drawWiFiScan() {
  TFT_eSPI *tft = _ui->getTft();
  tft->fillScreen(COLOR_BG);

  // Header (Small Font 1)
  tft->setTextFont(1);
  tft->setTextSize(1);
  tft->setTextColor(COLOR_PRIMARY, COLOR_BG);
  tft->setTextDatum(TC_DATUM);
  tft->drawString("SELECT WIFI NETWORK", SCREEN_WIDTH / 2, 5);

  // Nav Buttons
  // Skip button removed
  drawButton("SCAN", SCREEN_WIDTH - 55, 2, 50, 20, false);

  if (!_hasScanned) {
    tft->setTextColor(COLOR_TEXT, COLOR_BG);
    tft->setTextDatum(MC_DATUM);
    tft->drawString("Scanning...", SCREEN_WIDTH / 2, 120);

    // Perform Scan (Blocking)
    _scanCount = wifiManager.scanNetworks();
    _hasScanned = true;

    // Clear "Scanning..." text area only, don't wipe whole screen (prevents
    // flicker)
    tft->fillRect(0, 50, SCREEN_WIDTH, SCREEN_HEIGHT - 50, COLOR_BG);
  }

  // List Networks
  int startY = 50;
  int itemH = 40;
  int limit = 4; // Limit to 4 to keep "Manual" visible
  for (int i = 0; i < _scanCount && i < limit; i++) {
    int y = startY + (i * itemH);
    tft->drawRect(20, y, SCREEN_WIDTH - 40, 30, COLOR_SECONDARY);
    String ssid = wifiManager.getSSID(i);
    if (ssid.length() > 18)
      ssid = ssid.substring(0, 15) + "...";
    tft->setTextColor(TFT_WHITE, COLOR_BG);
    tft->setTextDatum(ML_DATUM);
    tft->drawString(ssid, 30, y + 15);
    int rssi = wifiManager.getRSSI(i);
    tft->setTextDatum(MR_DATUM);
    tft->drawString(String(rssi) + "dB", SCREEN_WIDTH - 30, y + 15);
  }

  // Custom Manual Entry Option
  int visibleCount = (_scanCount > limit) ? limit : _scanCount;
  int y = startY + visibleCount * itemH;
  tft->setTextColor(COLOR_HIGHLIGHT, COLOR_BG);
  tft->setTextDatum(MC_DATUM);
  tft->drawString("Manually Enter SSID", SCREEN_WIDTH / 2, y + 15);
}

// Updated WiFi Setup (Compact Layout)
void SetupScreen::drawWiFiSetup(bool fullRedraw) {
  TFT_eSPI *tft = _ui->getTft();

  if (fullRedraw) {
    tft->fillScreen(COLOR_BG);
    // Header (Small Font 1)
    tft->setTextFont(1);
    tft->setTextSize(1);
    tft->setTextColor(COLOR_PRIMARY, COLOR_BG);
    tft->setTextDatum(TC_DATUM);
    tft->drawString("ENTER WIFI PASSWORD", SCREEN_WIDTH / 2, 5);

    // Nav Buttons
    drawButton("BACK", 5, 2, 45, 20, false);
    drawButton("CONN", SCREEN_WIDTH - 55, 2, 50, 20, _wifiSSID.length() > 0);
  }

  // Fields (Compact: Y=30, Y=70)
  drawTextField("SSID", _wifiSSID, 30, _isEditingSSID, false);
  drawTextField("PASSWORD", _wifiPassword, 70, _isEditingPassword, true);

  // Keyboard (Y=110)
  if (_isEditingSSID || _isEditingPassword) {
    drawKeyboard(110, true);
  }
}

// ===== TOUCH HANDLERS =====

void SetupScreen::handleWelcomeTouch(int x, int y) {
  // Check if "TAP TO BEGIN" button was pressed
  if (y >= 180 && y <= 220 && x >= SCREEN_WIDTH / 2 - 80 &&
      x <= SCREEN_WIDTH / 2 + 80) {
    nextStep();
  }
}

void SetupScreen::handleWiFiScanTouch(int x, int y) {
  // Skip logic removed

  // Rescan (Top Right)
  if (y < 40 && x > SCREEN_WIDTH - 60) {
    _scanCount = 0;
    _hasScanned = false;
    drawWiFiScan();
    return;
  }
  // List Selection
  int startY = 50;
  int itemH = 40;
  int limit = 4;
  for (int i = 0; i < _scanCount && i < limit; i++) {
    int itemY = startY + (i * itemH);
    if (y > itemY && y < itemY + 30) {
      if (_lastWiFiTapIndex == i && (millis() - _lastWiFiTapTime < 500)) {
        // Double Tap confirmed!
        _wifiSSID = wifiManager.getSSID(i);
        _wifiPassword = "";
        _currentStep = STEP_WIFI;
        drawWiFiSetup();
        _lastWiFiTapIndex = -1;
      } else {
        // First Tap - Select/Highlight (Redraw list?)
        // ideally we should highlight it visually. For now just track it.
        _lastWiFiTapIndex = i;
        _lastWiFiTapTime = millis();
        // Force redraw to show selection (if we implement visual selection
        // later) drawWiFiScan();
      }
      return;
    }
  }
  // Manual Entry
  int visibleCount = (_scanCount > limit) ? limit : _scanCount;
  int manY = startY + visibleCount * itemH;
  if (y > manY && y < manY + 30) {
    _wifiSSID = "";
    _wifiPassword = "";
    _currentStep = STEP_WIFI;
    drawWiFiSetup();
  }
}

// Touch Handlers with updated Y coordinates
void SetupScreen::handleWiFiTouch(int x, int y) {
  // Check SSID field
  if (y >= 30 && y <= 60) {
    _isEditingSSID = true;
    _isEditingPassword = false;
    drawWiFiSetup(false);
    return;
  }

  // Toggle Show (Check BEFORE Field Focus)
  if (x >= SCREEN_WIDTH - 60 && y >= 70 && y <= 100) {
    _showPassword = !_showPassword;
    _isEditingPassword = true; // Also focus
    _isEditingSSID = false;
    drawWiFiSetup(false);
    return;
  }

  // Check Password field
  if (y >= 70 && y <= 100) {
    _isEditingPassword = true;
    _isEditingSSID = false;
    drawWiFiSetup(false);
    return;
  }

  // Keyboard
  if ((_isEditingSSID || _isEditingPassword) && y >= 110) {
    KeyboardComponent::KeyResult res = _keyboard.handleTouch(x, y, 110);
    String &target = _isEditingSSID ? _wifiSSID : _wifiPassword;

    if (res.type == KeyboardComponent::KEY_CHAR) {
      char c = res.value;
      if (!_isUppercase && c >= 'A' && c <= 'Z')
        c += 32; // To Lowercase
      handleKeyboardInput(target, c);
    } else if (res.type == KeyboardComponent::KEY_SHIFT)
      _isUppercase = !_isUppercase;
    else if (res.type == KeyboardComponent::KEY_DEL) {
      if (target.length() > 0)
        target.remove(target.length() - 1);
    } else if (res.type == KeyboardComponent::KEY_SPACE)
      target += " ";
    else if (res.type == KeyboardComponent::KEY_OK) {
      _isEditingSSID = false;
      _isEditingPassword = false;
    }

    drawWiFiSetup(false);
    return;
  }
  // Nav Buttons (Top)
  if (y <= 25) {
    if (x <= 60) {
      if (millis() - _lastBackTapTime < 500) {
        _hasScanned = false; // Force rescan
        _isEditingSSID = false;
        _isEditingPassword = false;
        _currentStep = STEP_WIFI_SCAN;
        drawWiFiScan();
        _lastBackTapTime = 0;
      } else {
        _lastBackTapTime = millis();
      }
      return;
    }
    if (x >= SCREEN_WIDTH - 90 && _wifiSSID.length() > 0) {
      TFT_eSPI *tft = _ui->getTft();

      // Clear Main Area below Header (30px down)
      tft->fillRect(0, 30, SCREEN_WIDTH, SCREEN_HEIGHT - 30, COLOR_BG);

      // _ui->drawStatusBar(true); // Removed to prevent overlap
      tft->setTextColor(TFT_WHITE, COLOR_BG);
      tft->setTextDatum(MC_DATUM);
      tft->drawString("CONNECTING...", SCREEN_WIDTH / 2, 120);

      bool connected =
          wifiManager.connect(_wifiSSID.c_str(), _wifiPassword.c_str());

      // Clear Status Area
      tft->fillRect(0, 100, SCREEN_WIDTH, 50, COLOR_BG);

      if (connected) {
        tft->setTextColor(TFT_GREEN, COLOR_BG);
        tft->drawString("CONNECTED!", SCREEN_WIDTH / 2, 120);
        delay(1500);
      } else {
        tft->setTextColor(TFT_RED, COLOR_BG);
        tft->drawString("FAILED!", SCREEN_WIDTH / 2, 120);
        delay(1500);
      }
      nextStep();
    }
  }
}

void SetupScreen::handleCompleteTouch(int x, int y) {
  if (y >= 180 && y <= 220 && x >= SCREEN_WIDTH / 2 - 80 &&
      x <= SCREEN_WIDTH / 2 + 80) {
    saveSetupComplete();
    _ui->switchScreen(SCREEN_MENU);
  }
}

void SetupScreen::handleKeyboardInput(String &target, char key) {
  if (target.length() < 30)
    target += key;
}

void SetupScreen::handleAccountTouch(int x, int y) {
  // Username: 30-60
  if (y >= 30 && y <= 60) {
    _isEditingUsername = true;
    _isEditingAccountPassword = false;
    drawAccountSetup(false);
    return;
  }

  // Toggle Show (Check BEFORE Field Focus)
  if (x >= SCREEN_WIDTH - 60 && y >= 70 && y <= 100) {
    _showPassword = !_showPassword;
    _isEditingAccountPassword = true; // Focus
    _isEditingUsername = false;
    drawAccountSetup(false);
    return;
  }

  // Password: 70-100
  if (y >= 70 && y <= 100) {
    _isEditingUsername = false;
    _isEditingAccountPassword = true;
    drawAccountSetup(false);
    return;
  }

  // Keyboard >= 110
  if ((_isEditingUsername || _isEditingAccountPassword) && y >= 110) {
    KeyboardComponent::KeyResult res = _keyboard.handleTouch(x, y, 110);
    String &target = _isEditingUsername ? _username : _password;

    if (res.type == KeyboardComponent::KEY_CHAR) {
      char c = res.value;
      if (!_isUppercase && c >= 'A' && c <= 'Z')
        c += 32; // To Lowercase
      handleKeyboardInput(target, c);
    } else if (res.type == KeyboardComponent::KEY_SHIFT)
      _isUppercase = !_isUppercase;
    else if (res.type == KeyboardComponent::KEY_DEL) {
      if (target.length() > 0)
        target.remove(target.length() - 1);
    } else if (res.type == KeyboardComponent::KEY_SPACE)
      target += " ";
    else if (res.type == KeyboardComponent::KEY_OK) {
      _isEditingUsername = false;
      _isEditingAccountPassword = false;
    }

    drawAccountSetup(false);
    return;
  }

  // Nav Buttons
  if (y <= 25) {
    // Skip Logic Removed
    if (x >= SCREEN_WIDTH - 60 && _username.length() > 0)
      nextStep();
  }
}

// Updated nextStep routing
void SetupScreen::nextStep() {
  switch (_currentStep) {
  case STEP_WELCOME:
    _currentStep = STEP_ACCOUNT;
    drawAccountSetup();
    break;
  case STEP_ACCOUNT:
    // Save Account
    if (_username.length() > 0) {
      Preferences prefs;
      prefs.begin("muchrace", false);
      prefs.putString("username", _username);
      if (_password.length() > 0)
        prefs.putString("password", _password);
      prefs.end();
    }
    // Go to Scan
    _currentStep = STEP_WIFI_SCAN;
    drawWiFiScan();
    break;
  case STEP_WIFI_SCAN:
    // Handled in touch, but logic flow: Scan -> Wifi Setup
    break;
  case STEP_WIFI:
    _currentStep = STEP_COMPLETE;
    drawComplete();
    break;
  default:
    break;
  }
}

void SetupScreen::saveSetupComplete() {
  Preferences prefs;
  prefs.begin("muchrace", false);
  prefs.putBool("setup_done", true);
  prefs.end();

  Serial.println("Setup marked as complete!");
}
