#include <SPI.h>
#include "wiring_private.h"

#define SPI_TRANSFER_CLOCK_FREQ_4 4000000
#define SPI_TRANSFER_CLOCK_FREQ_8 8000000
#define SPI_TRANSFER_CLOCK_FREQ_10 10000000
#define SPI_TRANSFER_CLOCK_FREQ_12 12000000
#define SPI_TRANSFER_CLOCK_FREQ SPI_TRANSFER_CLOCK_FREQ_8

const int NWORDS = 4;
const int NBYTES = NWORDS * 2;

/// TYPE DEFINITIONS
typedef union { uint16_t words[NWORDS]; uint8_t bytes[NBYTES]; } MODEL;
void format(MODEL &model, char *msg) {
  sprintf(
    msg, "%d %d %d %d",
    model.words[0], model.words[1], model.words[2], model.words[3]
  );
}
typedef union { uint16_t val; struct { uint8_t lsb; uint8_t msb; }; } WORD;

/// GLOBAL VARIABLES
SPISettings settings = SPISettings(SPI_TRANSFER_CLOCK_FREQ, MSBFIRST, SPI_MODE0);
char msg[1024];
MODEL data;
int samplingDelay = 20;
int byteOffset = 1;

void setup() {
  SerialUSB.begin(2000000);
  SPI.begin();
  pinMode(PIN_SPI_SS, OUTPUT);
}

void loop() {
  digitalWrite(PIN_SPI_SS, LOW);
  SPI.beginTransaction(settings);
  
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
  
  delay(samplingDelay);
}

