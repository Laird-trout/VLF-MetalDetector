# Sprint 4 — Neural-Network Target Classification ("actual AI" on the detector)

**How to use this doc:** this is a self-contained hand-off for a separate project/chat dedicated to the AI work. It contains everything that conversation needs to know about the underlying hardware project — no other context required.

---

## Project context (the machine this AI runs on)

**The project:** a homemade VLF induction-balance metal detector, designed and built from scratch (coils hand-wound, all circuitry from discrete parts). It is a working Arduino-family embedded system, not a kit.

**Hardware:**
- **Brain:** Arduino Nano ESP32 — this is an **ESP32-S3** (dual-core 240 MHz, vector instructions, ~512 kB RAM, USB-CDC serial). Programmed via Arduino IDE on the Arduino ESP32 core **2.x** (so use `ledcSetup`/`ledcAttachPin`-era APIs, not 3.x `ledcAttach`).
- **Coils:** two 8″ Double-D, hand-wound, ~100 turns 30 AWG. Measured: **R ≈ 18 Ω, L ≈ 4.3–4.6 mH each** (matched pair).
- **TX:** ESP32 generates 7.5 kHz square → TC4427 gate driver → series-resonant tank (coil + 100 nF ≥250 V film) → ~0.4 A clean sine in the coil.
- **RX:** mechanically nulled RX coil → OPA2134 preamp (~101×, AC-coupled, 2.5 V virtual ground) → bandpass → **74HC4053 synchronous demodulator** switched by 0°/90° references from the ESP32's own clock (coherent with TX) → RC low-pass → two slow DC voltages **I and Q** on ESP32 ADC pins.
- **Power:** battery (9–12 V) for TX driver + a clean 5 V analog rail; ESP32 on USB; single shared ground.

