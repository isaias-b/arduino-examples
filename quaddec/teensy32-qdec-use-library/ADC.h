namespace Analog
{
  const int ReadResolution = 12;

  void setup() {
    analogReadRes(ReadResolution);
  }
    
  uint32_t read(uint32_t pin) {
    return analogRead(pin);
  }
};

