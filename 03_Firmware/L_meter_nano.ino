/*
 * VLF Detector — Inductance Meter  (ARDUINO NANO ESP32 version)
 * ------------------------------------------------------------
 * Finds the resonant frequency of an L-C tank and computes:
 *     L = 1 / ( (2*pi*f0)^2 * C_known )
 * The Nano ESP32 acts as BOTH the signal source and the meter.
 *
 * *** VERSION-PROOF LEDC ***
 * This sketch auto-detects your ESP32 Arduino core version and uses the
 * correct LEDC (tone) API. Works on BOTH core 2.x and core 3.x.
 *
 * ---- WIRING (low-voltage, safe) ----
 *   D2 ──[ 1k resistor ]──┬── A0 (analog sense)
 *                         │
 *                         ├──[ C_known 0.1uF, + lead here ]── GND
 *                         │
 *                         └──[ COIL under test ]── GND
 *
 *   - Coil and known cap in parallel, from NODE A to GND.
 *   - Electrolytic cap: + (long leg) to NODE A, - (stripe) to GND.
 *   - Powered by USB only.
 *
 * ---- ARDUINO IDE SETUP ----
 *   Tools > Board  : "Arduino Nano ESP32"
 *   Tools > Pin Numbering : "By Arduino pin" (default) — uses D2/A0 labels
 *   Open Serial Monitor at 115200. Run once per coil (TX, then RX).
 */

#include <WiFi.h>

// ---- USER SETTINGS ----------------------------------------------------------
const int    TX_PIN     = D2;        // drive output -> 1k -> node A
const int    SENSE_PIN  = A0;        // analog input, reads node A
const double C_KNOWN_F  = 100e-9;    // <-- your cap's real value (0.1uF = 100e-9)

const int    F_START    = 3000;      // sweep start (Hz)
const int    F_STOP     = 20000;     // sweep stop  (Hz)
const int    F_STEP     = 100;       // sweep resolution (Hz)
const int    SETTLE_MS  = 3;
const int    N_SAMPLES  = 3000;

const int    LEDC_CH    = 0;         // LEDC channel (only used on core 2.x)
const int    LEDC_RES   = 10;        // resolution bits
// -----------------------------------------------------------------------------

int bestF = 0, bestAmp = 0;

// ----- Version-proof tone helpers -------------------------------------------
// Core 3.x has ledcAttach(pin,freq,res).  Core 2.x uses ledcSetup+ledcAttachPin.
void toneBegin(int pin, int startFreq) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcAttach(pin, startFreq, LEDC_RES);
#else
  ledcSetup(LEDC_CH, startFreq, LEDC_RES);
  ledcAttachPin(pin, LEDC_CH);
#endif
}

void toneSet(int pin, int freq) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcWriteTone(pin, freq);
#else
  ledcWriteTone(LEDC_CH, freq);
#endif
}
// -----------------------------------------------------------------------------

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
  delay(400);
  WiFi.mode(WIFI_OFF);                // quiet the radio

  analogReadResolution(12);
  toneBegin(TX_PIN, F_START);         // attach LEDC to D2 (version-proof)

  Serial.println("=== VLF Inductance Meter (Nano ESP32) ===");
  Serial.printf("Core v%d.%d.%d\n",
                ESP_ARDUINO_VERSION_MAJOR,
                ESP_ARDUINO_VERSION_MINOR,
                ESP_ARDUINO_VERSION_PATCH);
  Serial.printf("C_known = %.1f nF. Sweeping %d..%d Hz.\n\n",
                C_KNOWN_F * 1e9, F_START, F_STOP);

  for (int f = F_START; f <= F_STOP; f += F_STEP) {
    toneSet(TX_PIN, f);
    delay(SETTLE_MS);
    int amp = measureAmplitude();
    if (amp > bestAmp) { bestAmp = amp; bestF = f; }
    if (f % 1000 == 0) Serial.printf("  %5d Hz  amp=%4d\n", f, amp);
  }

  toneSet(TX_PIN, 0);                 // stop drive

  double f0 = bestF;
  double L  = 1.0 / (pow(2 * PI * f0, 2) * C_KNOWN_F);

  Serial.println("\n--- RESULT ---");
  Serial.printf("Resonant peak f0 : %d Hz  (amp=%d)\n", bestF, bestAmp);
  Serial.printf("Inductance L     : %.3f mH\n", L * 1e3);
  Serial.println("Re-run for the other coil. Expect ~3-6 mH, both similar.");
}

void loop() { }
