# M3 Walkthrough — RX Preamp (OPA2134), Fine Null, and the Coin Test

Slow-pace version. Battery unplugged while wiring. ESP32 stays on USB.
Reference state going in: M1 done (TX locked at **8400 Hz**, Rsense shorted), M2 done (coils nulled + taped, **baseline ≈ 100**).

---

## Part 1 — Wire the OPA2134 (~20 min, the delicate part)

**Pin map** (notch up; count down the left 1-4, up the right 5-8):

| Pin | Name | Connects to |
|---|---|---|
| 1 | OUT A | your new **PREAMP OUT** node |
| 2 | −IN A | gain network (below) |
| 3 | +IN A | RX signal in (below) |
| 4 | V− | GND rail |
| 5 | +IN B | Vgnd (parks unused half) |
| 6 | −IN B | jumper to pin 7 |
| 7 | OUT B | jumper to pin 6 |
| 8 | V+ | 5 V rail + **104 ceramic → GND, close** |

**Step by step:**

1. Plant the chip across the trench, notch up, a few rows clear of the NE5532.
2. Power first: pin 8 → 5 V rail. Pin 4 → GND rail. 104 ceramic from pin 8's row → GND, two-three holes away max.
3. Park the unused half: jumper pin 5 → Vgnd (the NE5532 pin-1 row). Jumper pin 6 → pin 7.
4. **Re-route the RX coil** (this changes M2's wiring):
   - RX lead 1: **move it from the GND rail to Vgnd.** The coil now "stands on" the 2.5 V platform.
   - RX lead 2 → **10 nF cap** (kit "103", or one of the blue TDK 10 nF trims) → **pin 3**.
   - **1 MΩ resistor** (brown-black-green) from pin 3 → Vgnd. (Gives the input its DC anchor.)
5. **Gain network:**
   - **1 kΩ** (brown-black-red) from pin 2 → a fresh row (call it **G**).
   - **2.2 µF cap** from G → Vgnd. (Electrolytic fine — both ends sit at 2.5 V DC, so polarity is uncritical; put + toward G by convention.)
   - **100 kΩ** (brown-black-yellow) from pin 2 → pin 1. (The feedback that sets gain ≈ 101.)
6. **Rewire A0** (retire the M2 rough-null network completely):
   - Remove: the 104+100 kΩ chain from RX lead 2, AND the old **10 k bias resistor to 3.3 V** at A0. (The 3.3 V bias era is over.)
   - Add: **pin 1 (PREAMP OUT) → 10 kΩ → A0**, and **10 kΩ from A0 → GND**. This halves the output swing and centers it ~1.25 V — inside ADC range.

**Wiring self-check before power:** pin 3 has exactly three things (cap from RX, 1 MΩ, nothing else). Pin 2 has exactly three (1 k, 100 k, nothing else). RX lead 1 is on Vgnd, NOT ground.

---

## Part 2 — Power-up health check

1. Battery in. DMM, DC volts: **pin 1 → GND should read ≈ 2.5 V** (the output resting on Vgnd).
   - Stuck at ~0 V or ~4.9 V = wiring error: check pin 2's network and that RX lead 1 really is on Vgnd.
2. DMM on A0 → GND: ≈ **1.25 V** DC. That's the divider doing its job.

---

## Part 3 — Fine null (the M2 dance, 100× sharper)

1. `f 8400` ↵, `x` ↵, then `n` ↵.
2. **Expect the meter to be loud or even pegged at first.** The preamp is magnifying your old "100" residual ×101 — it may be slamming its rails. This is normal and temporary.
3. Peel ONE coil's tape edge enough to allow millimeter nudges. Same rhythm as M2: nudge → hands away → read the number (not the hashtags).
4. Descend. As the null deepens the preamp comes off its rails and the numbers become meaningful — keep going until nudges in all directions make it worse.
5. Re-tape hard. Confirm the floor survived taping. **Write the new baseline number on the tape** next to the old one.

---

## Part 4 — GATE: the coin test

1. `n` running, hands away, note the resting number.
2. Sweep a coin flat over the **overlap seam** at ~10–15 cm height, at walking-pace.
3. **Pass = a clear, repeatable jump** on every pass — obvious against the resting flicker.
4. Also try: coin at 20 cm (weaker but visible?), a nail, pull-tab, your phone (should be huge now).

Record a video of the coin doing it. This is the "it's a real metal detector" clip.

**If the coin does nothing:** (a) confirm pin 1 sits at 2.5 V DC; (b) confirm TX is on (`x` after every `f`!); (c) your null may be TOO deep AND the residual phase unlucky — nudge the RX coil a half-millimeter off perfect null and retry; (d) check the 2.2 µF is actually in G→Vgnd (without it the gain collapses to 1).

---

## What M3 gives the next milestone
PREAMP OUT (pin 1) is a strong, clean, amplified copy of everything the RX coil hears, centered on Vgnd. M4's demodulator taps exactly that node — no rewiring of this stage needed again.
