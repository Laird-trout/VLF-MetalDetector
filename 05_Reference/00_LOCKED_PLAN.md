# VLF Detector — LOCKED PLAN & Fast Path to First Beep

This supersedes the decision-making in the big plan (`VLF_Metal_Detector_Plan.md`). That document still holds all the theory and the milestone detail — read it for *why*. This one is the *what we're actually doing*, decided.

> **Assumptions I made for you** (tell me if any are wrong): moderate budget (~$120–150 total), you'll have **scope access via OSU**, you have basic soldering gear, and you're buying most parts new. Power for the analog section starts as **two 9 V batteries** (dead simple ±9 V) so you don't need a bench supply on day one.

---

## 1. Every decision, locked

| Area | Decision | Why |
|---|---|---|
| **Demod path** | **Path A (analog), single-channel first → add I/Q later** | Guaranteed first beep, then grow into discrimination. Software-DSP version is a later "wow" upgrade. |
| **Operating frequency** | **7.5 kHz** | Best all-rounder: decent on coins, jewelry, and small targets; tolerant ground behavior. |
| **Coil geometry** | **DD (Double-D), ~8″** | Easiest to null and keep nulled. The classic forgiving first build. |
| **Coils** | **Wind your own**, AWG 30 enamelled | Cheap, and the winding/nulling is half the learning. |
| **Demodulator chip** | **74HC4053** switching demod (start) → AD630 optional upgrade | $1 vs $15. Get beeping cheap; upgrade only if noise annoys you. |
| **RX preamp op-amp** | **OPA2134** (or NE5532 if that's what's on hand) | Low-noise, cheap, easy. |
| **Power** | **2× 9 V batteries → ±9 V** for analog; ESP32 from USB | Zero-fuss bipolar rail for prototyping. Battery pack + charge pump comes at hardening stage. |
| **MCU** | **ESP32 WROOM-32 devkit** | Plenty for Path A; S3 only needed if you later chase the DSP path hard. |
| **Display** | **SSD1306 0.96″ OLED (I²C)** | Cheap, tiny, shows signal + target ID. |
| **Audio** | ESP32 → **PAM8403** → 8 Ω speaker, pitch-tracking tone | Simple, satisfying feedback. |
| **Controls** | 3× 10 k pots: **Sensitivity, Threshold, Ground Balance** | All you need; encoder/menu is optional polish later. |
| **Radio** | **Wi-Fi/BT OFF during detection** | It's a noise source. Logging is a stretch goal only. |

---

## 2. The fast path: three sprints

The whole point is **Sprint 1 ends with a device that beeps at metal.** No discrimination, no ground balance yet — just a working all-metal VLF detector. Then we add brains.

### 🏁 Sprint 1 — "Make it beep" (the off-the-bat win)
Minimum viable detector: transmit, null, amplify, *single* synchronous detector, threshold beep.

1. **M1 — Transmit.** Flash the included firmware, get a clean 7.5 kHz square on the TX pin, build the push-pull driver + series-resonant cap, confirm a strong sine across the TX coil on the scope.
2. **M2 — Coils + null.** Wind TX & RX (~100 turns each on an 8″ form), measure L, assemble the DD head, and drive the RX residual down to a tiny fraction with the scope as your guide.
3. **M3 — Receive.** OPA2134 preamp + simple bandpass at 7.5 kHz. Wave a coin → see the bump on the scope.
4. **M4-min — One demod channel.** A single 74HC4053 synchronous detector + RC low-pass → one DC voltage that swings when metal passes.
5. **M5-min — Beep.** ESP32 reads that DC on its ADC, compares to the Threshold pot, drives a tone. **🎉 It beeps. You have a working detector.**

### 🧠 Sprint 2 — "Make it smart"
6. **M4-full — Add the 90° channel** → now you have R and X (I/Q).
7. **M6 — Ground balance** via the GB pot (rotate the I/Q vector to null soil).
8. **M7 — Discrimination + motion filter + OLED** showing iron / foil / coin and a signal bar.

