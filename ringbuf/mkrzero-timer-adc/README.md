The purpose of this program is to provide a benchmarking platform for several different use case.
It has been useful to stress certain scenarios without simplifiying too much from a real application.
This program compiles to the arduino mkrzero.
The program gathers analog sensor information and streams it continously to its reciever fast as possible.
The program consists of two sub programs: main and timer interrupt.
Both sub programs communicate with a ring buffer.
Target data is acquired from the ADC signal lines.

# Timer Interrupt Program
A timer is used to maintain accurate data acquisition.
The timer can be adjuste with this parameter:
```
const int sampleRate = 3700;
```
All timer specific settings are made within file `Timer.h`.
The header provides a `namespace Timer` to access the relevant timer specifics.
Only the timer interrupt routine is required to be passed into the `Timer`.
Take a look into `setup` to see this:
```
Timer::setup(sampleRate, measure);
```


# Main Program
`main` is responsible to push all content out of the ring buffer to its reciever as fast as possible.
Possible recievers are SPI or USB.
There is a compiler flag to control this:
```
#define USE_SPI
```
It defaults to UART over USB when the flag is not defined.
This happens in `loop` routine.

Furthermore it is possible to switch between text or binary when using UART over USB.
There is a compiler flag to control this:
```
#define USE_BINARY
```
For details jump into `pushUART` to how its done.


# Ring Buffer
The current ring buffer implementation is a generic one using a template with compile time static size.
All ring buffer specifics are found in file `RingBuf.h`.
The signature of the type is as follows:
```
template<typename T, int S> class RingBuf { ... };
```
**There is only a single global variable:**
```
RingBuf<MODEL, bufferCapacity> rbuf;
```
Only the `main` and the `timer` interrupt program access it from file `mkrzero-timer-adc`.

# Data Model
The data model is defined in file `Data.h`.
Data model is customizable and can be extended to an extended version containing useful diagnostic information.
There is a compiler flag to control this:
```
#define USE_DIAGNOSTICS
```
There are two spots which need to change to make this work.

## Data Definition
The data definition uses an enumeration `enum Col` for all pieces of content:
```
  #ifdef USE_DIAGNOSTICS
    Time, SampleLength, InterruptLength, 
  #endif
```
These colums will not be included when the flag is missing.

## Data Acquisition
The other part which requires special care is the data acquisition.
All the data acquisition is done synchronously during the timer interrupt.
Timings are setup carefully to suit the requirements at hand.
Data acquisition takes place in `measure` routine.

# Data
The target data is acquired as follows:
```
m.words[Col::Id] = ++counter % 256;
m.words[Col::Data0] = Analog::read(A0);
m.words[Col::Data1] = Analog::read(A1);
m.words[Col::Data2] = Analog::read(A2);
```
These are:
- an identifier based on an 8 bit count.
- the analog signal data from signal line A0.
- the analog signal data from signal line A1.
- the analog signal data from signal line A2.


The compiler flag `USE_DIAGNOSTICS` enables the output of the following extra information:
```
m.words[Col::Time] = t1 % 256;
m.words[Col::InterruptLength] = interruptLength;
m.words[Col::SampleLength] = micros() - t1;
```
These are:
- the current time, but using only 8 bits of information.
- the time how long the interupt routine took since the last interrupt.
- the time how long the sample acquisition took within this interrupt.

# ADC
All analog specifics are defined inside file `ADC.h` and exposed using the `namespace Analog`.
Initialization occurs within `setup` using `Analog::setup`.
Data can be fetched using `Analog::read()`.
This is an optimized version of analog reading to meet the requirements at hand.