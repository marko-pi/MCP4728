# MCP4728
Raspberry Pi library for the MCP4728 Quad DAC chip.  It allows
- reading/writing the address by bitbanging at three arbitrary GPIO pins,
- changing the output voltages by kernel procedures on both i2c-0 and i2c-1 busses.

The core of the project is an OOP (object oriented programming) compatible C library (MCP4728.so).  There are two reasons for this step:
- There is no need for any other library (RPi.GPIO, pigpio...) to execute the code;
- The bitbanging part of the library is much faster and time consistent (see below).

Reading and writing address for the MCP4728 chip requires an additional LDAC line and is not I2C protocol compliant.  Therefore bitbanging must be used.  The target frequency for bitbanging is 100 kHz, but the de facto frequency is about 90 kHz.  Executing C programs is simply not as efficient as using kernel procedures.

![PulseView screenshot](/two_chips_read_address.png)

The output voltages are changed uses kernel procedures.

The library provides eight public functions: two functions for initialisation/de-initialisation, two functions for reading/writing the address and four functions for changing the output voltages.  The description of the functions is given in the Python test file

A simple Python script is provided as an example of how to use the library. A proper Python library could easily be created to wrap up all procedures into a standard Python class. Since I assume that only advanced users will use this library, I think that the Python example script will be sufficient.

When the chip operates in internal source mode, it offers gain=1 for voltages up to 2.048 and gain=2 for voltages up to 4.096 or VDD, whichever is smaller.  The library makes automatic choice there, gain=1 when a voltage up to 2 V is requested and gain=2 when a voltage above 2 V is requested.
