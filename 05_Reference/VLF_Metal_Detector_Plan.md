# Building a VLF Metal Detector from Scratch — ESP32 Edition

**A staged, breadboard-to-field build plan + design questionnaire**
Author's note: this is a v1 plan with sensible defaults already chosen for you. Read it top to bottom, then jump to the **Questionnaire** at the end. Answer as many as you can (even "use your default" is a valid answer) and I'll lock the design, generate schematics, the BOM with links, coil-winding math, and the ESP32 firmware skeleton.

---

## 0. How to read this document

The hard truth first: a VLF (Very Low Frequency) detector is the *hardest* common detector topology to build, harder than a BFO or a pulse-induction (PI) unit. It is also the one real detectors use, the one that can discriminate iron from silver, and by far the most impressive thing to put in front of an internship recruiter. You picked the right project to stand out — we just need to stage it so you get a working signal early and don't drown.

The plan is built around **milestones**. Each milestone produces something you can *see on a scope* or *hear in a speaker*. You never spend three weeks soldering before knowing if anything works. If you stall at any milestone, that's the conversation to have with me — not "rebuild the whole thing."

---

## 1. How a VLF detector actually works (the 5-minute version)

A VLF detector is an **induction-balance (IB)** system. There are two coils in the search head:

- A **TX (transmit) coil** driven with a continuous sine wave at the operating frequency `f0` (think 5–15 kHz). It radiates a magnetic field.
- An **RX (receive) coil** positioned so that it picks up **almost zero** of the TX field directly. The coils are mechanically arranged (overlapped) so their mutual coupling cancels — this is "the null" or "the balance." Getting this null is the single most fiddly part of the whole build.

When a metal target enters the field, eddy currents in the metal create their *own* tiny magnetic field, which **unbalances** the coils and induces a small voltage in RX. That tiny voltage is the target.

The clever part is **phase**. The target's response has two components:

- A **reactive (X)** component — roughly "how magnetic / how much it looks like ground or iron."
- A **resistive (R)** component — roughly "how conductive it is" (a silver coin is very conductive; foil and gold less so).

We recover both by **synchronous (phase-sensitive) demodulation**: multiply the amplified RX signal by a reference at `f0` (gives the in-phase / R channel) and by a 90°-shifted reference (gives the quadrature / X channel). The *angle* `atan2(X, R)` is the target's phase signature.

Two huge payoffs fall out of that angle:

1. **Ground balance** — soil minerals (magnetite/ferrite) sit at a known phase. We rotate our reference phase to null them out so the dirt goes quiet.
2. **Discrimination / target ID** — iron, foil, nickel, gold, and silver land at different angles, so we can label what's under the coil.

That's the whole game: **transmit a tone, null the direct coupling, amplify the residual, demodulate it into R and X, and read the angle.** Everything below is engineering in service of those five steps.

---

## 2. Recommended default architecture

Here is the design I'd point you at unless your questionnaire answers push elsewhere. It's deliberately staged so the *coherent clock* (the thing that makes synchronous demod work) comes from the ESP32 itself.

```
  ESP32 ───┬─ generates f0 square wave ──► TX driver (push-pull) ──► series-resonant LC ──► TX coil
           │                                                                                  │ (magnetic field)
           ├─ generates 0° and 90° reference phases (same clock = coherent)                   ▼
           │                                                          [ search head: TX + RX nulled ]
           │                                                                                  │
           │                          RX coil ──► low-noise preamp ──► bandpass @ f0 ──────────┘
           │                                                          │
           ├─ 0° ref ─► I demodulator (AD630 / 74HC4053) ─► LPF ─► "R" DC ─► ESP32 ADC
           ├─ 90° ref ─► Q demodulator (AD630 / 74HC4053) ─► LPF ─► "X" DC ─► ESP32 ADC
           │
           └─ firmware: ground balance, motion filter, target-ID angle, audio tone (DAC→amp→speaker), OLED display
```

**The one architectural fork you must decide** (Q-block A in the questionnaire):

