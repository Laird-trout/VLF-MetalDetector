# Build Log — VLF Induction-Balance Metal Detector
*Chronological record of what was done, what went wrong, what was learned. Raw material for the portfolio write-up.*

---

## Phase 0 — Design (before hardware)
- Chose detector type: **VLF induction-balance** with phase-sensitive (synchronous) demodulation, over BFO/PI alternatives.
- Locked architecture: 8″ Double-D coils, ESP32 as coherent clock source (TX + demod references from one crystal), TC4427 gate driver TX, OPA2134 preamp, 74HC4053 switching demod, single channel first → I/Q later.
- **Design correction logged:** original TX driver idea (2N3904/2N3906 pair) was wrong — resonant current is ~0.5 A, not the ~6–7 A first quoted, and a TC4427 handles it cleanly. Also: tuning cap must be film ≥250 V (tank hits ~100 V peak); electrolytics only for bench measurement.
- Roadmap defined: Sprint 1 (beep) → Sprint 2 (I/Q, discrimination, OLED) → Sprint 3 (shield, enclosure, field) → later Sprint 4 (on-device neural net).

## Phase 1 — Coils (Days 1–2)
- Wound two 8″ Double-D coils, ~100 turns each, 30 AWG magnet wire.
- **R measured (DMM):** both coils ≈ 18 Ω — matched, and matched the design model.
- **L measured** via self-built resonance method: ESP32 sweeps an L-C tank (coil ∥ "0.1 µF" electrolytic), finds peak, computes L = 1/((2πf₀)²·C). Result: **~4.3–4.6 mH both coils** (later revised — see Phase 3 lesson).
- Problems hit and fixed along the way:
  - `ledcAttach` compile error → Arduino ESP32 core 2.x vs 3.x API split → wrote version-proof firmware (`#if ESP_ARDUINO_VERSION_MAJOR`).
  - Confirmed Arduino's own Nano ESP32 board package is pinned to core 2.x; staying there (Espressif 3.x package has known Nano ESP32 quirks).
  - `dfu-util: No DFU capable USB device` upload failure → double-tap RESET → breathing LED → upload OK.
  - Serial Monitor baud confusion → moot: Nano ESP32 is native-USB, baud setting is cosmetic.
- **Verification habits established** (user-driven skepticism, kept as project culture):
  - Unplugged coil mid-run → still got plausible-looking ~6 mH → learned the sketch always prints *something*; mid-run changes give garbage timing.
  - Clean no-coil control run → predicted ~28 mH garbage at sweep floor → got exactly that → measurement confirmed real.
  - Metal near coil didn't move L reading → learned sweep resolution (100 Hz bins ≈ 2.7% L) and large-coil flux averaging hide small perturbations; amplitude is the more sensitive channel.

## Phase 2 — Parts (Order 1)
- Interactive BOM built; scoped to full detector; vendor split = ICs/film caps from DigiKey, commodity from Amazon.
- User did **Order 1 only** (~$34 incl. shipping): TC4427CPA ×2, OPA2134PA ×1, NE5532P ×4, CD74HC4053E ×2, L78L05ACZ ×2, WIMA MKP10 100 nF/400 V ×3, TDK B32529 trim caps (2.2/4.7/10 nF, 630 V) ×2 ea. Cart verified line-by-line before checkout.
- Part-number lessons: Kemet R76 suffixes I first gave weren't DigiKey-stocked variants → replaced with live TDK B32529 numbers; WIMA code letter (D vs F) encodes lead pitch.
- Power decisions: 8×AA (12 V) is the finished-detector answer (regulator headroom as cells sag); user's existing **6×AA (9 V) approved for all bench work**. Physics note logged: battery current ≈ 0.10–0.15 A average (tank recycles its circulating current; battery only replaces resistive losses).

## Phase 3 — Sprint 1 execution
### Stage 0 — Power rails ✅
- 9 V rail (6×AA), L78L05 → 5.0 V, 10k/10k + NE5532 buffer → 2.5 V Vgnd. All verified by DMM. No switch on hand — battery red lead is the switch.
- Learned: decoupling caps (104 ceramics) go *close to each chip*, not anywhere on the rail — wire inductance makes distant reservoirs useless for fast demands.

