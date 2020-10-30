# MCP4728
Raspberry Pi library for the MCP4728 chip.  It allows
- reading/writing the address by bitbanging at three arbitrary GPIO pins
- changing the output voltages by kernel procedures on both i2c-0 and i2c-1 busses.

The core of the project is an OOP (object oriented programming) compatible C library (MCP4728.so).  There are two reasons this step are:
- no other for any other library (RPi.GPIO, pigpio...) is necessary to execute the code;
- the bitbanging part of the library is much faster and time consistent.

Reading and writing address for MCP4728 chip requires additional LDAC line and is not I2C protocol compliant.  Therefore bitbanging has to be used.  The target bitbanging frequency is 100 kHz, but de-facto frequency is about 90 kHz.  Running C program is simply not as efficient as using kernel procedures.

Changing the output voltages uses kernel procedures.

The library provides eight public functions: two functions for initialization/deinitialization, two functions for reading/writing the address and four functions for changing the output voltages.

Simple Python script is provided as an example for using the library.  A proper Python library could be easily created to wrap up all the procedures into a standard Python class.  Since I expect that only advanced users will use this library, I think that the example Python script will suffice and therefore I haven't yet committed to write one.

When working in the internal source mode, chip provides gain=1 for voltages up to 2.048 and gain=2 for voltages up to 4.096 or VDD, whatever is smaller.  The library makes automatic choice there, gain=1 if voltage up to 2 V is requested and gain=2 if voltage above 2 V i requested.
