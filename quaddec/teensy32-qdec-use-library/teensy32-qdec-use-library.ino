#include <Encoder.h>
#include "Data.h"
#include "RingBuf.h"
#include "ADC.h"
#include "Timer.h"

const int  sampleRate       = 16000;
const long serialRate       = 2000000;
const int  bufferCapacity   = 256;
const int  POSITION_CLOCK   = 4;
const int  POSITION_ENC_A   = 5;
const int  POSITION_ENC_B   = 6;
const int  POSITION_RST     = 7;
const int  POSITION_CAL     = 8;
const int  POSITION_RST_VAL = 10000;
Encoder positionEnc(POSITION_ENC_A, POSITION_ENC_B);
RingBuf<MODEL, bufferCapacity> rbuf;

bool positionReset = false;
bool positionCalibrate = false;

struct BOUND {
  int value; 
  int since;
};

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(POSITION_RST, INPUT_PULLUP);
  pinMode(POSITION_CAL, INPUT_PULLUP);
  pinMode(POSITION_CLOCK, OUTPUT);
  //analogWriteFrequency(POSITION_CLOCK, 1000);
  attachInterrupt(digitalPinToInterrupt(POSITION_RST), doPositionReset, CHANGE);
  attachInterrupt(digitalPinToInterrupt(POSITION_CAL), doPositionCalibrate, CHANGE);
  
  SerialUSB.begin(serialRate);

  Analog::setup();
  Timer::setup(sampleRate, measure);
  positionEnc.write(POSITION_RST_VAL);
}

void doPositionReset() {
  positionReset = true;
}

void doPositionCalibrate() {
  positionCalibrate = true;
}

void loop() {
  static int counter = 0;
  bool state = (counter++ >> 8) % 2;
  digitalWrite(LED_BUILTIN, state ? HIGH : LOW);
  push();
  delayMicroseconds(5);
}

inline void push() {
  while (rbuf.hasElements()) {
    MODEL m = rbuf.pop();
    for (int i = 0; i < NWORDS; i++) {
      if (i != 0) SerialUSB.print('\t');
      SerialUSB.print(m.words[i], DEC);
    }
    SerialUSB.write('\n');
  }
}

void measure () {
  static bool initialized = false;
  static int gMin;
  static int gMax;
  static BOUND TOP, BOT;
  MODEL m;
  
  if(positionReset) {
    initialized = false;
    positionReset = false;
  }

  int x = positionEnc.read();

  if(positionCalibrate && x == gMin) {
    positionEnc.write(POSITION_RST_VAL);
    positionCalibrate = false;
  }

  if(!initialized) {
    gMin = gMax = x;
    TOP.since = 0;
    BOT.since = 0;
    initialized = true;
  }

  gMax = max(x, gMax);
  if(TOP.value == gMax && TOP.since++ > sampleRate * 2) {
    TOP.since = 0;
    TOP.value = min(x, TOP.value * (double) 0.9);
    gMax = x;
  }
  TOP.value = gMax;

  gMin = min(x, gMin);
  if(BOT.value == gMin && BOT.since++ > sampleRate * 2) {
    BOT.since = 0;
    BOT.value = max(x, BOT.value * (double) 1.1);
    gMin = x;
  }
  BOT.value = gMin;

  Analog::read(A0);
  Analog::read(A1);

  m.words[Col::Range] = BOT.value;
  m.words[Col::Data0] = BOT.value;
  m.words[Col::Data1] = x;
  m.words[Col::Data2] = TOP.value;

  rbuf.push(m);
}


