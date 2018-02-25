#include <SPI.h>

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
#define SPI_TRANSFER_CLOCK_FREQ_2 2000000
#define SPI_TRANSFER_CLOCK_FREQ_4 4000000
#define SPI_TRANSFER_CLOCK_FREQ_8 8000000
#define SPI_TRANSFER_CLOCK_FREQ_12 12000000

// BENCHMARK PARAMETERS
#define SPI_TRANSFER_CLOCK_FREQ SPI_TRANSFER_CLOCK_FREQ_4
#define SEND_16

// results with 8 rows
// freq    words   abs       per word   abs16   per word
// 12M     4       211 ± 2    6.59      -        -.-
// 12M     5       240 ± 2    6.00      -        -.-
//  8M     4       234 ± 1    7.31      266      8.31
//  8M     5       267 ± 1    6.67      310      7.75
//  4M     4       296 ± 2    9.25      328     10.25
//  4M     5       345 ± 2    8.63      388      9.70
//  2M     4       427 ± 1   13.34      -        -.-
//  2M     5       508 ± 1   12.70      -        -.-

const int wordsToWrite = 4;
const int bytesToWrite = wordsToWrite * WSIZE;
SPISettings settings = SPISettings(SPI_TRANSFER_CLOCK_FREQ, MSBFIRST, SPI_MODE0);

void setup() {
  Serial.begin(2000000);
  SPI.begin();
}

void loop() {
  uint16_t rxData;
  PORTA.reg |= 123;

  t0 = micros();
  for (int i=0; i<NROWS; i++) {
    SPI.beginTransaction(settings);
    #ifdef SEND_16
    for (int j=0; j<wordsToWrite; j++)
      rxData = SPI.transfer16(dataset[i].words[j]);
    #else
    for (int j=0; j<bytesToWrite; j++)
      rxData = SPI.transfer(dataset[i].bytes[j]);
    #endif
    SPI.endTransaction();
  }
  t1 = micros();
  delta = t1-t0;
  Serial.print( delta ); // absolute time
  Serial.print('\t');
  Serial.println( (double)delta / (double)NROWS / (double)bytesToWrite * WSIZE );
}

