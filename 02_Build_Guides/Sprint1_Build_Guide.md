# Sprint 1 Build Guide — "Make It Beep" (M1 → M5)

Everything on the breadboard, one milestone at a time. **Do not advance past a milestone until its verification gate passes.** Firmware for the whole sprint is one sketch: `Sprint1_console.ino` — you type single-letter commands in the Serial Monitor to switch between test modes.

**Safety note (the only one that matters):** once the TX tank is running, the junction **between the 100 nF cap and the coil** swings to ~±65–90 V. It won't hurt you badly (tiny energy), but it WILL instantly kill an ESP32 pin. **Never connect any ESP32 pin, or the preamp input, to the tank.** Every measurement in this guide goes through an ADC-safe sense network — build them exactly as written.

---

## ESP32 pin map for the whole sprint (lock this in now)

| Pin | Role |
|---|---|
| D2 | TX drive → TC4427 inputs |
| D3 | 0° demod reference → 74HC4053 select (same LEDC channel as D2 = phase-locked) |
| D5 | Buzzer |
| A0 | Multipurpose analog sense (tuning, null, preamp, demod — moves per milestone) |
| A4/A5 | Reserved — I2C for OLED in Sprint 2. Keep free. |

ESP32 stays on **USB power for all of Sprint 1**. ADC readable range is ~0–3.1 V.

---

## Stage 0 — Power rails (do this first, battery NOT connected to anything else yet)

