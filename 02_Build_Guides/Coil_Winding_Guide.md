# How to Wind the Search Coils (8″ Double-D, 30 AWG enamelled)

You're making **two identical coils**. One becomes TX, one becomes RX. Each is a "D" shape (a half-circle with a straight edge across the flat side). The two D's later sit flat-edge-to-flat-edge, overlapping, to form an 8″ circle. See `coil_winding_diagram.svg`.

---

## What you need
- Your **30 AWG enamelled** magnet wire (~52 m per coil, ~105 m total — confirm the spool has it).
- A **flat board** (plywood/MDF, ~12″×12″) as a winding jig.
- **~14–20 small nails or short wood screws** + hammer/screwdriver.
- Marker, ruler, and a **string compass** (pin + string + pen) to draw an 8″ circle.
- **Masking/electrical tape**, plus **zip ties or waxed thread** to bind the bundle.
- For the leads: fine sandpaper or a **soldering iron + solder** (to strip + tin the enamel).

---

## Step 1 — Lay out the D on the board
1. Mark a center point. With the string compass set to **4″ (101.6 mm)**, draw a full **8″ circle**.
2. Draw one **diameter line** straight across it. One half of the circle is your D: the **curved arc** + the **straight diameter edge**.
   *(Dimensions: each D is **8″ along the flat edge** and bulges **4″ out** at the apex — a true semicircle of radius 4″. Footprint 8″ × 4″. Two D's bulging opposite ways = an 8″ circle.)*

   **Per-turn wire length:** straight edge 8″ + arc (π × 4″ ≈ 12.6″) ≈ **20.6″ (0.52 m)** per turn → ~52 m for 100 turns.
3. Hammer nails **just outside** that D outline, spaced **~1.5 cm apart**, all along the arc and the straight edge. The nails form a fence the wire winds against. Leave them tall enough (~1.5–2 cm proud) to hold ~100 turns.

> The nail-jig method works for any shape and gives clean, repeatable coils. You'll reuse the same jig for the second D.

## Step 2 — Wind ~100 turns
1. Leave a **30 cm starting lead**, then anchor the wire to a corner nail or tape it to the board.
2. Wind **~100 turns** around the inside of the nail fence. Keep the wire **taut and the turns bunched** in a tight bundle — don't let them splay sideways. A loose, spread-out bundle changes the inductance and weakens the field.
3. **Count carefully.** Tick off every 10 turns on paper, or count in groups of ten out loud. Consistency between the two coils matters more than hitting exactly 100 — just make both the *same*.
4. Keep gentle, even tension. Don't kink or crush the wire — a cracked enamel coating creates a shorted turn that ruins the coil's Q. If it kinks, back up and re-lay it.
5. Finish with another **30 cm lead**.

## Step 3 — Bind it before removing
1. While still on the jig, wrap the bundle tightly with tape (or tie with zip ties / waxed thread) at **5–6 points** around the loop. This is critical — the moment you free it from the nails it wants to spring apart.
2. Lift the coil off the nails. It should hold its D shape as one stiff "rope."
3. Wrap the entire loop in tape or heat-shrink to protect it.

## Step 4 — Prepare the leads
1. On the last **~1 cm** of each lead, remove the enamel: scrape it with sandpaper/a knife, **or** touch it with a hot soldering iron loaded with solder — the heat burns the enamel and the lead **tins** (turns silver). Tinning is your proof the enamel is gone.
2. A bare, tinned end = good electrical contact. The rest of the wire stays insulated (that's the whole point of enamelled wire — turns can touch without shorting).
3. Optional but smart: label one coil's leads "start/finish" so you can keep track of winding direction.

## Step 5 — Repeat for the second coil
Wind the second D **the same way, same turn count, same direction**. Matching them makes the null far easier to achieve.

---

## Step 6 — Measure the inductance (don't skip)
This sets your TX tuning capacitor.
- **With an LCR meter:** just read L on each coil. Expect roughly **3–6 mH** for ~100 turns at 8″.
- **Without one (resonance method):** connect a known capacitor across the coil, drive it with a small AC signal, and find the frequency where the voltage peaks on the scope — I'll give you the exact bench steps when you're there. Then `L = 1 / ((2πf_peak)² · C)`.

Then the TX series-resonant cap is:
**C = 1 / ( (2π × 7500)² × L )** → about **100 nF** if L ≈ 4.5 mH. Trim with your cap assortment until the TX coil's sine peaks at 7.5 kHz.

---

## Assembly (preview — happens at the nulling milestone)
Lay both D's in the **same plane**, flat edges parallel and **overlapping slightly**, forming the 8″ circle. With TX energized, **slide one D** back and forth while watching the RX signal on the scope. There's a sweet spot where the RX residual drops to a tiny fraction — that's **the null**. Lock it there with epoxy/hot glue so it can't drift.

---

## Mistakes that bite beginners
- **Splayed turns** → inconsistent L, weak field. Keep the bundle tight.
- **Losing count** → the two coils won't match. Tally every 10.
- **Cracked enamel / kinks** → shorted turns kill the coil. Handle gently.
- **Not binding before removal** → the coil explodes off the jig. Tape first.
- **Ferrous nails left in the finished head** → fine in the *jig* (you remove the coil), but never leave steel hardware in the actual search head. Plastic/brass only there.
- **Forgetting to strip the enamel** → "open circuit" that's really just insulation. Tin the ends.
