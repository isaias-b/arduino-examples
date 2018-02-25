#include <FrequencyTimer2.h>
#include <ADC.h>
#include "Data.h"
#include "RingBuf.h"
#include "ADC.h"
#include "Timer.h"
#include "Qdec.h"

const int  sampleRate       = 10;
const long serialRate       = 2000000;
const int  bufferCapacity   = 256;
const long positionFreq     = 600000;

RingBuf<MODEL, bufferCapacity> rbuf;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  
  SerialUSB.begin(serialRate);

  Analog::setup();
  Timer::setup(sampleRate, measure);
  Qdec::setup(positionFreq);
}

void loop() {
  static int counter = 0;
  bool state = (counter++ >> 8) % 2;
  digitalWriteFast(LED_BUILTIN, state ? HIGH : LOW);
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
  MODEL m;
  
  Analog::readSynchronizedContinuous();

  m.words[Col::Data0] = Analog::Data0();
  m.words[Col::Data1] = Analog::Data1();
  m.words[Col::Range] = Qdec::range();
  m.words[Col::Min]   = Qdec::gMin;
  m.words[Col::Pos]   = Qdec::counter;
  m.words[Col::Max]   = Qdec::gMax;

  Analog::clearErrors();
  
  rbuf.push(m);
}


