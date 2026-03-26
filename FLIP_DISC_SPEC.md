# Flip Disc Literature Clock — Hardware Specification

## Display Technology: Flip Disc (Split-Flap Dot Matrix)

Each "pixel" is a small electromagnetic disc that flips between **yellow** (on) and **black** (off). No power needed to hold state — discs stay flipped until actively changed.

---

## Font & Character Grid

| Parameter | Value |
|---|---|
| Font size | **5 × 7** pixels (dots) per character |
| Character gap (horizontal) | 1 dot |
| Character gap (vertical) | 2 dots |
| **Cell size** (char + gaps) | **6 × 9** dots |
| **Discs per character cell** | **54** (6 × 9) |
| Max lit discs per character | 35 (5 × 7 active area) |
| Average lit discs per char | ~20 (varies by glyph) |

---

## Display Layout

The display has **3 content zones** stacked vertically:

| Zone | Content | Lines |
|---|---|---|
| **Quote** | Literary quote with time highlighted | Variable (see below) |
| **Attribution** | `— Author, Book Title` | 2 lines |
| **Brand** | `LITERATURE CLOCK` (right-aligned) | 1 line |

> [!NOTE]
> There is a 4-dot vertical spacer between the attribution and brand zones.

---

## Quote Analysis (3,626 quotes from CSV)

### Longest Quote
- **Time:** 09:00
- **Characters:** 675
- **Author:** Death in Venice — Thomas Mann
- **Preview:** *"Opening his window, Aschenbach thought he could smell the foul stench of the lagoon. A sudden despondency came over him…"*

### Longest Attribution
- **Text:** `— Narrative of a Journey Round the Dead Sea and in the Bible Lands in 1850 and 1851, Félicien de Saulcy`
- **Characters:** 103

### Quote Length Distribution (@ 32 chars/line)

| Percentile | Total Lines (quote + attrib + brand) |
|---|---|
| Median (50%) | 10 lines |
| 95th | 16 lines |
| 99th | 21 lines |
| **Maximum** | **25 lines** |

---

## Panel Size Options

### Option A — Fits ALL quotes (32 chars/line)

| Parameter | Value |
|---|---|
| Characters per line | 32 |
| Quote lines | 23 |
| Attribution lines | 2 |
| Brand line | 1 |
| **Total text lines** | **26** |
| Grid (dots) | **191 × 238** |
| **Total discs** | **45,458** |

#### Physical Size by Disc Diameter

| Disc Size | Pitch | Panel Width | Panel Height | Notes |
|---|---|---|---|---|
| **7 mm** | 7.5 mm | 143 cm | 179 cm | ✅ Wall-mountable |
| 10 mm | 11 mm | 210 cm | 262 cm | Large wall piece |
| 14 mm | 15 mm | 287 cm | 357 cm | ❌ Too big |

---

### Option B — Fits 99% of quotes (32 chars/line) ⭐ RECOMMENDED

| Parameter | Value |
|---|---|
| Characters per line | 32 |
| Quote lines | 18 |
| Attribution lines | 2 |
| Brand line | 1 |
| **Total text lines** | **21** |
| Grid (dots) | **191 × 193** |
| **Total discs** | **36,863** |

#### Physical Size by Disc Diameter

| Disc Size | Pitch | Panel Width | Panel Height | Notes |
|---|---|---|---|---|
| **7 mm** | 7.5 mm | 143 cm | 145 cm | ✅ Perfect square, wall art |
| 10 mm | 11 mm | 210 cm | 212 cm | ~2m square |
| 14 mm | 15 mm | 287 cm | 290 cm | ❌ Too big |

---

### Option C — Compact (28 chars/line, fits 95%)

| Parameter | Value |
|---|---|
| Characters per line | 28 |
| Quote lines | 15 |
| Attribution lines | 2 |
| Brand line | 1 |
| **Total text lines** | **18** |
| Grid (dots) | **167 × 166** |
| **Total discs** | **27,722** |

#### Physical Size by Disc Diameter

| Disc Size | Pitch | Panel Width | Panel Height | Notes |
|---|---|---|---|---|
| **7 mm** | 7.5 mm | 125 cm | 125 cm | ✅ Compact, ~4 ft square |
| 10 mm | 11 mm | 184 cm | 183 cm | ~6 ft square |
| 14 mm | 15 mm | 251 cm | 249 cm | Large |

---

## Character & Disc Count Breakdown

### Per-line breakdown (@ 32 chars/line, 191 dots wide)

| Element | Characters | Dots Wide | Dots Tall | Discs Used |
|---|---|---|---|---|
| One text line | 32 | 191 | 9 | 1,719 |
| Quote area (18 lines) | 576 max | 191 | 162 | 30,942 |
| Attribution (2 lines) | 64 max | 191 | 18 | 3,438 |
| Spacer | — | 191 | 4 | 764 |
| Brand ("LITERATURE CLOCK") | 14 | 191 | 7 | 1,337 |
| **Full panel** | — | **191** | **193** | **36,863** |

### "LITERATURE CLOCK" specifically
- 14 characters × ~20 avg lit dots = **~280 discs flipped** for the brand line

---

## Cost Estimate (Flip-disc module manufacturer 7mm modules)

| Item | Qty | Unit Cost | Total |
|---|---|---|---|
| Flip-disc module manufacturer 7×1 flip disc modules | ~5,300 | ~$2–3 | $10,600–$15,900 |
| Controller boards | ~20 | ~$25 | $500 |
| ESP32 controller | 1 | $8 | $8 |
| Power supply (5V, moderate) | 2 | $25 | $50 |
| Frame / mounting | 1 | ~$200 | $200 |
| **Estimated total** | | | **$11,400–$16,700** |

> [!IMPORTANT]
> Flip-disc module manufacturer modules are packaged as 7×1 or 28×1 strips.
> The 7mm disc is the smallest Flip-disc module manufacturer offers. For custom smaller discs, costs increase significantly.
> Consider bulk pricing from Flip-disc module manufacturer directly for orders >5,000 discs.

---

## Recommended Configuration

> [!TIP]
> **Option B with 7mm discs** gives a **143 × 145 cm** panel (~4.7 ft square) using **36,863 discs**, fitting 99% of all quotes. The 1% that overflow can be truncated gracefully.

```
Panel: 143 cm × 145 cm (nearly square)
Discs: 191 × 193 = 36,863 total
Font:  5×7 dot matrix, 32 characters per line
Lines: 18 quote + 2 attribution + 1 brand = 21 total
```
