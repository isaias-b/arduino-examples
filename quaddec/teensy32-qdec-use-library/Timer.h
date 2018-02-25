namespace Timer
{
  IntervalTimer myTimer;
  void (*callback)(void);
  
  void setup(int freqHz, void (*callback)()) {
    Timer::callback = callback;
    myTimer.begin(callback, 1000000 / freqHz);
  }
}

