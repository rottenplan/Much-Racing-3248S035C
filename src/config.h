#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ==========================================
// HARDWARE PIN DEFINITIONS
// ==========================================

// Display Pins (Matches platformio.ini)
#define PIN_TFT_MOSI 13
#define PIN_TFT_MISO 12
#define PIN_TFT_SCLK 14
#define PIN_TFT_CS 15
#define PIN_TFT_DC 2
#define PIN_TFT_BL 27

// Touch Screen (GT911 via I2C)
#define TOUCH_SDA 33
#define TOUCH_SCL 32
#define TOUCH_INT 21
#define TOUCH_RST 25
// Native Resolution (Portrait)
#define TOUCH_WIDTH 240
#define TOUCH_HEIGHT 320

// Touch Calibration / Mapping
// Common for Landscape on this board: Swap XY=true, Invert Y=true
#define TOUCH_SWAP_XY true
#define TOUCH_INVERT_X true
#define TOUCH_INVERT_Y false

// SD Card Pins (SPI)
#define PIN_SD_CS 5
#define PIN_SD_MOSI 23
#define PIN_SD_SCLK 18
#define PIN_SD_MISO 19

// Peripherals
#define PIN_RGB_RED 4
#define PIN_RGB_GREEN 16
#define PIN_RGB_BLUE 17
#define PIN_LIGHT_SENSOR 34
#define PIN_SPEAKER 26

// Legacy / Conflicting Pins (Disabled)
// #define PIN_GPS_RX 16 // CONFLICT with RGB Green
// #define PIN_GPS_TX 17 // CONFLICT with RGB Blue
// #define PIN_BATTERY 34 // CONFLICT with Light Sensor
#define BATTERY_VOLTAGE_MAX 4.2
#define BATTERY_VOLTAGE_MIN 3.0

// ==========================================
// SYSTEM CONSTANTS
// ==========================================
// Landscape Mode
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

#define SERIAL_DEBUG_BAUD 115200

// UI Colors (RaceBox / Dark Theme)
#define COLOR_BG 0x0000        // Black
#define COLOR_TEXT 0xFFFF      // White
#define COLOR_ACCENT 0x04DF    // Blue-Green (RaceBox Teal)
#define COLOR_HIGHLIGHT 0xF800 // Red
#define COLOR_SECONDARY 0x7BEF // Greyish

// ==========================================
// UI FONT CONFIGURATION
// ==========================================
// Font Sizes (1=Tiny, 2=Small, 3=Medium, 4=Large, 6=Huge)
#define FONT_SIZE_STATUS_BAR 1
#define FONT_SIZE_SPLASH_TEXT 1 // "ENGINE STARTING"
#define FONT_SIZE_MENU_TITLE 2
#define FONT_SIZE_MENU_ITEM 2
#define FONT_SIZE_LAP_SPEED 6 // Main Racing Speed
#define FONT_SIZE_LAP_TIME 2  // Current Lap Time
#define FONT_SIZE_LAP_LIST 2  // History List Text
#define FONT_SIZE_LAP_BEST 3  // Best Lap Time Display

#endif
