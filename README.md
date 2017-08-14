Series of arduino examples that emerged during my spare time project 

# SPI

Within this section i am trying to figure out how to communicate over SPI using two different boards:
An [Arduino MKRZero](https://store.arduino.cc/arduino-mkrzero) and
  a [SparkFun SAMD21 Mini Breakout](https://github.com/sparkfun/SAMD21_Mini_Breakout).
I will reference these boards for simplicity, `mkrzero` and `samd21mini` respectively.
Regard this section as a collection of template programs for certain communication patterns.
It is also a little comparison between both boards, but mainly focuses on the communication part.
But lets start with a small collection of references for both boards.

## Cheat Sheet

Both are using the same *Atmel [ATSAMD21G18](http://www.microchip.com/wwwproducts/en/ATSAMD21G18) µc*.
This facilitates porting programs between both boards.
However, board layouts and periphereals differ.
Best source for these information are the corresponding github repositories and manufacturer pages.
Thanks to the arduino platform both repositories have a similar structure.
Here are the most important files together with other technical references in comparison:

| mkrzero | samd21mini |
|---------|------------|
| [repo](https://github.com/arduino/ArduinoCore-samd/tree/master)                                        | [repo](https://github.com/sparkfun/Arduino_Boards/tree/master/sparkfun/samd) |
| [variants dir](https://github.com/arduino/ArduinoCore-samd/tree/master/variants/mkrzero)               | [variants dir](https://github.com/sparkfun/Arduino_Boards/tree/master/sparkfun/samd/variants/SparkFun_SAMD_Mini) |
| [variant.h](https://github.com/arduino/ArduinoCore-samd/blob/master/variants/mkrzero/variant.h)        | [variant.h](https://github.com/sparkfun/Arduino_Boards/blob/master/sparkfun/samd/variants/SparkFun_SAMD_Mini/variant.h) |
| [variant.cpp](https://github.com/arduino/ArduinoCore-samd/blob/master/variants/mkrzero/variant.cpp)    | [variant.cpp](https://github.com/sparkfun/Arduino_Boards/blob/master/sparkfun/samd/variants/SparkFun_SAMD_Mini/variant.cpp) |
| [manufacturer](https://store.arduino.cc/arduino-mkrzero)                                               | [manufacturer](https://github.com/sparkfun/SAMD21_Mini_Breakout) |
| [getting started](https://www.arduino.cc/en/Guide/ArduinoMKRZero)                                      | [getting started](https://learn.sparkfun.com/tutorials/samd21-minidev-breakout-hookup-guide/resources--going-further) |
| [schematic](https://www.arduino.cc/en/uploads/Main/ArduinoMKRZero-schematic.pdf)                       | [schematic](https://cdn.sparkfun.com/assets/learn_tutorials/4/5/4/sparkfun-atsamd21g-breakout-schematic.pdf) |
| [pcb backside](http://files.linuxgizmos.com/arduino_mkrzero_back.jpg) (nothing comparable)             | [graphical datasheet](https://cdn.sparkfun.com/assets/learn_tutorials/4/5/4/graphicalDatasheet-Mini.pdf) |

## Program's Origin

Programs that appear in slave configuration are based on one particular source.
User *Tom_the_banana* posted it [on the Arduino forum](https://forum.arduino.cc/index.php?topic=360026.0).
Thanks to this i was able to jump start with a simple slave configuration right away.
I appreciate his code very much :)
However i found some adaptions useful in general and others necessary to my underlying use case.

## Communication Patterns

### Unidirectional Slave Send and Master Recieve

The code of *Tom_the_banana* can be classified as what i call a **slave reciever**.
That is, because it is a slave that is mainly designed to recieve data from its master.
Therefore, i turned the program into a **slave sender**.
Furthermore, there is a second program to form the **slave sender**'s counter part what i call a **master reciever**.
These terms will make it easier to identify certain communication patterns.
At least during debugging when things are simplified in order to track down a certain problem.
However, these terms only apply to unidirectional communication patterns.
Well, or at least to those which have a clear focus to it.

### Bidirectional Loopback (TODO)

Within bidirectional communication patterns there is this notion of a **loopback** program.
Data that arrives is just send back to its sender.
This can be implemented on both a master and a slave device.
That is useful for debugging certain problems as well and is going to be included later on.

### Forwarder (TODO)

TODO

## Slave Send Program

The slave send program provides several options about what kind of data should be send to the master.
There is an implementation of this program for the MKRZero.
Available options are:

| expectation | description |
|-------------|-------------|
| <img src="./screenshots/slave-send-1_expected-const.png" align="left" width="200px" />          | constant bytes |
| <img src="./screenshots/slave-send-2_expected-counter.png" align="left" width="200px" />        | simple counters resulting in sawtooth shapes |
| <img src="./screenshots/slave-send-3_expected-counter-mod512.png" align="left" width="200px" /> | simple counters with smaller mod to better focus on certain bits |
| <img src="./screenshots/slave-send-4_expected-bit9range.png" align="left" width="200px" />      | ranged counters around the 9th bit that is good to illustrate a problem described below (not solved yet) |
| <img src="./screenshots/slave-send-5_expected-complex.png" align="left" width="200px" />        | complex datasets including sine wave shapes and real time counter |
