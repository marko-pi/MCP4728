/*
   Marko Pinteric 2020
   GPIO communication based on Tiny GPIO Access on http://abyz.me.uk/rpi/pigpio/examples.html

   Header for MCP4728.c
*/

#define UNDEFINED 0xFFFF

/* TINY GPIO VARIABLES */

#define GPSET0 7
#define GPSET1 8

#define GPCLR0 10
#define GPCLR1 11

#define GPLEV0 13
#define GPLEV1 14

#define GPPUD     37
#define GPPUDCLK0 38
#define GPPUDCLK1 39

#define PI_BANK (gpio>>5)
#define PI_BIT  (1<<(gpio&0x1F))

/* gpio modes */

#define PI_INPUT  0
#define PI_OUTPUT 1
#define PI_ALT0   4
#define PI_ALT1   5
#define PI_ALT2   6
#define PI_ALT3   7
#define PI_ALT4   3
#define PI_ALT5   2

/* values for pull-ups/downs off, pull-down && pull-up */

#define PI_PUD_OFF  0
#define PI_PUD_DOWN 1
#define PI_PUD_UP   2

/* chip data */

struct chip
{
   unsigned sda;
   unsigned scl;
   unsigned ldac;
   unsigned address;
   unsigned bus;
};


/* ---------------- public functions ----------------------- */

/* initialises communications */
struct chip *initialise(int sda, int scl, int ldac, int address);

/* deinitialise communications */
int deinitialise(struct chip *tempchip);

/* gets the DAC address */
int getaddress(struct chip *tempchip);

/* sets the DAC address */
int setaddress(struct chip *tempchip, unsigned addr);

/* writes single value to the selected DAC channel using internal reference - channels 1 to 4 */
int singleinternal(struct chip *tempchip, int channel, float volt, bool eeprom);

/* writes single value to the selected DAC channel using external reference - channels 1 to 4 */
int singleexternal(struct chip *tempchip, int channel, float rel, bool eeprom);

/* writes four values to the DAC channels using internal reference */
int multipleinternal(struct chip *tempchip, float volts[], bool eeprom);

/* writes four values to DAC channels using external reference */
int multipleexternal(struct chip *tempchip, float rels[], bool eeprom);

