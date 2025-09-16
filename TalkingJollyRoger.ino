/*
  realistic_yoho_jaw_refactored.ino
  Realistic jaw movement for multiple "Yo Ho..." phrases.
  Uses a helper function to avoid duplicated loops.
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

// Jaw constants (flipped servo: smaller = more open)
const int JAW_OPEN   = 0;
const int JAW_4      = 9;   
const int JAW_3      = 18;  
const int JAW_2      = 25;  
const int JAW_1      = 30;  
const int JAW_CLOSED = 45; 

// -------------------- Phrase arrays --------------------
// Phrase 1: "Yo ho, yo ho, a pirate's life for me"
const int yohoMovements[] = { JAW_OPEN, JAW_OPEN, JAW_OPEN, JAW_OPEN, JAW_4, JAW_3, JAW_3, JAW_3, JAW_3, JAW_OPEN };
const int yoHodelays[]    = { 450, 450, 450, 450, 350, 350, 250, 350, 250, 550 };

// Phrase 2: "We pillage plunder, we rifle and loot"
const int pillageMovements[] = { JAW_4, JAW_3, JAW_OPEN, JAW_3, JAW_OPEN, JAW_4, JAW_3, JAW_OPEN, JAW_2, JAW_OPEN };
const int pillageDelays[]    = { 150, 170, 230, 170, 230, 150, 170, 230, 150, 350 };

// Phrase 3: "Drink up me 'earties, yo ho"
const int drinkMovements[] = { JAW_2, JAW_OPEN, JAW_2, JAW_4, JAW_2, JAW_4, JAW_OPEN };
const int drinkDelays[]    = { 230, 180, 170, 250, 200, 280, 290 };

// Phrase 4: "We kidnap and ravage and don't give a hoot"
const int kidnapMovements[] = { JAW_3, JAW_2, JAW_3, JAW_2, JAW_3, JAW_2, JAW_2, JAW_3, JAW_2, JAW_3, JAW_OPEN };
const int kidnapDelays[]    = { 160, 190, 190, 160, 200, 180, 160, 200, 160, 250, 250 };

// -------------------- Movement helpers --------------------
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
    stepInc = (steps + duration_ms - 1) / duration_ms;
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

void moveSyllableBounce(int openTarget, int syllableMs, bool nextIsFullOpen = false, bool holdOpenUntilEnd = false) {
  if (syllableMs <= 40) {
    moveTo(openTarget, 12);
    if (syllableMs > 12) delay(syllableMs - 12);
    mouth.write(JAW_CLOSED);
    delay(40);
    return;
  }

  bool fullOpen = (openTarget == JAW_OPEN);

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

  int openPct   = fullOpen ? 12 : 18;
  int holdPct   = fullOpen ? 60 : 18;
  int closePct  = fullOpen ? 15 : 34;

  int openDur  = max(10, (syllableMs * openPct) / 100);
  int holdOpen = max(12, (syllableMs * holdPct) / 100);
  int closeDur = max(8,  (syllableMs * closePct) / 100);
  int settle   = syllableMs - (openDur + holdOpen + closeDur);
  if (settle < 0) settle = 0;

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

  moveTo(openTarget, openDur);
  if (holdOpen > 0) delay(holdOpen);
  moveTo(JAW_CLOSED, closeDur);
  if (settle > 0) delay(settle);
  mouth.write(JAW_CLOSED);
}

// -------------------- New helper to play a phrase --------------------
void playPhrase(const int *movements, const int *delays, int count, bool finalHold = true) {
  for (int i = 0; i < count; ++i) {
    bool nextIsFullOpen = (i + 1 < count) && (movements[i + 1] == JAW_OPEN);
    bool holdOpenUntilEnd = finalHold && (i == count - 1);
    moveSyllableBounce(movements[i], delays[i], nextIsFullOpen, holdOpenUntilEnd);
  }
}

// -------------------- Setup & loop --------------------
void setup() {
  mouth.attach(servoPin);
  mouth.write(JAW_CLOSED);
  delay(500);

  Serial.begin(115200);
#if (defined ESP32)
  DF1201SSerial.begin(115200, SERIAL_8N1, /*rx=*/D3, /*tx=*/D2);
#else
  DF1201SSerial.begin(115200);
#endif
  while (!DF1201S.begin(DF1201SSerial)) {
    Serial.println("Init failed, please check the wire connection!");
    delay(1000);
  }

  DF1201S.setVol(12);
  DF1201S.switchFunction(DF1201S.MUSIC);
}

void loop() {
  DF1201S.setPlayTime(0);

  playPhrase(yohoMovements,    yoHodelays,    sizeof(yohoMovements)/sizeof(int));
  delay(250);
  playPhrase(pillageMovements, pillageDelays, sizeof(pillageMovements)/sizeof(int));
  delay(250);
  playPhrase(drinkMovements,   drinkDelays,   sizeof(drinkMovements)/sizeof(int));
  delay(250);
  playPhrase(kidnapMovements,  kidnapDelays,  sizeof(kidnapMovements)/sizeof(int));
  delay(250);
  playPhrase(drinkMovements,   drinkDelays,   sizeof(drinkMovements)/sizeof(int));
  delay(750);
  playPhrase(yohoMovements,    yoHodelays,    sizeof(yohoMovements)/sizeof(int));

  mouth.write(JAW_CLOSED);
  DF1201S.pause();
  delay(10000);
}
