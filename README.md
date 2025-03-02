# MCP4728
Raspberry Pi library for the MCP4728 Quad DAC chip.  It allows
- reading/writing the address by bitbanging at three arbitrary GPIO pins,
- changing the output voltages by kernel procedures on both `i2c-0` and `i2c-1` busses.

Reading and writing address for the MCP4728 chip requires an additional `LDAC` line and is not compliant with the `I2C` protocol, so bitbanging must be used. Do not forget to provide a pull-up resistor for the `LDAC` line!

The core of the project is an OOP (object oriented programming) compatible C library.  There are two reasons for this step:
- There is no need for any other library (`RPi.GPIO`, `pigpio`...) to execute the code;
- The bitbanging part of the library is much faster and time consistent.

To create C library execute `make` (library in local directory) or `make install` (shared library).

Version 2 also offers additional improvements:
- A more precise 10kHz timing;
- The SDA line has the 7% clock period delay, mimicking Raspberry Pi's I2C kernel performance.

Running C programs is simply not as efficient as using kernel procedures, and it is still possible for the Raspberry Pi to be distracted by other operations.  If less than 7% of a total of 50% of the clock period remains until the next change, the timing is reset to ensure at least 7% of the clock period.

![PulseView screenshot](/one_chip_read_address.png)

The output voltages are changed by kernel procedures.

On [this page](http://www.pinteric.com/raspberry.html#dac) you will find the application circuits with some additional instructions.

The library provides eight public functions: two functions for initialisation/de-initialisation, two functions for reading/writing the address and four functions for changing the output voltages.  The description of the functions is given in the Python test file (`MCP4728_test.py`).

A simple Python script is provided as an example of how to use the library. A proper Python library could easily be created to wrap all procedures in a standard Python class. Since I assume that only advanced users will use this library, I think that the sample Python script is sufficient.

When the chip operates in internal source mode, it provides `gain=1` for voltages up to 2.048 V and `gain=2` for voltages up to 4.096 V or VDD, whichever is smaller.  The library automatically selects `gain=1` when a voltage up to 2 V is requested and `gain=2` when a voltage above 2 V is requested.