- **Path A — Analog demodulation, ESP32 reads slow DC (recommended for a first VLF build).** The phase-sensitive detection happens in hardware (AD630 balanced demodulators, or cheap 74HC4053 switching demods). The ESP32 only reads two slowly-varying DC voltages and does the "brains." Robust, forgiving of the ESP32's mediocre ADC, classic. **This is my default.**

- **Path B — Software-defined / DSP demodulation.** The ESP32 (or an external 24-bit I2S audio ADC like the PCM1808) samples the RX waveform fast and does the I/Q multiply *in software*. Fewer analog parts, infinitely tweakable, and *very* impressive in an interview ("I built a software-defined lock-in amplifier on a $5 micro"). But the ESP32's internal ADC is noisy and nonlinear, so doing it well basically forces the external ADC, and the timing/DSP is real work. Great as a **Milestone 4b upgrade** after Path A works.

My recommendation: **build Path A to a working detector, then bolt on Path B as the "wow" upgrade** if time allows. You get a guaranteed-working device *and* the impressive story.

---

## 3. Bill of materials (default architecture)

Specific parts named for clarity; alternatives in parentheses. Final linked BOM comes after your answers.

**Brain & power**
- ESP32 dev board — **ESP32-DevKitC / WROOM-32** (or ESP32-S3 if you want more DSP headroom for Path B)
- Clean analog supply: low-noise LDO(s), e.g., **MCP1700 / LP2985**, separate rail from the ESP32 digital 3V3
- Bipolar analog rail (±5–8 V) via charge-pump **ICL7660 / TC7660** or a small DC-DC, *or* single-supply-with-virtual-ground if we keep it simple
- Battery: 2× **18650 Li-ion** + protection/charger (TP4056 + boost), or a beefy USB power bank to start

**Transmit**
- TX driver: logic-level **push-pull** (e.g., transistor pair or a gate driver), or an H-bridge for more field
- Series-resonant tuning cap (film, low-loss, value computed from coil L and `f0`)

**Receive & demod**
- RX preamp: low-noise op-amp — **OPA2134 / NE5532** to start, **OPA1652 / LT1028 / AD797** for serious low noise
- Bandpass filter around `f0` (op-amp multiple-feedback, one or two op-amp sections)
- Demodulators: **2× AD630** (gold-standard balanced modulator/demodulator) *or* budget **74HC4053 / CD4066** analog switches + op-amp integrators
- Low-pass / integrator op-amps for the R and X channels (**TL072 / MCP6002** fine here)

**Output & UI**
- Audio: ESP32 DAC (or PWM) → **PAM8403** mini amp → small 8 Ω speaker
- Display: **SSD1306 0.96" OLED** (I2C) — target ID, signal bars, GB readout
- Controls: a couple of pots (sensitivity, GB, volume) and/or a rotary encoder + 2–3 buttons

**Coils & head**
- Magnet wire (e.g., **AWG 26–30 enamelled copper**) — exact gauge/turns computed from your chosen `f0` and coil diameter
- Coil formers (plywood/3D-printed jig to wind on)
- Electrostatic **Faraday shield** material (conductive paint, copper tape, or graphite) — **with a deliberate gap** so it isn't a shorted turn
- Non-ferrous hardware near the head: **plastic/nylon or brass screws only** (no steel near the coils)

**Mechanical**
- PVC pipe shaft + armrest, plastic search-head housing, project box for electronics

---

## 4. Test equipment & tooling (be honest about what you have)

A VLF build is *very* hard to debug blind. Strongly recommended, in priority order:

1. **Oscilloscope** — borderline mandatory. You need to *watch the null shrink* and *see the demod outputs move*. A cheap 2-channel USB scope or a school-lab scope is fine. (OSU likely has labs you can use — flag this.)
2. **Digital multimeter** — required, you have one or it's $15.
3. **Function generator** — nice to have; the ESP32 can stand in for most of it.
4. **LCR meter** — handy for measuring coil inductance, but we can measure L via resonance with the scope instead.
5. **Soldering iron, breadboard(s), jumper kit, helping hands** — required.
6. **LTspice (free)** — we'll simulate the preamp, filter, and demod *before* you solder. This is where a lot of the learning (and the internship-talkable depth) happens.

