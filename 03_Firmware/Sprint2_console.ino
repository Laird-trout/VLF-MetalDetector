/*
 * Sprint2_console.ino — Sprint 1 console + I/Q discrimination (Nano ESP32)
 * ------------------------------------------------------------------------
 * Everything from Sprint 1 (t / f <Hz> / x / n / d / b / s) PLUS:
 *   q   live I/Q stream:  I  Q  dI  dQ  mag  angle(deg)
 *   z   zero the I/Q baseline (no target, hands away — takes ~3 s)
 *   v   verdict mode: classifies IRON vs GOOD by phase angle, beeps + OLED
 *
 * NEW PINS: D4 = 90-degree reference -> 4053 pin 10 (Select B)
 *           A1 = Q channel DC in (from the B-channel RC ladder)
 * The 90-degree shift is done in LEDC hardware via the hpoint register, so
 * both references are phase-locked to the TX clock permanently.
 *
 * SET AFTER CALIBRATION (see Sprint2 guide, step 4):
 */
#define FE_MIN_DEG   90.0     // iron cluster lower bound  <-- placeholders!
#define FE_MAX_DEG  180.0     // iron cluster upper bound  <-- measure yours
#define MAG_THRESH   6.0      // counts of |vector| before classifying
#define USE_OLED     0        // 1 = SSD1306 on A4/A5 (needs Adafruit libs)

#include <WiFi.h>
#include <math.h>
#include "driver/ledc.h"

#if USE_OLED
  #include <Adafruit_GFX.h>
  #include <Adafruit_SSD1306.h>
  Adafruit_SSD1306 oled(128, 64, &Wire, -1);
#endif

// ---------------- pins ----------------
const int TX_PIN   = D2;    // TX drive
const int REF0_PIN = D3;    // 0-deg reference  (same LEDC channel as TX)
const int REF90_PIN= D4;    // 90-deg reference (same timer, hpoint-shifted)
const int BUZZ_PIN = D5;
const int I_PIN    = A0;    // channel A demod DC
const int Q_PIN    = A1;    // channel B demod DC

int  opFreq = 8400;
char mode   = 's';

// LEDC: channels 0 and 1 share timer 0 in the Arduino core -> same frequency.
const int CH_TX  = 0;       // D2 + D3
const int CH_90  = 1;       // D4
const int CH_BZ  = 2;       // buzzer (its own timer)
const int RES    = 10;      // 10-bit -> period = 1024 counts
const int DUTY   = 512;     // 50%
const int HPOINT = 256;     // quarter period = 90 degrees

// ---------------- tone / reference setup ----------------
void refsOn(int f) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcAttachChannel(TX_PIN,   f, RES, CH_TX);
  ledcAttachChannel(REF0_PIN, f, RES, CH_TX);
  ledcAttachChannel(REF90_PIN,f, RES, CH_90);
#else
  ledcSetup(CH_TX, f, RES);
  ledcAttachPin(TX_PIN,   CH_TX);
  ledcAttachPin(REF0_PIN, CH_TX);
  ledcSetup(CH_90, f, RES);          // same timer as CH_TX (0,1 -> timer 0)
  ledcAttachPin(REF90_PIN, CH_90);
#endif
  ledcWrite(CH_TX, DUTY);
  // 90-degree shift on the second channel via hpoint:
  ledc_set_duty_with_hpoint(LEDC_LOW_SPEED_MODE, (ledc_channel_t)CH_90, DUTY, HPOINT);
  ledc_update_duty(LEDC_LOW_SPEED_MODE, (ledc_channel_t)CH_90);
}

void refsOff() {
  ledcWrite(CH_TX, 0);
  ledc_set_duty_with_hpoint(LEDC_LOW_SPEED_MODE, (ledc_channel_t)CH_90, 0, 0);
  ledc_update_duty(LEDC_LOW_SPEED_MODE, (ledc_channel_t)CH_90);
}

void buzzBegin() {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcAttach(BUZZ_PIN, 1000, RES); ledcWriteTone(BUZZ_PIN, 0);
#else
  ledcSetup(CH_BZ, 1000, RES); ledcAttachPin(BUZZ_PIN, CH_BZ);
  ledcWriteTone(CH_BZ, 0);
#endif
}
void buzz(int f) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcWriteTone(BUZZ_PIN, f);
#else
  ledcWriteTone(CH_BZ, f);
#endif
}

// ---------------- measurement ----------------
int readAvg(int pin, int n) {
  long a = 0;
  for (int i = 0; i < n; i++) a += analogRead(pin);
  return (int)(a / n);
}
int swingPP(int pin, int n) {
  int lo = 4095, hi = 0;
  for (int i = 0; i < n; i++) { int v = analogRead(pin); if (v<lo) lo=v; if (v>hi) hi=v; }
  return hi - lo;
}

float I0 = 0, Q0 = 0;                 // zeroed baseline
int   dBase = 0;                      // sprint-1 beep baseline

void doZero() {
  Serial.println("Zeroing I/Q baseline: metal & hands AWAY (3 s)...");
  delay(500);
  long ai = 0, aq = 0;
  for (int k = 0; k < 30; k++) { ai += readAvg(I_PIN, 400); aq += readAvg(Q_PIN, 400); delay(60); }
  I0 = ai / 30.0;  Q0 = aq / 30.0;
  Serial.printf("baseline: I0=%.1f  Q0=%.1f\n", I0, Q0);
}

