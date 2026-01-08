#include "SetupScreen.h"
#include "../../core/WiFiManager.h"
#include "../fonts/Org_01.h"

extern WiFiManager wifiManager;

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

  drawWelcome();
}

void SetupScreen::update() {
  TFT_eSPI *tft = _ui->getTft();
  UIManager::TouchPoint tp = _ui->getTouchPoint();

  // Handle cursor blink for text fields
  if (millis() - _lastBlinkTime > 500) {
    _cursorVisible = !_cursorVisible;
    _lastBlinkTime = millis();

    // Redraw only the relevant field to update cursor
    switch (_currentStep) {
    case STEP_ACCOUNT:
      if (_isEditingUsername)
        drawTextField("USERNAME", _username, 70, _isEditingUsername, false);
      break;
    case STEP_WIFI:
      if (_isEditingSSID)
        drawTextField("SSID", _wifiSSID, 70, _isEditingSSID, false);
      else if (_isEditingPassword)
        drawTextField("PASSWORD", _wifiPassword, 100, _isEditingPassword, true);
      break;
    default:
      break;
    }
  }

  // Handle touch input
  if (tp.x >= 0 && tp.y >= 0) {
    // Debounce
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
  tft->fillScreen(COLOR_BG);

  tft->setFreeFont(&Org_01);
  tft->setTextSize(2);
  tft->setTextColor(COLOR_PRIMARY, COLOR_BG);
  tft->setTextDatum(MC_DATUM);

  // Title
  tft->drawString("WELCOME TO", SCREEN_WIDTH / 2, 60);
  tft->drawString("MUCH RACING", SCREEN_WIDTH / 2, 90);

  tft->setTextSize(1);
  tft->setTextColor(COLOR_TEXT, COLOR_BG);
  tft->drawString("LET'S GET STARTED", SCREEN_WIDTH / 2, 130);

  // Continue button
  drawButton("TAP TO BEGIN", SCREEN_WIDTH / 2 - 80, 180, 160, 40, false);
}

void SetupScreen::drawAccountSetup() {
  TFT_eSPI *tft = _ui->getTft();
  tft->fillScreen(COLOR_BG);

  tft->setFreeFont(&Org_01);
  tft->setTextSize(1);
  tft->setTextColor(COLOR_PRIMARY, COLOR_BG);
  tft->setTextDatum(MC_DATUM);

  // Title
  tft->drawString("ACCOUNT SETUP", SCREEN_WIDTH / 2, 30);

  tft->setTextSize(1);
  tft->setTextColor(COLOR_TEXT, COLOR_BG);
  tft->drawString("ENTER YOUR USERNAME", SCREEN_WIDTH / 2, 50);

  // Username field
  drawTextField("USERNAME", _username, 70, _isEditingUsername, false);

  // Password field
  drawTextField("PASSWORD", _password, 100, _isEditingAccountPassword, true);

  // Simple keyboard
  if (_isEditingUsername) {
    drawKeyboard(130, false);
  } else if (_isEditingAccountPassword) {
    drawKeyboard(130, true);
  }

  // Navigation buttons
  drawButton("SKIP", 20, 200, 60, 30, false);
  drawButton("NEXT", SCREEN_WIDTH - 80, 200, 60, 30,
             _username.length() > 0 && _password.length() > 0);
}

void SetupScreen::drawWiFiSetup() {
  TFT_eSPI *tft = _ui->getTft();
  tft->fillScreen(COLOR_BG);

  tft->setFreeFont(&Org_01);
  tft->setTextSize(1);
  tft->setTextColor(COLOR_PRIMARY, COLOR_BG);
  tft->setTextDatum(MC_DATUM);

  // Title
  tft->drawString("WI-FI SETUP", SCREEN_WIDTH / 2, 30);

  tft->setTextSize(1);
  tft->setTextColor(COLOR_TEXT, COLOR_BG);
  tft->drawString("CONNECT TO NETWORK", SCREEN_WIDTH / 2, 50);

  // SSID field
  drawTextField("SSID", _wifiSSID, 70, _isEditingSSID, false);

  // Password field
  drawTextField("PASSWORD", _wifiPassword, 100, _isEditingPassword, true);

  // Keyboard
  if (_isEditingSSID) {
    drawKeyboard(130, false);
  } else if (_isEditingPassword) {
    drawKeyboard(130, true);
  }

  // Navigation buttons
  drawButton("SKIP", 20, 200, 60, 30, false);
  drawButton("CONNECT", SCREEN_WIDTH - 100, 200, 80, 30,
             _wifiSSID.length() > 0);
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

void SetupScreen::drawTextField(const char *label, String value, int y,
                                bool isActive, bool isPassword) {
  TFT_eSPI *tft = _ui->getTft();

  tft->setTextSize(1);
  tft->setTextDatum(ML_DATUM);
  tft->setTextColor(COLOR_TEXT, COLOR_BG);
  tft->drawString(label, 10, y);

  // Field background
  uint16_t borderColor = isActive ? COLOR_PRIMARY : COLOR_SECONDARY;
  tft->drawRect(10, y + 5, SCREEN_WIDTH - 20, 25, borderColor);
  tft->fillRect(11, y + 6, SCREEN_WIDTH - 22, 23, TFT_DARKGREY);

  // Field value
  String displayValue = value;
  if (isPassword && !_showPassword && value.length() > 0) {
    displayValue = "";
    for (int i = 0; i < value.length(); i++) {
      displayValue += "*";
    }
  }

  tft->setTextColor(TFT_WHITE, TFT_DARKGREY);
  tft->setTextDatum(ML_DATUM);
  tft->drawString(displayValue, 15, y + 17);

  // Password Toggle Button for WiFi
  if (isPassword) {
    tft->setTextDatum(MR_DATUM);
    tft->setTextColor(COLOR_HIGHLIGHT, TFT_DARKGREY);
    tft->drawString(_showPassword ? "HIDE" : "SHOW", SCREEN_WIDTH - 15, y + 17);
  }

  // Cursor
  if (isActive && _cursorVisible) {
    int cursorX = 15 + tft->textWidth(displayValue);
    tft->drawFastVLine(cursorX, y + 10, 16, COLOR_PRIMARY);
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

// ===== TOUCH HANDLERS =====

void SetupScreen::handleWelcomeTouch(int x, int y) {
  // Check if "TAP TO BEGIN" button was pressed
  if (y >= 180 && y <= 220 && x >= SCREEN_WIDTH / 2 - 80 &&
      x <= SCREEN_WIDTH / 2 + 80) {
    nextStep();
  }
}

void SetupScreen::handleAccountTouch(int x, int y) {
  // Check username field
  if (y >= 75 && y <= 100) {
    _isEditingUsername = true;
    _isEditingAccountPassword = false;
    drawAccountSetup();
    return;
  }

  // Check password field
  if (y >= 105 && y <= 130) {
    _isEditingUsername = false;
    _isEditingAccountPassword = true;
    drawAccountSetup();
    return;
  }

  // Check Password visibility toggle
  if (_isEditingAccountPassword && x >= SCREEN_WIDTH - 60 && y >= 110 &&
      y <= 135) {
    _showPassword = !_showPassword;
    drawAccountSetup();
    return;
  }

  // Check keyboard if active
  if ((_isEditingUsername || _isEditingAccountPassword) && y >= 135) {
    // Logic below handles keyboard input
    String &target = _isEditingUsername ? _username : _password;
    int keyY = y - 135;

    // Continue with existing keyboard logic but using 'target' and adjusting Y

    int keyW = 28;
    int keyH = 25;
    int row = keyY / keyH;

    // QWERTY rows
    String rows[] = {"1234567890", "QWERTYUIOP", "ASDFGHJKL", "ZXCVBNM"};

    if (row >= 0 && row < 4) {
      String keys = rows[row];
      int numKeys = keys.length();
      int totalW = numKeys * keyW;
      int startX = (SCREEN_WIDTH - totalW) / 2;
      int col = (x - startX) / keyW;

      if (col >= 0 && col < numKeys) {
        char c = keys[col];
        if (!_isUppercase && c >= 'A' && c <= 'Z')
          c += ('a' - 'A');
        handleKeyboardInput(target, c);
        drawAccountSetup();
        return;
      }
    } else if (row == 4) {
      // Special keys row
      int shiftW = 45;
      int delW = 45;
      int spaceW = 80;
      int okW = 55;
      int gap = 5;
      int totalW = shiftW + delW + spaceW + okW + (3 * gap);
      int startX = (SCREEN_WIDTH - totalW) / 2;

      // SHIFT
      if (x >= startX && x < startX + shiftW) {
        _isUppercase = !_isUppercase;
        drawAccountSetup();
      }
      // DEL
      else if (x >= startX + shiftW + gap && x < startX + shiftW + gap + delW) {
        if (target.length() > 0)
          target.remove(target.length() - 1);
        drawAccountSetup();
      }
      // SPACE
      else if (x >= startX + shiftW + gap + delW + gap &&
               x < startX + shiftW + gap + delW + gap + spaceW) {
        target += " ";
        drawAccountSetup();
      }
      // OK
      else if (x >= startX + shiftW + gap + delW + gap + spaceW + gap) {
        _isEditingUsername = false;
        _isEditingAccountPassword = false;
        drawAccountSetup();
      }
    }
  }

  // Check SKIP button
  if (y >= 200 && y <= 230 && x >= 20 && x <= 80) {
    _username = "";
    nextStep();
  }

  // Check NEXT button
  if (y >= 200 && y <= 230 && x >= SCREEN_WIDTH - 80 &&
      _username.length() > 0) {
    nextStep();
  }
}

void SetupScreen::handleWiFiTouch(int x, int y) {
  // Check SSID field
  if (y >= 75 && y <= 100) {
    _isEditingSSID = true;
    _isEditingPassword = false;
    drawWiFiSetup();
    return;
  }

  // Check Password field
  if (y >= 105 && y <= 130) {
    _isEditingPassword = true;
    _isEditingSSID = false;
    drawWiFiSetup();
    return;
  }

  // Check Password visibility toggle
  if (_isEditingPassword && x >= SCREEN_WIDTH - 60 && y >= 110 && y <= 135) {
    _showPassword = !_showPassword;
    drawWiFiSetup();
    return;
  }

  // Check keyboard if active
  if ((_isEditingSSID || _isEditingPassword) && y >= 135) {
    String &target = _isEditingSSID ? _wifiSSID : _wifiPassword;
    int keyY = y - 135;
    int keyW = 28;
    int keyH = 25;
    int row = keyY / keyH;

    String rows[] = {"1234567890", "QWERTYUIOP", "ASDFGHJKL", "ZXCVBNM"};

    if (row >= 0 && row < 4) {
      String keys = rows[row];
      int numKeys = keys.length();
      int totalW = numKeys * keyW;
      int startX = (SCREEN_WIDTH - totalW) / 2;
      int col = (x - startX) / keyW;

      if (col >= 0 && col < numKeys) {
        char c = keys[col];
        if (!_isUppercase && c >= 'A' && c <= 'Z')
          c += ('a' - 'A');
        handleKeyboardInput(target, c);
        drawWiFiSetup();
        return;
      }
    } else if (row == 4) {
      int shiftW = 45;
      int delW = 45;
      int spaceW = 80;
      int okW = 55;
      int gap = 5;
      int totalW = shiftW + delW + spaceW + okW + (3 * gap);
      int startX = (SCREEN_WIDTH - totalW) / 2;

      // SHIFT
      if (x >= startX && x < startX + shiftW) {
        _isUppercase = !_isUppercase;
        drawWiFiSetup();
      }
      // DEL
      else if (x >= startX + shiftW + gap && x < startX + shiftW + gap + delW) {
        if (target.length() > 0)
          target.remove(target.length() - 1);
        drawWiFiSetup();
      }
      // SPACE
      else if (x >= startX + shiftW + gap + delW + gap &&
               x < startX + shiftW + gap + delW + gap + spaceW) {
        target += " ";
        drawWiFiSetup();
      }
      // OK
      else if (x >= startX + shiftW + gap + delW + gap + spaceW + gap) {
        _isEditingSSID = false;
        _isEditingPassword = false;
        drawWiFiSetup();
      }
    }
  }

  // Check SKIP button
  if (y >= 200 && y <= 230 && x >= 20 && x <= 80) {
    nextStep();
  }

  // Check CONNECT button
  if (y >= 200 && y <= 230 && x >= SCREEN_WIDTH - 100 &&
      _wifiSSID.length() > 0) {
    // Show connecting message
    TFT_eSPI *tft = _ui->getTft();
    tft->fillRect(0, 50, SCREEN_WIDTH, SCREEN_HEIGHT - 50, COLOR_BG);
    _ui->drawStatusBar(true);

    tft->setTextColor(TFT_WHITE, COLOR_BG);
    tft->setTextDatum(MC_DATUM);
    tft->drawString("CONNECTING...", SCREEN_WIDTH / 2, 120);

    // Attempt to connect
    Serial.println("Attempting WiFi connection...");
    bool connected =
        wifiManager.connect(_wifiSSID.c_str(), _wifiPassword.c_str());

    // Clear "CONNECTING..." area
    tft->fillRect(0, 100, SCREEN_WIDTH, 50, COLOR_BG);

    if (connected) {
      Serial.println("WiFi connected successfully!");
      tft->setTextColor(TFT_GREEN, COLOR_BG);
      tft->drawString("CONNECTED!", SCREEN_WIDTH / 2, 120);
      delay(1500);
    } else {
      Serial.println("WiFi connection failed, continuing anyway...");
      tft->setTextColor(TFT_RED, COLOR_BG);
      tft->drawString("FAILED!", SCREEN_WIDTH / 2, 120);
      delay(1500);
    }

    nextStep();
  }
}

void SetupScreen::handleCompleteTouch(int x, int y) {
  // Check "START RACING" button
  if (y >= 180 && y <= 220 && x >= SCREEN_WIDTH / 2 - 80 &&
      x <= SCREEN_WIDTH / 2 + 80) {
    saveSetupComplete();
    _ui->switchScreen(SCREEN_MENU);
  }
}

void SetupScreen::handleKeyboardInput(String &target, char key) {
  if (target.length() < 30) { // Max length limit
    target += key;
  }
}

// ===== NAVIGATION =====

void SetupScreen::nextStep() {
  switch (_currentStep) {
  case STEP_WELCOME:
    _currentStep = STEP_ACCOUNT;
    drawAccountSetup();
    break;
  case STEP_ACCOUNT:
    // Save username if provided
    if (_username.length() > 0) {
      Preferences prefs;
      prefs.begin("muchrace", false);
      prefs.putString("username", _username);
      if (_password.length() > 0) {
        prefs.putString("password", _password);
      }
      prefs.end();
    }
    _currentStep = STEP_WIFI;
    drawWiFiSetup();
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
