Purpose of this program is to gather UART data on the `rpi` from the mkrzero using some program like the [ringbuf/mkrzero-timer-adc](../../ringbuf/mkrzero-timer-adc/README.md) program.
It has been useful to stress certain scenarios without simplifiying too much from a real application.

# Anatomy
This program compiles to the `rpi` but is also compatible to `macosx`.
Connection is tried to be established only at the beginnning of the program.
Later it tries to read either an amount of bytes.
Amount of bytes can be setup in two ways: infinite and fixed.
To cancel infinite use `CTRL`+`C` .

# Quickstart

Checkout the repository folder, cd to the programs folder and then compile the code:
```
git clone git@github.com:isaias-b/arduino-examples.git
cd serial/rpi-uart-reciever
.bin/compile
```

Now there should be an executable binary file `.bin/run` next to `.bin/compile`.
```
$ ls -lah .bin/
total 40
drwxr-xr-x  5 isaias  staff   170B Dec 28 00:28 .
drwxr-xr-x  9 isaias  staff   306B Dec 28 02:14 ..
-rwxr-xr-x  1 isaias  staff    45B Dec 27 20:26 compile
-rwxr-xr-x  1 isaias  staff    49B Dec 28 00:27 compile-O3
-rwxr-xr-x  1 isaias  staff   9.0K Dec 28 00:28 run
```

Call it with `.bin/run` or if you have the `.bin` folder on your `path` like me, then just call:
```
$ run
try opening serial port /dev/ttyACM1...
Error - Unable to open UART.  Ensure it is not in use by another application
now reading bytes...
nothing to read
nothing to read
...
...
nothing to read
^C11
closing port /dev/cu.usbmodem1411...
```
**Don't forget to call `CTRL`+`C` to quit**


The upper output states that no device is connected to `/dev/ttyACM1` which is the current default in the program.
If in fact there is no device connected onto `/dev/ttyACM1` proceed.
Connect your `mkrzero` or any other UART over USB board, which runs some sort of sender program.
A program like the [ringbuf/mkrzero-timer-adc](../../ringbuf/mkrzero-timer-adc/README.md).
Once its uploaded and hooked up lookup the device name on your machine.

For arduino like boards on the mac use:
```
$ ls /dev/cu.usb*
/dev/cu.usbmodem1411
```

On the rpi use:
```
$ ls /dev/ttyACM*
/dev/cu.usbmodem1411
```

To tell the program to stop after a fixed amount of bytes, e.g. 20 bytes, and to use a different device name, call:
```
$ run /dev/cu.usbmodem1411 20
overwrite default port /dev/ttyACM1 with /dev/cu.usbmodem1411
overwrite default #bytes-to-read -1 with 20
try opening serial port /dev/cu.usbmodem1411...
now reading 20 bytes...
1385  1385
0 62  1381  1385  1385
0 63  1381  1385  1385
1 64  1381  1385  1385
1 65  1381  1385  1385
0 66  1381  1385  1385
1 67  1382
closing port /dev/cu.usbmodem1411...
```


# Interpretation
This strongly depends on your sender, but this will focus on the [ringbuf/mkrzero-timer-adc](../../ringbuf/mkrzero-timer-adc/README.md) program.
The output from above shows 5 complete samples separated by newlines:
```
0 62  1381  1385  1385
0 63  1381  1385  1385
1 64  1381  1385  1385
1 65  1381  1385  1385
0 66  1381  1385  1385
```
Each column is separated by a `\t` character.
The first column is the buffer size and is just a diagnostic information for the current benchmark.
It doesn't belong to the [target data of ringbuf/mkrzero-timer-adc](../../ringbuf/mkrzero-timer-adc/README.md#data).
However, the following 4 columns represent the 4 from the target data model:
- an identifier based on an 8 bit count.
- the analog signal data from signal line A0.
- the analog signal data from signal line A1.
- the analog signal data from signal line A2.


# Analysis
Now, the buffer size output serves as buffer overflow detector.
In such a case it will predominantly raise until it reaches the static buffer limit.
How fast can the reciever pull out the data over the UART signal lines from the senders device buffers?
What influences the throughput?

A small comparison between: mac and rpi.
The mkrzero serves data with `3300` samples per second.

|              | mac  | rpi  |
|--------------|------|------|
|fast enough   | yes  | no   |
|avg           | 15±5 | n/a  |
|max           | 45   | 2047 |
|cpu           | ~10% | ~10% |

## Mac
```
13  229 1382  1385  1365
13  230 1381  1385  1385
14  231 1382  1365  1385
14  232 1382  1365  1385
13  233 1382  1365  1381
13  234 1385  1385  1381
14  235 1382  1365  1365
14  236 1382  1366  1366
13  237 1381  1369  1385
14  238 1382  1369  1385
14  239 1381  1385  1365
13  240 1381  1385  1385
```
The mac is able to reliably pull out the data.
When the mac is stressed during process schedule the throuput drops and the buffer size raises.
During idle periods the mac is capable to pull out more elements from the buffer to normalize buffer size levels.

