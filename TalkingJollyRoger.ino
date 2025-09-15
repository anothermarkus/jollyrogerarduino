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



const int yohoMovements[] = {
  JAW_OPEN,    // 1: "Yo"        (small-mid)
  JAW_OPEN,    // 2: "ho"        (more open)
  JAW_OPEN,    // 3: "yo"        (small-mid)
  JAW_OPEN,    // 4: "ho"        (more open)
  JAW_4,    // 5: "a"         (very small opening / filler)
  JAW_3, // 6: "pi-"       (stressed, widest)
  JAW_3,    // 7: "-rate's"   (follow-through)
  JAW_3,    // 8: life  (connect to next)
  JAW_3, // 9: "for"      (stressed, wide)
  JAW_OPEN     // 10: "me"   (smaller opening)
};

const int yoHodelays[] = {
  450, // Yo
  450, // ho
  450, // yo
  450, // ho
  350, // a
  350, // pi-
  250, // -rate's
  350, // life
  250, // for
  550  //  me
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


// Improved moveSyllableBounce that accepts "nextIsFullOpen" and "holdOpenUntilEnd"
void moveSyllableBounce(int openTarget, int syllableMs, bool nextIsFullOpen = false, bool holdOpenUntilEnd = false) {
  if (syllableMs <= 40) {
    moveTo(openTarget, 12);
    if (syllableMs > 12) delay(syllableMs - 12);
    mouth.write(JAW_CLOSED);
    delay(40);
    return;
  }

  bool fullOpen = (openTarget == JAW_OPEN);

  // Special-case: if caller asked to hold open until the very end (final "me")
  if (holdOpenUntilEnd && fullOpen) {
    int closeDur = max(6, syllableMs / 20);         // small snappy close window
    int openDur  = max(10, syllableMs * 10 / 100);  // quick open
    int holdMs   = syllableMs - openDur - closeDur; // hold almost all time

    moveTo(openTarget, openDur);
    if (holdMs > 0) delay(holdMs);
    moveTo(JAW_CLOSED, closeDur);
    mouth.write(JAW_CLOSED);
    return;
  }

  // Default percentage profile (can be tuned)
  int openPct   = fullOpen ? 12 : 18;
  int holdPct   = fullOpen ? 60 : 18;
  int closePct  = fullOpen ? 15 : 34;
  int settlePct = 100 - (openPct + holdPct + closePct);

  int openDur  = max(10, (syllableMs * openPct) / 100);
  int holdOpen = max(12, (syllableMs * holdPct) / 100);
  int closeDur = max(8,  (syllableMs * closePct) / 100);
  int settle   = syllableMs - (openDur + holdOpen + closeDur);
  if (settle < 0) settle = 0;

  // If the next syllable will be a full-open (e.g., "for" -> "me"), make this close snappier
  // and ensure a short closed hold so the next open isn't staggered.
  if (nextIsFullOpen) {
    // shorten the closing window, increase closed hold
    closeDur = max(6, closeDur / 2);           // much quicker close
    int closedHold = max(30, syllableMs * 10 / 100); // e.g., 10% but at least 30ms

    // Execute: open -> hold -> fast close -> hold closed -> settle remainder
    moveTo(openTarget, openDur);
    if (holdOpen > 0) delay(holdOpen);

    moveTo(JAW_CLOSED, closeDur);
    mouth.write(JAW_CLOSED);
    delay(closedHold);

    int used = openDur + holdOpen + closeDur + closedHold;
    if (used < syllableMs) delay(syllableMs - used);
    return;
  }

  // Normal path: open -> hold -> close -> settle
  moveTo(openTarget, openDur);
  if (holdOpen > 0) delay(holdOpen);
  moveTo(JAW_CLOSED, closeDur);
  if (settle > 0) delay(settle);
  mouth.write(JAW_CLOSED);
}



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

  DF1201S.setVol(/*VOL = */12);
  DF1201S.switchFunction(DF1201S.MUSIC);
}

void loop() {
  DF1201S.setPlayTime(0); // only works in music mode

  int N = sizeof(yohoMovements)/sizeof(yohoMovements[0]);
for (int i = 0; i < N; ++i) {
    bool nextIsFullOpen = (i + 1 < N) && (yohoMovements[i + 1] == JAW_OPEN);
    bool holdOpenUntilEnd = (i == N - 1); // last syllable "me"

    moveSyllableBounce(yohoMovements[i], yoHodelays[i], nextIsFullOpen, holdOpenUntilEnd);
}

   
  mouth.write(JAW_CLOSED); 

  DF1201S.pause();
  delay(10000); // pause 10 seconds before repeating / trigger pause
}
