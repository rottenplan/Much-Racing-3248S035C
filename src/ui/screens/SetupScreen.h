#ifndef SETUP_SCREEN_H
#define SETUP_SCREEN_H

#include "../UIManager.h"
#include "../components/KeyboardComponent.h"
#include <Preferences.h>

class SetupScreen : public UserScreen {
public:
  void begin(UIManager *ui) override { _ui = ui; }
  void onShow() override;
  void update() override;

private:
  UIManager *_ui;
  KeyboardComponent _keyboard;
  int _cursorPos;
  unsigned long _lastBlinkTime;
  bool _cursorVisible;

  // Setup flow state
  enum SetupStep { STEP_WELCOME, STEP_ACCOUNT, STEP_WIFI, STEP_COMPLETE };

  SetupStep _currentStep;

  // Account data
  String _username;
  String _password; // Account Password

  // WiFi data
  String _wifiSSID;
  String _wifiPassword;

  // Input handling
  bool _isEditingUsername;
  bool _isEditingSSID;
  bool _isEditingPassword;        // For WiFi
  bool _isEditingAccountPassword; // For Account
  bool _isUppercase;
  bool _showPassword;
  // Touch state
  unsigned long _lastTouchTime;
  int _lastTapY;

  // Drawing methods
  void drawWelcome();
  void drawAccountSetup();
  void drawWiFiSetup();
  void drawComplete();

  void drawKeyboard(int y, bool isPassword = false);
  void drawTextField(const char *label, String value, int y, bool isActive,
                     bool isPassword = false);
  void drawButton(const char *label, int x, int y, int w, int h,
                  bool isHighlighted = false);

  // Input methods
  void handleWelcomeTouch(int x, int y);
  void handleAccountTouch(int x, int y);
  void handleWiFiTouch(int x, int y);
  void handleCompleteTouch(int x, int y);

  void handleKeyboardInput(String &target, char key);

  // Navigation
  void nextStep();
  void saveSetupComplete();
};

#endif