After some data crunching this became evident for even 500000 bytes:
```
$ cat data/result.csv | tr '\n' ','
1,2,3,4,5,5,5,6,6,6,6,6,7,8,9,10,10,10,11,12,12,11,11,12,12,12,13,13,13,13,13,13,13,13,13,13,13,13,12,12,12,12,12,12,12,11,11,12,13,13,13,13,12,12,13,13,12,11,10,10,11,12,12,11,11,11,11,12,12,11,10,10,11,11,10,10,10,10,10,10,10,10,11,11,11,12,12,11,10,10,11,11,11,12,13,13,13,13,13,13,13,13,12,11,11,12,12,11,11,12,13,13,13,13,12,11,11,11,11,11,11,12,13,14,15,16,17,18,18,18,19,20,21,22,23,24,24,24,25,26,27,27,27,28,28,28,28,28,29,30,30,30,30,29,28,28,28,27,27,27,27,27,27,27,27,27,27,27,26,25,24,23,22,21,21,22,22,22,22,21,20,20,20,19,18,17,16,15,15,16,17,17,16,16,16,15,15,16,16,15,14,13,13,13,12,11,10,10,11,11,11,11,10,10,10,10,11,12,13,14,15,16,17,17,17,18,19,20,21,22,23,24,25,26,27,28,29,29,29,30,31,32,33,34,35,36,37,37,37,38,38,37,36,35,34,33,32,31,31,31,30,29,28,27,26,26,26,25,24,23,22,21,21,21,20,19,18,17,17,17,16,15,15,15,15,16,16,15,15,15,15,16,17,17,16,16,17,18,18,18,18,17,16,16,17,18,18,17,16,16,17,17,16,15,14,13,12,11,10,10,11,12,13,13,12,11,11,11,11,12,12,12,12,12,13,13,13,13,12,11,11,12,12,12,13,13,12,12,12,12,13,13,12,11,10,10,10,10,11,11,11,11,10,10,10,10,10,10,11,11,10,10,10,10,11,12,12,11,11,12,12,12,13,13,12,11,10,10,10,10,10,10,11,12,12,12,13,13,12,12,12,11,11,11,10,10,11,12,12,12,13,13,12,12,12,12,12,12,12,12,13,13,12,11,11,11,11,12,13,13,12,12,12,11,11,11,11,11,10,9,9,10,11,11,11,12,13,13,12,11,11,12,13,13,12,12,13,13,12,11,10,10,11,11,11,12,12,12,12,12,13,13,12,11,10,10,11,11,11,11,11,11,10,10,10,10,11,11,10,10,11,11,10,10,11,11,10,10,10,9,9,10,11,11,10,10,10,9,9,10,10,10,11,11,11,12,12,11,11,11,10,10,11,12,13,13,12,11,11,12,13,13,12,11,10,10,10,10,11,12,12,12,12,11,11,12,12,12,12,12,12,12,12,11,11,12,12,12,13,13,12,12,12,11,11,11,10,10,11,11,11,11,10,10,11,12,12,12,12,11,11,12,12,11,10,10,11,12,12,12,13,14,15,16,17,18,19,20,21,22,22,21,21,22,23,24,25,26,27,28,28,28,28,28,29,30,31,32,33,34,35,36,37,37,37,37,37,38,39,40,40,40,40,40,41,42,43,44,45,46,46,46,47,47,46,46,46,45,45,45,45,45,45,46,46,45,44,43,42,42
```
There was a rise and fall of 13 to 38 and again back to 10.


## RPI
```
$ cat data/500.log
overwrite default port /dev/ttyACM1 with /dev/ttyACM1
overwrite default #bytes-to-read -1 with 500
try opening serial port /dev/ttyACM1...
now reading 500 bytes...
1370 1365
5   235 1446 1445 1445
8   236 1429 1434 1429
12  237 1429 1366 1429
16  238 1366 1370 1381
19  239 1365 1685 1689
23  240 1685 1686 1385
27  241 1685 1690 1685
30  242 1689 1685 1686
34  243 1690 1685 1685
37  244 1685 1689 1689
41  245 1684 1685 1685
45  246 1685 1689 1689
48  247 1685 1686 1386
52  248 1690 1366 1365
56  249 1685 1433 1380
59  250 1689 1433 1429
63  251 1386 1445 1429
66  252 1365 1429 1446
70  253 1365 1445 1445
74  254 1433 1446 1370
77  255 1434 1366 1365
81  0   1433 1370 1382
85  1   1445 1381 1381
89  2   1445 1385 1365
92  3   1366 1369 1370
96  4   1381 1369 1366
100 5   1385 1365 1365
103 6   1369 1365 2369
107 7   1370 2373 2373
110 8   1366 2305 2373
114 9   2369 2389 2385
118 10  2369 2305 2389
121 11  2373 2305 2305
125 12  2372 2309 2309
129 13  2389 2309 2309
132 14  2305 2309 2308
136 15  2305 2309 2309
140 16  2389 2305 2305
143 17  2389 2373 2385
147 18  2389 2373 2373
150 19  2372 2372 2372
154 20  2372 2372 2372
158 21  2373 2373 2373
161 22  2372 2373 2372
165 23  2373 2372 2369
169 24  2373 2369 2369
172 25  2372 2369 2369
176 26  2369 2369 2369
179 27  2369 2369 2369
183 28  2369 2369 1620
187 29
closing port /dev/ttyACM1...
```
However, the rpi is not able to even reduce the buffer size once within the first 5000 bytes being requested.

And this is reproducible for at least 10 calls, all producing similar or even worse results:
```
$ run /dev/ttyACM1 | head -n -4 | tail -n 5
161 22  2372 2373 2372
165 23  2373 2372 2369
169 24  2373 2369 2369
172 25  2372 2369 2369
176 26  2369 2369 2369
```

Even when compiled with `-O3` and using `setpriority`.
Although, i must admit that i'm a newbie to kind of otpimisations and maybe there is something obvious missing here.

The CPU kept constantly around 10%.
So i don't acually know where to look for this bottleneck.

