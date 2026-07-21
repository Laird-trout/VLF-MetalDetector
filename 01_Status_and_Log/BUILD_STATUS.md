# Build Status & Index — read this to see the whole plan at a glance

This is the top-level map of the project. Everything else in the folder is a detail doc under one of these headings.

---

## Where you are right now
- ✅ **Coils wound** — two 8″ Double-D coils, ~100 turns each, 30 AWG, taped.
- ✅ **R measured** — TX ≈ **18 Ω**, RX ≈ **18 Ω** (matched).
- ✅ **L measured** — both coils **~4.3–4.6 mH** via the ESP32 `L_meter` sketch + 0.1 µF cap. Matched.
- ✅ **Tuning cap finalized** — **100 nF / ≥250 V film** (4.5 mH → 7.5 kHz). Fine-trim later.
- ✅ **Schematic delivered** — TX driver (TC4427) + RX preamp (OPA2134) + power/grounding. See `TX_driver_preamp_schematic.svg` + `TX_driver_preamp_notes.md`.
- ✅ **Parts in hand** — Order 1 (ICs + film caps) received; bench power is the 6×AA pack (9 V, fine for all of Sprint 1).
- ✅ **Stage 0 passed** — 9 V / 5 V / 2.5 V rails verified.
- ✅ **M1 TRANSMIT COMPLETE** — tank resonates at **f₀ ≈ 8400 Hz** (DMM-verified via Rsense current readings; flat peak 8300–8500). **Operating frequency locked: 8400 Hz** — all later stages (null, demod reference, tuning) use this, not 7.5 kHz.
  - Note: earlier 7.5 kHz target was based on L≈4.5 mH measured with a ±20% electrolytic reference cap; true L is ≈3.5 mH. Lesson logged: measurement is only as good as the known.
  - ~20 V AC observed on tank node at resonance (voltage magnification confirmed). Rsense cluster shorted after tuning.
- ✅ **M2 NULL COMPLETE** — rough null found and taped: loud = 4095 (clipped) → floor ≈ **100** (≥40× rejection). **Null baseline: 100** (written on the tape). Large-target detection (phone, steel) already observed at this stage.
- ✅ **M3 PREAMP COMPLETE — COIN DETECTED.** Preamp running at **gain ≈ 3 (2.2 k feedback)**, coils re-nulled via **DMM-across-the-coil method**, coin test passes.
  - The saga (full detail in BUILD_LOG): a week of "railed at 2500 everywhere" was ultimately gain miscalibration — the DMM under-reads sines at 8.4 kHz, so the null residual was ~5–10× bigger than its readings implied; gains 101 and 11 could never unclip. Along the way: fixed a broken A0 DC bias, eliminated capacitive-coupling and ground-path theories by experiment, verified resistors/DC/coil continuity.
  - **Rule now standing: this DMM's absolute AC readings at 8.4 kHz are untrustworthy (comparisons/minima are fine). Preamp gain must fit the actual null depth; more gain returns after Faraday shielding (Sprint 3).**
- ✅ **M4 DEMOD COMPLETE** — CD74HC4053 synchronous demod running. Baseline **682 ± 1** (superb stability — demod's phase-selectivity on display). RC-ladder DC points verified against divider math (X=1.88 V, Y=1.25 V — textbook). Preamp gain re-climbed to **11 (10 k feedback)** after fine-null; coin = **±10 counts, 10:1 SNR**, repeatable.
- ✅ **M5 BEEP COMPLETE — 🏁 SPRINT 1 DONE.** Buzzer beeps on coin. **Sensitivity baseline (record for all future comparisons): coin ≈ 2 cm, flat-on, at gain 11, unshielded.** Coin-angle dependence noted (flat-on couples, edge-on near-invisible — physics, not fault).
  - Free upgrades queued: `dcAvg(800)→(3000)` + `THRESH→2` in b-mode (≈2× sensitivity). Big unlock = Sprint 3 shields → deep null → gain 101 (~9× signal).
- 🔄 **SPRINT 2 UNDERWAY — discrimination kit delivered:** `Sprint2_Discrimination_Guide.md` + `Sprint2_console.ino`. Plan: wake 4053 channel B (pins 2/15/10), clone the RC ladder → A1, D4 = hardware 90° reference (LEDC hpoint), then `q`→`z`→calibrate phase map with real targets→set FE window→`v` verdict mode (+ optional OLED on A4/A5). Gate: nail vs coin ≥8/10.

Coil characterization is complete; the design is now precise because the numbers are in.

---

## Locked decisions (consistent across every doc)
| Area | Decision |
|---|---|
| Type | VLF induction-balance, phase-sensitive (synchronous) demod |
| Frequency | 7.5 kHz |
| Coils | 8″ Double-D, ~100 turns, 30 AWG, nulled by mechanical overlap |
| TX driver | **TC4427** gate driver (off the AA rail) |
| Demod | 74HC4053 switching demod, single channel first → I/Q later |
| Op-amps | OPA2134 / NE5532 / TL072 |
| Brain | ESP32, generates TX + demod reference from one coherent clock |
| Power | 9× AA (~13.5 V) analog + virtual ground; ESP32 on USB |
| Instrument | **Scope-free** — ESP32 does sweep/tuning + null-metering; free sound-card scope optional |
| Output | pitch-tracking audio + OLED (Sprint 2) |

---

## Corrections already folded in (nothing stale left behind)
1. **TX driver is NOT 2N3904/2N3906.** The real resonant current is ~0.5 A (the 18 Ω coil limits it, not the ~6–7 A that was wrongly quoted). A TC4427 gate driver handles it cleanly.
2. **Tuning cap must be film, ≥250 V** — the powered tank hits ~100 V peak. Electrolytics are fine *only* for bench L-measurement, never in the live tank.

---

## What is NOT yet locked (the honest part)
1. ~~The full schematic~~ — ✅ **TX driver + preamp + power now drawn.** Still to come as we reach them: bandpass, demod, and output scaling.
2. ~~Power/grounding decision~~ — ✅ **LOCKED:** 13.5 V feeds the TX driver only; signal chain on a clean 5 V rail with a 2.5 V virtual ground; single shared 0 V. The AC preamp feeds the demod, not the ADC — only the demod's DC output gets scaled to 0–3.3 V (at M4).
3. **Master pinout** — a single consolidated map of every ESP32 pin (TX, 0°/90° references, I/Q ADC inputs, pots, audio, OLED). I'll publish it as the demod stage lands so firmware and wiring agree.

---

## Roadmap (ordered)
**Sprint 1 — make it beep:** M1 transmit → M2 null → M3 preamp → M4 one demod channel → M5 beep.
**Sprint 2 — make it smart:** add 90° channel → ground balance → discrimination + OLED.
**Sprint 3 — make it real:** shielding, enclosure, field test, portfolio write-up.

You've finished coil characterization and are now entering **M1 (transmit)** — wire the power rails and TX driver, confirm the coil rings at 7.5 kHz. On track and in order.
