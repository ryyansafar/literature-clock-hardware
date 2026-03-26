/**
 * Literature Clock — ESP32 NeoPixel Firmware
 *
 * Drives a WS2812B LED matrix displaying literary quotes with
 * the time phrase highlighted in blue and the rest in white.
 *
 * Hardware:
 *   - ESP32 DevKitC (or similar)
 *   - WS2812B LED strip/panels wired serpentine, data on GPIO16
 *   - 5V SMPS for LED power (inject every ~100 LEDs)
 *
 * Software dependencies (install via Arduino Library Manager):
 *   - FastLED
 *   - LittleFS (included in ESP32 Arduino core)
 *
 * Upload the CSV:
 *   Use the ESP32 LittleFS upload tool to flash litclock_annotated.csv
 *   to the LittleFS partition. In Arduino IDE: Tools → ESP32 Sketch Data Upload
 *   Place the CSV in a "data/" folder next to this .ino file.
 */

#include <FastLED.h>
#include <WiFi.h>
#include <time.h>
#include <LittleFS.h>
#include "config.h"
#include "font.h"

// ---- LED Array ----
CRGB leds[NUM_LEDS];

// ---- State ----
int lastMinute = -1;
String currentQuote = "";
String currentTimeText = "";
String currentTitle = "";
String currentAuthor = "";

// ======================================================================
// Serpentine XY → strip index mapping
// Even rows: left to right. Odd rows: right to left.
// This must match your physical wiring.
// ======================================================================
uint16_t xyToIndex(int x, int y) {
    if (x < 0 || x >= GRID_W || y < 0 || y >= GRID_H) return NUM_LEDS; // out of bounds sentinel
    if (y % 2 == 0) {
        return (uint16_t)(y * GRID_W + x);
    } else {
        return (uint16_t)(y * GRID_W + (GRID_W - 1 - x));
    }
}

// ======================================================================
// LED Buffer Operations
// ======================================================================
void clearAllLEDs() {
    fill_solid(leds, NUM_LEDS, COLOR_OFF);
}

void setLED(int x, int y, CRGB color) {
    uint16_t idx = xyToIndex(x, y);
    if (idx < NUM_LEDS) {
        leds[idx] = color;
    }
}

// ======================================================================
// Bitmap Font Rendering
// ======================================================================
void drawChar(char ch, int startX, int startY, CRGB color) {
    const uint8_t* bitmap = getCharBitmap(ch);
    if (!bitmap) return;

    for (int row = 0; row < CHAR_H; row++) {
        uint8_t rowBits = pgm_read_byte(&bitmap[row]);
        for (int col = 0; col < CHAR_W; col++) {
            if (rowBits & (1 << (CHAR_W - 1 - col))) {
                setLED(startX + col, startY + row, color);
            }
        }
    }
}

/**
 * Render a line of text at a fixed pixel Y position, optionally centered.
 */
void renderFixedLine(const char* text, int pixelY, CRGB color, bool centered) {
    int len = strlen(text);
    int startCharX = 0;
    if (centered) {
        startCharX = max(0, (COLS_CHARS - len) / 2);
    }
    for (int i = 0; i < len && (startCharX + i) < COLS_CHARS; i++) {
        drawChar(text[i], (startCharX + i) * CELL_W, pixelY, color);
    }
}

// ======================================================================
// Word-wrapped text rendering with color spans
// ======================================================================
struct TextSpan {
    String text;
    CRGB color;
};

void renderQuoteSpans(TextSpan* spans, int spanCount) {
    int cursorX = 0;  // character position
    int cursorY = 0;  // line number

    for (int s = 0; s < spanCount; s++) {
        const char* str = spans[s].text.c_str();
        CRGB color = spans[s].color;

        // Split into words by space
        String word = "";
        for (int i = 0; i <= (int)spans[s].text.length(); i++) {
            char c = (i < (int)spans[s].text.length()) ? str[i] : ' '; // treat end as space

            if (c == ' ' || i == (int)spans[s].text.length()) {
                if (word.length() > 0) {
                    // Check word wrap
                    if (cursorX > 0 && cursorX + (int)word.length() > COLS_CHARS) {
                        cursorX = 0;
                        cursorY++;
                        if (cursorY >= QUOTE_LINES) break;
                    }

                    // Draw each character
                    for (int ci = 0; ci < (int)word.length(); ci++) {
                        if (cursorX >= COLS_CHARS) {
                            cursorX = 0;
                            cursorY++;
                            if (cursorY >= QUOTE_LINES) break;
                        }
                        drawChar(word[ci], cursorX * CELL_W, cursorY * CELL_H, color);
                        cursorX++;
                    }
                    word = "";

                    if (cursorY >= QUOTE_LINES) break;

                    // Add space
                    if (c == ' ' && cursorX < COLS_CHARS) {
                        cursorX++;
                    }
                }
            } else {
                word += c;
            }
        }
        if (cursorY >= QUOTE_LINES) break;
    }
}