### M1 — Transmit ✅ (with a full debugging saga)
- TC4427 wired (channels paralleled), series tank = WIMA 100 nF + TX coil, Rsense = 4×100 Ω parallel (25 Ω) as tuning instrument, ADC-safe sense network to A0.
- **Bug found by user:** commands `f 7600 x` typed as one line — parser ate the `x`; TX never started. Fix: separate lines.
- **Phantom-power discovery:** entire first sweep (peak 7625 Hz) ran with battery unplugged — TC4427 was phantom-powered through its input protection diodes from D2. Reading looked plausible and even matched (wrong) prediction.
- **Instrument disagreement:** powered sweeps said ~8000s and wandered; phantom said 7625. Resolution: **DMM as referee** — manual AC-volts readings across Rsense at stepped frequencies. Result: 1.3 V @ 7600, 1.5 V @ 7900, 1.7 V @ 8300, flat 1.7 to 8500.
- **Verdict: true f₀ ≈ 8400 Hz. Operating frequency locked at 8400** (running at the tank's real resonance beats forcing 7.5 kHz).
- **Root cause of the 7.5 kHz miss:** original L was measured against a ±20% electrolytic reference; true coil L ≈ 3.5 mH, not 4.5. **Lesson: a measurement is only as good as its reference standard.**
- Corroborating physics observed: ~20 V AC on the tank node from a 9 V battery (resonant voltage magnification, predicted ~19 V RMS).
- Post-tune: Rsense shorted with jumper (full Q). Rule established: never sweep with Rsense shorted; re-tune in the same metal-clear bench state you test in.

### M2 — Null ✅
- Rough-null sense network (÷21) on A0; live meter (`n` mode).
- Technique: 3 degrees of freedom (overlap, rotation, coplanarity); nudge → hands away → read; pencil-trace then tape.
- **Result: loud = 4095 (ADC clipped) → floor ≈ 57 pre-tape, ≈ 100 after taping. ≥40× rejection. Null baseline = 100, written on the tape.**
- **First detection!** Phone / chunk of steel over the seam visibly moved the meter at rough-null stage. (Coin not yet visible — expected; needs preamp gain.)
- User theory ("residual induced fields decay slowly") tested against physics: coil time constant L/R ≈ 0.2 ms — the slow bar decay is measurement/display lag + hand-travel, not field memory. (Pulse-induction detectors do exploit real decay, at µs scales.)

### M3 — Preamp ✅ COMPLETE — first coin detection
**Resolution (read this before trusting any instrument again):** after eliminating capacitive coupling (foil test), ground-path coupling (pin-3 gag test), wrong resistors (ohmed them), broken DC (found & fixed a bad A0 bias mid-saga), and a flaky coil connection (18 Ω through the board), the root cause was **gain miscalibrated to the true null depth**: the DMM under-reads 8.4 kHz sines ~5–10×, so the null residual was really ~0.5–2 V, not "<100 mV" — gains 101 AND 11 were saturated at every coil position, which read as "constant 2500, nulling does nothing." **Fix: DMM-guided re-null (meter clipped directly on the unplugged coil — relative readings are valid even when absolutes aren't), then feedback dropped to 2.2 k (gain ≈ 3). Meter came alive, fine-null worked, coin detected.**
Key transferable lessons: (1) a saturated amplifier reads constant and position-independent — "nothing changes" often means "meter pegged," not "no signal"; (2) cheap DMM absolute AC values are only valid near 50–400 Hz — minima/comparisons survive, thresholds don't; (3) residual null signal never needs to be zero — it only must not saturate the front end (baseline-zeroing + demod handle the rest); (4) debug by elimination with one decisive test per suspect.

### (history of the M3 debugging arc, kept for the write-up)
- OPA2134 wired per walkthrough (gain ≈ 101, AC-coupled, all referenced to Vgnd; RX lead 1 moved from GND to Vgnd). Health checks passed (pin 1 ≈ 2.5 V DC).
- **Problem: floor is high and coil-independent** — removing the RX coil entirely didn't change the `n` reading. User caught this with a proper control experiment.
- **Diagnosis so far:**
  - Pulled D2 (TX drive dead) → floor collapsed → junk is TX-derived, entering *not* through the coil.
  - Moved preamp far from driver section → **no improvement** → not neighbor-to-neighbor board coupling.
  - Remaining suspects: (a) TX coil's field washing the whole board/leads, (b) **shared ground-path coupling** — tank return current (~0.4 A @ 8.4 kHz) flowing along the same ground rail the analog references hang from (ground bounce; explained in log as "ground is a topology problem, not a node").
- **Next actions (current state of play):**
  1. WIMA-unplug test (driver switching, no tank current/field) → splits suspect (a) from (b).
  2. If (a): more TX-coil distance from board + reroute RX leads. If (b): **star grounding** — tank return + TC4427 pin 3 land at the battery-entry point of the rail; bulk electrolytic at TC4427 VDD.
  3. Pragmatic arbiter regardless: coin over the seam — if it clearly beats the floor, M3 gate passes and the floor becomes a Sprint 3 (shield/perfboard) cleanup item.
- Second parts order placed/planned: resistor kit, passive piezo buzzer, 2N3904s, OLED, pots, 8×AA holder, perfboard/sockets/wire, foil tape.

### M4 — Synchronous demodulator ✅
- CD74HC4053 channel A wired: preamp → pin 12, Vgnd → pin 13, D3 (0° ref, same LEDC channel as TX = hardware phase-lock) → pin 11, demod out pin 14 → two-stage RC low-pass → ÷2 divider → A0.
- DC checkpoints on the RC ladder verified **against divider-loading math**: X = 1.88 V, Y = 1.25 V measured vs 1.875/1.25 predicted (2.5 V source dropping 0.625 V per 10 k of the 40 k chain). Walkthrough's original "≈2.5 V" note was wrong; measurements + math agreed and were adopted.
- **Result: demod baseline 682 ± 1 count** — the phase-selectivity of synchronous detection on display (raw `n` meter noise was ~100× worse). Off-682 offset = measured in-phase null residual.
- Feared 3.3 V-vs-3.5 V select-pin issue never materialized — ESP32 drives the 4053 select directly, fine.
- Preamp gain re-climbed 3 → **11** after a proper fine-null (the ladder working as designed). Coin = **±10 counts, 10:1 SNR**. Gate passed.

### M5 — Beep ✅ → 🏁 SPRINT 1 COMPLETE
- Passive piezo on D5. `b` mode: auto-zeroed baseline, pitch ∝ |deviation|.
- **Sensitivity baseline recorded: coin ≈ 2 cm flat-on** (gain 11, unshielded, pre-averaging). Coin angle-dependence understood (flat-on couples, edge-on near-invisible — flux geometry, not fault).
- Free sensitivity upgrades applied: b-mode averaging 800→3000 samples, THRESH lowered.
- Range expectations calibrated honestly: signal ∝ 1/r⁶ for small targets; realistic end-state ~10–15 cm on a coin after Sprint 3 (shields → gain 101, 12 V, tight layout). Current 2–6 cm is on-script for this stage.

## Sprint 2 (in progress)
- **Kit built:** `Sprint2_Discrimination_Guide.md` + `Sprint2_console.ino` — wakes 4053 channel B as the Q demodulator, D4 carries a **hardware 90° reference** (same LEDC timer as TX, quarter-period `hpoint` offset — phase-locked in silicon), Q ladder → A1, new modes: `q` (I/Q + angle stream), `z` (baseline zero), `v` (verdict: IRON low-buzz / GOOD high-beep, optional SSD1306 OLED).
- **Channel B bring-up debugging** (self-served with the checkpoint method): pin 15 read 0.13 V → traced backwards → pin 2's jumper to preamp-out not landed (pin 2 read 0.13 V, pin 1 read 2.5 V) → reseated → healthy. A1 divider read 1.0 V instead of 0.63 V → suspected 22 k in the 10 k bottom slot (matched the math) — matched dividers matter because atan2(ΔQ, ΔI) assumes equal I/Q scaling.
- **Drift-tracking saga:** constant false beeping traced to baseline drift vs snapshot zero. Added motion-mode tracking → first version tracked at ~5 counts/s and **absorbed coin sweeps as fast as they arrived** (no beeps at all) → slowed to ~0.6/s in `b` (1 count per 8 quiet loops) and 0.005/loop in `v`. Lesson logged: *an adaptive baseline is a race — tune the tracker slower than the slowest real sweep, faster than the fastest real drift.* Detector is now a "motion mode" machine like the commercial ones, including their park-it-and-it-vanishes behavior.
- **Next in sprint:** calibration session — build the empirical phase map (object → angle table), set FE_MIN/FE_MAX iron window, pass the 8-of-10 nail-vs-coin gate, OLED.

---

## Standing decisions & numbers (single source of truth)
| Item | Value |
|---|---|
| Operating frequency | **8400 Hz** (locked; tank's true f₀) |
| Coil R / true L | ~18 Ω / **≈3.5 mH** each (matched) |
| Tuning cap | WIMA MKP10 100 nF/400 V (trims on hand: 2.2/4.7/10 nF 630 V) |
| Preamp gain | **11** (10 k feedback; ladder-climb headroom depends on null depth; 101 returns after Sprint 3 shields) |
| Demod baseline (I) | **682 ± 1 count** |
| Coin signal | **±10 counts** @ ~2 cm flat-on (10:1 SNR) — official sensitivity baseline |
| Bench power | 6×AA ≈ 9 V (8×AA/12 V for final build) |
| Rails | 9 V → 78L05 → 5.0 V → 10k/10k+NE5532 → Vgnd 2.5 V |
| Pin map | D2 TX · D3 0° ref · **D4 90° ref** · D5 buzzer · A0 I-chan · **A1 Q-chan** · A4/A5 I2C (OLED) |
| Firmware | **`Sprint2_console.ino`** (t f x n d b s + q z v; drift-tracking baselines) — supersedes Sprint1_console |

## Lessons learned (running list — write-up gold)
1. A measurement is only as good as its reference (electrolytic ±20% → 900 Hz tuning miss).
2. Phantom powering exists: protection diodes can run a chip "off" its input signal.
3. Never trust a single instrument; arbitrate disagreements with an independent method (DMM vs ADC sweep).
4. Resonance frequency is environment-sensitive — tune in the state you test in; steel battery packs count as environment.
5. Control experiments (remove the sensor!) separate signal from artifact.
6. High-gain analog and switching power fight: distance, tight input wiring, and ground *topology* (star grounding) are the weapons.
7. Ground is not a node, it's a network — heavy return currents must not share a path with sensitive references.
8. Breadboard rules: decouple at the chip, hands off during readings, one change at a time, write baselines on the hardware in sharpie.
9. **A saturated amplifier reads constant** — "nothing changes when I adjust" usually means the meter is pegged, not that the signal is absent. Check for rails before theorizing.
10. **Cheap DMM absolutes are only valid ~50–400 Hz** — comparisons and minima survive at 8.4 kHz; thresholds computed from absolutes do not. (It also over-reads sharp-edged waveforms.)
11. **Gain is a ladder, not a dial:** each rung's null must fit under the next rung's saturation ceiling. Residual never needs to be zero — only small enough not to clip the front end.
12. **Verify DC operating points after every component change** — a 30-second DMM ritual that catches broken bias before it poisons hours of measurements.
13. **Predict node voltages, then measure them** (the divider-loading ladder: predicted 1.875/1.25, measured 1.88/1.25) — checkpoint debugging localizes faults in minutes (pin-15 → pin-2 trace).
14. **Adaptive baselines are a race:** tracker must be slower than the slowest real sweep, faster than the fastest drift. Tuned wrong in either direction = false beeps or no beeps.
15. Matched signal paths matter where math assumes symmetry — I/Q dividers must scale identically or atan2 angles warp.
