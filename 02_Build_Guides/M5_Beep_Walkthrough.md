# M5 Walkthrough — Beep (Sprint 1 Finale)

Going in: M4 passed (demod DC on A0, stable baseline, coin moves it ≥20 counts).
This milestone is 10 minutes of wiring and one command.

---

## Wiring — pick your loudness

**Option A — direct (quietest, zero extra parts):**
- Passive piezo buzzer: **+ lead → ESP32 D5**, **− lead → GND rail**. Done.
- Must be a **passive** piezo (needs to be *played* like a speaker). An "active" buzzer (one fixed self-generated tone) can't pitch-track.

**Option B — transistor boost (much louder, needs a 2N3904):**
- **D5 → 1 kΩ → base** (middle leg, flat face toward you).
- **Emitter (left leg, flat face toward you) → GND rail.**
- **Buzzer: + lead → 5 V rail, − lead → collector (right leg).**
- Optional: 100 Ω in series with the buzzer if it's obnoxious.

No buzzer on hand? A cheap wired earbud/headphone speaker works for a bench demo: tip → 100 Ω → D5, sleeve → GND. Quiet but real.

---

## Procedure
1. `f 8400` ↵, `x` ↵.
2. `b` ↵ — the firmware announces it's zeroing: **keep all metal (and yourself) away from the coils for the 3-second baseline capture.**
3. It prints the baseline, then beep mode is live:
   - silence while nothing's there
   - rising pitch as metal approaches the seam, falling as it leaves
4. Tune feel if needed (edit two constants at the bottom of the sketch):
   - `THRESH` (dead-band, default 6): raise it if it chirps at nothing; lower it for more sensitivity.
   - The `* 25` pitch scale: bigger = more dramatic pitch swing.
5. Re-zero anytime by typing `b` again (do it after any bench change).

**✅ GATE / SPRINT 1 COMPLETE:** coin sweep = clean rising-falling tone, silence otherwise, repeatable ten times out of ten.

---

## The victory lap (do not skip)
- **Video:** coin sweep with audible beep, one take, phone-quality is fine. This is the Sprint 1 money-shot for the portfolio.
- Test the zoo: coin, nail, pull-tab, foil, phone. Note (write down!) how far away each triggers — that's your sensitivity baseline for comparing after Sprint 2/3 improvements.
- Update the log: baseline numbers, f₀ (8400), null floor, detection ranges.

## Where you now stand
A working VLF induction-balance metal detector: hand-wound nulled coils, tuned resonant TX, low-noise preamp, synchronous demodulator, audio feedback. Everything after this is refinement: Sprint 2 = I/Q + discrimination + OLED, Sprint 3 = shielding/enclosure/field, Sprint 4 = the neural net. The physics is done; the rest is engineering.
