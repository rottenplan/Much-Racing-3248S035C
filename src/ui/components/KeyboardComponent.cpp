#include "KeyboardComponent.h"

void KeyboardComponent::draw(TFT_eSPI *tft, int startY, bool isUppercase) {
  tft->setTextFont(1);
  tft->setTextSize(1);

  // QWERTY keyboard rows
  String rows[] = {"1234567890", "QWERTYUIOP", "ASDFGHJKL", "ZXCVBNM"};

  for (int row = 0; row < 4; row++) {
    String keys = rows[row];
    int numKeys = keys.length();
    int totalW = numKeys * KEY_W;
    int startX = (tft->width() - totalW) / 2;

    for (int col = 0; col < numKeys; col++) {
      int x = startX + (col * KEY_W);
      int ky = startY + (row * KEY_H);

      tft->drawRect(x, ky, KEY_W, KEY_H, TFT_WHITE);
      tft->setTextColor(TFT_WHITE, COLOR_BG);
      tft->setTextDatum(MC_DATUM);

      char c = keys[col];
      if (!isUppercase && c >= 'A' && c <= 'Z') {
        c = c + ('a' - 'A');
      }
      tft->drawString(String(c), x + KEY_W / 2, ky + KEY_H / 2);
    }
  }

  // Special keys
  int specialY = startY + (4 * KEY_H);
  int shiftW = 45;
  int delW = 45;
  int spaceW = 80;
  int okW = 55;
  int totalW = shiftW + delW + spaceW + okW + (3 * GAP);
  int startX = (tft->width() - totalW) / 2;

  int shiftX = startX;
  int delX = shiftX + shiftW + GAP;
  int spaceX = delX + delW + GAP;
  int okX = spaceX + spaceW + GAP;

  // SHIFT
  uint16_t shiftColor = isUppercase ? COLOR_HIGHLIGHT : COLOR_BG;
  uint16_t shiftTxtColor = isUppercase ? TFT_BLACK : TFT_WHITE;
  tft->fillRect(shiftX, specialY, shiftW, KEY_H, shiftColor);
  if (!isUppercase)
    tft->drawRect(shiftX, specialY, shiftW, KEY_H, TFT_WHITE);
  tft->setTextColor(shiftTxtColor, shiftColor);
  tft->setTextDatum(MC_DATUM);
  tft->drawString("SHFT", shiftX + shiftW / 2, specialY + KEY_H / 2);

  // DEL
  tft->drawRect(delX, specialY, delW, KEY_H, TFT_WHITE);
  tft->setTextColor(TFT_WHITE, COLOR_BG);
  tft->drawString("DEL", delX + delW / 2, specialY + KEY_H / 2);

  // SPACE
  tft->drawRect(spaceX, specialY, spaceW, KEY_H, TFT_WHITE);
  tft->drawString("SPACE", spaceX + spaceW / 2, specialY + KEY_H / 2);

  // OK
  tft->fillRect(okX, specialY, okW, KEY_H, COLOR_PRIMARY);
  tft->setTextColor(TFT_BLACK, COLOR_PRIMARY);
  tft->drawString("OK", okX + okW / 2, specialY + KEY_H / 2);
}

KeyboardComponent::KeyResult KeyboardComponent::handleTouch(int x, int y,
                                                            int startY) {
  KeyResult result = {KEY_NONE, 0};

  int keyY = y - startY;
  if (keyY < 0)
    return result;

  int row = keyY / KEY_H;
  String rows[] = {"1234567890", "QWERTYUIOP", "ASDFGHJKL", "ZXCVBNM"};

  if (row >= 0 && row < 4) {
    String keys = rows[row];
    int numKeys = keys.length();
    int totalW = numKeys * KEY_W;
    int startX = (320 - totalW) / 2; // Assuming 320 width
    int col = (x - startX) / KEY_W;

    if (col >= 0 && col < numKeys) {
      result.type = KEY_CHAR;
      result.value = keys[col];
    }
  } else if (row == 4) {
    int shiftW = 45;
    int delW = 45;
    int spaceW = 80;
    int okW = 55;
    int totalW = shiftW + delW + spaceW + okW + (3 * GAP);
    int startX = (320 - totalW) / 2;

    if (x >= startX && x < startX + shiftW) {
      result.type = KEY_SHIFT;
    } else if (x >= startX + shiftW + GAP && x < startX + shiftW + GAP + delW) {
      result.type = KEY_DEL;
    } else if (x >= startX + shiftW + GAP + delW + GAP &&
               x < startX + shiftW + GAP + delW + GAP + spaceW) {
      result.type = KEY_SPACE;
    } else if (x >= startX + shiftW + GAP + delW + GAP + spaceW + GAP) {
      result.type = KEY_OK;
    }
  }

  return result;
}
