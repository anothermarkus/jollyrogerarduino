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

// Arduino Transmits on Pin 3 Receives on Pin 2
// DFPlayer RX is wired to Pin 3 (TX -> RX)
// DFPlayer TX is wired to Pin 2 (RX -> TX)
// Cross wiring is intentional
SoftwareSerial DF1201SSerial(2, 3);  //RX TX
#else
#define DF1201SSerial Serial1
#endif

DFRobot_DF1201S DF1201S;

Servo mouth;
const int servoPin = 9;
const int triggerPin = 10;
const int echoPin = 13;
const int LEDPin = 8;


// Jaw constants (flipped servo: smaller = more open)
const int JAW_OPEN = 0;
const int JAW_4 = 9;
const int JAW_3 = 18;
const int JAW_2 = 25;
const int JAW_1 = 30;
const int JAW_CLOSED = 45;

// Dead Men Tell No Tales 
int deadMenTellNoTales[] = { JAW_2, JAW_OPEN, JAW_1, JAW_3, JAW_OPEN, JAW_2, JAW_1, JAW_OPEN, JAW_2, JAW_OPEN, JAW_2, JAW_1, JAW_OPEN, JAW_3, JAW_2 };
int deadMenTellNoTalesDelays[] = { 179, 491, 268, 268, 357, 268, 179, 357, 268, 491, 268, 179, 491, 268, 670 };


// -------------------- Phrase arrays --------------------
// "Yo ho, yo ho, a pirate's life for me"
const int yohoMovements[] = { JAW_OPEN, JAW_OPEN, JAW_OPEN, JAW_OPEN, JAW_4, JAW_3, JAW_3, JAW_3, JAW_3, JAW_OPEN };
const int yoHoDelays[] = { 450, 450, 450, 450, 350, 350, 250, 350, 250, 550 };

// "We pillage plunder, we rifle and loot"
const int pillageMovements[] = { JAW_4, JAW_3, JAW_OPEN, JAW_3, JAW_OPEN, JAW_4, JAW_3, JAW_OPEN, JAW_2, JAW_OPEN };
const int pillageDelays[] = { 150, 170, 230, 170, 230, 150, 170, 230, 150, 350 };

// "Drink up me 'earties, yo ho"
const int drinkMovements[] = { JAW_2, JAW_OPEN, JAW_2, JAW_4, JAW_2, JAW_4, JAW_OPEN };
const int drinkDelays[] = { 230, 180, 170, 250, 200, 280, 290 };

// "We kidnap and ravage and don't give a hoot"
const int kidnapMovements[] = { JAW_3, JAW_2, JAW_3, JAW_2, JAW_3, JAW_2, JAW_2, JAW_3, JAW_2, JAW_3, JAW_OPEN };
const int kidnapDelays[] = { 160, 190, 190, 160, 200, 180, 160, 200, 160, 250, 250 };

// "We extort and pilfer, we filch and sack"
const int extortMovements[] = { JAW_3, JAW_3, JAW_2, JAW_2, JAW_3, JAW_OPEN, JAW_2, JAW_4, JAW_2, JAW_OPEN };
const int extortDelays[] = { 180, 260, 200, 170, 260, 200, 180, 270, 170, 150 };

// "Maraud and embezzle and even highjack"
const int maraudMovements[] = { JAW_OPEN, JAW_2, JAW_2, JAW_OPEN, JAW_3, JAW_2, JAW_2, JAW_OPEN, JAW_3, JAW_OPEN, JAW_2 };
const int maraudDelays[] = { 178, 178, 154, 178, 178, 166, 154, 178, 178, 209, 246 };

// "We kindle and char and enflame and ignite"
const int kindleMovements[] = { JAW_3, JAW_2, JAW_3, JAW_2, JAW_OPEN, JAW_2, JAW_OPEN, JAW_3, JAW_2, JAW_2, JAW_OPEN, JAW_3 };
const int kindleDelays[] = { 181, 181, 158, 158, 203, 158, 181, 181, 169, 158, 214, 259 };

