/*
 * VLF Metal Detector — Milestone 1: TRANSMIT
 * -------------------------------------------
 * Goal: get a clean, stable 7.5 kHz square wave out of the ESP32 to drive
 * the TX coil's push-pull driver + series-resonant tank.
 *
 * What to verify (Done-when for M1):
 *   1. Scope GPIO 25 -> clean 7.5 kHz square, ~50% duty, 3.3 V swing.
 *   2. After the driver + resonant cap, scope across the TX coil -> a STRONG
 *      sine at 7.5 kHz that peaks when the cap value matches your coil's L.
 *
 * Board: ESP32-WROOM-32 devkit.  Arduino-ESP32 core 3.x (uses ledcAttach()).
 *   (On core 2.x, see the note at the bottom for the older ledcSetup/ledcAttachPin API.)
 *
 * Wi-Fi/BT are forced OFF — the radio is a noise source for the analog front end.
 */

#include <WiFi.h>
#include "esp_bt.h"

// ---- Configuration ----------------------------------------------------------
const int   TX_PIN     = 25;       // TX drive output -> push-pull driver input
const double F0_HZ     = 7500.0;   // operating frequency (locked design = 7.5 kHz)
const int   PWM_BITS   = 10;       // 10-bit -> 1024 duty steps (max freq ~78 kHz, plenty)
const int   DUTY_50PCT = (1 << PWM_BITS) / 2;   // 512 = 50% duty

// Optional: a second output you can later use as the demod reference / phase pin.
// For M1 we only need TX. Precise 0/90 phase references come in the M4 firmware
// (we'll use MCPWM there for clean phase control). Leave this commented for now.
// const int REF_PIN = 26;

void setup() {
  Serial.begin(115200);
  delay(300);

  // --- Kill the radios: cleaner analog front end ---
  WiFi.mode(WIFI_OFF);
  btStop();

  // --- Start the TX square wave on TX_PIN ---
  // ledcAttach(pin, freq, resolution_bits) configures + attaches in one call (core 3.x).
  if (!ledcAttach(TX_PIN, F0_HZ, PWM_BITS)) {
    Serial.println("ERROR: ledcAttach failed — check pin / core version.");
    while (true) { delay(1000); }
  }
  ledcWrite(TX_PIN, DUTY_50PCT);   // 50% duty square at F0

  Serial.println("=== VLF M1: TRANSMIT ===");
  Serial.printf("TX_PIN  : GPIO %d\n", TX_PIN);
  Serial.printf("Freq    : %.1f Hz\n", F0_HZ);
  Serial.printf("Duty    : 50%% (%d / %d)\n", DUTY_50PCT, (1 << PWM_BITS));
  Serial.println("Scope GPIO 25 now: you should see a 7.5 kHz square.");
  Serial.println("Then build the driver + resonant cap and scope the coil for a sine.");
}

void loop() {
  // Nothing to do — the LEDC peripheral generates the waveform in hardware.
  // Heartbeat so you know it's alive while you're at the scope.
  static uint32_t t = 0;
  if (millis() - t > 5000) {
    t = millis();
    Serial.println("TX running @ 7.5 kHz ...");
  }
}

/* -----------------------------------------------------------------------------
 * NOTE — Arduino-ESP32 core 2.x (older) uses a different API. If ledcAttach()
 * is undefined for you, replace the setup block with:
 *
 *     const int CH = 0;
 *     ledcSetup(CH, F0_HZ, PWM_BITS);
 *     ledcAttachPin(TX_PIN, CH);
 *     ledcWrite(CH, DUTY_50PCT);
 *
 * Easiest fix though: update the ESP32 boards package to 3.x in Boards Manager.
 * ---------------------------------------------------------------------------*/
