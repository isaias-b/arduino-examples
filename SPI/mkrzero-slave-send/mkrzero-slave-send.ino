/*
 *  Code based on code snippets of:
 *  User:     Tom_the_banana
 *  Found At: https://forum.arduino.cc/index.php?topic=360026.0
 */
#include "wiring_private.h"

#define MY_SPI_MOSI    A3
#define MY_SPI_SCK     A4
#define MY_SPI_SS      A5
#define MY_SPI_MISO    A6
#define MY_SERCOM           SERCOM0
#define MY_SERCOM_IRQn      SERCOM0_IRQn
#define MY_SERCOM_HANDLER   SERCOM0_Handler
#define MY_GCM_SERCOM_CORE  GCM_SERCOM0_CORE

const int NWORDS = 4;
const int NBYTES = NWORDS * sizeof(uint16_t);

/// TYPE DEFINITIONS
typedef union { uint16_t words[NWORDS]; uint8_t bytes[NBYTES]; } MODEL;
void format(MODEL &model, char *msg) {
  sprintf(
    msg, "%d %d %d %d",
    model.words[0], model.words[1], model.words[2], model.words[3]
  );
}

/// GLOBAL VARIABLES
MODEL current;            // used to hold the currently measured sensor data
MODEL last;               // used to transfer the last measured sensor data
bool withinTx = false;    // used to sync loop and avoid data races
char msg[1024];           // used to create a message sent via serial
bool needsUpdate = false; // used to print 'msg' only on certain events
void (*doUpdate)(MODEL&) = updateConst; // callback function pointer
bool preload = false;

void setup() {
  Serial.begin(2000000);
  pinMode(LED_BUILTIN, OUTPUT);

  pinPeripheral(MY_SPI_MISO,  PIO_SERCOM_ALT);
  pinPeripheral(MY_SPI_SCK,   PIO_SERCOM_ALT);
  pinPeripheral(MY_SPI_MOSI,  PIO_SERCOM_ALT);
  pinPeripheral(MY_SPI_SS,    PIO_SERCOM_ALT);

  //Disable SPI
  MY_SERCOM->SPI.CTRLA.bit.ENABLE = 0;
  while(MY_SERCOM->SPI.SYNCBUSY.bit.ENABLE);
  
  //Reset SPI
  MY_SERCOM->SPI.CTRLA.bit.SWRST = 1;
  while(MY_SERCOM->SPI.CTRLA.bit.SWRST || MY_SERCOM->SPI.SYNCBUSY.bit.SWRST);
  
  //Setting up NVIC
  NVIC_EnableIRQ(MY_SERCOM_IRQn);
  NVIC_SetPriority(MY_SERCOM_IRQn, 2);
  
  //Setting Generic Clock Controller
  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(MY_GCM_SERCOM_CORE) | // Generic Clock 0
            GCLK_CLKCTRL_GEN_GCLK0 | // Generic Clock Generator 0 is the source
            GCLK_CLKCTRL_CLKEN;      // Enable Generic Clock Generator
  
  while(GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY); //Wait for synchronisation
  
  
  //Set up SPI Control A Register
  MY_SERCOM->SPI.CTRLA.bit.DORD = 0;      //MSB first
  MY_SERCOM->SPI.CTRLA.bit.CPOL = 0;      //SCK is low when idle, leading edge is rising edge
  MY_SERCOM->SPI.CTRLA.bit.CPHA = 0;      //data sampled on leading SCK edge and changed on a trailing SCK edge
  MY_SERCOM->SPI.CTRLA.bit.FORM = 0x0;    //Frame format = SPI
  MY_SERCOM->SPI.CTRLA.bit.DIPO = 0;      //DATA PAD0 MOSI is used as input (slave mode)
  MY_SERCOM->SPI.CTRLA.bit.DOPO = 0x2;    //DATA PAD3 MISO is used as output
  MY_SERCOM->SPI.CTRLA.bit.MODE = 0x2;    //SPI in slave mode
  MY_SERCOM->SPI.CTRLA.bit.IBON = 0x1;    //Buffer Overflow notification
  MY_SERCOM->SPI.CTRLA.bit.RUNSTDBY = 1;  //wake on RCX
  
  //Set up SPI control B register
  MY_SERCOM->SPI.CTRLB.bit.RXEN = 0x1;      //Enable Receiver
  MY_SERCOM->SPI.CTRLB.bit.SSDE = 0x1;      //Slave Selecte Detection Enabled
  MY_SERCOM->SPI.CTRLB.bit.CHSIZE = 0;      //character size 8 Bit
  MY_SERCOM->SPI.CTRLB.bit.PLOADEN = 0x1;   //Enable Preload Data Register
  while (MY_SERCOM->SPI.SYNCBUSY.bit.CTRLB);  
  
  //Set up SPI interrupts
  MY_SERCOM->SPI.INTENSET.bit.SSL = 0x1;    //Slave Select Low interrupt        
  MY_SERCOM->SPI.INTENSET.bit.RXC = 0x1;    //Receive Complete interrupt
  MY_SERCOM->SPI.INTENSET.bit.TXC = 0x1;    //Transmit Complete interrupt
  MY_SERCOM->SPI.INTENSET.bit.ERROR = 0x1;  //Error interrupt
  MY_SERCOM->SPI.INTENSET.bit.DRE = 0x1;    //Data Register Empty interrupt

  //Enable SPI
  MY_SERCOM->SPI.CTRLA.bit.ENABLE = 1;
  while(MY_SERCOM->SPI.SYNCBUSY.bit.ENABLE);
}

