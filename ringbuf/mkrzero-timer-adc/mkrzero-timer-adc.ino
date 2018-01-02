//#define USE_DIAGNOSTICS
//#define USE_BINARY

//           DIAGNOSTICS
//              no    yes
// UART bin  20000  17000
// UART txt   4400   3000

#include "wiring_private.h"
#include "Data.h"
#include "RingBuf.h"
#include "ADC.h"
#include "Timer.h"

const int  sampleRate     = 3000;
const long serialRate     = 2000000;
const int  bufferCapacity = 2048;
RingBuf<MODEL, bufferCapacity> rbuf;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  SerialUSB.begin(serialRate);

  Analog::setup();
  Timer::setup(sampleRate, measure);
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
#ifdef USE_BINARY
    SerialUSB.write(rbuf.isFull() ? 'f' : 'n');
    SerialUSB.write(m.bytes, NBYTES);
#else
    SerialUSB.print(rbuf.size());
    for (int i = 0; i < NWORDS; i++) {
      SerialUSB.print('\t');
      SerialUSB.print(m.words[i], DEC);
    }
#endif
    SerialUSB.write('\n');
  }
}

void measure () {
  static long counter = 0;
  MODEL m;
  m.words[Col::Id] = ++counter % 256;
  m.words[Col::Data0] = Analog::read(A0);
  m.words[Col::Data1] = Analog::read(A1);
  m.words[Col::Data2] = Analog::read(A2);

#ifdef USE_DIAGNOSTICS
  static long t0 = micros();
  long t1 = micros();
  long interruptLength = t1 - t0;
  t0 = t1;
  m.words[Col::Time] = t1 % 256;
  m.words[Col::InterruptLength] = interruptLength;
  m.words[Col::SampleLength] = micros() - t1;
#endif

  rbuf.push(m);
}


