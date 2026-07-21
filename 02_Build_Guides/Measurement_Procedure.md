# Coil Measurement Procedure

Characterize **both** coils before building the TX tank. You're measuring three things: **R** (resistance), **L** (inductance), and then computing **C** (the tuning cap). Air-core coils have no magnetic core, so L doesn't change with frequency — a 1 kHz LCR reading is valid at your 7.5 kHz operating point.

**Tools:** DMM (for R), plus **either** an LCR meter (easy path for L) **or** scope + signal source + a known cap (resonance method for L).

---

## Part A — Resistance R (DMM) — do this for BOTH coils
1. Set the DMM to its lowest ohms range (e.g., 200 Ω).
2. Touch the two probes together and note the reading — that's your **lead resistance** (often 0.2–0.5 Ω). You'll subtract it.
3. Clip the probes to the coil's two **stripped/tinned** leads.
   - If you read **OL / infinite**, the enamel isn't fully removed — re-strip/tin the ends and retry.
4. Read the resistance, **subtract the lead resistance**. Expect ~**15–20 Ω**.
5. Record it as **R_TX** and **R_RX**. Metal nearby doesn't affect a DC resistance reading, so location doesn't matter here.

---

## Part B — Inductance L — do this for BOTH coils

### Option 1: LCR meter (easiest)
1. Disconnect the coil from everything.
2. Set the meter to inductance (L). Any test frequency (100 Hz or 1 kHz) is fine — air-core L is flat.
3. Hold the coil **in air, ~1 ft from any metal**, and don't wrap your hand around the windings.
4. Clip the leads, read **L**. Expect ~**3–6 mH**.
5. Record **L_TX** and **L_RX**.

### Option 2: Resonance method (scope + signal source + known cap)
Use this if there's no LCR meter. You need a **known** capacitor (e.g., a trusted 100 nF film), a signal source (lab function generator, or ask me for an ESP32 frequency-sweep sketch), a ~1 kΩ resistor, and the scope.
1. Wire the coil **in parallel** with the known cap `C_known` → that's a tank.
2. Drive the tank through the ~1 kΩ resistor from the function generator (sine). Put the scope probe **across the tank**.
3. Sweep the generator frequency up and down. The voltage across the tank **peaks** at the resonant frequency — note that frequency as **f_peak**.
4. Compute:  **L = 1 / ( (2π · f_peak)² · C_known )**
   - Example: `C_known` = 100 nF, peak at 7.5 kHz → L ≈ 4.5 mH.
5. Keep metal away and hands off the coil during the sweep. Record **L_TX** and **L_RX**.

---

## Part C — Compute the TX tuning capacitor
Using the **TX coil's** measured inductance:

**C = 1 / ( (2π × 7500)² × L_TX )**

- Example: L_TX = 4.5 mH → **C ≈ 100 nF**.
- Pick the nearest **film cap rated ≥250 V** (it sees ~100 V peak at resonance).
- You'll **fine-trim** this later: with the TX driver running, adjust the cap value (add/swap in parallel) until the coil's sine peaks exactly at 7.5 kHz on the scope.

---

## Part D — Record sheet
| Coil | R (Ω) | L (mH) |
|------|-------|--------|
| TX   |       |        |
| RX   |       |        |

- **Computed TX cap C:** ______ nF  → **chosen film cap:** ______ nF / ≥250 V
- **Sanity checks:** R and L should each be within ~10% between the two coils. R in the 15–20 Ω range, L in the 3–6 mH range. If something's wildly off (e.g., L = 0.5 mH or R = 200 Ω), tell me before proceeding — likely a miscount or a bad lead connection.

---

## What comes next (not now)
The **null test** — energizing TX and sliding the RX coil for minimum signal — needs the TX driver built, so it's the next milestone. Once you've got R, L, and the chosen cap, send me the numbers and I'll give you the TX driver + preamp schematic so you can power the coil and null it.
