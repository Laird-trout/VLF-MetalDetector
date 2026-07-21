# Shopping List — Have vs. Need (authoritative)

This is the single source of truth for parts. It folds in the corrected TX driver. Order the **Sprint 1** block to get a beeping detector; everything else can wait.

---

## ✅ You already have
- 2× breadboards
- 2× ESP32
- Resistors, general capacitors, hookup wire
- 30 AWG enamelled magnet wire (coils in progress)
- 9× AA battery holder (your analog supply) + LiPo packs (final-build option, TBD)
- Oscilloscope (borrowing from OSU)

That covers the brain, the prototyping surface, the coils, and power. Nice position to be in.

---

## 🛒 Sprint 1 — order this to get it beeping

| Part | Qty | Why | ~$ |
|---|---|---|---|
| **OPA2134** (or NE5532 ×3) | 2 | RX preamp + filter + integrators | 8 |
| **TL072** | 2–3 | jellybean op-amp for filter/integrator/virtual-ground buffer | 4 |
| **74HC4053** | 3 | synchronous demodulator (switching) | 3 |
| **TC4427** gate-driver IC *(or TC4426/4428)* | 2 | **TX driver** — drives the resonant tank directly | 4 |
| **100 nF film cap, ≥250 V** (polypropylene) + a couple nearby values (68 n / 120 n) to trim | ~3 | TX resonating cap — **sees ~100 V peak, must be high-voltage film** | 3 |
| **10 kΩ potentiometers** + knobs | 3 | Sensitivity, Threshold, Ground Balance | 5 |
| **PAM8403 mini amp + small 8 Ω speaker** *(or a $1 piezo buzzer for the first beep)* | 1 | audio output | 4 |
| *(optional)* **TLE2426** rail splitter | 1 | one-chip virtual ground (else build from a TL072 + 2 resistors) | 2 |

**Sprint 1 total: ~$25–30.** That's the whole critical path to a working detector.

> ⚠️ Two corrections baked in here:
> - **TX driver is a TC4427 gate driver, not 2N3904/2N3906.** The thin 30 AWG coil is ~18 Ω, so the tank only pulls ~0.5 A peak — but small-signal transistors still run too hot at that level. The gate driver handles it cleanly and runs straight off your AA rail.
> - **The resonating cap must be rated ≥250 V.** At resonance the voltage across the coil/cap rises to ~100 V peak (Q ≈ 12). Your everyday 50 V caps will fail there. Everywhere else in the circuit, your existing caps are fine.

---

## 🛒 Sprint 2–3 — order later (not needed to beep)

| Part | Qty | Why | ~$ |
|---|---|---|---|
| SSD1306 0.96″ OLED (I²C) | 1 | target-ID + signal display | 4 |
| Copper tape | 1 | Faraday shield on the coils (leave a gap!) | 7 |
| PVC pipe + fittings, project box | — | shaft + enclosure | 15 |
| **Nylon/brass screws only** near the head | — | no steel near the coils | 5 |
| ICL7660/TC7660 charge pump *(only if you want a bipolar rail from a single LiPo later)* | 1 | final-build power | 3 |

**Sprint 2–3 total: ~$35.**

---

## Honest bottom line
- **To first beep:** ~**$25–30**. That's it.
- **To a finished, housed, field-ready unit:** ~**$60–70** total beyond what you own.

You were right — it really isn't much. The expensive-sounding stuff (ESP32, scope, wire, breadboards, power) you already have or are borrowing.

---

## Your sequencing plan — verdict: 👍 solid
Finishing the coils and getting the scope **before** finalizing the driver is the correct order, and not just to save time:
- The exact driver and tank cap depend on the coil's **measured L and R**, which you can't know until the coils are wound and you can put them on the scope.
- Coil winding is the long-lead manual task — doing it now while parts ship is efficient.
- Nothing about the driver choice blocks coil work.

So: finish both coils → get the scope → measure L and R → *then* we lock the driver and cap exactly and I hand you the schematic. If the measured numbers differ from the ~18 Ω / ~4.5 mH estimate, we just adjust the cap value and confirm the TC4427 still has margin (it will, unless L comes out wildly different).
