# Sprint 3 — Physical Build: Shopping List & Plan
*Target: a field-usable, portfolio-photogenic handheld. Build style: perfboard electronics (PCB reserved for V2), 3D-printed housings, 8×AA power.*

---

## The three rules that make this look "product," not "prototype"

1. **Lock the circuit before you solder.** Finish shields + final gain + discrimination calibration on the breadboard first. Perfboard captures a *proven* design, not a moving one.
2. **The coils must never move relative to each other.** This is the whole ballgame for a detector. Print rigid coil housings, null the pair *inside* them, then **pot with epoxy** so the geometry is frozen forever. This single step kills the null-drift you fought.
3. **One ground philosophy, carried into hardware.** Star-ground the perfboard (heavy TX-return and analog references meet at one point), run a **shielded cable** coil→box, and ground the coil Faraday shields at *one* end only. Everything you learned in the M3 saga becomes layout discipline here.

Build order: **shields+discrimination on breadboard → perfboard the proven circuit → print+pot coils → print control box → shaft & assembly → field-test.**

---

## TIER A — Finish & permanent-build the electronics
*(Cross off anything you already have from the earlier Order 2.)*

- [ ] **Perfboard / protoboard** — double-sided FR4 with plated holes, assorted sizes (5×7 & 7×9 cm). Get a multipack. ~$8
- [ ] **DIP IC sockets** — 8-pin ×5, 16-pin ×2 (never solder ICs directly; sockets = swappable + no heat damage). ~$4
- [ ] **22 AWG solid-core hookup wire** — 6-color spool set (perfboard jumping; solid, not stranded). ~$8
- [ ] **Male/female header pins** — 0.1″ breakaway strips + a few female sockets (board-to-board, coil-cable landing, OLED plug). ~$5
- [ ] **2-pin screw terminals, 5 mm pitch** ×4 — clean, re-openable coil & battery connections. ~$3
- [ ] **2N3904 transistors** (10-pack) — buzzer boost + the 4053 level-shifter fallback. *(may already have)* ~$2
- [ ] **SSD1306 OLED, 0.96″ I²C, 4-pin** — discrimination display. *(Sprint 2)* ~$4
- [ ] **B10K linear pots, panel-mount, with knobs** ×3 — Sensitivity, Discrimination threshold, Volume. ("B" = linear taper.) ~$6
- [ ] **Momentary push buttons, panel-mount** ×2 — Re-zero/pinpoint, Mode. ~$3

*Optional UI upgrade:* an **EC11 rotary encoder + knob** (~$2) lets you drive a menu on the OLED instead of multiple pots — slicker, more "product," slightly more firmware.

## TIER B — Power & switching
- [ ] **8×AA holder with wire leads** — *(you have the 6×AA for bench; get the 8-cell for the final unit)*. ~$6
- [ ] **AA batteries ×8** (or use rechargeables you own). ~$6
- [ ] **SPST panel-mount toggle or rocker switch** — the real power switch. ~$2

## TIER C — Coils: housing, shield, cable (**the critical rigid subsystem**)
- [ ] **2-part epoxy resin** (slow-cure, e.g. 30-min) — pot the coils in their printed housings so the null never drifts. ~$8
- [ ] **Copper OR aluminum foil tape, conductive-adhesive** — Faraday shield around each coil winding (gap in the ring! ground at one point). ~$7
  - *Premium alt:* **nickel/graphite conductive spray paint** (e.g. MG Chemicals Super Shield) — paint the inside of the printed coil cavity, cleaner than tape for a printed housing. ~$18
- [ ] **4-conductor shielded cable** (or 2× shielded pair / mic cable), ~1.5 m — carries TX pair + RX pair + shield down the shaft. Shielding here matters as much as at the coil. ~$8

## TIER D — Shaft & mechanical (printer + a hardware-store run)
- [ ] **PVC pipe, ½″ or ¾″** (or fiberglass tube) — the shaft. **Non-metallic near the coil is mandatory** — no aluminum by the coil end. Hardware store, ~$5.
- [ ] **M3 nylon screws + nuts** assortment — coil-end fasteners stay non-metallic. ~$7
- [ ] **M3 brass standoffs + screws** — mount the perfboard inside the control box. ~$6
- [ ] **M3 brass heat-set threaded inserts** (for 3D prints) — **the single biggest "pro" upgrade**: melt them into printed bosses with your soldering iron so the box screws shut repeatably without stripping plastic. ~$8 (+ optional insert tip ~$5)
- [ ] **Heat-shrink tubing assortment** — strain relief & tidy joints. ~$6
- [ ] Adhesive cable clips / small zip ties — run the cable down the shaft cleanly. ~$4

## TIER E — Optional audio & polish (nice-to-have "product feel")
- [ ] **PAM8302 mini amp + 8 Ω 0.5 W speaker** — real speaker volume vs. the piezo. ~$5
- [ ] **3.5 mm panel-mount headphone jack** — classic detector feature; auto-mutes speaker when plugged. ~$3
- [ ] Small clear acrylic offcut — OLED window (or print a thin bezel + use packaging plastic). ~$4

---

## What you'll 3D-print (design these; no purchase)
- **Coil housings** — a flat former/pocket for each Double-D that holds it rigid; assemble+null the pair, then epoxy-pot. Add a cable strain-relief boss and a shaft-mount ear.
- **Coil-to-shaft yoke** — the adjustable-angle "shoe" that clamps the coil to the lower shaft (printed cuff + nylon bolt + wingnut).
- **Control box** — fits the perfboard, 8×AA holder, 3 pots + 2 buttons on the face, OLED window, cable gland hole. Design in bosses for heat-set inserts.
- **Shaft cuffs / clamps** — armrest cradle, hand-grip, and a cuff that couples the control box to the upper shaft.
- **Knob caps** (optional) — if you don't buy knobs.

*(When the circuit's locked I'll help you turn the schematic + measured part footprints into a control-box layout so the print fits on the first try.)*

---

## Rough budget
Essentials (Tiers A–D, minus what you own): **~$60–80.** Add optional audio/polish: **+$15.** Filament: pennies. Total well under $100 for a genuinely field-usable, photographable unit.

## Buy-now vs. buy-later
- **Now** (unblocks Sprint 2/3 work in progress): OLED, pots, foil tape / conductive paint, shielded cable, perfboard, sockets, hookup wire. These let you finish discrimination + add shields on the breadboard.
- **Later** (once the circuit is locked, before you enclose): 8×AA holder, switch, epoxy, heat-set inserts, PVC, fasteners, audio.

## How this maps to Sprint 3 milestones
1. **Shield the coils** (foil/paint + single-point ground) → re-null → climb gain back toward 101. *Biggest sensitivity jump in the whole project.*
2. **Perfboard the proven circuit** (star ground, sockets, screw terminals).
3. **Print + pot the coil housings** (null frozen in epoxy).
4. **Print the control box**, mount board + power + UI (heat-set inserts).
5. **Shaft assembly** (PVC, printed cuffs, shielded cable, strain relief).
6. **Field test** on buried coins/targets → record the range table → portfolio write-up + the V2-PCB decision.
