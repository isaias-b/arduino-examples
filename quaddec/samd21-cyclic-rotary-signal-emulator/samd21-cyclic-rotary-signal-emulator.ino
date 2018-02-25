const double stepFreq      = 400 * 16;
const int    revs          = 1600;

const double base          = 200000L / stepFreq;
const int  M               = 2 * revs;
const int  S               = 4;          // number of states per rev
const int  points          = S * revs;
const int  minValue        = 0;
const int  maxValue        = 2047;
const int  POSITION_ENC_A  = 6;
const int  POSITION_ENC_B  = 7;
const double PERIOD        = PI * 2;
const long serialRate      = 2000000;
const long printDelay      = 10;

typedef struct VALUE {
  int a;
  int b;
} VALUE;

typedef struct STATE {
  int counter;
  int pos;
} STATE;

VALUE values[S] = { {0,0}, {0,1}, {1,1}, {1,0} };
int delays[revs];

inline double scale(double x, double minValue, double maxValue) {
  return ((x - minValue) / (maxValue - minValue));
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(POSITION_ENC_A, OUTPUT);
  pinMode(POSITION_ENC_B, OUTPUT);
  SerialUSB.begin(serialRate);
  
  for (int counter = 0; counter < revs; counter++) {
    double t = scale(counter+1, 0, revs);
    delays[counter] = base * scale(cos(PERIOD * t), -1, 1);
  }
}

inline STATE nextState(STATE current) {
  STATE next;
  next.counter = (current.counter + 1) % points;

  bool dir = current.counter / M;
  
  int incr = dir ? +1 : -1;
  next.pos = (current.pos + incr + M) % M;
  next.pos = (next.pos == 0 && next.counter == 0) ? M : next.pos;
  return next;
}

void loop() {
  static STATE state;
  static VALUE v = values[0];
  static long d = 0;
  static long lastWrite = micros();  
  if ((micros() - lastWrite) > d) {
    d = delays[(state.pos / 2) % revs];
    v = values[state.pos % S];
    digitalWrite(POSITION_ENC_A, v.a);
    digitalWrite(POSITION_ENC_B, v.b);
    lastWrite = micros();
    state = nextState(state);
  }
  
  static long lastPrint = millis();
  if((millis() - lastPrint) > printDelay) {
    SerialUSB.print((.2 + v.a) * base, DEC);
    SerialUSB.print(' ');
    SerialUSB.print((.1 + v.b) * base, DEC);
    SerialUSB.print(' ');
    SerialUSB.println(d, DEC);
    lastPrint = millis();
  }  
}