#if USE_OLED
void oledShow(const char* line1, float mag, float ang) {
  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);
  oled.setTextSize(2); oled.setCursor(0, 0);  oled.println(line1);
  oled.setTextSize(1); oled.setCursor(0, 26);
  oled.printf("mag %.0f   ang %.0f", mag, ang);
  int bar = constrain((int)(mag * 2), 0, 127);
  oled.fillRect(0, 44, bar, 12, SSD1306_WHITE);
  oled.display();
}
#endif

void setup() {
  Serial.begin(115200);
  delay(400);
  WiFi.mode(WIFI_OFF);
  analogReadResolution(12);
  refsOn(opFreq); refsOff();
  buzzBegin();
#if USE_OLED
  Wire.begin();                       // A4/A5 on Nano ESP32
  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  oled.clearDisplay(); oled.display();
#endif
  Serial.println("=== Sprint 2 console ===");
  Serial.println("t=tune f<Hz> x=tx n=null d=demodI b=beep | q=I/Q z=zero v=verdict | s=stop");
}

void loop() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n'); cmd.trim();
    if (cmd.length()) {
      char c = cmd.charAt(0);
      if      (c=='f') { int v=cmd.substring(1).toInt();
                         if (v>=3000 && v<=20000){opFreq=v;Serial.printf("opFreq=%d\n",v);} }
      else if (c=='x') { refsOn(opFreq); mode='x'; Serial.printf("TX+refs on @ %d Hz\n",opFreq); }
      else if (c=='n') { refsOn(opFreq); mode='n'; Serial.println("null meter. s=stop"); }
      else if (c=='d') { refsOn(opFreq); mode='d'; Serial.println("demod I monitor. s=stop"); }
      else if (c=='q') { refsOn(opFreq); mode='q'; Serial.println("I/Q stream. z to zero. s=stop"); }
      else if (c=='z') { refsOn(opFreq); doZero(); }
      else if (c=='b') { refsOn(opFreq);
                         Serial.println("Zeroing beep baseline (3 s)...");
                         delay(500); long a=0; for(int k=0;k<30;k++){a+=readAvg(I_PIN,400);delay(80);}
                         dBase=a/30; Serial.printf("baseline=%d. Beeping. s=stop\n",dBase); mode='b'; }
      else if (c=='v') { refsOn(opFreq);
                         if (I0==0 && Q0==0) doZero();
                         mode='v'; Serial.println("verdict mode. s=stop"); }
      else if (c=='t') { mode='s';
                         Serial.println("sweep 6000..9000 (Rsense net on A0 required!)");
                         int bf=0,ba=0;
                         for(int f=6000;f<=9000;f+=25){
                           refsOn(f); delay(4); int a=swingPP(I_PIN,1500);
                           if(a>ba){ba=a;bf=f;} }
                         refsOff();
                         Serial.printf("PEAK f0=%d (amp %d)\n",bf,ba); }
      else if (c=='s') { mode='s'; refsOff(); buzz(0); Serial.println("stopped."); }
    }
  }

  if (mode=='n') { int a=swingPP(I_PIN,3000);
                   Serial.print('|'); for(int i=0;i<min(a/60,50);i++)Serial.print('#');
                   Serial.printf(" %d\n",a); delay(120); }

  if (mode=='d') { Serial.println(readAvg(I_PIN,2000)); delay(150); }

  if (mode=='b') { static int trk=0;
                   int v=readAvg(I_PIN,3000); int dev=abs(v-dBase);
                   const int TH=3;
                   if (dev>TH) buzz(constrain(500+(dev-TH)*50,500,4000));
                   else { buzz(0);
                          // drift-tracking, SLOW: 1 count every 8 quiet loops (~0.6/s)
                          if (++trk >= 8) { trk=0;
                            if (v > dBase) dBase++; else if (v < dBase) dBase--; } }
                   delay(30); }

  if (mode=='q' || mode=='v') {
    float I=readAvg(I_PIN,1500), Q=readAvg(Q_PIN,1500);
    float dI=I-I0, dQ=Q-Q0;
    float mag=sqrtf(dI*dI+dQ*dQ);
    float ang=atan2f(dQ,dI)*180.0/PI;          // -180..180
    if (mode=='q') {
      Serial.printf("I=%5.0f Q=%5.0f  dI=%+6.1f dQ=%+6.1f  mag=%6.1f  ang=%7.1f\n",
                    I,Q,dI,dQ,mag,ang);
      delay(150);
    } else {
      if (mag > MAG_THRESH) {
        bool iron = (ang>=FE_MIN_DEG && ang<=FE_MAX_DEG);
        Serial.printf("mag=%5.1f ang=%6.1f  -> %s\n", mag, ang, iron?"IRON":"GOOD");
        buzz(iron ? 300 : 2500);
#if USE_OLED
        oledShow(iron?"IRON":"GOOD", mag, ang);
#endif
      } else {
        buzz(0);
        // drift-tracking ("motion mode"): baseline slowly follows slow drift.
        // Slow creep gets absorbed; a normal-speed sweep still spikes clear.
        I0 += 0.005f * (I - I0);
        Q0 += 0.005f * (Q - Q0);
#if USE_OLED
        oledShow("----", mag, 0);
#endif
      }
      delay(60);
    }
  }
}
