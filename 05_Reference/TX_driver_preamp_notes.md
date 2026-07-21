# TX Driver + RX Preamp — build notes, values & BOM

This is the schematic companion. It covers **M1 (transmit)** and the front end of **M3 (preamp)** on the roadmap. Build the TX side first, confirm the coil is ringing at 7.5 kHz, then add the preamp.

Your locked coil numbers: **TX & RX ≈ 18 Ω, ≈ 4.5 mH each, matched.** Tuning cap: **100 nF / ≥250 V film.**

---

## How it works (in one pass)

**Transmit.** The ESP32 outputs a 7.5 kHz square wave. That's a weak logic-level signal, so it goes into a **TC4427** gate driver, which is really just a beefy buffer: it takes the 3.3 V logic edge and slams a low-impedance 0–13.5 V square wave onto the tank. The tank is your **TX coil in series with the 100 nF cap**. At 7.5 kHz that series L-C is at resonance, so its impedance collapses to basically just the coil's 18 Ω — current peaks and the square wave's harmonics get filtered out, leaving a clean ~0.5 A sine wave circulating in the coil. That current is what makes the magnetic field the detector transmits.

**Why the cap must be film / ≥250 V.** At resonance the voltage across the cap (and across the coil) is the current times the reactance: ~0.5 A × ~210 Ω ≈ **~100 V peak**, even though the battery is only 13.5 V. That voltage magnification is normal for a resonant tank — it's also exactly why a little electrolytic would die here and a film cap rated ≥250 V is required.

**Receive.** The RX coil sits in the TX field but is mechanically nulled (overlapped) so most of the direct coupling cancels. What's left — the tiny imbalance plus anything a buried target reflects — is microvolts to millivolts. The **OPA2134** preamp lifts that by ~100× so the later demod stage has something to work with. Input is AC-coupled (10 nF) and biased to the 2.5 V virtual ground; gain is set by the 100 kΩ / 1 kΩ pair.

**Power split (the one integration decision, now locked).** The 13.5 V AA pack feeds **only** the TX driver. The op-amps get a **clean, separate 5 V rail** (78L05) with a **2.5 V virtual ground** so the AC signal can swing both directions. Everything shares **one 0 V ground** tied to the ESP32's USB ground. The reason the preamp can live on 5 V without worrying about the ESP32's 3.3 V ADC limit: the preamp output is AC and goes to the **demodulator**, not the ADC. Only the demod's slow DC output (next milestone) gets scaled into 0–3.3 V before touching the ADC.

---

## Component values

| Ref | Part | Value / notes |
|-----|------|---------------|
| U1 | TC4427 | dual MOSFET driver; tie both inputs together, both outputs together (parallel) for lower drive impedance |
| U2 | 78L05 | +5 V regulator; 0.33 µF on input, 0.1 µF on output |
| U3 | OPA2134 (½) | preamp. NE5532 or TL072 also fine — OPA2134 is quietest |
| U3 (other ½) or TL072 | Vgnd buffer | unity-gain buffer on a 10k/10k divider off +5 V → 2.5 V |
| C_tune | 100 nF | **≥250 V film** — series with TX coil |
| C_in | 10 nF | RX input AC-coupling |
| C_g | 2.2 µF | sets preamp DC gain to 1 (keeps offset down) |
| R_f | 100 kΩ | preamp feedback |
| R_g | 1 kΩ | preamp gain-set (gain ≈ 1 + Rf/Rg ≈ 101) |
| R_bias | 1 MΩ | RX input bias to Vgnd |
| Coils | your TX/RX | ~18 Ω, ~4.5 mH each |

---

## Build order

1. **Power first.** Wire the 9×AA pack, the 78L05 → 5 V, and the 2.5 V virtual-ground buffer. Confirm with a DMM: 13.5 V, 5.0 V, and 2.5 V, all referenced to the one ground.
2. **TX driver.** ESP32 7.5 kHz → TC4427 → 100 nF → TX coil → ground. Power it up.
3. **Preamp.** Build the OPA2134 stage last, once TX is confirmed.

## First-light test (TX)

- Load a sketch that outputs a steady **7.5 kHz** square on the TX pin (I'll give you the exact one — it's a two-line change from your L-meter sketch).
- With the driver running, the coil should be pulling ~0.5 A. **Fine-trim to 7.5 kHz:** if you have the sound-card scope, watch the coil sine peak; add a small cap in parallel with the 100 nF (a few nF at a time) to nudge resonance down onto 7.5 kHz, or trim up if it's low.
- **Heads up on battery drain:** ~0.5 A from AAs is real current — the pack will run warm and won't last for bench marathons. Fine for testing; for field use we can drop the drive or add a small series resistor to trade field strength for runtime.

## What comes after this

Once TX is confirmed ringing at 7.5 kHz and the preamp is amplifying, the next milestone is **M2 null** (slide the RX coil for minimum breakthrough) and then **M4 demod** (the 74HC4053 synchronous detector), where the ADC-scaling I mentioned finally comes into play.
