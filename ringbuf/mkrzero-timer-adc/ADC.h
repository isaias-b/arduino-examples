namespace Analog
{
  const int ReadResolution = 12;
  const int ConvResolution = 12;

  static inline uint32_t mapResolution(uint32_t value, uint32_t from, uint32_t to)
  {
    if (from == to) return value;
    if (from > to)  return value >> (from-to);
    else            return value << (to-from);
  }
  
  void setup() {
    while (DAC->STATUS.bit.SYNCBUSY);
    DAC->CTRLA.bit.ENABLE = false;              // Disable DAC
    DAC->CTRLB.bit.EOEN = false;                // The DAC output is turned off.
    while (DAC->STATUS.bit.SYNCBUSY);
    
    pinPeripheral(A0, PIO_ANALOG);
    pinPeripheral(A1, PIO_ANALOG);
    pinPeripheral(A2, PIO_ANALOG);
  
    ADC->CTRLA.bit.ENABLE = false;              // Disable ADC
    while (ADC->STATUS.bit.SYNCBUSY);
    
    ADC->CTRLB.reg = (
      ADC_CTRLB_PRESCALER_DIV4                  // Divide Clock
      | ADC_CTRLB_RESSEL_12BIT
    ); 
    ADC->AVGCTRL.reg = (
      ADC_AVGCTRL_SAMPLENUM_1                   // 1 sample 
      | ADC_AVGCTRL_ADJRES(0x00ul)              // Adjusting result by 0
    );
    ADC->SAMPCTRL.reg = 0x00;                   // Sampling Time Length = 0
    ADC->CTRLB.bit.DIFFMODE = 0;
    while (ADC->STATUS.bit.SYNCBUSY);
    
    ADC->CTRLA.bit.ENABLE = true;               // Enable ADC
    while (ADC->STATUS.bit.SYNCBUSY);
  }
  
  uint32_t read(uint32_t pin) {
    PinDescription p = g_APinDescription[pin];
    int channelNumber = p.ulADCChannelNumber;
    
    while (ADC->STATUS.bit.SYNCBUSY);
    ADC->INPUTCTRL.bit.MUXPOS = channelNumber;  // Selection for the positive ADC input

    ADC->INTFLAG.bit.RESRDY = true;             // Clear the Data Ready flag
  
    while (ADC->STATUS.bit.SYNCBUSY);
    ADC->SWTRIG.bit.START = true;               // Start conversion
    
    while (!ADC->INTFLAG.bit.RESRDY);           // Waiting for conversion to complete
    uint32_t valueRead = ADC->RESULT.reg;       // Store the value
  
    while (ADC->STATUS.bit.SYNCBUSY);
  
    return mapResolution(valueRead, ConvResolution, ReadResolution);
  }
};