void MY_SERCOM_HANDLER() {
  static SERCOM_SPI_INTFLAG_Type intflag;
  uint8_t offset = preload ? 1 : 0;
  static uint8_t bytesPos = offset;
  uint8_t data = 0;
  
  //Read SPI interrupt register
  intflag.reg = MY_SERCOM->SPI.INTFLAG.reg;

  // This bit is set when a high to low transition is detected 
  // on the _SS pin in slave mode 
  // and Slave Select Low Detect (CTRLB.SSDE) is enabled.
  // This flag is cleared by writing '1' to it.
  if(intflag.bit.SSL)
  {
    withinTx = true;
    bytesPos = offset;
    MY_SERCOM->SPI.INTFLAG.bit.SSL = 1;             //clear SSL interrupt
  }

  // This flag is set when there are unread data in the receive buffer.
  // This flag is cleared by reading the DATA register or by disabling the receiver.
  if(intflag.bit.RXC)
  {
    data = MY_SERCOM->SPI.DATA.reg;                 //Read data register and clear interrupt
  }

  // In slave mode, this flag is set when the _SS pin is pulled high.
  // This flag is cleared by writing '1' to it or by writing new data to DATA.
  if(intflag.bit.TXC)
  {
    MY_SERCOM->SPI.INTFLAG.bit.TXC = 1;             //clear RXC interrupt
    withinTx = false;
    needsUpdate = true;
  }
  
  // This flag is set when DATA is empty and ready for new data to transmit.
  // This flag is cleared by writing new data to DATA.
  if(intflag.bit.DRE)
  {
    MY_SERCOM->SPI.DATA.reg = last.bytes[bytesPos++ % NBYTES]; //send byte
  }

  // This bit is set when any error is detected. 
  // Errors that will set this flag have corresponding status flags in the STATUS register. 
  // The BUFOVF error will set this interrupt flag.
  // This flag is cleared by writing '1' to it.
  if(intflag.bit.ERROR)
  {
    MY_SERCOM->SPI.INTFLAG.bit.ERROR = 1;           //clear ERROR interrupt
  }
}

void loop() {
  if(!withinTx) last = current;
  if(preload && !withinTx) MY_SERCOM->SPI.DATA.reg = last.bytes[0];
  if(needsUpdate) {
    doUpdate(current);
    needsUpdate = false;
    format(current, msg);
    Serial.println(msg);
  }
}

void updateConst(MODEL &model) {
  model.words[0] = 10;
  model.words[1] = 20;
  model.words[2] = 30;
  model.words[3] = 40;
}

void updateCounter(MODEL &model) {
  static uint16_t counter = 0;
  model.words[0] = counter * 1;
  model.words[1] = counter * 2;
  model.words[2] = counter * 3;
  model.words[3] = counter * 4;
  ++counter;
}

void updateCounterMod(MODEL &model) {
  static uint16_t counter = 0;
  uint16_t mod = 1 << 10;
  model.words[0] = counter * 1 % mod;
  model.words[1] = counter * 2 % mod;
  model.words[2] = counter * 3 % mod;
  model.words[3] = counter * 4 % mod;
  ++counter;
}

void updateRangeBit9(MODEL &model) {
  static uint16_t counter = 0;
  uint16_t bit9 = 1 << 9;
  uint16_t range = 1 << 6;
  uint16_t offset = bit9 - range / 2;
  model.words[0] = offset + counter % range;
  model.words[1] = 20;
  model.words[2] = 30;
  model.words[3] = offset + counter % range;
  ++counter;
}

void updateComplex(MODEL &model) {
  static uint16_t x = 0;
  static uint16_t y1 = 0;
  static uint16_t y2 = 0;
  static const float PI2 = PI * 2;

  int n = 1 << 8;
  int maxY = 1 << 11;
  x = (x + 1) % n;
  float t = x / (float) n;
  y1 = maxY * (sin(PI2 * t) + 1);
  y2 = maxY * (sin(PI2 * t + PI) + 1);

  model.words[0] = millis() % 1000;
  model.words[1] = x;
  model.words[2] = y1;
  model.words[3] = y2;
}


