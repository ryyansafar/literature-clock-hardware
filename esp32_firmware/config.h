/**
 * config.h — Literature Clock NeoPixel Configuration
 *
 * Edit these values for your specific hardware setup.
 */

#ifndef CONFIG_H
#define CONFIG_H

// ---- WiFi ----
#define WIFI_SSID     "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// ---- NTP Time ----
#define NTP_SERVER    "pool.ntp.org"
#define GMT_OFFSET    19800        // IST = UTC+5:30 = 19800 seconds
#define DST_OFFSET    0

// ---- LED Hardware ----
#define LED_PIN       16           // GPIO pin for NeoPixel data line
#define LED_TYPE      WS2812B
#define COLOR_ORDER   GRB
#define BRIGHTNESS    40           // 0–255. Keep LOW to save power. 40 is plenty for text.

// ---- Grid Dimensions (must match physical layout) ----
// These match the simulator exactly.
#define CHAR_W        5            // bitmap font char width
#define CHAR_H        7            // bitmap font char height
#define CHAR_GAP_X    1            // horizontal gap between chars
#define CHAR_GAP_Y    2            // vertical gap between lines
#define COLS_CHARS    32           // characters per line
#define QUOTE_LINES   6            // lines for quote text
#define CELL_W        (CHAR_W + CHAR_GAP_X)   // 6
#define CELL_H        (CHAR_H + CHAR_GAP_Y)   // 9

#define GRID_W        (COLS_CHARS * CELL_W - CHAR_GAP_X)   // 191
#define QUOTE_H       (QUOTE_LINES * CELL_H - CHAR_GAP_Y)  // 52
#define ATTRIB_Y      (QUOTE_H + CHAR_GAP_Y)               // 54
#define ATTRIB_H      CHAR_H                                // 7
#define SPACER_PX     4
#define BRAND_Y       (ATTRIB_Y + ATTRIB_H + SPACER_PX)    // 65
#define BRAND_H       CHAR_H                                // 7
#define GRID_H        (BRAND_Y + BRAND_H)                   // 72

#define NUM_LEDS      (GRID_W * GRID_H)                     // 13752

// ---- Colors (CRGB values) ----
#define COLOR_WHITE   CRGB(190, 190, 180)
#define COLOR_BLUE    CRGB(30, 140, 255)
#define COLOR_DIM     CRGB(50, 55, 55)
#define COLOR_AMBER   CRGB(180, 120, 30)
#define COLOR_OFF     CRGB(0, 0, 0)

// ---- CSV file path on LittleFS ----
#define CSV_FILE      "/litclock_annotated.csv"

// ---- Update interval ----
#define CHECK_INTERVAL_MS 30000    // check time every 30 seconds

#endif
