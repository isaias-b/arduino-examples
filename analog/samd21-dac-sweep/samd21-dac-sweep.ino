const long serialRate  = 2000000;
const int resolution   = 10;
const int stepSize     = 1 << 1;
const int maxValue     = (1 << resolution) - 1;
const int minValue     = 0; //maxValue >> 3;
const int waitInterval = 4000;

void setup() {
  SerialUSB.begin(serialRate);
  analogWriteResolution(resolution);
  analogReference(AR_DEFAULT);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {  
  static long counter = 0;
  float t = counter / (float)(maxValue);;
  long ft = (maxValue - minValue) * (cos(2.0 * PI * t) + 1.0) / 2.0 + minValue;
  
  SerialUSB.println(ft, DEC);
  
  analogWrite(A0, ft);
  digitalWrite(LED_BUILTIN, counter < 100 ? HIGH : LOW);
  delayMicroseconds(waitInterval);
  counter = (counter + stepSize) % maxValue;
}


