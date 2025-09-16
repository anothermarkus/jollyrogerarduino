/*
  realistic_yoho_jaw_with_pillage.ino
  Realistic jaw movement for "Yo Ho..." then "We pillage plunder, we rifle and loot".
  Drop-in replacement for your current sketch.
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

// Jaw constants (your flipped servo: smaller = more open)
const int JAW_OPEN = 0;
const int JAW_4 = 9;   
const int JAW_3 = 18;  
const int JAW_2 = 25;  
const int JAW_1 = 30;  
const int JAW_CLOSED = 45; 

// -------------------- Phrase 1 (your existing Yo ho...) --------------------
const int yohoMovements[] = {
  JAW_OPEN,    // 1: "Yo"
  JAW_OPEN,    // 2: "ho"
  JAW_OPEN,    // 3: "yo"
  JAW_OPEN,    // 4: "ho"
  JAW_4,       // 5: "a"
  JAW_3,       // 6: "pi-"
  JAW_3,       // 7: "-rate's"
  JAW_3,       // 8: "life"
  JAW_3,       // 9: "for"
  JAW_OPEN     // 10: "me"
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
  550  // me
};

// -------------------- Phrase 2 (We pillage plunder, we rifle and loot) -----
const int pillageMovements[] = {
  JAW_4,    // 1: "We"
  JAW_3,    // 2: "pil-"
  JAW_OPEN, // 3: "-lage"
  JAW_3,    // 4: "plun-"
  JAW_OPEN, // 5: "-der"
  JAW_4,    // 6: "we"
  JAW_3,    // 7: "ri-"
  JAW_OPEN, // 8: "-fle"
  JAW_2,    // 9: "and"
  JAW_OPEN  //10: "loot"
};

const int pillageDelays[] = {
  150, // We
  170, // pil-
  230, // -lage
  170, // plun-
  230, // -der
  150, // we
  170, // ri-
  230, // -fle
  150, // and
  350  // loot
};

const int drinkMovements[] = {
  JAW_2,    // 1: "Drink"
  JAW_OPEN,    // 2: "up"
  JAW_2,    // 3: "me"
  JAW_4, // 4: "'ear-"
  JAW_2,    // 5: "-ties"
  JAW_4, // 7: "yo"
  JAW_OPEN  // 8: "ho"
};

// Delays sum = 2000 ms
const int drinkDelays[] = {
  230, // Drink (heavier open at start)
  180, // up (short, quick)
  170, // me (light)
  250, // 'ear- (accented, longer)
  200, // -ties (snappy)
  280, // yo (hold open wide)
  290  // ho (hold open wide, end strong)
};

const int kidnapMovements[] = {
  JAW_3,    // 1: "We"
  JAW_2,    // 2: "kid-"
  JAW_3,    // 3: "-nap"
  JAW_2,    // 4: "and"
  JAW_3,    // 5: "rav-"
  JAW_2,    // 6: "-age"
  JAW_2,    // 7: "and"
  JAW_3,    // 8: "don't"
  JAW_2,    // 9: "give"
  JAW_3,    // 10: "a"
  JAW_OPEN  // 11: "hoot"
};

// total ≈ 2000 ms
const int kidnapDelays[] = {
  160, // We
  190, // kid-
  190, // -nap
  160, // and
  200, // rav-
  180, // -age
  160, // and
  200, // don't
  160, // give
  250, // a
  250  // hoot (strong hold at the end)
};




// -------------------- Movement helpers (your improved functions) ----------
void moveTo(int target, int duration_ms = 120) {
  int current = mouth.read();
  int steps = abs(target - current);
  if (steps == 0) {
    if (duration_ms > 0) delay(duration_ms);
    return;
  }

  int dir = (target > current) ? 1 : -1;
  int stepInc = 1;
  if (duration_ms > 0 && duration_ms < steps) {
    stepInc = (steps + duration_ms - 1) / duration_ms; // ceil
  }

  int loopCount = (steps + stepInc - 1) / stepInc;
  int delayMs = (duration_ms + loopCount/2) / max(1, loopCount);
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

// moveSyllableBounce: improved behavior including faster close when next is full open
void moveSyllableBounce(int openTarget, int syllableMs, bool nextIsFullOpen = false, bool holdOpenUntilEnd = false) {
  if (syllableMs <= 40) {
    moveTo(openTarget, 12);
    if (syllableMs > 12) delay(syllableMs - 12);
    mouth.write(JAW_CLOSED);
    delay(40);
    return;
  }

  bool fullOpen = (openTarget == JAW_OPEN);

  // Special case: hold open until nearly the very end (for final syllable 'me' / 'loot' if desired)
  if (holdOpenUntilEnd && fullOpen) {
    int closeDur = max(6, syllableMs / 20);
    int openDur  = max(10, syllableMs * 10 / 100);
    int holdMs   = syllableMs - openDur - closeDur;
    moveTo(openTarget, openDur);
    if (holdMs > 0) delay(holdMs);
    moveTo(JAW_CLOSED, closeDur);
    mouth.write(JAW_CLOSED);
    return;
  }

  // Default percentage profile
  int openPct   = fullOpen ? 12 : 18;
  int holdPct   = fullOpen ? 60 : 18;
  int closePct  = fullOpen ? 15 : 34;

  int openDur  = max(10, (syllableMs * openPct) / 100);
  int holdOpen = max(12, (syllableMs * holdPct) / 100);
  int closeDur = max(8,  (syllableMs * closePct) / 100);
  int settle   = syllableMs - (openDur + holdOpen + closeDur);
  if (settle < 0) settle = 0;

  // If the next syllable will be a full-open, make this close snappier and hold closed a bit
  if (nextIsFullOpen) {
    closeDur = max(6, closeDur / 2);
    int closedHold = max(30, syllableMs * 10 / 100);

    moveTo(openTarget, openDur);
    if (holdOpen > 0) delay(holdOpen);

    moveTo(JAW_CLOSED, closeDur);
    mouth.write(JAW_CLOSED);
    delay(closedHold);

    int used = openDur + holdOpen + closeDur + closedHold;
    if (used < syllableMs) delay(syllableMs - used);
    return;
  }

  // Normal path
  moveTo(openTarget, openDur);
  if (holdOpen > 0) delay(holdOpen);
  moveTo(JAW_CLOSED, closeDur);
  if (settle > 0) delay(settle);
  mouth.write(JAW_CLOSED);
}

// -------------------- Setup & main loop ----------------------------------
bool played = false;

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

  // Phrase 1: "Yo ho, yo ho, a pirate's life for me"
  int N1 = sizeof(yohoMovements)/sizeof(yohoMovements[0]);
  for (int i = 0; i < N1; ++i) {
    bool nextIsFullOpen = (i + 1 < N1) && (yohoMovements[i + 1] == JAW_OPEN);
    bool holdOpenUntilEnd = (i == N1 - 1); // hold last "me" until end
    moveSyllableBounce(yohoMovements[i], yoHodelays[i], nextIsFullOpen, holdOpenUntilEnd);
  }

  // small musical gap between phrases (adjust if you need tighter coupling)
  delay(250);

  // Phrase 2: "We pillage plunder, we rifle and loot"
  int N2 = sizeof(pillageMovements)/sizeof(pillageMovements[0]);
  for (int i = 0; i < N2; ++i) {
    bool nextIsFullOpen = (i + 1 < N2) && (pillageMovements[i + 1] == JAW_OPEN);
    bool holdOpenUntilEnd = (i == N2 - 1); // hold final "loot"
    moveSyllableBounce(pillageMovements[i], pillageDelays[i], nextIsFullOpen, holdOpenUntilEnd);
  }

   // small musical gap between phrases (adjust if you need tighter coupling)
  delay(250);

   // Phrase 3: "Drink up me 'earties, yo ho"
  int N3 = sizeof(drinkMovements)/sizeof(drinkMovements[0]);
  for (int i = 0; i < N3; ++i) {
    bool nextIsFullOpen = (i + 1 < N3) && (drinkMovements[i + 1] == JAW_OPEN);
    bool holdOpenUntilEnd = (i == N3 - 1); // hold final "loot"
    moveSyllableBounce(drinkMovements[i], drinkDelays[i], nextIsFullOpen, holdOpenUntilEnd);
  }

   delay(250);


  // Phrase 4: "We kidnap and ravage and don't give a hoot"
  int N4 = sizeof(kidnapMovements)/sizeof(kidnapMovements[0]);
  for (int i = 0; i < N4; ++i) {
    bool nextIsFullOpen = (i + 1 < N4) && (kidnapMovements[i + 1] == JAW_OPEN);
    bool holdOpenUntilEnd = (i == N4 - 1); // hold final "loot"
    moveSyllableBounce(kidnapMovements[i], kidnapDelays[i], nextIsFullOpen, holdOpenUntilEnd);
  }

  delay(250);

 // Phrase 5: "Drink up me 'earties, yo ho"
  for (int i = 0; i < N3; ++i) {
    bool nextIsFullOpen = (i + 1 < N3) && (drinkMovements[i + 1] == JAW_OPEN);
    bool holdOpenUntilEnd = (i == N3 - 1); // hold final "loot"
    moveSyllableBounce(drinkMovements[i], drinkDelays[i], nextIsFullOpen, holdOpenUntilEnd);
  }

  delay(750);

  // Phrase 6: ""Yo ho, yo ho, a pirate's life for me"
  for (int i = 0; i < N1; ++i) {
    bool nextIsFullOpen = (i + 1 < N1) && (yohoMovements[i + 1] == JAW_OPEN);
    bool holdOpenUntilEnd = (i == N1 - 1); // hold last "me" until end
    moveSyllableBounce(yohoMovements[i], yoHodelays[i], nextIsFullOpen, holdOpenUntilEnd);
  }



  // close jaw & stop audio, mark as played
  mouth.write(JAW_CLOSED);

  DF1201S.pause();
  delay(10000); // pause 10 seconds before repeating / trigger pause
}
