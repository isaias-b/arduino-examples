namespace Qdec {
  const int  stepPin    = 3;
  const int  dirPin     = 4;
  const int  clockPin   = FREQUENCYTIMER2_PIN;
  const int  calPin     = 6;

  volatile int counter = 0;
  volatile int gMin = 0;
  volatile int gMax = 0;

  inline int range() { return gMax - gMin; }
  
  void doCount() {
    int c = counter;
    c += digitalReadFast(dirPin) == HIGH ? +1 : -1;
    counter = c;
    gMin = min(gMin, c);
    gMax = max(gMax, c);
  }
  
  void doReset() {
    counter -= gMin;
    gMin = 0;
    gMax = 0;
  }

  void setup(double freq) {
    pinMode(clockPin, OUTPUT);
    pinMode(dirPin, INPUT);
    pinMode(stepPin, INPUT);    
    pinMode(calPin, INPUT_PULLUP);
  
    attachInterrupt(digitalPinToInterrupt(stepPin), doCount, RISING);
    attachInterrupt(digitalPinToInterrupt(calPin), doReset, FALLING);
    
    FrequencyTimer2::setPeriod(1000000L/freq);
    FrequencyTimer2::setOnOverflow(0);
    FrequencyTimer2::enable();
  }
}

