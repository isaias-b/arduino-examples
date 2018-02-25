const long serialRate       = 2000000;
const long freq             = 375000;
const int FREQ_PIN          = 4;
const int LOOP_PIN          = 6;

void setup() {
  pinMode(FREQ_PIN, OUTPUT);
  pinMode(LOOP_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  analogWriteFrequency(FREQ_PIN, freq);
  
  SerialUSB.begin(serialRate);
}

void loop() {
  static bool state = false;

  digitalWrite(LED_BUILTIN, state ? HIGH : LOW);
  digitalWrite(LOOP_PIN, state ? HIGH : LOW);
  SerialUSB.println(state, DEC);  
  delayMicroseconds(1000000L / freq);
  
  state = !state;
}

