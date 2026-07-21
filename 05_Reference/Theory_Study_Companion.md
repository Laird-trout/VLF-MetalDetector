# Metal Detector Theory — Study Companion & Project Snapshot

**How to use this:** attach or paste this at the start of a new chat dedicated to theory. It gives that conversation everything it needs to teach *your* detector, not a generic one. Work the syllabus top to bottom — each layer builds on the previous.

---

## Project snapshot (my real build — use these numbers in explanations)

- **Detector type:** VLF induction-balance (IB), phase-sensitive synchronous demodulation
- **Operating frequency:** 7.5 kHz
- **Coils (I wound these):** two 8″ Double-D, ~100 turns each, 30 AWG magnet wire
  - Measured: **R ≈ 18 Ω each, L ≈ 4.3–4.6 mH each** (matched pair)
  - L measured via resonance method: ESP32 swept an L-C tank (coil ∥ 0.1 µF), found the peak at ~7400 Hz, computed L = 1/((2πf₀)²·C)
- **TX tank:** series-resonant — coil + 100 nF ≥250 V film cap, driven by a TC4427 gate driver from a 12 V AA pack; ~0.4 A peak circulating, ~90–100 V peak across the cap
- **RX chain:** mechanical null (DD overlap) → OPA2134 preamp (~101× gain, AC-coupled, biased to 2.5 V virtual ground) → bandpass → 74HC4053 switching demodulator driven by 0° and 90° references from the ESP32 → RC low-pass → ESP32 ADC
- **Brain:** Arduino Nano ESP32 generates the TX square wave AND the demod references from one clock (coherent detection)
- **Power architecture:** 12 V to TX driver only; clean 5 V rail (78L05) for the analog chain; 2.5 V virtual ground; single shared 0 V
- **End goal:** working detector with pitch-tracking audio, OLED readout, and **iron discrimination** via I/Q phase angle

---

## Syllabus — in dependency order

### 1. Magnetic fields from current
Ampère's law in plain terms; why a loop concentrates field; why 100 turns ≈ 100× the field. Right-hand rule. What the field around my flat DD coil actually looks like in space.

### 2. Induction & inductance
Faraday's law and Lenz's law; self-inductance as "electrical inertia" (V = L·dI/dt); energy storage ½LI². Why L scales with turns *squared*. Why my 8″/100-turn coil lands at ~4.5 mH — walk the estimate formula and compare to my measurement. Mutual inductance M — the quantity my whole detector runs on.

### 3. AC circuit behavior & impedance
Reactance of L and C vs frequency; phasors; why at 7.5 kHz my coil's reactance (~212 Ω) dwarfs its 18 Ω resistance. Q factor — compute my tank's Q (~12) and what it means physically.

### 4. Resonance
Series vs parallel L-C tanks (my TX tank is SERIES; my L-meter was PARALLEL — why each choice). Voltage magnification: how 12 V becomes ~100 V across my cap. The mass-on-spring analogy made rigorous. Bandwidth vs Q.

### 5. Eddy currents & skin effect
What happens inside a conductor in an alternating field; why eddy currents oppose (Lenz again); loss vs field-distortion effects. Why frequency choice matters: what 7.5 kHz "sees" vs 15 kHz — target size, conductivity, ground penetration.

### 6. Ferrous vs non-ferrous response ← the discrimination core
Permeability (iron pulls field lines in, raising L) vs conductivity (eddy currents push field out, lowering effective L). Why iron shifts the received phase one way and copper/aluminum the other. The target "phase signature."

### 7. Induction balance & the Double-D geometry
Why two overlapped D coils can have M ≈ 0; what the null actually is; residual signal and why perfect null is impossible (and undesirable). What a target does to re-couple the coils.

### 8. The receive problem: tiny signals
Scale of the problem: µV target signals riding near a huge TX field. Preamp design choices — why AC coupling, why virtual ground, why gain ~100 and not ~10,000 in one stage. Noise sources and why the OPA2134 (or any low-noise op-amp) matters at the input and nowhere else.

### 9. Synchronous (phase-sensitive) demodulation ← the crown jewel
Why simple amplitude detection isn't enough. Multiplying by a reference: how it extracts the component at exactly 7.5 kHz and at a chosen phase, and rejects everything else. The 74HC4053 as a switching multiplier. Why the reference MUST share the TX clock (coherence). Low-pass filtering to DC.

### 10. I/Q and discrimination
0° and 90° channels as coordinates; atan2(Q, I) as the target's phase angle; where iron vs coins land on the I/Q plane. Ground mineralization as a phase vector, and how ground balance subtracts it. Why this is the same math as radio receivers and lock-in amplifiers.

### 11. Real-world limits
Ground effects, EMI at VLF (mains harmonics!), coil microphony, temperature drift of the null, why electrostatic (Faraday) shielding of coils helps and how it must NOT form a shorted turn.

---

## Standing instructions for the theory chat

- Tie every concept back to the snapshot numbers above; make me compute things with my own values.
- Quiz me — after each topic, ask me 2–3 questions and make me explain back in my own words before moving on.
- When I hand-wave, call it out. The goal is that I can defend every design decision in this build to an engineer.
- I've already built: the coils, an ESP32-based resonance-sweep inductance meter. I understand the basics of inductance, tanks, and the sweep program. Start at layer 1 anyway, but move fast until I slow down.
