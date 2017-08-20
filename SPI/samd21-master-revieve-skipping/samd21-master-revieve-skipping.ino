#include <SPI.h>
#include "wiring_private.h"

#define SPI_TRANSFER_CLOCK_FREQ_4 4000000
#define SPI_TRANSFER_CLOCK_FREQ_8 8000000
#define SPI_TRANSFER_CLOCK_FREQ_10 10000000
#define SPI_TRANSFER_CLOCK_FREQ_12 12000000
#define SPI_TRANSFER_CLOCK_FREQ SPI_TRANSFER_CLOCK_FREQ_8

const int NWORDS = 4;
const int NBYTES = NWORDS * sizeof(uint16_t);
const char* AHEX_FMT = "%04x %04x %04x %04x";
const char* DEC_FMT  = "%d %d %d %d";
const char* FORMAT = DEC_FMT;

/// TYPE DEFINITIONS
typedef union { uint16_t words[NWORDS]; uint8_t bytes[NBYTES]; } MODEL;
void format(MODEL &model, char *msg) {
  sprintf(
    msg, FORMAT,
    model.words[0], model.words[1], model.words[2], model.words[3]
  );
}

/// GLOBAL VARIABLES
SPISettings settings = SPISettings(SPI_TRANSFER_CLOCK_FREQ, MSBFIRST, SPI_MODE0);
char msg[1024];
MODEL data;
int samplingDelay = 20000;
int skippedBytes = 3;
int byteOffset = 0;

void setup() {
  SerialUSB.begin(2000000);
  SPI.begin();
  pinMode(PIN_SPI_SS, OUTPUT);
}

void loop() {
  digitalWrite(PIN_SPI_SS, LOW);
  SPI.beginTransaction(settings);

  for(int i=0; i<skippedBytes; i++)
    SPI.transfer(0xF);

  for(int i=0; i<NBYTES; i++) {
    delayMicroseconds(2); // play with this parameter
    int k = (i + NBYTES - byteOffset) % NBYTES;
    data.bytes[k] = SPI.transfer(data.bytes[k]);
  }
  delayMicroseconds(2); // play with this parameter
  SPI.endTransaction();
  digitalWrite(PIN_SPI_SS, HIGH);

  format(data, msg);
  SerialUSB.println(msg);

  delayMicroseconds(samplingDelay);
}

