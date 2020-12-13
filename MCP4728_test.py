import time
from ctypes import cdll, c_int, c_float, c_bool, c_void_p, POINTER

###### Wrapping C library ######
MCP4728 = cdll.LoadLibrary("./MCP4728.so")

"""
# object = initialise(sda, scl, ldac, address)

Creates an instance of the chip.  The initialisation type depends on the SDA, SCL and LDAC pins provided.  If LDAC is irregular, reading and writing the chip address is disabled.  If SDA and SCL do not match the predefined I2C pins, changing the output voltages is disabled.  Both i2c-0 and i2c-1 busses can be used.  Do not forget to provide pull-up resistors for the LDAC line and the I2C-0 lines.

Examples:

chip = initialise(2,3,-1,0x61)       # opens an instance of the chip at address 0x61 and allows changing the output voltages over the i2c-1 bus (the chip address is mandatory)
chip = initialise(16,20,21,-1)       # opens an instance of the chip at an unknown address and allows reading/writing the address
chip = initialise(0,1,16,-1)         # opens an instance of the chip at an unknown address and allows both reading/writing the address and changing the output voltages over the i2c-0 bus
"""
initialise = MCP4728.initialise
initialise.restype = c_void_p
initialise.argtypes = [c_int, c_int, c_int, c_int]

"""
deinitialise(object)

Removes the instance of the chip.  Recommended but not mandatory at the end of the program.
"""
deinitialise = MCP4728.deinitialise
deinitialise.argtypes = [c_void_p]

"""
address = getaddress(object)

Reads the current chip address.  The obtained address is automatically stored by the library.  LDAC mandatory.
"""
getaddress = MCP4728.getaddress
getaddress.argtypes = [c_void_p]

"""
setaddress(object, address)

Writes the new chip address.  The current chip address must be supplied by the programmer or read with getaddress beforehand.  LDAC mandatory.
"""
setaddress = MCP4728.setaddress
setaddress.argtypes = [c_void_p, c_int]

"""
singleinternal(object, channel, voltage, eeprom)

Sets the absolute voltange on the channel using the internal voltage reference.  The eeprom indicates whether the voltage is written to the EEPROM of the chip.  SDA/SLC must be 0/1 or 2/3.
"""
singleinternal = MCP4728.singleinternal
singleinternal.argtypes = [c_void_p, c_int, c_float, c_bool]

"""
singleexternal(object, channel, voltage, eeprom)

Sets the relative voltange on the channel using the external voltage reference.  The eeprom indicates whether the voltage is written to the EEPROM of the chip.  SDA/SLC must be 0/1 or 2/3.
"""
singleexternal = MCP4728.singleexternal
singleexternal.argtypes = [c_void_p, c_int, c_float, c_bool]

"""
multipleinternal(object, [voltages], eeprom)

Sets the absolute voltanges on all four channels using the internal voltage reference.  The eeprom indicates whether the voltages are written to the EEPROM of the chip.  SDA/SLC must be 0/1 or 2/3.
"""
multipleinternal = MCP4728.multipleinternal
multipleinternal.argtypes = [c_void_p, POINTER(c_float), c_bool]

"""
multipleexternal(object, [voltages], eeprom)

Sets the relative voltanges on all four channels using the external voltage reference.  The eeprom indicates whether the voltages are written to the EEPROM of the chip.  SDA/SLC must be 0/1 or 2/3.
"""
multipleexternal = MCP4728.multipleexternal
multipleexternal.argtypes = [c_void_p, POINTER(c_float), c_bool]


###### Test example ######

# creates instances of two chips with unknown addresses, both reading/writing the address and changing the output voltages over the i2c-1 bus allowed.
chip1 = initialise(2,3,16,-1)
chip2 = initialise(2,3,20,-1)

# gets both addresses
res1 = getaddress(chip1)
res2 = getaddress(chip2)

print('Chip I2C addresses: 0x%x 0x%x' % (res1, res2))

# sets four absolute voltages on the first chip
voltages=[1.0,1.5,2.0,2.5]
floats = (c_float*4)(*voltages) # voltage list has to be trasferred in the C compatible array
multipleinternal(chip1,floats,False)

# sets four absolute voltages on the second chip
voltages=[0.5,1.0,1.5,2.0]
floats = (c_float*4)(*voltages) # voltage list has to be trasferred in the C compatible array
multipleinternal(chip2,floats,False)

# sets one absolute voltage on the first chip on channel 4
singleinternal(chip1,4,3.0,False)

# sets one absolute voltage on the second chip on channel 4
singleinternal(chip2,4,2.5,False)

# removes instances of two chips
deinitialise(chip1)
deinitialise(chip2)

# this should be added if voltages are written to the EEPROM accoring to the manual
time.sleep(0.05)