1. Breadboard top rail = **VBAT** (from 6×AA +). Bottom rail = **GND** (6×AA −). Put your power switch in the + lead.
2. **5 V regulator:** L78L05 flat face toward you, legs down: **left = OUT, middle = GND, right = IN** (yes, output on the left — it's reversed vs. the big 7805; check the datasheet drawing on the DigiKey page if unsure). IN → VBAT, GND → GND rail, OUT → a free rail/row = your **+5 V rail**. 100 nF from IN→GND and OUT→GND, close to the chip. Add 10 µF electrolytic on OUT if you have one.
3. **Virtual ground (2.5 V):** two 10 kΩ in series from +5 V to GND; midpoint → NE5532 **+IN A (pin 3)**. Output **pin 1 → −IN A (pin 2)** (unity buffer). **Pin 8 → +5 V, pin 4 → GND.** Pin 1 = your **Vgnd** node. 100 nF from Vgnd → GND. Tie the unused half's +IN (pin 5) to Vgnd and −IN (pin 6) to its output (pin 7) — never leave op-amp inputs floating.
4. Connect **ESP32 GND → GND rail** (one jumper). Nothing else from the ESP32 yet.

**✅ GATE 0:** DMM says: VBAT ≈ 9 V, +5 V rail = 4.9–5.1 V, Vgnd = 2.45–2.55 V. All steady. If 5 V is missing, you've likely mirrored the 78L05 pinout.

---

## M1 — Transmit (TX driver + tank + tuning)

### Wiring
**TC4427** (dip in the trench, dot = pin 1):
- Pin 6 (VDD) → **VBAT** · Pin 3 (GND) → GND · 100 nF from pin 6→GND, close
- Pins 2 + 4 (IN A+B) tied together → **ESP32 D2**
- Pins 5 + 7 (OUT A+B) tied together = **DRIVE node**

**Tank:** DRIVE → **WIMA 100 nF** → COIL lead 1. COIL lead 2 → **Rsense 10 Ω** → GND.
(Rsense is temporary — it's your tuning instrument.)

**Sense network** (reads the coil current safely): from the COIL-2/Rsense junction → **100 nF (kit)** → **10 kΩ** → **A0**. At A0 also: **10 kΩ → 3.3 V** and **10 kΩ → GND** (bias divider centering A0 at ~1.65 V).

### Procedure
1. Upload `Sprint1_console.ino`. Open Serial Monitor.
2. Battery ON. Command **`t`** = tuning sweep: firmware sweeps 6.0–9.0 kHz and prints the amplitude curve + the peak (your tank's actual f₀).
3. If f₀ ≈ 7.2–7.4 kHz (likely, your L is mid-4s): resonance is *below* 7.5 k — you need *less* C, but you can't remove any… so instead we **accept f₀ and run there**: command **`f 7300`** sets the operating frequency to whatever the sweep found. *(Running at the tank's real f₀ beats forcing 7.5000 kHz — everything downstream derives from the ESP32 clock anyway.)* If f₀ is *above* 7.5 k, add trim caps in parallel with the WIMA (2.2/4.7/10 nF, they're rated for the tank) to pull it down onto 7.5 k.
4. Command **`x`** = steady transmit at the chosen frequency.

**✅ GATE M1:** sweep shows a clear single peak (amplitude at peak ≥ 3–4× the ends); DMM on AC volts across Rsense reads roughly 1–1.5 V RMS at resonance. The coil is transmitting. **Now short out Rsense with a jumper** (leave it in place for re-tuning later) — full Q for everything that follows. The sense network can stay.

---

## M2 — Rough null

RX coil comes in. TX keeps transmitting (`x` mode).

1. Overlap the two Ds roughly as designed (~10% overlap).
2. **Rough-null meter:** RX lead 1 → GND. RX lead 2 → **100 nF → 100 kΩ → A0** (same bias divider stays). This is ÷21-ish — safe even far off null.
3. Command **`n`** = live amplitude readout (prints a number + bar graph ~5×/sec).
4. Slide/rotate the RX coil in tiny increments. The number falls, bottoms out, rises again. Park at minimum. Tape the coils.

**✅ GATE M2:** a clear minimum exists and readings drop ≥10× from badly-overlapped to nulled. Near the bottom the meter may floor out at noise — that's expected; fine-nulling comes after the preamp exists. **Disconnect RX from the sense network.**

---

## M3 — Preamp (then fine-null)

**OPA2134** (pin 8 → +5 V, pin 4 → GND, 100 nF across, close):
- RX lead 1 → **Vgnd** (not GND!)
- RX lead 2 → **10 nF (TDK trim cap is fine, or kit) → +IN A (pin 3)**. Also **1 MΩ from pin 3 → Vgnd** (bias).
- Gain network: **1 kΩ from −IN A (pin 2) → one leg of 2.2 µF cap**, other cap leg → **Vgnd**. **100 kΩ from pin 2 → OUT A (pin 1)**. Gain ≈ 101 at 7.5 kHz.
- Tie unused half: pin 5 → Vgnd, pin 6 → pin 7.

**Preamp sense:** OUT A → **10 kΩ → A0**, plus **10 kΩ from A0 → GND** (÷2, centers ~1.25 V, keeps the 0–5 V swing inside ADC range). Remove the old 100k/100nF RX network from A0 first; the bias divider to 3.3 V comes off too — this divider replaces it.

1. TX on (`x`), command **`n`** again — now you're metering *through the preamp* (~100× more sensitive).
2. **Fine-null:** nudge the RX coil for minimum again. You'll find structure the rough null couldn't see. Park, tape firmly, ideally hot-glue.
3. Wave a coin over the coils — the number should jump visibly.

**✅ GATE M3:** metal within ~15–20 cm of the coil face moves the reading clearly and repeatably. Congratulations — physically, you already have a metal detector; the rest is making it convenient.

---

## M4 — One demod channel

**CD74HC4053E** (pin numbers per TI datasheet — verify against the diagram on page 1):
- Pin 16 (VCC) → **+5 V** · Pin 8 (GND) → GND · Pin 7 (VEE) → GND · Pin 6 (/INH) → GND · 100 nF across supply
- Channel A: **pin 12 (A0-in) → preamp OUT A** · **pin 13 (A1-in) → Vgnd** · **pin 14 (A-common) = demod output**
- **Pin 11 (select A) → ESP32 D3** · Tie unused selects (pins 9, 10) → GND, unused ins/commons → Vgnd
- **Low-pass:** pin 14 → **10 kΩ** → node X; node X → **1 µF (kit) → GND**; node X → **10 kΩ** → node Y; node Y → **1 µF → GND**. Node Y = clean DC.
- **ADC scaling:** node Y → **10 kΩ → A0**, **10 kΩ from A0 → GND** (reuse the M3 divider).

What's happening: D3 flips the switch at exactly the TX frequency (same LEDC channel = phase-locked). The switch alternately passes the preamp signal and Vgnd; the RC smooths that to a DC level proportional to the signal's in-phase component. Your first synchronous demodulator.

⚠️ *Known-risk item:* 74HC logic at 5 V officially wants ≥3.5 V for "high"; the ESP32 gives 3.3 V. In practice it almost always works. If channel A won't switch: add a 2N3904 (collector → 10 kΩ → 5 V, emitter → GND, base → 1 kΩ → D3, collector → pin 11). That inverts the reference — which just flips the sign of your readings; ignore or account in firmware.

1. Command **`d`** = streams the demod DC value.
2. Baseline should sit near mid-scale and be *stable* (±few counts). Bring a coin near: value walks off in one direction. Iron (nail/screwdriver): other direction, or at least a clearly different magnitude-vs-distance flavor. *(Full discrimination needs the Q channel — Sprint 2 — but you'll often see directionality already.)*

**✅ GATE M4:** stable baseline; coin vs. no-coin separated by ≥20 counts; response repeatable.

---

## M5 — Beep

- Buzzer + lead → **D5**, − lead → GND (passive piezo, direct drive — quiet but fine for the bench). Louder option: D5 → 1 kΩ → 2N3904 base; emitter → GND; buzzer from +5 V → collector.
- Command **`b`** = beep mode: firmware auto-zeros the baseline for 3 s (keep metal away), then maps |demod − baseline| → pitch (bigger signal = higher pitch), with a silence threshold.

**✅ GATE M5 = SPRINT 1 COMPLETE:** sweep a coin over the coils and it *sings*, pitch rising as the coin closes in, silent when nothing's there. Record a video. That's the portfolio money-shot for phase one.

---

## If something misbehaves — first checks, in order
1. **Rails:** 9 V / 5 V / 2.5 V still correct under load? (Weak batteries sag — check FIRST, always.)
2. **Grounds:** ESP32 GND tied to the rail? (No shared ground = nothing makes sense.)
3. **Decoupling:** every IC has its 100 nF, physically close?
4. **The tank moved:** re-run `t` — resonance shifts if coil leads moved or something metal sits near the coils. Keep the bench area clear of metal.
5. Off-board coils: keep both coils ≥30 cm from the breadboard/ESP32 — the TX field happily couples into breadboard wiring.
