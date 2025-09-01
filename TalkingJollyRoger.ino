/*
  realistic_deadmen_jaw.ino
  Realistic slow jaw movement for "dead men tell no tales" (~11s).
  Uses multiple jaw positions and smooth transitions.
  Servo signal -> pin 9. Use external 5-6V supply for servo; connect grounds together.
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

// Jaw positions (scaled 0–45°)
// const int JAW_CLOSED = 0;
// const int JAW_1 = 9;   
// const int JAW_2 = 18;  
// const int JAW_3 = 27;  
// const int JAW_4 = 36;  
// const int JAW_OPEN = 45; 

const int JAW_OPEN = 0;
const int JAW_4 = 9;   
const int JAW_3 = 18;  
const int JAW_2 = 27;  
const int JAW_1 = 36;  
const int JAW_CLOSED = 45; 


// Timing (ms)
const int SNAP   = 80;   // very fast closure
const int QUICK  = 120;  
const int SHORT  = 160;  
const int MED    = 220;  
const int LONG   = 300;  

// Mouth sequence for "Dead Men Tell No Tales"
int phrase[] = {
  // "Dead"
  JAW_2,   // D (quick snap, not full close)
  JAW_OPEN, // ea (vowel, open wide)
  JAW_1,   // d (end consonant, partial close)

  // "Men"
  JAW_3,   // M (mid)
  JAW_OPEN, // e (open)
  JAW_2,   // n (half-close)

  // "Tell"
  JAW_1,   // T (snap shut-ish)
  JAW_OPEN, // e (open vowel)
  JAW_2,   // l (relax close)

  // "No"
  JAW_OPEN, // N + o (smooth wide)
  JAW_2,    // soft close

  // "Tales"
  JAW_1,    // T (snap)
  JAW_OPEN, // a (wide open)
  JAW_3,    // l (soft mid)
  JAW_2     // s (resting close)
};

int delays[] = {
  // Dead
  SNAP, MED, QUICK,
  // Men
  QUICK, SHORT, QUICK,
  // Tell
  SNAP, SHORT, QUICK,
  // No
  MED, QUICK,
  // Tales
  SNAP, MED, QUICK, LONG
};

// Smooth motion function
void moveTo(int target, int stepDelay = 10) {
  int current = mouth.read(); // last commanded position
  if (current < target) {
    for (int pos = current; pos <= target; pos++) {
      mouth.write(pos);
      delay(stepDelay);
    }
  } else {
    for (int pos = current; pos >= target; pos--) {
      mouth.write(pos);
      delay(stepDelay);
    }
  }
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

  DF1201S.setVol(/*VOL = */15);
  DF1201S.switchFunction(DF1201S.MUSIC);
}

void loop() {
  DF1201S.setPlayTime(0); // only works in music mode

  for (int i = 0; i < sizeof(phrase)/sizeof(phrase[0]); i++) {
    moveTo(phrase[i], 8);  // smooth move (~8ms per step)
    delay(delays[i]);      // hold at position
  }
   
  mouth.write(JAW_CLOSED); 

  DF1201S.pause();
  delay(10000); // pause 10 seconds before repeating / trigger pause
}


