# Full-Build Parts List — VLF Metal Detector (through iron discrimination)

Scope: everything needed to finish **Sprints 1 + 2** — TX, null, preamp, I/Q synchronous demod, iron discrimination, pitch-tracking audio, OLED. Field hardware (shaft, housing, shielding) is Sprint 3 and **not** on this list.

**Vendor plan (your "mix/cheapest" choice):** ICs and the high-voltage film caps from **DigiKey or Mouser** (guaranteed genuine — counterfeit OPA2134s are common on Amazon). Everything under "Commodity hardware" is safe from **Amazon/AliExpress**.

---

## Already owned — NOT on this list
Arduino Nano ESP32 + USB cable · both coils · DMM · breadboard + jumpers · soldering iron + solder · resistor assortment kit · capacitor assortment kit · the 1 kΩ resistor and bench 0.1 µF electrolytic from the L-meter.

---

## Order 1 — DigiKey or Mouser (the "must be genuine / must be exact" parts)

| # | Part | Exact part number | Qty | ~Price | Used for |
|---|------|------------------|-----|--------|----------|
| 1 | TC4427 MOSFET driver, DIP-8 | **TC4427CPA** | 2 (1 spare) | $1.60 ea | TX driver |
| 2 | OPA2134 dual audio op-amp, DIP-8 | **OPA2134PA** | 1 | $3.75 | RX preamp + bandpass (both halves used) |
| 3 | NE5532 dual op-amp, DIP-8 | **NE5532P** | 4 (incl. spare) | $0.80 ea | Vgnd buffer, I/Q filters, audio/scaling stages |
| 4 | 74HC4053 triple analog switch, DIP-16 | **CD74HC4053E** | 2 (1 spare) | $0.75 ea | Synchronous demodulator (one chip does both I and Q) |
| 5 | 5 V regulator, TO-92 | **L78L05ACZ** | 2 (1 spare) | $0.50 ea | Clean 5 V analog rail |
| 6 | 100 nF film cap, **≥250 V** (MKP/MKT) | e.g. WIMA **MKP1G031004F00KSSD** (100 nF/400 V) or any 100 nF ≥250 V film | 3 | $1.20 ea | TX tuning cap + spares |
| 7 | Trim film caps, **≥250 V**: 2.2 nF, 4.7 nF, 10 nF | any film ≥250 V in those values | 2 ea | ~$0.60 ea | Fine-trimming the tank onto exactly 7.5 kHz |

⚠️ **The trim caps must ALSO be ≥250 V film.** They go in parallel with the tuning cap, so they see the same ~100 V peak. Do not trim with kit ceramics.

**Order 1 subtotal: ~$18**

---

## Order 2 — Amazon / AliExpress (commodity, authenticity doesn't matter)

| # | Item | Spec | Qty | ~Price | Used for |
|---|------|------|-----|--------|----------|
| 8 | AA battery holder **with leads** | **8×AA** (12 V) — see note below | 1 | $6 | Analog power |
| 9 | AA batteries | alkaline | 8 | $6 | — |
| 10 | Toggle or slide switch | SPST, panel/inline | 1 | $2 | Power switch |
| 11 | OLED display | **SSD1306, 0.96″, 128×64, I2C** (4-pin: VCC GND SCL SDA) | 1 | $4 | Discrimination readout |
| 12 | Passive piezo buzzer | **passive** (not active — it must accept a variable tone) | 2 | $2 | Pitch-tracking audio |
| 13 | 2N3904 NPN transistors | TO-92, any 10-pack | 1 pack | $2 | Buzzer driver (louder audio) |
| 14 | 10 kΩ potentiometers, linear | breadboard-friendly or panel mount w/ leads | 2 | $3 | Sensitivity + threshold knobs |
| 15 | Perfboard | ~7×9 cm, plated holes | 3 pcs | $8 | Permanent build after breadboard |
| 16 | DIP IC sockets | 8-pin ×4, 16-pin ×2 | 1 set | $3 | Never solder ICs directly |
| 17 | Solid-core hookup wire, 22 AWG | multi-color spool set | 1 | $8 | Perfboard wiring (jumpers don't solder well) |
| 18 | 2-pin screw terminal blocks, 5 mm | perfboard mount | 4 | $3 | Clean coil + battery connections |
| 19 | Electrolytic caps **(skip if your kit has them)** | 100 µF/25 V ×4, 10 µF/25 V ×4 | 8 | $2 | Rail bulk decoupling |

**Order 2 subtotal: ~$35–40 (less what you skip)**

### Battery holder note (the one deviation from earlier docs)
Earlier docs said 9×AA (13.5 V). **9-cell holders are genuinely hard to find** — 8-cell (12 V) holders are everywhere and 12 V changes nothing that matters: the TC4427 is rated to 18 V, and the tank current only drops from ~0.48 A to ~0.43 A peak (a field-strength difference you will never notice). **Buy the 8×AA holder.** If you happen to find a 9-cell, that's fine too — everything works 12–13.5 V.

---

## Coverage check — every stage → its parts

| Stage | Parts (list # / owned) |
|---|---|
| Power rails | 8, 9, 10, 5, 19, divider Rs from kit |
| Virtual ground (2.5 V) | ½ of one NE5532 (#3), 10k+10k from kit |
| TX driver + tank | 1, 6, 7, TX coil (owned) |
| RX preamp | 2 (½), Rs from kit, small caps from kit |
| Bandpass filter | 2 (other ½), RC from kit |
| Demod (I and Q) | 4, driven by ESP32 0°/90° pins |
| I/Q low-pass + gain | 3 (two chips' worth), RC from kit |
| ADC scaling into ESP32 | 3 (remaining half), Rs from kit |
| Audio | 12, 13, R from kit |
| Display | 11 (I2C: just 4 jumpers) |
| Controls | 14 |
| Permanent build | 15, 16, 17, 18 |
| Spares | one extra of every IC |

No stage is waiting on an unlisted part.

---

## Total damage: **~$55 all-in**, and the DigiKey order is the only one where exact part numbers matter.

### Ordering tips
- DigiKey/Mouser both have ~$5–7 shipping under ~$50 — order everything in Order 1 at once.
- If a WIMA cap is out of stock, the filter is: **100 nF, film (MKP or MKT), rated ≥250 V DC**. Brand is irrelevant.
- Buy the DIP (through-hole) package codes exactly as written — the same chips exist in surface-mount, which you can't breadboard.
