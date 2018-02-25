#include "wiring_private.h"

#define LEN(arr) ((int) (sizeof (arr) / sizeof (arr)[0]))

long t0, t1, delta;
const int NWORDS = 5;
const int WSIZE = sizeof(uint16_t);
const int NBYTES = NWORDS * WSIZE;
typedef union { uint16_t words[NWORDS]; uint8_t bytes[NBYTES]; } MODEL;
MODEL dataset[] = {
  MODEL { { 0x0000, 0x01fc, 0x0014, 0x001e, 0x01fc } },
  MODEL { { 0x0001, 0x01fd, 0x0014, 0x001e, 0x01fd } },
  MODEL { { 0x0002, 0x01fe, 0x0014, 0x001e, 0x01fe } },
  MODEL { { 0x0003, 0x01ff, 0x0014, 0x001e, 0x01ff } },
  MODEL { { 0x0004, 0x0200, 0x0014, 0x001e, 0x0200 } },
  MODEL { { 0x0005, 0x0201, 0x0014, 0x001e, 0x0201 } },
  MODEL { { 0x0006, 0x0202, 0x0014, 0x001e, 0x0202 } },
  MODEL { { 0x0007, 0x0203, 0x0014, 0x001e, 0x0203 } },
};
const int NROWS  = LEN(dataset);

// BENCHMARK PARAMETERS
const int bytesToWrite = 5 * WSIZE;
const int baudrate = 2000000;
#define SEND_BINARY

// results with 8 rows
// baudrate   words   binary      per word   text       text/binary
// 2M         4       315 ± 0.5    9.84       585 ± 4   1.857
// 2M         5       395 ± 1      9.87       760 ± 5   1.924
// 1M         4       630 ± 0     19.68       874 ± 3   1.387
// 1M         5       785 ± 0.5   19.62      1114 ± 5   1.419

void setup() {
  SerialUSB.begin(2000000);
  Serial1.begin(baudrate);
}

void loop() {
  t0 = micros();
  for (int i=0; i<NROWS; i++) {
    #ifdef SEND_BINARY
    Serial1.write(dataset[i].bytes, bytesToWrite);
    #else
    for (int j=0; j<bytesToWrite; j++)
      Serial1.print(dataset[i].bytes[j]);
    #endif
  }
  t1 = micros();
  delta = t1-t0;
  Serial.print( delta ); // absolute time
  Serial.print('\t');
  Serial.println( (double)delta / (double)NROWS / (double)bytesToWrite * WSIZE );
}

