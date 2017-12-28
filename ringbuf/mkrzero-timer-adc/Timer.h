namespace Timer
{
  TcCount16* TC = (TcCount16*) TC3;
  void (*callback)();
  
  //https://forum.arduino.cc/index.php?topic=332275.15
  void setup(int freqHz, void (*callback)()) {
    Timer::callback = callback;
    
    const long CPU_HZ = 48000000;
    const long PRESCALER_CTRL = TC_CTRLA_PRESCALER_DIV16;
    const long PRESCALER_VAL  = 16;
    const long matchValue = CPU_HZ / PRESCALER_VAL / freqHz;

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
  
  void handler(void) {
    if (TC->INTFLAG.bit.MC0) {
      callback();
      TC->INTFLAG.bit.MC0 = true;
    }  
  }
}

void TC3_Handler (void) { Timer::handler(); }

