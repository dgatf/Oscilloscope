# Oscilloscope Serial

A simple oscilloscope application that reads the values at serial port generated by an Arduino board and based on the value of the analog pin A0

<p align="center"><img src="./images/img1.png" width="600"><br>
  <i>Main window</i><br><br></p>

<p align="center"><img src="./images/img2.png" width="300"><br>
  <i>Settings</i><br><br></p>

It is optimized for AVR ATmega MCUs with 26μs sampling resolution. For other boards the resolution is 200μs

Flash _oscilloscope_. In the Oscilloscope app configure the serial port. By default 1000000bps, 8bits and no parity. Interval is 26μs for AVR ATmega and 200μs for other MCUs

You can connect any signal up to 5 volts to the analog pin A0. For signals over 5V a voltage divider is needed.

For ATmega boards it is defined a PWM signal at pin OC1B. To test the Oscilloscope app connect pin OC1B to pin A0
Adjust PWM signal with PWM_FREQ and PWM_DUTY. Default is 1000Hz, 50% duty.

Pin OC1B location:
 - Uno:        pin 10 (PB2)
 - Leonardo:   pin 10 (PB6)
 - ATmega2560: pin 12 (PB6)

Find binaries [here](https://github.com/dgatf/Oscilloscope/tree/main/binaries)

Or build from sources:
- install qt (>=5.9)
- cd to src/qt folder
- qmake
- make

Supported OS:

 - Linux
 - Windows
 - Android (experimental)
 - MacOS (not tested)
