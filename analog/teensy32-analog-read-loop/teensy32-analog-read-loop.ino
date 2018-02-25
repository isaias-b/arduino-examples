void setup() {
  analogReadRes(12);
  SerialUSB.begin(2000000);
}

void loop() {
  SerialUSB.print(analogRead(A0));
  SerialUSB.print("\t");
  SerialUSB.print(analogRead(A1));
  SerialUSB.print("\t");
  SerialUSB.print(analogRead(A2));
  SerialUSB.println();
}

