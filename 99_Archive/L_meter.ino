/*
 * VLF Detector — Inductance Meter (scope-free, LCR-free)
 * ------------------------------------------------------
 * Measures a coil's inductance by finding the resonant frequency of an
 * L-C tank, using the ESP32 as BOTH the signal source and the meter.
 *
 *   L = 1 / ( (2*pi*f0)^2 * C_known )
 *
 * ACCURACY NOTE: your L result is only as accurate as C_known. Use a decent
 * film cap and enter its real value below. ±5–10% is fine for our purposes.
 *
 * ---- WIRING (low-voltage, safe — this is NOT the powered TX tank) ----
 *   GPIO25 ──[ 2.2k resistor ]──┬── GPIO34 (ADC sense)
 *                               │
 *                               ├──[ C_known, ~100nF ]── GND
 *                               │
 *                               └──[ COIL under test ]── GND
 *
 *   - The coil and the known cap are in parallel, from node A to GND.
 *   - Driven through 2.2k from GPIO25; node A is read by ADC pin GPIO34.
 *   - Everything swings 0–3.3V here, so any ~100nF cap works (even 50V).
 *     The >=250V requirement is only for the real powered TX tank later.
 *
 * Board: ESP32-WROOM-32, Arduino-ESP32 core 3.x.
 * Open Serial Monitor at 115200. It prints the resonant peak and L.
 * Run it once per coil (TX, then RX).
 */

#include <WiFi.h>
#include "esp_bt.h"

// ---- USER SETTINGS ----------------------------------------------------------
const int    TX_PIN     = 25;        // drive output -> 2.2k -> node A
const int    SENSE_PIN  = 34;        // ADC input, reads node A (input-only pin)
const double C_KNOWN_F  = 100e-9;    // <-- set this to YOUR cap's real value (100nF here)

const int    F_START    = 3000;      // sweep start (Hz)
const int    F_STOP      = 20000;     // sweep stop  (Hz)
const int    F_STEP      = 100;       // sweep resolution (Hz)
const int    SETTLE_MS   = 3;         // let tank settle after changing freq
const int    N_SAMPLES   = 3000;      // ADC samples per frequency (amplitude window)
// -----------------------------------------------------------------------------

int    bestF   = 0;
int    bestAmp = 0;

int measureAmplitude() {
  int lo = 4095, hi = 0;
  for (int i = 0; i < N_SAMPLES; i++) {
    int v = analogRead(SENSE_PIN);
    if (v < lo) lo = v;
    if (v > hi) hi = v;
  }
  return hi - lo;                     // peak-to-peak in ADC counts
}

void setup() {
  Serial.begin(115200);
  delay(300);
  WiFi.mode(WIFI_OFF);
  btStop();

  analogReadResolution(12);
  ledcAttach(TX_PIN, F_START, 10);    // attach LEDC; freq set per-step below

  Serial.println("=== VLF Inductance Meter ===");
  Serial.printf("C_known = %.1f nF. Sweeping %d..%d Hz.\n\n",
                C_KNOWN_F * 1e9, F_START, F_STOP);

  for (int f = F_START; f <= F_STOP; f += F_STEP) {
    ledcWriteTone(TX_PIN, f);         // output 50%% square at frequency f
    delay(SETTLE_MS);
    int amp = measureAmplitude();
    if (amp > bestAmp) { bestAmp = amp; bestF = f; }
    // light progress every 1 kHz
    if (f % 1000 == 0) Serial.printf("  %5d Hz  amp=%4d\n", f, amp);
  }

  ledcWriteTone(TX_PIN, 0);           // stop drive

  double f0 = bestF;
  double L  = 1.0 / (pow(2 * PI * f0, 2) * C_KNOWN_F);

  Serial.println("\n--- RESULT ---");
  Serial.printf("Resonant peak f0 : %d Hz  (amp=%d)\n", bestF, bestAmp);
  Serial.printf("Inductance L     : %.3f mH\n", L * 1e3);
  Serial.println("Re-run for the other coil. Expect ~3-6 mH, both coils similar.");
}

void loop() { }