If you do **not** have scope access, tell me — it changes the plan (we lean harder on simulation, on the ESP32 sampling the waveform to act as a poor-man's scope, and on a more forgiving coil arrangement).

---

## 5. Coil design notes

- **TX coil:** tens to ~100+ turns, tuned with the series cap so the LC resonates at `f0` → big circulating current → strong field. Typical inductance a few hundred µH to a couple mH; exact numbers computed once you pick `f0` and diameter.
- **RX coil:** similar size; we *can* tune it to `f0` for more signal, but that adds phase shift we must account for — for a first build I'd keep RX lightly tuned or broadband to keep phase honest.
- **Geometry — pick one (Q-block C):**
  - **DD (Double-D):** two overlapping D-shapes. Easiest to null *and keep* nulled, stable, beginner-friendly. **My default.**
  - **Concentric (with a bucking/nulling winding):** RX inside TX plus a small reverse winding to cancel coupling. Classic narrow "cone" field, great pinpointing, but fussier to balance.
- **Nulling procedure:** energize TX, watch RX on the scope, and physically tweak coil overlap / a small "garbage" tuning loop until the residual is minimized, then fix everything in epoxy so it can't drift. We'll add a trim network for fine electrical nulling.
- **Faraday shield:** wrap each coil in a grounded electrostatic shield to kill capacitive/hand-capacitance effects — **leave a small gap** so the shield isn't one shorted turn (that would kill your TX).
- **Diameter trade-off:** bigger coil = more depth on big targets but less sensitivity to small ones and more ground noise; smaller = better on small/shallow targets and in trashy ground. ~8–10 inch is the classic all-rounder default.

---

## 6. ESP32 firmware architecture (Path A)

Roughly the software stack, smallest-to-largest:

1. **Clock/timer** produces `f0` on a GPIO (LEDC/MCPWM), plus the 0°/90° reference phases for the demods. Coherence guaranteed because it's all one clock.
2. **ADC sampling** of the two slow DC channels (R and X) — oversample + average to beat down the ESP32 ADC noise.
3. **Auto-null / drift tracking** — slowly recenter R/X so temperature drift doesn't trip the threshold (this is the "motion filter": a high-pass that ignores slow change, reacts to a target swept under the coil).
4. **Ground balance** — rotate the (R,X) vector by a stored angle so soil response → zero. Manual pot first, "grab" auto-GB later.
5. **Target processing** — `magnitude = hypot(R,X)`, `phase = atan2(X,R)`; map phase to a target-ID label and the magnitude to detection strength.
6. **Outputs** — threshold audio tone (pitch/volume tracks signal) via DAC→PAM8403→speaker; OLED shows ID + bars; optional BLE/Wi-Fi data logging (**keep the radio off while detecting** — it's a noise source).

*(Keep Wi-Fi/BT disabled during detection; star-ground the analog section; physically separate the noisy digital board from the preamp.)*

---

## 7. Staged build plan (the actual roadmap)

Each milestone has a **done-when** test. Don't move on until it's green.

- **M0 — Bench & theory (week 1).** Gather equipment, install LTspice, simulate the preamp + bandpass + one demod channel. *Done when:* you can explain the I/Q angle idea back to me and a sim shows a demod DC output that moves with input phase.
- **M1 — Transmit (week 1–2).** ESP32 generates `f0`; build push-pull driver + series-resonant tank on a test coil. *Done when:* scope shows a clean strong sine at `f0` across the TX coil at resonance.
- **M2 — Coils & the null (week 2–3).** Wind TX+RX, measure L, assemble the DD head, achieve the induction-balance null. *Done when:* RX residual on the scope is driven down to a small fraction of the un-nulled level and *stays* there.
- **M3 — Receive chain (week 3).** Low-noise preamp + bandpass. *Done when:* sweeping a coin/can near the head produces a visible RX bump on the scope.
- **M4 — Demodulation (week 4).** Build the I and Q demods + LPF. *Done when:* R and X DC outputs visibly swing when a target passes, and ferrous vs non-ferrous move them *differently*.
- **M5 — ESP32 brains (week 4–5).** Read R/X, threshold, audio tone, basic OLED. *Done when:* it beeps for metal. **This is a real working detector — celebrate here.**
- **M6 — Ground balance (week 5–6).** Null soil/ferrite. *Done when:* waving the coil over mineralized dirt stays quiet, but a coin still beeps.
- **M7 — Discrimination / ID (week 6–7).** Phase→target-ID, motion filter, tuned audio. *Done when:* it can tell iron from a coin with reasonable reliability.
- **M8 — Hardening (week 7).** Faraday shields, hum rejection, shielding, calibration, sensitivity tuning. *Done when:* stable and quiet, repeatable detection depth.
- **M9 — Mechanical + portfolio (week 8).** Shaft, housing, field test, and the **write-up/demo video** for internships. *Done when:* you've found something in real dirt and documented the whole thing.
- **(Optional) M4b/M5b — DSP upgrade.** Swap analog demod for software I/Q (external I2S ADC). The interview flex.

---

## 8. Likely pitfalls (so they don't surprise you)

- **The null drifts** with temperature/flex — that's *normal*; the motion filter and epoxy potting handle it.
- **50/60 Hz mains hum** swamps everything — bandpass at `f0`, shielding, and differential layout fix it.
- **ESP32 ADC is noisy** — oversample, or move to Path B's external ADC.
- **Shorted-turn shield** kills your TX — remember the gap in the Faraday shield.
- **Ferrous hardware near the coil** ruins the null — plastic/brass only.
- **Ground loops & digital noise** from the ESP32 leak into the preamp — separate supplies, star ground, radio off.
- **Microphonics** — coil wire movement makes noise; pot it solid.

---

## 9. Internship / portfolio framing

This project hits an unusually broad set of skills recruiters like: analog front-end design (low-noise amplification, filtering), RF-ish coil/resonance work, **synchronous detection / lock-in amplifier** theory, embedded firmware, DSP (if you do Path B), PCB/mechanical packaging, and rigorous test methodology. Keep a build log with scope photos at each milestone, your LTspice sims, and a short demo video — that documentation is worth as much as the device. We can turn the final write-up into a clean PDF/portfolio page.

---

## 10. THE QUESTIONNAIRE — narrow it all down

Answer what you can. For any you don't care about, just say "default" and I'll use the recommendation in **bold**. Grouped so you can skim.

### A. The big architectural fork
1. **Demod path:** Path A (analog demod, ESP32 reads DC) / Path B (software DSP) / **A first, then B as upgrade (recommended)**?
2. How important is the "software-defined lock-in amplifier" story for your internship pitch vs. just having a working detector? (shapes how hard we push Path B)
3. Are you comfortable doing real-time DSP/timing code, or do you want to keep firmware light? (**light to start**)

### B. Goals & constraints
4. **Primary goal:** learning/resume depth, a genuinely usable detector, or both equally? (**both, learning-weighted**)
5. What do you most want it to find: coins, jewelry/gold, relics, beach, or "doesn't matter, it's a demo"? (drives `f0`) (**all-rounder**)
6. **Budget** ceiling for the whole build? (rough band is fine: <$75 / $75–150 / $150–300 / $300+)
7. **Timeline:** how many weeks, and how many hours/week? (the plan assumes ~8 weeks; tell me your real number)
8. Hard deadline (internship application date, demo day)? 
9. Solo build or do you have lab/mentor/club support at OSU?

### C. The search head
10. **Coil geometry:** **DD (recommended)** / concentric / let me decide?
11. **Coil diameter:** ~6" (small targets/trashy) / **~8–10" all-rounder (default)** / bigger (depth)?
12. Wind your own coils (cheaper, more learning) or buy a ready-made coil to de-risk? (**wind your own**)
13. Do you want pinpoint accuracy (favors concentric) or stability/ease (favors DD)?

### D. Operating frequency
14. **Target `f0`:** ~5 kHz (deep/high-conductors) / **~7–8 kHz all-rounder (default)** / ~14 kHz (small gold/jewelry) / multi-frequency (advanced)?
15. Any interest in selectable/multi-frequency, or single fixed frequency to keep it sane? (**single fixed**)

### E. Electronics depth
16. **RX preamp:** budget (NE5532/OPA2134) or splurge low-noise (LT1028/AD797)? (**budget to start, upgrade later**)
17. **Demod hardware:** **AD630 (clean, ~$10–15 each ×2)** or 74HC4053 switching (cheap, more parts)?
18. **Power:** battery-portable from day one, or bench-powered while prototyping then battery later? (**bench first, battery at M8**)
19. Single-supply-with-virtual-ground (simpler) or true bipolar ±rail (cleaner)? (**bipolar via charge pump**)
20. ESP32 flavor: plain **WROOM-32 (default)** or **S3** (more DSP muscle for Path B)?

### F. User interface & features
21. **Display:** **SSD1306 OLED (default)** / larger color TFT / no display (audio only)?
22. **Audio:** simple threshold beep / VCO-style pitch-tracking tone (**default**) / multi-tone by target type?
23. Target-ID readout style: numeric ID, named categories (iron/foil/coin), or visual "target-ID screen"? (**named categories**)
24. Want **data logging / Bluetooth app / Wi-Fi dashboard**, or keep the radio off for noise reasons? (**off by default, optional stretch goal**)
25. Controls: pots (simple) vs rotary encoder + menu (slicker)? (**a few pots + encoder**)

### G. Mechanical & environment
26. Where will you actually swing it — parks/dirt, beach/wet sand (needs waterproofing), or just bench demos? (**bench + dirt**)
27. How polished must the final enclosure be — "works on a breadboard" / tidy project box / **field-ready waterproof-ish (default)** / show-piece?
28. 3D printer access for the coil formers and housing? (yes makes life easier)

### H. Your background & equipment (so I pitch the depth right)
29. Comfort with analog electronics: never / some coursework / solid? 
30. Comfort with embedded C / Arduino-ESP32 / ESP-IDF: none / Arduino-level / advanced?
31. **Do you have oscilloscope access?** (yes/your own/lab/no) — this genuinely changes the plan.
32. Do you have: DMM, function generator, LCR meter, soldering setup, breadboards? (list what's missing)
33. Have you built anything analog/RF before, or is this your first from-scratch analog project?

### I. How you want me to help
34. **Deliverables you want next:** schematics, a linked BOM with prices, coil-winding calculator/math, LTspice sim files, the ESP32 firmware skeleton, a weekly schedule, a parts-order list, a portfolio write-up template — which first? (rank them)
35. Want this whole plan as a polished **PDF** for your records/portfolio, or keep it as a living markdown doc I update as you answer? (**living doc, PDF at the end**)
36. Should I set up a **weekly check-in** that pings you with that week's milestone and a "what's blocking you?" prompt? (great for not stalling mid-summer)
37. Do you want me to keep everything in this **Metal detector** project folder as we go (build log, sims, firmware, BOM)? (**yes, recommended**)

---

## 11. Immediate next steps

1. You answer the questionnaire (even partially).
2. I lock the architecture and produce: schematic for M1–M4, the coil-winding numbers for your chosen `f0`/diameter, a priced BOM, and the M1 firmware to get `f0` out of the ESP32.
3. You order parts and knock out **M0 (sim) and M1 (transmit)** while they ship.
4. We iterate milestone by milestone, scope photo by scope photo, into your build log.

Tell me how much of the questionnaire you want to answer now — and if you'd rather I just pick smart defaults for *everything* and hand you a complete v2 plan you can edit, say the word and I'll do that instead.
