#include "config.h"
#include "core/GPSManager.h"
#include <TAMC_GT911.h>

#include "core/SessionManager.h"
#include "ui/UIManager.h"
#include <Arduino.h>

// Hardware Objects
TFT_eSPI tft = TFT_eSPI();
TAMC_GT911 touch(TOUCH_SDA, TOUCH_SCL, TOUCH_INT, TOUCH_RST, TOUCH_WIDTH,
                 TOUCH_HEIGHT);
// TouchLib touch(
//     Wire,
//     0x00); // Placeholder, waiting for list_dir results to know constructor
UIManager uiManager(&tft);
GPSManager gpsManager;
SessionManager sessionManager;

// SPIClass touchSpi = SPIClass(HSPI); // Not needed for I2C

void setup() {
  Serial.begin(115200);
  Serial.println("Starting RaceBox Clone...");

  // Init Pins

  // Init UI (TFT)
  tft.init();
  tft.setRotation(1); // 0=Portrait, 1=Landscape. Check your mounting!
  tft.fillScreen(COLOR_BG);

  // Init Touch
  // Initialize the specific SPI instance for Touch
  // Pins from Schematic: CLK=14, MISO=12, MOSI=13, CS=33
  touch.begin();
  // touch.setRotation(1); // Try 1 (Left/Landscape) to match TFT Rotation 1.
  // touch.setRotation(ROTATION_RIGHT); // Match TFT rotation (1)
  // if (!touch.begin()) { // XPT begin returns void usually, or we assume it
  // works
  //   Serial.println("Touch initialization failed!");
  // }

  // Init Backlight PWM
  ledcSetup(0, 5000, 8); // Channel 0, 5kHz, 8-bit
  ledcAttachPin(PIN_TFT_BL, 0);
  ledcWrite(0, 255); // Default Full Brightness

  // Init Core
  gpsManager.begin();
  sessionManager.begin();

  uiManager.setTouch(&touch); // Pass touch object to UI
  uiManager.begin();

  Serial.println("System Ready");
}

void loop() {
  gpsManager.update();

  uiManager.update();
  // sessionManager autosaves on buffer flush if needed, but we write direct for
  // now
}