### 🛠️ Sprint 3 — "Make it real"
9. **M8 — Hardening:** Faraday shields on the coils (remember the gap!), hum rejection, calibration, sensitivity tuning.
10. **M9 — Mechanical + portfolio:** PVC shaft, housing, field test, and the build-log write-up + demo video for internship applications.

### ⭐ Optional Sprint 4 — the interview flex
Swap the analog demod for a **software-defined I/Q lock-in** on the ESP32 (external I²S ADC). Only after Sprint 1–3 work.

---

## 3. Parts-order list (buy these now while the scope gets sorted)

Rough USD prices; quantities sized so a fried chip doesn't stop you.

**Core electronics**
- ESP32 WROOM-32 devkit ×1 — ~$8 *(grab 2 if budget allows; cheap insurance)*
- OPA2134 ×2 (or NE5532 ×3) — ~$8
- TL072 ×3 (filters/integrators, jellybean) — ~$4
- 74HC4053 ×3 — ~$3
- SSD1306 0.96″ OLED I²C ×1 — ~$4
- PAM8403 mini amp board ×1 + small 8 Ω speaker — ~$4
- 10 k linear pots ×3 + knobs — ~$5
- ICL7660/TC7660 charge pump ×2 (for later battery ±rail) — ~$3

**Coil + tuning**
- AWG 30 enamelled magnet wire, ~1 lb spool — ~$18 *(need ~130 m total; spool has plenty)*
- Film capacitor assortment incl. ~100 nF / 68 nF / 47 nF for resonating — ~$10 *(rate the resonating cap **≥250 V** — it sees ~100 V peak at resonance)*
- Copper tape (Faraday shield) — ~$7

**Passives & prototyping**
- Resistor kit (¼ W) — ~$8
- Ceramic + film cap kit — ~$10
- Solderless breadboards ×2 + jumper wire kit — ~$12
- 2× 9 V batteries + 2 clips (±9 V analog supply) — ~$6
- TX driver: **TC4427/TC4426 gate-driver IC ×2** (drives the tank directly; ~0.5 A peak, within its 1.5 A rating) — ~$4  *(corrected: small-signal 2N3904/2N3906 run too hot)*
- Perfboard, headers, hookup wire — ~$8

**Mechanical (can wait until Sprint 3)**
- PVC pipe + fittings (shaft), project box, **plastic/nylon or brass screws only** near the head — ~$20

**Ballpark total: ~$120–150.** I can cut it toward ~$80 (drop spares, NE5532 instead of OPA2134, skip the charge pump until later) or push toward a nicer build — say the word.

> Order priority if you want to stage spend: **ESP32 + breadboard + magnet wire + op-amps + 74HC4053 + caps/resistors first** (that's everything for Sprint 1). Display, amp, mechanical can come later.

---

## 4. Coil starting numbers (8″ DD, 7.5 kHz)

Wind both coils on a ~**8″ (203 mm)** circular former.

- **TX:** ~**100 turns** AWG 30 → expect ~**4–5 mH**.
- **RX:** ~**100 turns** AWG 30 → similar.
- **Resonating cap (TX):** after you *measure* the real inductance L, compute
  **C = 1 / ( (2π·7500)² · L )**.
  At L ≈ 4.5 mH that lands near **100 nF** — start there and trim with the cap assortment until the TX sine peaks at 7.5 kHz on the scope.
- Wire needed: ~63 m per coil (100 turns × ~0.63 m circumference) → ~130 m total, well within one spool.

We finalize exact turns after your first L measurement — air-core inductance predictions are only ballpark, so **wind, measure, tune.**

---

## 5. What to do this week

1. **Order the Sprint-1 parts** above.
2. **Confirm the scope** with OSU (and ask if their lab has a function generator + LCR meter you can use — bonus).
3. The moment your ESP32 lands, **flash `M1_transmit.ino`** (included) and scope the TX pin — that's M1 underway before anything else arrives.
4. Tell me when parts are on the way and I'll hand you the **M1–M4 schematic** (TX driver, preamp, bandpass, 4053 demod) and the **M5-min firmware** that turns the DC reading into a beep.

That's the plan. It's decided, it's cheap to start, and Sprint 1 gets you a real beeping VLF detector before you touch discrimination or ground balance.
