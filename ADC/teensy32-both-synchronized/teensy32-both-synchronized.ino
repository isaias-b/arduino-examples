#include <ADC.h>

namespace Analog {
  const int ReadResolution = 8;
  ADC::Sync_result result;

  const int readPin1 = A9;
  const int readPin2 = A3;
  
  ADC *adc = new ADC();

  void setup() {
    pinMode(readPin1, INPUT);
    pinMode(readPin2, INPUT);
    adc->setAveraging(1);
    adc->setResolution(ReadResolution);

    adc->setConversionSpeed(ADC_CONVERSION_SPEED::HIGH_SPEED);
    adc->setSamplingSpeed(ADC_SAMPLING_SPEED::HIGH_SPEED);

    adc->setAveraging(1, ADC_1);
    adc->setResolution(ReadResolution, ADC_1);
    adc->setConversionSpeed(ADC_CONVERSION_SPEED::HIGH_SPEED, ADC_1);
    adc->setSamplingSpeed(ADC_SAMPLING_SPEED::HIGH_SPEED, ADC_1);

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
}


void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  Analog::setup();
  delay(100);
  Serial.println("end setup");
}

void loop() {
  Analog::readSynchronizedContinuous();

  Serial.print(Analog::Data0()*3.3/Analog::adc->getMaxValue(ADC_0), DEC);
  Serial.print(" ");
  Serial.println(Analog::Data1()*3.3/Analog::adc->getMaxValue(ADC_1), DEC);

  Analog::clearErrors(0);
  #if ADC_NUM_ADCS>1
  Analog::clearErrors(1);
  #endif

  digitalWriteFast(LED_BUILTIN, !digitalReadFast(LED_BUILTIN));

  //delay(100);
}

