const long serialRate = 2000000;
const int ledPin = A0;
const int resolution = 10;
const int minValue = 0;
const int maxValue = pow(2, resolution) - 1;
const int waitInterval = 500;

void setup() {
  SerialUSB.begin(serialRate);
  analogWriteResolution(resolution);
  analogReference(AR_DEFAULT);
}

int parseInput(int defaultValue) {
  if (!(Serial.available() > 0)) return defaultValue;
  
  int input = Serial.parseInt();
  SerialUSB.print("input=");
  SerialUSB.print(input, DEC);
  int next = max(minValue, min(maxValue, input));
  SerialUSB.print("\ttrim(input)=");
  SerialUSB.println(next, DEC);
  return next;
}

void loop() {
  static int fadeValue = 0;
  fadeValue = parseInput(fadeValue);
  analogWrite(ledPin, fadeValue);
  SerialUSB.print("fadeValue=");
  SerialUSB.println(fadeValue, DEC);
  delay(waitInterval);
}