// "We burn up the city, we're really a fright"
const int burnMovements[] = { JAW_3, JAW_2, JAW_OPEN, JAW_3, JAW_2, JAW_OPEN, JAW_3, JAW_2, JAW_OPEN };
const int burnDelays[] = { 206, 194, 218, 206, 194, 242, 206, 218, 315 };

// "We're rascals and scoundrels, we're villains and knaves"
const int rascalMovements[] = { JAW_3, JAW_2, JAW_3, JAW_2, JAW_3, JAW_OPEN, JAW_3, JAW_2, JAW_3, JAW_2, JAW_OPEN };
const int rascalDelays[] = { 190, 180, 190, 170, 200, 260, 180, 190, 200, 170, 270 };

// "We're devils and black sheep, we're really bad eggs"
const int devilsMovements[] = { JAW_3, JAW_2, JAW_3, JAW_2, JAW_OPEN, JAW_3, JAW_2, JAW_OPEN, JAW_3, JAW_2, JAW_OPEN };
const int devilsDelays[] = { 190, 180, 200, 170, 250, 190, 180, 250, 200, 180, 260 };

// "We're beggars and blighters and ne'er do-well cads"
const int beggarsMovements[] = { JAW_3, JAW_2, JAW_3, JAW_OPEN, JAW_2, JAW_3, JAW_2, JAW_3, JAW_2, JAW_OPEN, JAW_3, JAW_OPEN };
const int beggarsDelays[] = { 190, 170, 180, 210, 170, 190, 170, 180, 170, 210, 200, 260 };

// "Aye, but we're loved by our mommies and dads"
const int ayeMovements[] = { JAW_OPEN, JAW_2, JAW_3, JAW_2, JAW_OPEN, JAW_3, JAW_2, JAW_3, JAW_OPEN };
const int ayeDelays[] = { 210, 170, 190, 170, 220, 190, 170, 210, 270 };






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
  int delayMs = (duration_ms + loopCount / 2) / max(1, loopCount);
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
    int openDur = max(10, syllableMs * 10 / 100);
    int holdMs = syllableMs - openDur - closeDur;
    moveTo(openTarget, openDur);
    if (holdMs > 0) delay(holdMs);
    moveTo(JAW_CLOSED, closeDur);
    mouth.write(JAW_CLOSED);
    return;
  }

  int openPct = fullOpen ? 12 : 18;
  int holdPct = fullOpen ? 60 : 18;
  int closePct = fullOpen ? 15 : 34;

  int openDur = max(10, (syllableMs * openPct) / 100);
  int holdOpen = max(12, (syllableMs * holdPct) / 100);
  int closeDur = max(8, (syllableMs * closePct) / 100);
  int settle = syllableMs - (openDur + holdOpen + closeDur);
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

  Serial.begin(115200); // This declaration occupies pin 0 and 1 for serial communication Serial.println()

  pinMode(triggerPin, OUTPUT);
  pinMode(echoPin, INPUT);

  DF1201SSerial.begin(115200);

  while (!DF1201S.begin(DF1201SSerial)) {
    Serial.println("Init failed, please check the wire connection!");
    delay(1000);
  }

  DF1201S.disableAMP();

  DF1201S.setVol(50);
  DF1201S.switchFunction(DF1201S.MUSIC);
  DF1201S.setPlayMode(DF1201S.SINGLE);
}


void playYoHoVerse1() {
  playPhrase(yohoMovements, yoHoDelays, sizeof(yohoMovements) / sizeof(int));
  delay(250);
  playPhrase(pillageMovements, pillageDelays, sizeof(pillageMovements) / sizeof(int));
  delay(250);
  playPhrase(drinkMovements, drinkDelays, sizeof(drinkMovements) / sizeof(int));
  delay(250);
  playPhrase(kidnapMovements, kidnapDelays, sizeof(kidnapMovements) / sizeof(int));
  delay(250);
  playPhrase(drinkMovements, drinkDelays, sizeof(drinkMovements) / sizeof(int));
  delay(750);
}

