#include <FrequencyTimer2.h>

const long serialRate       = 2000000;
const long freq             = 600000;
const int  POSITION_STEP    = 3;
const int  POSITION_DIR     = 4;
const int  POSITION_CLOCK   = FREQUENCYTIMER2_PIN;
const int  POSITION_CAL     = 6;

volatile int counter = 0;
volatile int gMin = 0;
volatile int gMax = 0;

void setup() {
  pinMode(POSITION_CLOCK, OUTPUT);
  pinMode(POSITION_DIR, INPUT);
  pinMode(POSITION_STEP, INPUT);
  pinMode(POSITION_CAL, INPUT_PULLUP);

  SerialUSB.begin(serialRate);
  
  attachInterrupt(digitalPinToInterrupt(POSITION_STEP), doCount, RISING);
  attachInterrupt(digitalPinToInterrupt(POSITION_CAL), doReset, FALLING);
  
  FrequencyTimer2::setPeriod(1000000L/freq);
  FrequencyTimer2::setOnOverflow(0);
  FrequencyTimer2::enable();
}

void doCount() {
  int c = counter;
  c += digitalRead(POSITION_DIR) == HIGH ? +1 : -1;
  counter = c;
  gMin = min(gMin, c);
  gMax = max(gMax, c);
}

void doReset() {
  counter -= gMin;
  gMin = 0;
  gMax = 0;
}

void loop() {
  SerialUSB.print(counter, DEC);
  SerialUSB.print('\t');
  SerialUSB.print(gMin, DEC);
  SerialUSB.print('\t');
  SerialUSB.print(gMax, DEC);
  SerialUSB.println();
  delayMicroseconds(200);
}

