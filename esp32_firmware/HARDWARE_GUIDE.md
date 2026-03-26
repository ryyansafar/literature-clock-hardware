# Literature Clock — Hardware Layout & Wiring Guide

## LED Grid Specifications

Each letter is formed by turning **individual LEDs** on or off in a 5×7 pixel pattern.
There are no fonts — just a lookup table that maps each character to which LEDs to light up.

| Parameter | Value |
|-----------|-------|
| LED pattern per letter | 5 wide × 7 tall |
| Gap between letters | 1 LED column |
| Gap between lines | 2 LED rows |
| Letters per line | 32 |
| Quote lines | 6 |
| Attribution line | 1 (author + book) |
| Brand line | 1 ("LITERATURE CLOCK") |
| **Grid width** | **191 LEDs** |
| **Grid height** | **72 LEDs** |
| **Total LEDs** | **13,752** |

## Physical Size (by strip density)

| Strip Type | Width | Height | Cost Estimate |
|-----------|-------|--------|---------------|
| WS2812B 144/m | **1.33 m** | **0.50 m** | ~$200 in strips |
| WS2812B 60/m | 3.18 m | 1.20 m | ~$250 in strips |

> **Recommended: 144 LED/m** for a compact ~133cm × 50cm wall piece.

## Wiring — Serpentine Pattern

All LEDs are on a **single data line** from the ESP32. Rows are wired in a
zigzag (serpentine) pattern to minimize long wire runs:

```
DATA IN (GPIO 16)
    │
    ▼
Row  0: LED[0] ──► LED[1] ──► LED[2] ──► ... ──► LED[190]
                                                       │
Row  1: LED[381] ◄── LED[380] ◄── ... ◄── LED[192] ◄──┘
    │
Row  2: LED[382] ──► LED[383] ──► ... ──► LED[572]
                                               │
Row  3: LED[763] ◄── LED[762] ◄── ... ◄──────┘
    │
   ...continues for 72 rows...
    │
Row 71: LED[13751] ◄── ... ◄── LED[13561]
```

The ESP32 firmware converts (x, y) grid coordinates to strip index:
```
if row is even:  index = row × 191 + x        (left → right)
if row is odd:   index = row × 191 + (190 - x) (right → left)
```

## Power

| Scenario | Current Draw |
|----------|-------------|
| All white, full brightness | ~825A @ 5V (never do this!) |
| Text only, brightness=40 | ~8-12A @ 5V |
| **Recommended PSU** | **5V 20A SMPS** (with headroom) |

- **Inject 5V power** every 100 LEDs (every ~50cm of strip)
- Connect all GND rails together
- ESP32 runs on its own USB power or a 3.3V regulator from the 5V rail

## Wiring Diagram

```
                    ┌─────────────────────────────────┐
                    │         5V 20A SMPS              │
                    │   AC IN ──→  +5V / GND           │
                    └──────┬──────────┬────────────────┘
                           │          │
                      +5V  │     GND  │
                           │          │
    ┌──────────┐      ┌────┴──────────┴────────────────┐
    │  ESP32   │      │                                │
    │          │      │   WS2812B LED Matrix            │
    │  GPIO16 ─┼──►───┤   DATA IN                      │
    │          │      │                                │
    │  GND ────┼──────┤   GND (shared)                 │
    │          │      │                                │
    └──────────┘      │   +5V injected every ~100 LEDs │
                      └────────────────────────────────┘
```

## Mounting

1. Cut WS2812B 144/m strip into 72 pieces of 191 LEDs each (~1.33m)
2. Mount strips on a rigid board (MDF, acrylic, or aluminum composite)
   with 7mm spacing between rows (matching LED pitch)
3. Solder data pads: end of Row 0 → start of Row 1 (reversed), etc.
4. Solder 5V power injection wires every ~100 LEDs
5. Attach ESP32 at the back, connect GPIO16 to Row 0 data input

## Software Setup

1. Install [Arduino IDE](https://www.arduino.cc/en/software) or PlatformIO
2. Install **ESP32 board support** and **FastLED** library
3. Open `esp32_firmware/esp32_firmware.ino`
4. Edit `config.h`: set your WiFi SSID, password, and timezone offset
5. Create a `data/` folder next to the `.ino` file
6. Copy `litclock_annotated.csv` into `data/`
7. Upload filesystem: **Tools → ESP32 Sketch Data Upload** (installs the CSV to flash)
8. Upload sketch to ESP32
9. Open Serial Monitor at 115200 baud to verify boot sequence