void playYoHoVerse2() {
  playPhrase(yohoMovements, yoHoDelays, sizeof(yohoMovements) / sizeof(int));
  delay(250);
  playPhrase(extortMovements, extortDelays, sizeof(extortMovements) / sizeof(int));
  delay(50);
  playPhrase(drinkMovements, drinkDelays, sizeof(drinkMovements) / sizeof(int));
  delay(600);
  playPhrase(maraudMovements, maraudDelays, sizeof(maraudMovements) / sizeof(int));
  delay(100);
  playPhrase(drinkMovements, drinkDelays, sizeof(drinkMovements) / sizeof(int));
  delay(750);
}

void playYoHoVerse3() {
  playPhrase(yohoMovements, yoHoDelays, sizeof(yohoMovements) / sizeof(int));
  delay(250);
  playPhrase(kindleMovements, kindleDelays, sizeof(kindleMovements) / sizeof(int));
  delay(150);
  playPhrase(drinkMovements, drinkDelays, sizeof(drinkMovements) / sizeof(int));
  delay(350);
  playPhrase(burnMovements, burnDelays, sizeof(burnMovements) / sizeof(int));
  delay(150);
  playPhrase(drinkMovements, drinkDelays, sizeof(drinkMovements) / sizeof(int));
  delay(550);
}

void playYoHoVerse4() {
  playPhrase(rascalMovements, rascalDelays, sizeof(rascalMovements) / sizeof(int));
  delay(150);
  playPhrase(drinkMovements, drinkDelays, sizeof(drinkMovements) / sizeof(int));
  delay(150);
  playPhrase(devilsMovements, devilsDelays, sizeof(devilsMovements) / sizeof(int));
  delay(150);
  playPhrase(drinkMovements, drinkDelays, sizeof(drinkMovements) / sizeof(int));
  delay(650);
}

void playYoHoVerse5() {
  playPhrase(yohoMovements, yoHoDelays, sizeof(yohoMovements) / sizeof(int));
  delay(350);
  playPhrase(beggarsMovements, beggarsDelays, sizeof(beggarsMovements) / sizeof(int));
  delay(50);
  playPhrase(drinkMovements, drinkDelays, sizeof(drinkMovements) / sizeof(int));
  delay(350);
  playPhrase(ayeMovements, ayeDelays, sizeof(ayeMovements) / sizeof(int));
  delay(150);
  playPhrase(drinkMovements, drinkDelays, sizeof(drinkMovements) / sizeof(int));
  delay(550);
}

void playYoHoSong() {
  DF1201S.playFileNum(1);
  DF1201S.setPlayTime(0);

  playYoHoVerse1();
  playYoHoVerse2();
  playYoHoVerse3();
  playYoHoVerse4();
  playYoHoVerse5();

  mouth.write(JAW_CLOSED);
  DF1201S.pause();
}


void playDeadMen() {
  DF1201S.playFileNum(2);
  DF1201S.setPlayTime(0);  // only works in music mode

  for (int i = 0; i < sizeof(deadMenTellNoTales) / sizeof(deadMenTellNoTales[0]); i++) {
    moveTo(deadMenTellNoTales[i], 8);  // smooth move (~8ms per step)
    delay(deadMenTellNoTalesDelays[i]);      // hold at position
  }
}


void loop() {
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);

  float duration, distance;

  duration = pulseIn(echoPin, HIGH);

  distance = (duration / 2) * 0.0343;  // speed of sound constant

  if (distance <= 100 && distance >= 2) {
    // something is here
    digitalWrite(LEDPin, HIGH);  // LED to indicate, in range
    DF1201S.setVol(50);
    playDeadMen();
    delay(4000);
    playYoHoSong();
    DF1201S.pause();
    DF1201S.setVol(0); // This is a hack as .pause does not work in this scenario
    digitalWrite(LEDPin, LOW); // LED to indiciate, loop is finished
    delay(10000);
  }

  delay(1000);  // loop agressively
}
