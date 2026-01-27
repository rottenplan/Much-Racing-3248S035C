#include "KeyboardComponent.h"

void KeyboardComponent::draw(TFT_eSPI *tft, int startY, bool isUppercase) {
  tft->setTextFont(1);
  tft->setTextSize(1); // Standard font size 1? Or 2 for visibility?
  // Let's stick to 1 per previous code, or try 2 if buttons are big enough.
  // 40px height allows Size 2 comfortably? Size 2 is ~16px high. Yes.
  // But let's keep Size 1 for safety first, can upgrade later.

  // Colors
  uint16_t keyColor = TFT_DARKGREY;
  uint16_t txtColor = TFT_WHITE;
  uint16_t actColor = 0x04DF; // Primary/Accent
  uint16_t hlColor = 0xF800;  // Highlight/Red

  // Rows
  String rows[] = {"1234567890", "QWERTYUIOP", "ASDFGHJKL", "ZXCVBNM"};

  // Draw alpha-numeric rows (0-3)
  for (int row = 0; row < 4; row++) {
    String keys = rows[row];
    int numKeys = keys.length();
    int totalW = (numKeys * KEY_W) + ((numKeys - 1) * GAP);
    int startX = (SCREEN_WIDTH - totalW) / 2;

    for (int col = 0; col < numKeys; col++) {
      int x = startX + (col * (KEY_W + GAP));
      int ky = startY + (row * (KEY_H + GAP));

      // Draw Key Background
      tft->fillRoundRect(x, ky, KEY_W, KEY_H, 4, keyColor);

      // Draw Character
      tft->setTextColor(txtColor, keyColor);
      tft->setTextDatum(MC_DATUM);

      char c = keys[col];
      if (!isUppercase && row > 0) { // Keep numbers as is
        if (c >= 'A' && c <= 'Z') {
          c = c + ('a' - 'A');
        }
      }
      tft->drawString(String(c), x + KEY_W / 2, ky + KEY_H / 2);
    }
  }

  // Row 4: Special Keys (SHIFT, SPACE, DEL, OK)
  int rowY = startY + (4 * (KEY_H + GAP));

  // Widths
  int shiftW = 55;
  int delW = 55;
  int okW = 70;
  // Space takes remaining width with gaps
  // Total target width ~460?
  // 55+55+70 = 180.
  // Gaps: 3 * 4 = 12.
  // Remaining for Space: 460 - 192 = 268? Or explicitly set.
  int spaceW = 240;

  int totalActionW = shiftW + spaceW + delW + okW + (3 * GAP);
  int startX = (SCREEN_WIDTH - totalActionW) / 2;

  int currentX = startX;

  // 1. SHIFT
  uint16_t shiftBg = isUppercase ? hlColor : keyColor;
  uint16_t shiftTxt = isUppercase ? TFT_WHITE : TFT_WHITE; // Always white text
  tft->fillRoundRect(currentX, rowY, shiftW, KEY_H, 4, shiftBg);
  tft->setTextColor(shiftTxt, shiftBg);
  tft->drawString("SHFT", currentX + shiftW / 2, rowY + KEY_H / 2);
  currentX += shiftW + GAP;

  // 2. SPACE
  tft->fillRoundRect(currentX, rowY, spaceW, KEY_H, 4, keyColor);
  tft->setTextColor(txtColor, keyColor);
  tft->drawString("SPACE", currentX + spaceW / 2, rowY + KEY_H / 2);
  currentX += spaceW + GAP;

  // 3. DEL
  tft->fillRoundRect(currentX, rowY, delW, KEY_H, 4,
                     hlColor); // Red for delete? Or just Grey?
  // User requested "redesign", let's make it intuitive. Red for Delete is
  // common.
  tft->setTextColor(TFT_WHITE, hlColor);
  tft->drawString("DEL", currentX + delW / 2, rowY + KEY_H / 2);
  currentX += delW + GAP;

  // 4. OK
  tft->fillRoundRect(currentX, rowY, okW, KEY_H, 4, actColor); // Green/Primary
  tft->setTextColor(TFT_WHITE, actColor);
  tft->drawString("OK", currentX + okW / 2, rowY + KEY_H / 2);
}

KeyboardComponent::KeyResult KeyboardComponent::handleTouch(int x, int y,
                                                            int startY) {
  KeyResult result = {KEY_NONE, 0};

  // Check vertical bounds first
  if (y < startY)
    return result;

  // Calculate generic row height including gap
  int rowHeight = KEY_H + GAP;
  int keyY = y - startY;

  // Determine row index
  int row = keyY / rowHeight;

  // Boundary check for gap area (if touch is in the gap)
  int yInRow = keyY % rowHeight;
  if (yInRow > KEY_H) {
    // Touch is in the vertical gap
    return result;
  }

  String rows[] = {"1234567890", "QWERTYUIOP", "ASDFGHJKL", "ZXCVBNM"};

  if (row >= 0 && row < 4) {
    String keys = rows[row];
    int numKeys = keys.length();
    int totalW = (numKeys * KEY_W) + ((numKeys - 1) * GAP);
    int startX = (SCREEN_WIDTH - totalW) / 2;

    // Check horizontal bounds for the whole row
    if (x < startX || x > startX + totalW)
      return result;

    int relX = x - startX;
    int keyWidthWithGap = KEY_W + GAP;
    int col = relX / keyWidthWithGap;
    int xInKey = relX % keyWidthWithGap;

    // Check if touch is within the key width (not in the gap)
    if (xInKey <= KEY_W && col < numKeys) {
      result.type = KEY_CHAR;
      result.value = keys[col];
    }
  } else if (row == 4) {
    // Row 4: Special Keys (SHIFT, SPACE, DEL, OK)
    // Widths must match draw() exactly
    int shiftW = 55;
    int delW = 55;
    int okW = 70;
    int spaceW = 240;

    int totalActionW = shiftW + spaceW + delW + okW + (3 * GAP);
    int startX = (SCREEN_WIDTH - totalActionW) / 2;

    int currentX = startX;

    // SHIFT
    if (x >= currentX && x < currentX + shiftW) {
      result.type = KEY_SHIFT;
      return result;
    }
    currentX += shiftW + GAP;

    // SPACE
    if (x >= currentX && x < currentX + spaceW) {
      result.type = KEY_SPACE;
      return result;
    }
    currentX += spaceW + GAP;

    // DEL
    if (x >= currentX && x < currentX + delW) {
      result.type = KEY_DEL;
      return result;
    }
    currentX += delW + GAP;

    // OK
    if (x >= currentX && x < currentX + okW) {
      result.type = KEY_OK;
      return result;
    }
  }

  return result;
}
