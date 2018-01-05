#include "wiring_private.h"

//#define USE_BINARY

//           DIAGNOSTICS
//              no    yes
// UART bin  20000  17000
// UART txt   4400   3000

const int  sampleRate     = 3000;
const long serialRate     = 2000000;
const int NWORDS          = 4;
const int WSIZE           = sizeof(uint16_t);
const int NBYTES          = NWORDS * WSIZE;

bool pushData = false;
typedef union { uint16_t words[NWORDS]; uint8_t bytes[NBYTES]; } MODEL;
MODEL m;

TcCount16* TC = (TcCount16*) TC3;

//https://forum.arduino.cc/index.php?topic=332275.15
void setupTimer(int freqHz) {  
  const long CPU_HZ         = 48000000;
  const long PRESCALER_CTRL = TC_CTRLA_PRESCALER_DIV16;
  const long PRESCALER_VAL  = 16;
  const long matchValue     = CPU_HZ / PRESCALER_VAL / freqHz;

  REG_GCLK_CLKCTRL = (uint16_t) (
    GCLK_CLKCTRL_CLKEN 
      | GCLK_CLKCTRL_GEN_GCLK0 
      | GCLK_CLKCTRL_ID (GCM_TCC2_TC3)
  );
  while (GCLK->STATUS.bit.SYNCBUSY);

  TC->CTRLA.bit.ENABLE = false;
  TC->CTRLA.reg |= (
    TC_CTRLA_MODE_COUNT16
    | TC_CTRLA_WAVEGEN_MFRQ  // Use match mode, that resets when match hit
    | PRESCALER_CTRL
  );    
  TC->CC[0].reg = matchValue - 1;          // 0 is counted as well, thus -1

  TC->INTENSET.reg = 0;                    // Disable all interrupts
  TC->INTENSET.bit.MC0 = true;             // Enable the compare interrupt

  NVIC_EnableIRQ(TC3_IRQn);

  TC->CTRLA.bit.ENABLE = true;
  while (TC->STATUS.bit.SYNCBUSY);
}

void TC3_Handler (void) { 
  if (TC->INTFLAG.bit.MC0) {
    TC->INTFLAG.bit.MC0 = true;
    pushData = true;
  }  
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  SerialUSB.begin(serialRate);
  setupTimer(sampleRate);
  for (int i = 0; i < NWORDS; i++)
    m.words[i] = i+1000;
}

void loop() {
  static int counter = 0;
  bool state = (counter++ >> 12) % 2;
  digitalWrite(LED_BUILTIN, state ? HIGH : LOW);
  int t = SerialUSB.availableForWrite();
  if (pushData && t > NBYTES) {
    pushData = false;
    #ifdef USE_BINARY
    SerialUSB.write(m.bytes, NBYTES);
    #else
    SerialUSB.print(t, DEC);
    for (int i = 0; i < NWORDS; i++) {
      SerialUSB.print('\t');
      SerialUSB.print(m.words[i], DEC);
    }
    #endif
    SerialUSB.write('\n');
  }
}
