/*
 * Sprint1_console.ino — one sketch for all of Sprint 1 (Nano ESP32)
 * -----------------------------------------------------------------
 * Serial commands (115200 baud, type letter + Enter):
 *   t        tank tuning sweep 6000..9000 Hz -> prints curve + peak f0
 *   f 7300   set operating frequency (Hz)
 *   x        steady transmit at operating frequency
 *   n        null / amplitude meter (AC swing on A0), live bar graph
 *   d        demod monitor (DC average on A0), live stream
 *   b        beep mode (auto-zeros baseline, then pitch ~ |signal|)
 *   s        stop TX + all modes
 *
 * Pins: D2 = TX drive, D3 = 0-deg reference (same LEDC channel -> phase
 * locked), D5 = buzzer, A0 = sense input (network changes per milestone,
 * see Sprint1_Build_Guide.md).
 *
 * Works on ESP32 Arduino core 2.x and 3.x (version-proof LEDC helpers).
 */

#include <WiFi.h>

// ---------------- pins & settings ----------------
const int TX_PIN    = D2;
const int REF_PIN   = D3;
const int BUZZ_PIN  = D5;
const int SENSE_PIN = A0;

int  opFreq = 7500;              // operating frequency; set with 'f'
char mode   = 's';               // current mode

const int TX_CH   = 0;           // LEDC channel for TX + ref (core 2.x)
const int BUZZ_CH = 2;           // LEDC channel for buzzer  (core 2.x)

// ---------------- version-proof LEDC ----------------
#if ESP_ARDUINO_VERSION_MAJOR >= 3
void txBegin()            { ledcAttach(TX_PIN, opFreq, 10);
                            ledcAttach(REF_PIN, opFreq, 10); }
void txTone(int f)        { ledcWriteTone(TX_PIN, f); ledcWriteTone(REF_PIN, f); }
void buzzBegin()          { ledcAttach(BUZZ_PIN, 1000, 10); ledcWriteTone(BUZZ_PIN, 0); }
void buzzTone(int f)      { ledcWriteTone(BUZZ_PIN, f); }
#else
void txBegin()            { ledcSetup(TX_CH, opFreq, 10);
                            ledcAttachPin(TX_PIN, TX_CH);
                            ledcAttachPin(REF_PIN, TX_CH); }   // same channel = same phase
void txTone(int f)        { ledcWriteTone(TX_CH, f); }
void buzzBegin()          { ledcSetup(BUZZ_CH, 1000, 10); ledcAttachPin(BUZZ_PIN, BUZZ_CH);
                            ledcWriteTone(BUZZ_CH, 0); }
void buzzTone(int f)      { ledcWriteTone(BUZZ_CH, f); }
#endif

// ---------------- measurement helpers ----------------
int swingPP(int nSamples) {              // AC peak-to-peak on SENSE_PIN
  int lo = 4095, hi = 0;
  for (int i = 0; i < nSamples; i++) {
    int v = analogRead(SENSE_PIN);
    if (v < lo) lo = v;
    if (v > hi) hi = v;
  }
  return hi - lo;
}

int dcAvg(int nSamples) {                // DC average on SENSE_PIN
  long acc = 0;
  for (int i = 0; i < nSamples; i++) acc += analogRead(SENSE_PIN);
  return (int)(acc / nSamples);
}

void bar(int v, int fullScale) {         // crude serial bar graph
  int n = constrain((v * 50) / fullScale, 0, 50);
  Serial.print('|');
  for (int i = 0; i < n; i++) Serial.print('#');
  Serial.printf(" %d\n", v);
}

// ---------------- modes ----------------
void doSweep() {
  Serial.println("Tuning sweep 6000..9000 Hz (Rsense network on A0)...");
  int bestF = 0, bestA = 0;
  for (int f = 6000; f <= 9000; f += 25) {
    txTone(f);
    delay(4);
    int a = swingPP(1500);
    if (a > bestA) { bestA = a; bestF = f; }
    if (f % 250 == 0) { Serial.printf("%5d Hz ", f); bar(a, 3000); }
  }
  txTone(0);
  Serial.printf("\n--- PEAK: f0 = %d Hz (amp %d) ---\n", bestF, bestA);
  Serial.printf("Suggest:  f %d   then  x\n\n", bestF);
}

void setup() {
  Serial.begin(115200);
  delay(400);
  WiFi.mode(WIFI_OFF);
  analogReadResolution(12);
  txBegin();
  txTone(0);
  buzzBegin();

  Serial.println("=== Sprint 1 console ===");
  Serial.println("t=tune  f <Hz>=set freq  x=transmit  n=null meter  d=demod  b=beep  s=stop");
}

int baseline = 0;

void loop() {
  // ---- command handling ----
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd.length()) {
      char c = cmd.charAt(0);
      if (c == 'f') {
        int v = cmd.substring(1).toInt();
        if (v >= 3000 && v <= 20000) { opFreq = v; Serial.printf("opFreq = %d Hz\n", opFreq); }
        else Serial.println("usage: f 7300   (3000..20000)");
      } else if (c == 't') {
        mode = 's'; doSweep();
      } else if (c == 'x') {
        txTone(opFreq); mode = 'x';
        Serial.printf("TX on @ %d Hz (ref on D3, phase-locked)\n", opFreq);
      } else if (c == 'n') {
        txTone(opFreq); mode = 'n'; Serial.println("null meter (pp swing). s=stop");
      } else if (c == 'd') {
        txTone(opFreq); mode = 'd'; Serial.println("demod DC monitor. s=stop");
      } else if (c == 'b') {
        txTone(opFreq);
        Serial.println("Zeroing baseline: keep metal away (3 s)...");
        delay(500);
        long acc = 0; for (int i = 0; i < 30; i++) { acc += dcAvg(400); delay(80); }
        baseline = acc / 30;
        Serial.printf("baseline = %d. Beeping. s=stop\n", baseline);
        mode = 'b';
      } else if (c == 's') {
        mode = 's'; txTone(0); buzzTone(0); Serial.println("stopped.");
      }
    }
  }

  // ---- mode bodies ----
  if (mode == 'n') { bar(swingPP(3000), 3000); delay(120); }

  if (mode == 'd') { Serial.println(dcAvg(2000)); delay(150); }

  if (mode == 'b') {
    int v    = dcAvg(800);
    int dev  = abs(v - baseline);
    const int THRESH = 6;                       // counts of dead-band; raise if chatty
    if (dev > THRESH) {
      int pitch = constrain(200 + (dev - THRESH) * 25, 200, 2500);
      buzzTone(pitch);
    } else buzzTone(0);
    delay(30);
  }
}