// ======================================================================
// CSV Parsing — scan file for matching time
// ======================================================================
/**
 * Clean HTML tags and entities from a string.
 */
String cleanHTML(String s) {
    // Replace <br> variants with space
    s.replace("<br/>", " ");
    s.replace("<br>", " ");
    s.replace("<br />", " ");
    // Strip remaining HTML tags
    String result = "";
    bool inTag = false;
    for (int i = 0; i < (int)s.length(); i++) {
        if (s[i] == '<') { inTag = true; continue; }
        if (s[i] == '>') { inTag = false; continue; }
        if (!inTag) result += s[i];
    }
    // Decode common entities
    result.replace("&amp;", "&");
    result.replace("&quot;", "\"");
    result.replace("&#39;", "'");
    result.replace("&lt;", "<");
    result.replace("&gt;", ">");
    return result;
}

/**
 * Scan the CSV file on LittleFS for all entries matching timeKey (e.g. "13:02").
 * Picks one at random and populates the global state variables.
 * Returns true if a quote was found.
 */
bool findQuoteForTime(const char* timeKey) {
    File file = LittleFS.open(CSV_FILE, "r");
    if (!file) {
        Serial.println("ERROR: Cannot open CSV file");
        return false;
    }

    // Collect matching line offsets for random selection
    int matchCount = 0;
    String chosenLine = "";

    while (file.available()) {
        String line = file.readStringUntil('\n');
        line.trim();
        if (line.length() == 0) continue;

        // Check if line starts with our time key
        if (line.startsWith(timeKey)) {
            // Verify it's an exact time match (next char should be '|')
            if (line.length() > 5 && line[5] == '|') {
                matchCount++;
                // Reservoir sampling: pick each match with probability 1/matchCount
                if (random(1, matchCount + 1) == 1) {
                    chosenLine = line;
                }
            }
        }
    }
    file.close();

    if (matchCount == 0) return false;

    // Parse the chosen line: time|timeText|quote|title|author|sfw
    int fieldIdx = 0;
    String fields[6];
    String current = "";
    for (int i = 0; i < (int)chosenLine.length(); i++) {
        if (chosenLine[i] == '|' && fieldIdx < 5) {
            fields[fieldIdx++] = current;
            current = "";
        } else {
            current += chosenLine[i];
        }
    }
    fields[fieldIdx] = current;

    currentTimeText = fields[1];
    currentQuote = cleanHTML(fields[2]);
    currentTitle = fields[3];
    currentAuthor = (fields[4].length() > 0) ? fields[4] : "Unknown";

    currentTimeText.trim();
    currentQuote.trim();
    currentTitle.trim();
    currentAuthor.trim();

    Serial.printf("Found %d quotes for %s. Selected: \"%s\" from %s\n",
                  matchCount, timeKey, currentTitle.c_str(), currentAuthor.c_str());

    return true;
}

// ======================================================================
// Display Logic
// ======================================================================
void displayCurrentQuote() {
    clearAllLEDs();

    // 1) Build color spans for the quote
    TextSpan spans[3];
    int spanCount = 0;

    if (currentTimeText.length() > 0) {
        String lowerQuote = currentQuote;
        String lowerTime = currentTimeText;
        lowerQuote.toLowerCase();
        lowerTime.toLowerCase();
        int idx = lowerQuote.indexOf(lowerTime);

        if (idx >= 0) {
            String before = currentQuote.substring(0, idx);
            String match = currentQuote.substring(idx, idx + currentTimeText.length());
            String after = currentQuote.substring(idx + currentTimeText.length());

            if (before.length() > 0) {
                spans[spanCount++] = { before, COLOR_WHITE };
            }
            spans[spanCount++] = { match, COLOR_BLUE };
            if (after.length() > 0) {
                spans[spanCount++] = { after, COLOR_WHITE };
            }
        } else {
            spans[spanCount++] = { currentQuote, COLOR_WHITE };
        }
    } else {
        spans[spanCount++] = { currentQuote, COLOR_WHITE };
    }

    renderQuoteSpans(spans, spanCount);

    // 2) Attribution line
    String attrib = "- " + currentAuthor + ", " + currentTitle;
    if ((int)attrib.length() > COLS_CHARS) {
        attrib = attrib.substring(0, COLS_CHARS - 2) + "..";
    }
    renderFixedLine(attrib.c_str(), ATTRIB_Y, COLOR_DIM, false);

    // 3) "LITERATURE CLOCK" branding
    renderFixedLine("LITERATURE CLOCK", BRAND_Y, COLOR_AMBER, true);

    FastLED.show();
}

