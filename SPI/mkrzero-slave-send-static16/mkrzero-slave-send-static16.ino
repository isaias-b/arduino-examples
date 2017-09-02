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

#define LEN(arr) ((int) (sizeof (arr) / sizeof (arr)[0]))

const int NWORDS = 4;
const int NBYTES = NWORDS * sizeof(uint16_t);
typedef union { uint16_t words[NWORDS]; uint8_t bytes[NBYTES]; } MODEL;
MODEL dataset[] = {
  MODEL { { 0x01fc, 0x0014, 0x001e, 0x01fc } },
  MODEL { { 0x01fd, 0x0014, 0x001e, 0x01fd } },
  MODEL { { 0x01fe, 0x0014, 0x001e, 0x01fe } },
  MODEL { { 0x01ff, 0x0014, 0x001e, 0x01ff } },
  MODEL { { 0x0200, 0x0014, 0x001e, 0x0200 } },
  MODEL { { 0x0201, 0x0014, 0x001e, 0x0201 } },
  MODEL { { 0x0202, 0x0014, 0x001e, 0x0202 } },
  MODEL { { 0x0203, 0x0014, 0x001e, 0x0203 } },
};
const int NROWS  = LEN(dataset);

/// GLOBAL VARIABLES
char msg[1024];
bool isWithinTx = false;        // syncs loop and ISR to avoid data races
bool preload = true;

void loop() {
  static bool ledState = false;
  static uint8_t rowPos = 0;

  printWords(dataset[rowPos]);
  Serial.print(" -> ");
  printBytes(dataset[rowPos]);  
  Serial.println();
  rowPos = (rowPos + 1) % NROWS;

  digitalWrite(LED_BUILTIN, ledState);
  ledState = !ledState;
  
  delay(200);
}

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
  MY_SERCOM->SPI.CTRLB.bit.PLOADEN = preload; //set Preload Data Register Bit
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
  static uint8_t bytesPos = 0;
  static uint8_t rowPos = 0;
  static MODEL *row;
  uint8_t data = 0;
  
  //Read SPI interrupt register
  intflag.reg = MY_SERCOM->SPI.INTFLAG.reg;

  // This bit is set when a high to low transition is detected 
  // on the _SS pin in slave mode 
  // and Slave Select Low Detect (CTRLB.SSDE) is enabled.
  // This flag is cleared by writing '1' to it.
  if(intflag.bit.SSL)
  {
    isWithinTx = true;
    bytesPos = preload ? 1 : 0;
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
    isWithinTx = false;
    rowPos = (rowPos + 1) % NROWS;
    row = &dataset[rowPos];
    if(preload) MY_SERCOM->SPI.DATA.reg = row->bytes[0];
  }
  
  // This flag is set when DATA is empty and ready for new data to transmit.
  // This flag is cleared by writing new data to DATA.
  if(intflag.bit.DRE)
  {
    MY_SERCOM->SPI.DATA.reg = row->bytes[bytesPos]; //send byte
    bytesPos = (bytesPos + 1) % NBYTES;
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

void printBytes(MODEL &row) {
  for(int i=0; i<NBYTES; i++) {
    sprintf(msg, "%02x ", row.bytes[i]);
    Serial.print(msg);
  }
}

void printWords(MODEL &row) {
  for(int i=0; i<NWORDS; i++) {
    sprintf(msg, "%04x ", row.words[i]);
    Serial.print(msg);
  }
}

