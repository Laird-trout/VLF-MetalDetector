# M4 Walkthrough — Synchronous Demodulator (CD74HC4053E)

Turns the preamp's fast AC into one slow DC number that moves when metal is near.
Going in: M3 passed (preamp out = OPA2134 pin 1, resting at 2.5 V; coin test works on `n`).
Battery unplugged while wiring.

---

## What it does (one paragraph)
The 4053 is three electronic ON-OFF-ON switches. We use one: flipped by the ESP32's D3 reference — which is the *same clock* as the transmitter — it alternately passes the preamp signal and Vgnd. Folding the signal in rhythm with the TX like this makes its average land NOT at zero but at a DC level proportional to the received signal's in-phase component. Two RC stages then smooth that average into a lazy DC voltage the ADC can read at leisure. AC in, one slow number out.

---

## Pin map (CD74HC4053E, DIP-16, notch up; 1-8 down the left, 9-16 up the right)
Verify against the pin diagram on page 1 of the TI datasheet before wiring — one glance.

| Pin | Name | Connects to |
|---|---|---|
| 16 | VCC | 5 V rail + **104 → GND close** |
| 8 | GND | GND rail |
| 7 | VEE | GND rail (we're single-supply) |
| 6 | /INH (enable) | GND rail (always enabled) |
| 12 | A0 in | **PREAMP OUT** (OPA2134 pin 1) |
| 13 | A1 in | **Vgnd** |
| 14 | A common | **DEMOD OUT** → low-pass (below) |
| 11 | Select A | **ESP32 D3** (the 0° reference) |
| 9, 10 | Select B, C (unused) | GND rail |
| 1, 2, 15 | B/C ins & commons (unused) | tie each to **Vgnd** |
| 3, 4, 5 | B/C ins & commons (unused) | tie each to **Vgnd** |

## Wiring steps
1. Chip across the trench, notch up, clear of neighbors. Pin 16 → 5 V, pins 8 & 7 & 6 → GND, 104 ceramic at pin 16, close.
2. Signal path: jumper from OPA2134 pin 1 → **pin 12**. Jumper from Vgnd → **pin 13**.
3. Reference: jumper from **ESP32 D3** → **pin 11**.
4. Housekeeping: pins 9, 10 → GND. All six unused analog pins (1,2,3,4,5,15) → Vgnd.
5. **Low-pass filter** (two stages):
   - **Pin 14 → 10 kΩ → row X.** Row X → **1 µF (kit "105") → GND**.
   - **Row X → 10 kΩ → row Y.** Row Y → **1 µF → GND**.
6. **ADC feed:** re-use the M3 divider — but move its input: remove the jumper (OPA2134 pin 1 → 10 k → A0) input side and connect **row Y → that 10 kΩ → A0** instead. (The 10 k from A0 → GND stays.)

---

## Power-up + procedure
1. Battery in. DMM DC on row Y: some steady voltage near-ish 2.5 V (exact value depends on residual phase — steady is what matters).
2. `f 8400` ↵, `x` ↵, `d` ↵ — streams the demod DC value ~6×/sec.
3. **Baseline check:** the number should sit still (± a few counts). Write it down.
4. **Metal check:** coin sweep over the seam → the number walks away from baseline and returns. Iron (nail) → often walks the *other* direction (single channel can't discriminate reliably yet — that's Sprint 2 — but you'll frequently see directionality).

**✅ GATE:** stable baseline ± few counts; a coin pass moves it ≥ 20 counts, repeatably.

---

## Known-risk item (from the design notes)
74HC logic at 5 V officially wants ≥ 3.5 V for a "high"; the ESP32 gives 3.3 V. It almost always works anyway. If the demod output never moves and pin 11's signal is confirmed present:
- Add a 2N3904 level shifter: D3 → 1 kΩ → base; emitter → GND; collector → pin 11; 10 kΩ from collector → 5 V.
- This *inverts* the reference — your readings flip sign (baseline moves the other way). Harmless; just note it.

## Troubleshooting
- Baseline drifts continuously → weak batteries (check VBAT first), or a floating unused 4053 pin.
- No response to metal but `n` mode (A0 temporarily re-jumpered to preamp divider) still shows the coin → reference not switching: check D3 jumper, then the level-shifter fix.
- Reading jumps when YOU move without metal → your body near the coils; keep distance, ground yourself by touching the GND rail before tests.
