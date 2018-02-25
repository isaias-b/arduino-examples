namespace Analog {
  const int ReadResolution = 12;
  ADC::Sync_result result;

  const int readPin1 = A9;
  const int readPin2 = A3;
  const ADC_CONVERSION_SPEED convSpeed = ADC_CONVERSION_SPEED::MED_SPEED;
  const ADC_SAMPLING_SPEED   sampSpeed = ADC_SAMPLING_SPEED::MED_SPEED;
  const int averaging = 100;
  ADC *adc = new ADC();

  void setup() {
    pinMode(readPin1, INPUT);
    pinMode(readPin2, INPUT);
    
    
    adc->setAveraging(averaging);
    adc->setResolution(ReadResolution);
    adc->setConversionSpeed(convSpeed);
    adc->setSamplingSpeed(sampSpeed);

    adc->setAveraging(averaging, ADC_1);
    adc->setResolution(ReadResolution, ADC_1);
    adc->setConversionSpeed(convSpeed, ADC_1);
    adc->setSamplingSpeed(sampSpeed, ADC_1);

    adc->startSynchronizedContinuous(readPin1, readPin2);
  }

  inline uint16_t Data0() { return (uint16_t)result.result_adc0; }
  inline uint16_t Data1() { return (uint16_t)result.result_adc1; }

  inline void readSynchronizedContinuous() {
    result = adc->readSynchronizedContinuous();
  }

  void clearErrors(unsigned int number) {
    ADC_Module *m = number == ADC_0 ? adc->adc0 : adc->adc1;
    
    if(m->fail_flag) {
      Serial.print("ADC");
      Serial.print(number);
      Serial.print(": error flags: 0x");
      Serial.println(m->fail_flag, HEX);
      if(m->fail_flag == ADC_ERROR_COMPARISON) {
        m->fail_flag &= ~ADC_ERROR_COMPARISON;
        Serial.print("ADC");
        Serial.print(number);
        Serial.println(": comparison error");
      }
    }
  }

  void clearErrors() {
    clearErrors(ADC_0);
    clearErrors(ADC_1);
  }
}

