//#define USE_BINARY

const int  sampleRate     = 8000;
const long serialRate     = 2000000;
const int NWORDS          = 20;
const int WSIZE           = sizeof(uint16_t);
const int NBYTES          = NWORDS * WSIZE;

volatile bool pushData = false;
typedef union { uint16_t words[NWORDS]; uint8_t bytes[NBYTES]; } MODEL;
MODEL m;

IntervalTimer myTimer;

void setup(void) {
  for (int i = 0; i < NWORDS; i++) m.words[i] = i+1000;
  pinMode(LED_BUILTIN, OUTPUT);
  SerialUSB.begin(serialRate);
  myTimer.begin(blinkLED, 1000000 / sampleRate);
}

void blinkLED(void) {
  pushData = true;
}

void loop(void) {
  static int counter = 0;
  bool state = (counter++ >> 12) % 2;
  digitalWrite(LED_BUILTIN, state ? HIGH : LOW);
  if (pushData) {
    pushData = false;
    #ifdef USE_BINARY
    SerialUSB.write(m.bytes, NBYTES);
    #else
    for (int i = 0; i < NWORDS; i++) {
      if (i!=0) SerialUSB.print('\t');
      SerialUSB.print(m.words[i], DEC);
    }
    #endif
    SerialUSB.write('\n');
  }
}

