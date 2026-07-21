# Sprint 2 — I/Q Discrimination Walkthrough (ferrous vs non-ferrous)

Going in: Sprint 1 complete. Demod channel A works (`d` mode stable, coin moves it). Gain at your sweet-spot rung. Coils taped at null.
New firmware for this sprint: **`Sprint2_console.ino`** (keeps every Sprint 1 command, adds the Q channel + discrimination modes).

---

## 0. The idea in four sentences
Your single demod channel measures the part of the RX signal **in phase** with the reference (call it **I**). A second, identical channel switched by a reference shifted **90°** measures the out-of-phase part (**Q**). A target's echo lands at an angle in the (I, Q) plane — `atan2(ΔQ, ΔI)` — and that angle is a *material signature*: iron (permeability-dominated) lands in one direction, non-ferrous conductors (eddy-dominated) in another. Measure the angle → name the metal.

Why this is nearly free: the 74HC4053 already contains three switches; you've used one. The ESP32 generates the 90° reference in hardware (LEDC `hpoint` — a phase-offset register), so both references stay phase-locked to the TX forever.

---

## 1. Hardware — three small jobs

### Job 1: wake up 4053 channel B (~10 min)
Currently parked. Rewire (battery off):
| Pin | Was | Becomes |
|---|---|---|
| 2 (B0 in) | → Vgnd | → **PREAMP OUT** (same node as pin 12) |
| 1 (B1 in) | → Vgnd | → **Vgnd** (already there — leave) |
| 15 (B common) | → Vgnd | → **Q ladder input** (below) |
| 10 (Select B) | → GND | → **ESP32 D4** |

Channel C (pins 3, 4, 5, 9) stays parked exactly as is.

### Job 2: the Q low-pass ladder (a copy of the I ladder)
From **pin 15**: → **10 kΩ** → row XQ → **1 µF → GND**; row XQ → **10 kΩ** → row YQ → **1 µF → GND**.
Then the ADC divider: row YQ → **10 kΩ → A1**, and **10 kΩ from A1 → GND**.
(Identical topology to the A0 chain. Expect the same DC ladder values: XQ ≈ 1.88 V, YQ ≈ 1.25 V, A1 ≈ 0.63 V.)

### Job 3 (optional but glorious): the OLED
SSD1306 0.96″ I2C: **VCC → 3.3V pin, GND → GND, SDA → A4, SCL → A5.** Four wires, no other parts.
In Arduino IDE: Library Manager → install **"Adafruit SSD1306"** (accept the GFX dependency). In the sketch set `#define USE_OLED 1`. If you skip the OLED for now, leave it 0 — everything works over serial.

---

## 2. Firmware — what's new in Sprint2_console.ino
- **D4 = 90° reference.** Same LEDC timer as TX/D3, offset a quarter-period via the `hpoint` register → hardware-exact, drift-free 90°.
- All Sprint 1 commands still work: `t f x n d b s`.
- New commands:
  - **`q`** — live stream: `I  Q  ΔI  ΔQ  mag  angle°`
  - **`z`** — zero: captures the no-target baseline (I₀, Q₀). Do this with metal away, hands away. Re-zero after any change.
  - **`v`** — verdict mode: when magnitude exceeds a threshold, prints the angle and a classification (`IRON` / `GOOD`), beeps low for iron, high for good, and drives the OLED if enabled.
- Classification boundaries live at the top of the sketch (`FE_MIN_DEG`, `FE_MAX_DEG`) — **you will set these from your own measurements** in step 4. The defaults are placeholders.

## 3. Bring-up sequence
1. Wire jobs 1–2. Battery on. DMM: XQ/YQ/A1 DC ladder values ≈ 1.88 / 1.25 / 0.63 V.
2. Upload `Sprint2_console.ino`. `f 8400` ↵, `x` ↵.
3. **`q`** ↵ — both channels stream. Check: I matches your old `d` values-ish; Q sits at its own stable baseline (different number — that's fine and meaningful: it's the residual's quadrature part). Both stable ±1–2.
4. **`z`** ↵ (everything away) — baselines captured. Now ΔI/ΔQ read ≈ 0, mag ≈ 0.

## 4. Calibration — the fun part (this is real lab work)
You're building your machine's *phase map* empirically:
1. `q` running, zeroed. Bring a **coin** flat-on to fixed close range. Read the streaming **angle** — it will cluster around some value (say, −40°). Write it down. Repeat at two distances — angle should stay put while mag changes (that's the whole magic: **angle = material, magnitude = size/distance**).
2. Repeat with: a **steel nail** (test 2–3 orientations), a screw, foil, another coin denomination, a pull-tab, your phone.
3. You'll get a little table: iron things clustered in one angular region, non-ferrous in another, probably ~90–180° apart. There will be some spread (nail orientation moves it) — that's normal.
4. Set `FE_MIN_DEG` / `FE_MAX_DEG` in the sketch to bracket the iron cluster with margin. Re-upload.
5. **`v`** ↵ — wave targets: nail → low buzz + `IRON`; coin → high beep + `GOOD`. That's discrimination.

Record the phase table in the build log — it's portfolio gold AND it's literally the beginning of the Sprint 4 dataset (those angles are what the neural net will learn to refine).

## 5. Gate
- Nail vs coin correctly classified **≥ 8 of 10 alternating passes** at your working range.
- Known honest limitations at this stage: very deep/edge-on targets misread (weak signal → noisy angle — the firmware refuses to classify below the mag threshold rather than guessing); some trash (bottle caps) reads GOOD — even commercial machines suffer that one.

## 6. Troubleshooting
- **Q baseline dead flat at ladder-idle value / never responds:** D4 → pin 10 jumper, and the same 3.3 V-vs-3.5 V select-pin caveat as channel A — if A needed no fix, B won't either; if B alone misbehaves, swap the select wiring D3↔D4 to test the pin, not the channel.
- **Angles wander wildly on strong targets:** you're saturating the preamp up close — back the target off or drop a gain rung.
- **Angle changes with distance (shouldn't):** re-zero (`z`); drifted baseline skews the vector math.
- **OLED blank:** address is usually 0x3C (set in sketch); check SDA/SCL not swapped.
