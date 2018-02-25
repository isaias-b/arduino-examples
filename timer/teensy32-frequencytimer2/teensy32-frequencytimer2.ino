#include <FrequencyTimer2.h>

const int  LOOP_PIN         = 6;
const long serialRate       = 2000000;
const long freq             = 375000;



void setup() {
  pinMode(LOOP_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(FREQUENCYTIMER2_PIN, OUTPUT);

  SerialUSB.begin(serialRate);
  
  FrequencyTimer2::setPeriod(1000000L/freq);
  FrequencyTimer2::setOnOverflow(Burp);
  FrequencyTimer2::enable();
}

volatile unsigned long burpCount = 0;

void Burp(void) {
  burpCount++;
}

void loop() {
  static bool state = false;

  digitalWrite(LED_BUILTIN, state ? HIGH : LOW);
  digitalWrite(LOOP_PIN, state ? HIGH : LOW);
  SerialUSB.println(burpCount, DEC);
  delayMicroseconds(1000000L / freq);
  
  state = !state;
}

