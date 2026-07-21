# VLF Induction-Balance Metal Detector — built from scratch

A working metal detector designed and built from first principles: hand-wound coils, discrete analog signal chain, ESP32-based coherent transmit/demodulate architecture, and (roadmapped) on-device neural-network target classification.

**Current state: Sprint 1 complete — it detects and beeps.** Sprint 2 (I/Q phase discrimination: iron vs. treasure) is in progress.

```
signal path:  8.4 kHz clock ─ TC4427 driver ─ series-resonant tank ─ TX coil
                   │                                                    ↓ (field)
                   │                                          target eddy currents
                   │                                                    ↓ (echo)
              0°/90° refs                  nulled RX coil ─ OPA2134 preamp ─ 74HC4053
                   └──────────────────────────────────────────── synchronous demod
                                                                        ↓
                                                     I/Q → ESP32 ADC → beep / verdict
```

## Highlights (the parts that were hard)
- **Coils characterized with a self-built instrument** — an ESP32 resonance-sweep inductance meter (built before owning an LCR meter), later cross-checked against the live TX tank. The two disagreed by 10%; root cause was the ±20% tolerance of the electrolytic reference cap. *Lesson: a measurement is only as good as its reference standard.*
- **Resonant TX tuned with a multimeter** after the ADC-based sweep proved self-polluted — including catching the driver being *phantom-powered through its own input protection diodes* during an unpowered test.
- **Mechanical null of the Double-D coils** to ~40×+ rejection of direct coupling, done by millimeter geometry — then re-found from scratch with a DMM-across-the-coil method when the electronic meter turned out to be saturated.
- **The M3 debugging saga** (see `01_Status_and_Log/BUILD_LOG.md`): a week of "constant 2500, nothing changes" defeated by systematic elimination — capacitive-coupling theory (killed by grounded-foil experiment), ground-path theory (killed by input-gag test), wrong-resistor theory (killed by the ohmmeter), flaky-coil theory (killed by continuity check) — landing on the real culprit: *gain miscalibrated to null depth because the DMM under-reads at 8.4 kHz*. Fifteen transferable lessons logged.
- **Synchronous demodulation** with hardware-phase-locked references (LEDC `hpoint` for the 90° channel): demod baseline stable to **±1 ADC count** on a breadboard.
- **"Motion mode" drift-tracking baseline** — including empirically finding both failure modes of an adaptive baseline (tracker too fast eats targets; no tracker = drift false-alarms).

## Repo map
| Folder | Contents |
|---|---|
| `01_Status_and_Log/` | **BUILD_STATUS.md** (dashboard) · **BUILD_LOG.md** (full chronological record + lessons — start here) |
| `02_Build_Guides/` | Sprint 1 master guide + per-milestone pin-by-pin walkthroughs (M3/M4/M5) · Sprint 2 discrimination guide · coil winding & measurement procedures |
| `03_Firmware/` | **Sprint2_console.ino** (current: TX, tuning sweep, null meter, demod, beep, I/Q, verdict modes) · L-meter · Sprint 1 console |
| `04_Schematics_and_Diagrams/` | Main schematic (SVG) · chip pinout cards annotated with this build's wiring · coil geometry |
| `05_Reference/` | Locked architecture plan · BOM · circuit theory notes · theory study syllabus · Sprint 4 AI/edge-ML plan |
| `06_Media/` | Photos & videos of the build (see its README) |
| `99_Archive/` | Superseded files kept for history |

## The machine (locked decisions)
| | |
|---|---|
| Architecture | VLF induction-balance, phase-sensitive (synchronous) demod, single coherent clock |
| Frequency | 8400 Hz (the tank's measured f₀ — originally designed 7.5 kHz; see lesson #1) |
| Coils | 2× hand-wound 8″ Double-D, ~100 turns 30 AWG — 18 Ω / ~3.5 mH each, mechanically nulled |
| TX | TC4427 gate driver → 100 nF/400 V film + coil series tank (~20 V resonant rise from a 9 V pack) |
| RX | OPA2134 preamp (gain-ladder 3→11→…101 post-shielding), 2.5 V buffered virtual ground |
| Demod | CD74HC4053, 0° + 90° channels → RC ladders → ESP32 ADC (I/Q) |
| Brain | Arduino Nano ESP32 (ESP32-S3) — generates TX + both demod references from one clock |
| Output | Pitch-tracking piezo; OLED verdict display (Sprint 2); iron/non-ferrous by atan2(Q, I) |

## Roadmap
- **Sprint 1 — make it beep** ✅ (transmit → null → preamp → demod → audio)
- **Sprint 2 — make it smart** 🔄 (90° channel ✅ wired → phase calibration → discrimination + OLED)
- **Sprint 3 — make it real** (Faraday-shielded coils → deep null → full gain → perfboard, enclosure, field test)
- **Sprint 4 — make it intelligent** (self-collected I/Q sweep dataset → 1-D CNN → int8 TFLite Micro *on the detector*)

## Sensitivity record (same test: coin, flat-on, air, slow sweep)
| Date / config | Range |
|---|---|
| Sprint 1 complete — gain 11, unshielded, breadboard | ~2 cm |
| *(updated as upgrades land)* | |

---
*Built by Laird. Docs, firmware, and debugging companionship by Claude (Anthropic). Every measurement, solder joint, and hard-won lesson: human.*