// ======================================================================
// WiFi & NTP
// ======================================================================
void connectWiFi() {
    Serial.printf("Connecting to WiFi: %s", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 40) {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("\nConnected! IP: %s\n", WiFi.localIP().toString().c_str());
    } else {
        Serial.println("\nWiFi connection failed. Will retry later.");
    }
}

void syncTime() {
    configTime(GMT_OFFSET, DST_OFFSET, NTP_SERVER);
    Serial.println("Waiting for NTP time sync...");

    struct tm timeinfo;
    int attempts = 0;
    while (!getLocalTime(&timeinfo) && attempts < 10) {
        delay(1000);
        attempts++;
    }

    if (getLocalTime(&timeinfo)) {
        Serial.printf("Time synced: %02d:%02d:%02d\n",
                      timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    } else {
        Serial.println("NTP sync failed. Display may show wrong time.");
    }
}

// ======================================================================
// Setup & Loop
// ======================================================================
void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n=== Literature Clock — NeoPixel ===");

    // Initialize FastLED
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
    FastLED.setBrightness(BRIGHTNESS);
    clearAllLEDs();

    // Show boot message
    renderFixedLine("BOOTING...", 0, COLOR_BLUE, false);
    renderFixedLine("LITERATURE CLOCK", BRAND_Y, COLOR_AMBER, true);
    FastLED.show();

    // Mount LittleFS
    if (!LittleFS.begin(true)) {
        Serial.println("ERROR: LittleFS mount failed!");
        renderFixedLine("FS ERROR", 0, CRGB(255, 0, 0), false);
        FastLED.show();
        while (1) delay(1000);
    }
    Serial.println("LittleFS mounted.");

    // Check CSV exists
    if (!LittleFS.exists(CSV_FILE)) {
        Serial.println("ERROR: CSV file not found on LittleFS!");
        Serial.println("Upload litclock_annotated.csv to LittleFS 'data/' folder.");
        clearAllLEDs();
        renderFixedLine("NO CSV FILE", 0, CRGB(255, 0, 0), false);
        renderFixedLine("LITERATURE CLOCK", BRAND_Y, COLOR_AMBER, true);
        FastLED.show();
        while (1) delay(1000);
    }
    Serial.println("CSV file found.");

    // Connect WiFi and sync time
    renderFixedLine("WIFI...", 0, COLOR_BLUE, false);
    renderFixedLine("LITERATURE CLOCK", BRAND_Y, COLOR_AMBER, true);
    FastLED.show();

    connectWiFi();
    syncTime();

    // Seed random
    randomSeed(analogRead(0) + millis());

    Serial.println("Ready. Entering main loop.\n");
}

void loop() {
    struct tm timeinfo;

    if (!getLocalTime(&timeinfo)) {
        // Time not available — show error briefly
        delay(5000);
        return;
    }

    int currentMinute = timeinfo.tm_hour * 60 + timeinfo.tm_min;

    if (currentMinute != lastMinute) {
        lastMinute = currentMinute;

        char timeKey[6];
        snprintf(timeKey, sizeof(timeKey), "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
        Serial.printf("\n--- Time: %s ---\n", timeKey);

        if (findQuoteForTime(timeKey)) {
            displayCurrentQuote();
        } else {
            // No quote for this time
            clearAllLEDs();
            renderFixedLine(timeKey, 0, COLOR_BLUE, false);
            renderFixedLine("- no quote -", CELL_H + CHAR_GAP_Y, COLOR_DIM, true);
            renderFixedLine("LITERATURE CLOCK", BRAND_Y, COLOR_AMBER, true);
            FastLED.show();
            Serial.printf("No quote found for %s\n", timeKey);
        }
    }

    // Re-sync NTP every 6 hours
    static unsigned long lastNtpSync = 0;
    if (millis() - lastNtpSync > 6UL * 3600UL * 1000UL) {
        syncTime();
        lastNtpSync = millis();
    }

    delay(CHECK_INTERVAL_MS);
}
