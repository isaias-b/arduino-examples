//#define USE_DIAGNOSTICS
//#define USE_BINARY
//#define USE_SPI

const int sampleRate = 3700;
const long serialRate = 2000000;
const long spiClockRate = 12000000;
const int bufferCapacity = 2048;

//           DIAGNOSTICS
//           no     yes
// SPI       23500  16000
// UART bin  20000  17000
// UART       4400   3000

#include <SPI.h>
#include "wiring_private.h"
#include "Data.h"
#include "RingBuf.h"
#include "ADC.h"
#include "Timer.h"


SPISettings settings = SPISettings(spiClockRate, MSBFIRST, SPI_MODE0);
RingBuf<MODEL, bufferCapacity> rbuf;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(serialRate);
  SPI.begin();

  Analog::setup();
  Timer::setup(sampleRate, measure);
}

void loop() {
  static int counter = 0;
  bool state = (counter++ >> 5) % 2;
  digitalWrite(LED_BUILTIN, state ? HIGH : LOW);
#ifdef USE_SPI
  pushSPI();
#else
  pushUART();
#endif
  delayMicroseconds(5);
}

inline void pushSPI() {
  while (rbuf.hasElements()) {
    MODEL m = rbuf.pop();
    SPI.beginTransaction(settings);
    SPI.transfer(rbuf.size());
    for (int i = 0; i < NBYTES; i++)
      SPI.transfer(m.bytes[i]);
    SPI.endTransaction();
  }
}

inline void pushUART() {
  while (rbuf.hasElements()) {
    MODEL m = rbuf.pop();
#ifdef USE_BINARY
    Serial.write(rbuf.isFull() ? 'f' : 'n');
    Serial.write(m.bytes, NBYTES);
#else
    Serial.print(rbuf.size());
    for (int i = 0; i < NWORDS; i++) {
      Serial.print('\t');
      Serial.print(m.words[i], DEC);
    }
#endif
    Serial.write('\n');
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


