/*
  realistic_yoho_jaw.ino
  Realistic slow jaw movement for the "Yo Ho (A Pirate's Life for Me)" first clip.
  Uses multiple jaw positions and smooth transitions.
  Servo signal -> pin 9. Use external 5-6V supply for servo; connect grounds together.
  NOTE: plays only audio file #1 on the DF1201S.
*/

#include <Servo.h>
#include <DFRobot_DF1201S.h>
#include "SoftwareSerial.h"

#if defined(ARDUINO_AVR_UNO) || defined(ESP8266)
#include "SoftwareSerial.h"
SoftwareSerial DF1201SSerial(2, 3);  //RX  TX
#else
#define DF1201SSerial Serial1
#endif

DFRobot_DF1201S DF1201S;

Servo mouth;
const int servoPin = 9;

const int JAW_OPEN = 0;
const int JAW_4 = 9;   
const int JAW_3 = 18;  
const int JAW_2 = 25;  
const int JAW_1 = 30;  
const int JAW_CLOSED = 45; 


// Timing (ms)
const int SNAP = 80;     // very short (consonant)
const int QUICK = 175;   // short syllable
const int MED = 320;     // normal syllable
const int LONG = 300;    // long/stressed syllable
const int BEAT = 500;    // full beat baseline


const int phrase[] = {
  JAW_3,    // 1: "Yo"        (small-mid)
  JAW_4,    // 2: "ho"        (more open)
  JAW_3,    // 3: "yo"        (small-mid)
  JAW_4,    // 4: "ho"        (more open)
  JAW_1,    // 5: "a"         (very small opening / filler)
  JAW_OPEN, // 6: "pi-"       (stressed, widest)
  JAW_4,    // 7: "-rate's"   (follow-through)
  JAW_3,    // 8: transition  (connect to next)
  JAW_OPEN, // 9: "life"      (stressed, wide)
  JAW_2     // 10: "for/me"   (smaller opening)
};

const int delays[] = {
  333, // Yo
  400, // ho
  333, // yo
  400, // ho
  200, // a
  667, // pi-
  467, // -rate's
  333, // transition
  600, // life
  267  // for / me
};


void moveTo(int target, int duration_ms = 120) {
  int current = mouth.read();
  int steps = abs(target - current);
  if (steps == 0) {
    // Nothing to do, just wait duration_ms (so timing stays consistent)
    if (duration_ms > 0) delay(duration_ms);
    return;
  }

  // direction
  int dir = (target > current) ? 1 : -1;

  // choose step increment so per-step delay >= 1 ms
  int stepInc = 1;
  if (duration_ms > 0 && duration_ms < steps) {
    stepInc = (steps + duration_ms - 1) / duration_ms; // ceil(steps/duration_ms)
  }

  int loopCount = (steps + stepInc - 1) / stepInc;
  int delayMs = (duration_ms + loopCount/2) / max(1, loopCount); // approx round(duration_ms/loopCount)
  if (delayMs < 1) delayMs = 1;

  int pos = current;
  while (pos != target) {
    pos += dir * stepInc;
    if ((dir == 1 && pos > target) || (dir == -1 && pos < target)) pos = target;
    mouth.write(pos);
    delay(delayMs);
  }
  mouth.write(target);
}

void moveToWithBounce(int target, int holdMs) {
  // Move to target quickly (approx 1/3 of syllable)
  int moveDur = holdMs / 3;
  if (moveDur < 25) moveDur = 25;
  moveTo(target, moveDur);

  int remaining = holdMs - moveDur;
  if (remaining <= 25) {
    if (remaining > 0) delay(remaining);
    return;
  }

  // Damped bounce parameters (tweak baseAmplitude/resolution to taste)
  const int maxBounces = 2;
  const int baseAmplitude = 8;                 // servo units to nudge
  const int midpoint = (JAW_OPEN + JAW_CLOSED) / 2;
  int sign = (target <= midpoint) ? +1 : -1;   // if mostly open, bounce toward closed

  int steps = maxBounces * 2;
  int perStep = remaining / steps;
  if (perStep < 12) perStep = 12;

  for (int b = 0; b < maxBounces; ++b) {
    int amp = baseAmplitude / (b + 1);         // dampening: 8 then 4...
    int peak = target + sign * amp;
    if (peak < 0) peak = 0;
    if (peak > 180) peak = 180; // safety clamp (your jaw range is smaller)
    mouth.write(peak);
    delay(perStep);
    mouth.write(target);
    delay(perStep);
  }

  // Compensate any tiny rounding remainder
  int used = moveDur + perStep * steps;
  if (used < holdMs) delay(holdMs - used);
}

// void moveTo(int target, int duration_ms = 120) {
//   int current = mouth.read(); // last commanded position
//   int steps = abs(target - current);
//   if (steps == 0) return;

//   int dir = (target > current) ? 1 : -1;


//   int stepInc = 1;
//   if (duration_ms > 0 && duration_ms < steps) {
//     stepInc = (steps + duration_ms - 1) / duration_ms; 
//   }

 
//   int loopCount = (steps + stepInc - 1) / stepInc; 

//   int delayMs = (duration_ms + loopCount/2) / max(1, loopCount); 
//   if (delayMs < 1) delayMs = 1;

//   int pos = current;
//   while (pos != target) {
//     pos += dir * stepInc;
//     if ((dir == 1 && pos > target) || (dir == -1 && pos < target)) pos = target;
//     mouth.write(pos);
//     delay(delayMs);
//   }
//   mouth.write(target); 
// }

void setup() {
  mouth.attach(servoPin);
  mouth.write(JAW_CLOSED); // start closed
  delay(500);

  Serial.begin(115200);
#if (defined ESP32)
  DF1201SSerial.begin(115200, SERIAL_8N1, /*rx =*/D3, /*tx =*/D2);
#else
  DF1201SSerial.begin(115200);
#endif
  while (!DF1201S.begin(DF1201SSerial)) {
    Serial.println("Init failed, please check the wire connection!");
    delay(1000);
  }

  DF1201S.setVol(/*VOL = */15);
  DF1201S.switchFunction(DF1201S.MUSIC);
}

void loop() {
  DF1201S.setPlayTime(0); // only works in music mode

  // for (int i = 0; i < sizeof(phrase)/sizeof(phrase[0]); i++) {
  //   moveTo(phrase[i]);  // smooth move (~8ms per step)
  //   delay(delays[i]);      // hold at position
  // }

  for (int i = 0; i < sizeof(phrase)/sizeof(phrase[0]); i++) {
  // moveToWithBounce will move into position and perform small damped bounces inside the hold period
  moveToWithBounce(phrase[i], delays[i]);
}

   
  mouth.write(JAW_CLOSED); 

  DF1201S.pause();
  delay(10000); // pause 10 seconds before repeating / trigger pause
}