**What the signals mean (the AI's raw material):**
- Classical discrimination = a single phase angle atan2(Q, I): ferrous targets shift phase one direction, non-ferrous the other. Amplitude √(I²+Q²) = target size/proximity.
- The AI upgrade uses the full **I/Q trajectory over a ~1 s coil sweep** instead of one phase number.

**Project status at hand-off:** coils built and characterized; TX/preamp/demod schematic finalized; parts ordered; Sprints 1–2 (through working I/Q) are in progress in the main build. The AI work (this doc) is Sprint 4 — its firmware, dataset, and training pipeline can be designed and even prototyped with synthetic data before the analog build is done, but real data collection needs working I/Q.

**Builder profile:** learning-focused; owns every design decision; wants to genuinely understand the ML (not paste code) — explain choices, quiz understanding, flag pitfalls (data leakage, overfitting, quantization loss) before they happen.

---

Goal: replace the hand-tuned phase-cutoff discrimination with a learned classifier that identifies targets (coin / nail / foil / pull-tab / ring / trash) from the detector's raw signals — trained on a dataset you collect, running **on the detector itself**.

Prerequisite: Sprints 1–2 complete (working I/Q output on the ESP32 ADC). This plan changes no analog hardware.

---

## 1. The physics bridge: "time constants" → what YOUR machine measures

Your instinct is right and it's worth being precise about it. Every conductive target has a characteristic **eddy-current time constant τ = L/R** of the current loops induced in it (small foil: short τ; thick silver coin: long τ; iron: dominated by permeability instead, which flips the response direction).

A **pulse-induction** detector measures τ directly by watching the decay curve. Your **VLF machine measures τ's fingerprint a different way**: at a drive frequency ω, a target with time constant τ produces a received phase shift where **tan(φ) ≈ ωτ**. Your I/Q demodulator outputs exactly that φ (plus amplitude). So:

- **One frequency (7.5 kHz)** = one projection of the target's τ signature → you get one phase number. This is what classical discrimination uses.
- **The sweep trajectory** — how (I, Q) *evolves over time* as the coil passes over the target — adds shape, size, depth, and orientation information the single number throws away. **This is the free lunch your dataset should capture.**
- **Multiple frequencies** = multiple projections → you reconstruct much more of the τ distribution. This is exactly why high-end machines went multi-frequency.

**The realistic roadmap: Phase A uses the I/Q trajectory at 7.5 kHz (no hardware changes). Phase B adds a second frequency (one relay + one cap). Don't start at Phase B.**

---

## 2. What the network actually sees (feature design)

One "detection event" = the coil sweeping over a target ≈ 1 second of data.

- Sample I and Q at **200 Hz** each (trivial for the ESP32) → a **2 × 200 window** per event
- Normalize amplitude per-window (so the net learns *shape*, not just "strong = shallow"), but keep the raw peak amplitude as a separate scalar input — it carries depth/size info
- Optional engineered channels alongside the raw trace: instantaneous phase atan2(Q,I), amplitude envelope, dφ/dt

This trajectory is surprisingly rich: a nail's φ swings differently sweeping lengthwise vs crosswise; a bottle cap traces a different loop on the I/Q plane than a coin of the same phase; deep targets stretch flatter envelopes. That structure is precisely what a small NN exploits and a threshold can't.

---

## 3. Dataset — honest sizing and how to collect it

There is no public dataset for hobby VLF I/Q sweeps. **You will make the dataset, and that's a feature** — data collection and curation is the most portfolio-credible part of the whole project.

**Rig:** coil flat on a plastic table/floor grid. A "target stick": plastic ruler with the target taped on, so you can sweep at repeatable heights (5/10/15/20 cm) and speeds. NO metal shelving nearby.

**Firmware logging mode:** button (or serial command) starts a 2 s capture; ESP32 streams the I/Q window over USB as CSV with a label you type once per session. ~50 lines of code.

**Target zoo (start here):** several coins (different denominations/metals), iron nails (2 sizes), screws, aluminum foil (wad + flat), pull tabs, bottle caps, a brass fitting, a ring or two, and — critically — **"nothing" sweeps** over clean ground for the reject class.

**Realistic scale:** 12 target classes × 4 depths × 2 orientations × 2 speeds × 5 repetitions ≈ **~2,000 labeled sweeps ≈ a weekend or two.** That is genuinely enough for a small model with augmentation:
- time-stretch ±30% (sweep speed variation)
- amplitude scale ±50% (depth variation)
- additive noise from your real "nothing" captures
- random window shifts

**The trap that invalidates results (memorize this):** split train/validation **by session/day, never by individual sweep**. Two sweeps recorded a minute apart are near-duplicates; splitting them across train/val leaks and produces fake 99% accuracy. Collect on multiple days, hold out entire days.

---

## 4. Models — baseline first, then the NN

**Baseline (mandatory):** logistic regression / gradient boosting on ~8 engineered features (peak amplitude, mean phase, phase spread, envelope width, I/Q loop area…). Takes an afternoon in scikit-learn. If the NN can't beat this, the NN isn't earning its complexity — showing that comparison is exactly what impresses technical interviewers.

**The NN:** a small **1-D CNN** on the 2×200 window:
- 3× [Conv1D → ReLU → MaxPool] → GlobalAveragePool → Dense → softmax
- Target **≤ 30k parameters** — deliberately tiny
- Train in PyTorch or Keras on your laptop (minutes, no GPU needed)
- Outputs: class probabilities + a confidence you can threshold ("iffy → say nothing" beats wrong beeps)

A GRU/LSTM also fits the data shape, but the CNN quantizes better for the next step, so start there.

---

## 5. Deployment — real inference ON the detector

Key fact: the **Arduino Nano ESP32 is an ESP32-S3**, which has SIMD-ish vector instructions that Espressif's **ESP-NN** library uses to accelerate quantized networks, via **TensorFlow Lite Micro** (Arduino-installable as Chirale_TensorFlowLite or via ESP-IDF).

Pipeline: train (Keras) → convert to TFLite with **full int8 quantization** → `xxd` the model into a C array → TFLM interpreter in your sketch → feed it the same normalized window the training saw.

Budget check: a 30k-param int8 CNN on an S3 ≈ **a few ms per inference** and ~100–200 kB flash / ~50 kB RAM — comfortable. The detector still beeps instantly off amplitude (fast path); the NN classification labels the beep on the OLED a few ms later: `COIN 84%` / `IRON 97%` / `??`.

Fallback that is still "actual AI": stream I/Q to a laptop/phone and run inference there. Works, but on-device is more impressive and fully field-usable — do on-device.

---

## 6. Phase B (later): second frequency = second look at τ

Add one MOSFET/relay-switched extra capacitor across the tank so the ESP32 can retune between **7.5 kHz and ~15 kHz** (tank needs ~25 nF total for 15 kHz with your 4.5 mH — one more film cap). Alternate frequencies every half second; the feature window becomes 4×200 (I/Q at two frequencies); everything else in the pipeline is unchanged. Low-τ targets (foil, small gold) light up at 15 kHz; high-τ (thick coins) at 7.5 kHz — the NN gets a real spectral fingerprint. Only attempt after Phase A works end to end.

---

## 7. Order of operations

1. Finish Sprints 1–2 (through working I/Q + discrimination by phase cutoff) — **the NN needs the I/Q plumbing**
2. Firmware logging mode + collection rig
3. Collect v1 dataset (~2k sweeps, multiple days)
4. Baseline model → beat it with the 1-D CNN
5. Quantize → TFLite Micro on the S3 → OLED shows class + confidence
6. Field test; collect the failures; retrain (v2 dataset — this loop is the real ML engineering)
7. (Stretch) Phase B second frequency

---

## Why this is a killer portfolio piece

End-to-end: analog physics → signal processing → **self-collected labeled dataset** → baseline vs NN comparison → int8 quantization → **edge inference on a microcontroller** → field-test iteration loop. That's the entire modern edge-ML lifecycle demonstrated on hardware you wound with your own hands, and every claim in it is defensible because you measured it.
