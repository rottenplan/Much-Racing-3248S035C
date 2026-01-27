#ifndef KEYBOARD_COMPONENT_H
#define KEYBOARD_COMPONENT_H

#include "../../config.h"
#include <Arduino.h>
#include <TFT_eSPI.h>

class KeyboardComponent {
public:
  enum KeyType { KEY_CHAR, KEY_SHIFT, KEY_DEL, KEY_SPACE, KEY_OK, KEY_NONE };

  struct KeyResult {
    KeyType type;
    char value;
  };

  void draw(TFT_eSPI *tft, int startY, bool isUppercase);
  KeyResult handleTouch(int x, int y, int startY);

private:
  static const int KEY_W = 44; // 44*10 + 9*4 = 476px (Fits in 480)
  static const int KEY_H = 40; // Taller for easier touch
  static const int GAP = 4;
};

#endif
