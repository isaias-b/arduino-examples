void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  SerialUSB.begin(2000000);
}

void loop() {
  static int counter = 0;
  SerialUSB.println(counter, DEC);
  counter = (counter + 1) % (1 << 8);
  digitalWrite(LED_BUILTIN, counter >> 7 ? HIGH : LOW);
  delay(20);
}
