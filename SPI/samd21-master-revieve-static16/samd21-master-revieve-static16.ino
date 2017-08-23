#include <SPI.h>
#include "wiring_private.h"

#define SPI_TRANSFER_CLOCK_FREQ_4 4000000
#define SPI_TRANSFER_CLOCK_FREQ_8 8000000
#define SPI_TRANSFER_CLOCK_FREQ_10 10000000
#define SPI_TRANSFER_CLOCK_FREQ_12 12000000
#define SPI_TRANSFER_CLOCK_FREQ SPI_TRANSFER_CLOCK_FREQ_8

const int NWORDS = 4;
const int NBYTES = NWORDS * sizeof(uint16_t);
typedef union { uint16_t words[NWORDS]; uint8_t bytes[NBYTES]; } MODEL;

/// GLOBAL VARIABLES
SPISettings settings = SPISettings(SPI_TRANSFER_CLOCK_FREQ, MSBFIRST, SPI_MODE0);
char msg[1024];
MODEL data;
int samplingDelay = 200000;
int skippedBytes = 2;

void setup() {
  SerialUSB.begin(2000000);
  SPI.begin();
  pinMode(PIN_SPI_SS, OUTPUT);
}

void loop() {
  digitalWrite(PIN_SPI_SS, LOW);
  SPI.beginTransaction(settings);
  delayMicroseconds(2); // play with this parameter
  for(int i=0; i<skippedBytes; i++)
    SPI.transfer(0xF);
  for(int i=0; i<NBYTES; i++) {
    delayMicroseconds(2); // play with this parameter
    data.bytes[i] = SPI.transfer(data.bytes[i]);
  }
  delayMicroseconds(2); // play with this parameter
  SPI.endTransaction();
  digitalWrite(PIN_SPI_SS, HIGH);

  printBytes(data);
  SerialUSB.print(" -> ");
  printWords(data);
  SerialUSB.println();

  delayMicroseconds(samplingDelay);
}

void printBytes(MODEL &row) {
  for(int i=0; i<NBYTES; i++) {
    sprintf(msg, "%02x ", row.bytes[i]);
    SerialUSB.print(msg);
  }
}

void printWords(MODEL &row) {
  for(int i=0; i<NWORDS; i++) {
    sprintf(msg, "%04x ", row.words[i]);
    SerialUSB.print(msg);
  }
}

