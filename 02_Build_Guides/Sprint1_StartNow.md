# Sprint 1 — What You Can Start TODAY

You already own most of a Sprint 1 bench. Here's the inventory check, the short shopping list, the locked power plan, and a coil-winding guide you can begin right now.

---

## 1. Inventory check

| Item | Status |
|---|---|
| 2× breadboards | ✅ Covered |
| 2× ESP32 | ✅ Covered (one for the build, one spare — nice) |
| Resistors, caps, wire | ✅ Covered (see cap note below) |
| 30 AWG copper wire | ✅ Covered for coils — **confirm it's enamelled magnet wire** (or otherwise insulated). Bare copper will short turn-to-turn and won't work. |
| Power | ✅ Use the **9× AA holder** as the analog supply (see §3). LiPo can come later — tell me its voltage / cell count. |

### Cap note
For the **TX resonating capacitor** (the one tuning the coil to 7.5 kHz), use **film** or **C0G/NP0 ceramic** if you have them. Avoid high-K ceramics (X7R/Y5V) there — they're lossy and microphonic, which hurts a resonant tank. Regular ceramics are fine everywhere else. **Important:** at resonance the tank cap sees ~**100 V peak**, so rate it **≥250 V** (a 100 nF / 250 V polypropylene film cap is ideal). Your general-purpose 50 V caps are *not* safe there.

---

## 2. The short shopping list (everything else for Sprint 1)

Small, cheap order — this is all that's between you and a beeping detector:

- **Op-amps:** OPA2134 ×2 (preferred) **or** NE5532 ×3 — preamp, filter, integrators, and the virtual-ground buffer. ~$8
- **74HC4053 ×3** — the synchronous demodulator. ~$3
- **TX driver:** **TC4427** (or TC4426/TC4428) MOSFET gate-driver IC ×2 — drives the resonant tank directly. The tank only pulls ~0.5 A peak (the 18 Ω coil limits it), well inside the chip's 1.5 A rating. ~$4  *(Correction: NOT 2N3904/2N3906 — small-signal parts run too hot here.)*
- *(Optional but tidy)* **TLE2426 ×1** — one-chip "virtual ground" rail splitter. Saves you building it from an op-amp. ~$2

**Later (Sprint 2–3, not needed to beep):** SSD1306 OLED, PAM8403 + 8 Ω speaker, 3× 10 k pots, copper tape for Faraday shields, PVC/enclosure.

So the critical path is ~**$15** of chips. Order those and you can complete Sprint 1.

---

## 3. Power — LOCKED (adapted to your AA pack)

Forget the two-9V-battery idea from the earlier plan; your 9×AA holder is better. Plan:

- **Analog supply:** 9× AA in series ≈ **13.5 V** (alkaline) / ~10.8 V (NiMH). This single rail powers all the op-amps and the demod. TL072 / NE5532 / OPA2134 all handle it easily.
- **Virtual ground:** create a mid-rail "0 V" reference at ~½ the pack voltage so the op-amps can swing both ways. Use a **TLE2426** (cleanest), or two equal 10 k resistors from +V to ground with the midpoint **buffered by a spare op-amp** and a 10 µF bypass cap.
- **ESP32:** keep it on **USB** during development. Tie ESP32 GND to the battery-pack negative (the system 0 V).
- ⚠️ **Integration detail for M4/M5 (I'll handle it in the demod schematic):** the demodulator's DC output must arrive at the ESP32 ADC referenced to **ESP32 GND** and sitting **inside 0–3.3 V**. We design the final output stage around that — just flagging it so it's not a surprise.

---

## 4. Coil winding — DO THIS NOW (Milestone 2, the long-lead task)

Winding is hands-on and doesn't need any of the parts you're ordering, so start here while chips ship. You're making **two D-shaped coils** that sit flat-side-to-flat-side to form an ~8″ circle (Double-D).

**You need:** a former in a "D" shape whose straight edge ≈ **8″ (203 mm)**. Easiest options:
- Hammer a ring of small nails/screws into a board following a half-circle + straight edge outline, and wind inside them, **or**
- Wind around any rigid half-circle object (half of a paint-can lid, a printed ring, stiff cardboard cut to shape).

**Steps (repeat for both coils):**
1. Leave a ~30 cm lead, then wind **~100 turns** of the 30 AWG, keeping turns bunched neatly in a tight bundle (not spread out).
2. Leave another ~30 cm lead. You'll use ~63 m of wire per coil (~130 m total).
3. Bind the bundle every few cm with thin tape or small zip ties so it holds its shape.
4. Carefully slip it off the former and wrap the whole loop in tape/heat-shrink to lock the turns.
5. Scrape/sand the enamel off the very ends of each lead so you can make electrical contact.

**Then measure inductance** (don't skip — this sets your tuning cap):
- If the lab has an **LCR meter**, just read L. Expect a few mH (ballpark 3–6 mH for ~100 turns at 8″).
- No LCR meter? Measure it by **resonance**: put a known cap across the coil, drive it, and find the peak frequency on the scope — I'll give you that exact procedure when you're at the bench.

Once you know the real **L**, the TX tuning cap is
**C = 1 / ( (2π × 7500)² × L )** → around **100 nF** if L lands near 4.5 mH. We trim from there.

> DD is the locked choice (most stable, easiest to keep nulled). If you'd rather get a *bench* null even faster, two plain **circular** overlapping coils are simpler to wind and null on a table — but they don't make a practical swinging head, so you'd rewind for the real build. I'd just wind the two D's once.

---

## 5. So, right now you can:

1. **Order ~$15 of chips** (op-amps, 74HC4053, 2N3904/3906; optional TLE2426).
2. **Flash `M1_transmit.ino`** to an ESP32 and scope GPIO 25 → confirm the 7.5 kHz square. ✅ M1 firmware step done without buying anything.
3. **Wind both DD coils** with your 30 AWG wire and measure L.

When the chips arrive and your coils are wound, ping me — I'll deliver the **TX driver + preamp + bandpass + single-channel 4053 demod schematic** and the **Sprint-1 firmware** that turns the demod voltage into a beep. That's the whole rest of Sprint 1.
