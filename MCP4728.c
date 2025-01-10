/*
    Marko Pinteric 2020
    GPIO communication based on Tiny GPIO Access on http://abyz.me.uk/rpi/pigpio/examples.html

    to compile shared object: gcc -o MCP4728.so -shared -fPIC MCP4728.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <stdbool.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

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

/* GPIO address */
static volatile uint32_t  *gpioReg = MAP_FAILED;

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

/* LOCAL VARIABLES */

#define UNDEFINED 0xFFFF

struct chip
{
    unsigned sda;
    unsigned scl;
    unsigned ldac;
    unsigned address;
    unsigned bus;
};

struct chip *curchip;

/* struct chip mychipdata;
struct chip *mychip = &mychipdata; */

struct timespec ttime;
/* communication initialised */
bool init_gpio=false, init_i2c_0=false, init_i2c_1=false;
/* I2C address */
int file_i2c_0, file_i2c_1;
/* I2C clients count */
int count_i2c_0=0, count_i2c_1=0;

/* TINY GPIO METHODS */

void gpioSetMode(unsigned gpio, unsigned mode)
{
    int reg, shift;

    reg   =  gpio/10;
    shift = (gpio%10) * 3;

    gpioReg[reg] = (gpioReg[reg] & ~(7<<shift)) | (mode<<shift);
}

int gpioGetMode(unsigned gpio)
{
    int reg, shift;

    reg   =  gpio/10;
    shift = (gpio%10) * 3;

    return (*(gpioReg + reg) >> shift) & 7;
}

void gpioSetPullUpDown(unsigned gpio, unsigned pud)
{
    *(gpioReg + GPPUD) = pud;
    usleep(20);
    *(gpioReg + GPPUDCLK0 + PI_BANK) = PI_BIT;
    usleep(20);
    *(gpioReg + GPPUD) = 0;
    *(gpioReg + GPPUDCLK0 + PI_BANK) = 0;
}

int gpioRead(unsigned gpio)
{
    if ((*(gpioReg + GPLEV0 + PI_BANK) & PI_BIT) != 0) return 1;
    else return 0;
}

void gpioWrite(unsigned gpio, unsigned level)
{
    if (level == 0) *(gpioReg + GPCLR0 + PI_BANK) = PI_BIT;
    else *(gpioReg + GPSET0 + PI_BANK) = PI_BIT;
}

int gpioInitialise(void)
{
    int fd;

    fd = open("/dev/gpiomem", O_RDWR | O_SYNC) ;
    if (fd < 0)
    {
        fprintf(stderr, "failed to open /dev/gpiomem\n");
        return -1;
    }
    gpioReg = (uint32_t *)mmap(NULL, 0xB4, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);

    if (gpioReg == MAP_FAILED)
    {
        fprintf(stderr, "Bad, mmap failed\n");
        return -1;
    }
    return 0;
}

/* LOCAL GPIO METHODS */
/* all assume && leave SCL low, except when specified differently */

/* start GPIO communication */
void start_gpio()
{
    gpioWrite(curchip->sda,0);
    gpioWrite(curchip->scl,0);
    gpioWrite(curchip->ldac,0);
    gpioSetPullUpDown(curchip->sda,PI_PUD_OFF);
    gpioSetPullUpDown(curchip->scl,PI_PUD_OFF);
    gpioSetPullUpDown(curchip->ldac,PI_PUD_OFF);
    gpioSetMode(curchip->sda, PI_INPUT);
    gpioSetMode(curchip->scl, PI_INPUT);
    gpioSetMode(curchip->ldac, PI_INPUT);
}

/* stop GPIO communication */
void stop_gpio()
{
    if ((curchip->bus == 0) || (curchip->bus == 1))
    {
        gpioSetPullUpDown(curchip->sda,PI_PUD_UP);
        gpioSetPullUpDown(curchip->scl,PI_PUD_UP);
        gpioSetMode(curchip->sda, PI_ALT0);
        gpioSetMode(curchip->scl, PI_ALT0);
    }
}

void clock_wait()
{
    struct timespec ctime;
    uint64_t ntime;
    ntime = ttime.tv_sec * (uint64_t)1000000000L + ttime.tv_nsec + 5000;
    while(1)
    {
        clock_gettime(CLOCK_MONOTONIC,&ctime);
        if (ctime.tv_sec * (uint64_t)1000000000L + ctime.tv_nsec >= ntime) return;
    }
}

/* following method assumes SCL && SDA high */
void i2cstart()
{
    gpioSetMode(curchip->sda, PI_OUTPUT);
    clock_gettime(CLOCK_MONOTONIC,&ttime);
    clock_wait();
    gpioSetMode(curchip->scl, PI_OUTPUT);
    clock_gettime(CLOCK_MONOTONIC,&ttime);
    clock_wait();
    clock_gettime(CLOCK_MONOTONIC,&ttime);
}

void i2crestart()
{
    gpioSetMode(curchip->sda, PI_INPUT);
    clock_gettime(CLOCK_MONOTONIC,&ttime);
    clock_wait();
    gpioSetMode(curchip->scl, PI_INPUT);
    clock_gettime(CLOCK_MONOTONIC,&ttime);
    clock_wait();
    gpioSetMode(curchip->sda, PI_OUTPUT);
    clock_gettime(CLOCK_MONOTONIC,&ttime);
    clock_wait();
    gpioSetMode(curchip->scl, PI_OUTPUT);
    clock_gettime(CLOCK_MONOTONIC,&ttime);
    clock_wait();
    clock_gettime(CLOCK_MONOTONIC,&ttime);
}

/* following methods leave SCL && SDA high */
void i2cstop()
{
    gpioSetMode(curchip->sda, PI_OUTPUT);
    clock_gettime(CLOCK_MONOTONIC,&ttime);
    clock_wait();
    gpioSetMode(curchip->scl, PI_INPUT);
    clock_gettime(CLOCK_MONOTONIC,&ttime);
    clock_wait();
    gpioSetMode(curchip->sda, PI_INPUT);
    clock_gettime(CLOCK_MONOTONIC,&ttime);
    clock_wait();
    clock_gettime(CLOCK_MONOTONIC,&ttime);
}

unsigned i2cgetbyte()
{
    unsigned data=0x00;
    unsigned i;

    gpioSetMode(curchip->sda, PI_INPUT);
    clock_gettime(CLOCK_MONOTONIC,&ttime);
    for (i=1; i<=8; i++)
    {
        /* slave sends bit */
        clock_wait();
        gpioSetMode(curchip->scl, PI_INPUT);
        clock_gettime(CLOCK_MONOTONIC,&ttime);
        data = data << 1;
        clock_wait();
        /* I2C clock stretching */
        while(gpioRead(curchip->scl) == 0) ;
        if (gpioRead(curchip->sda) == 1) data = data | 0x01;
        gpioSetMode(curchip->scl, PI_OUTPUT);
        clock_gettime(CLOCK_MONOTONIC,&ttime);
    }
    return(data);
}

void i2csendbyte(unsigned data)
{
    unsigned i;

    for (i=1; i<=8; i++)
    {
        if ((data & 0x80) == 0) gpioSetMode(curchip->sda, PI_OUTPUT);
        else gpioSetMode(curchip->sda, PI_INPUT);
        clock_gettime(CLOCK_MONOTONIC,&ttime);
        data = data << 1;
        clock_wait();
        gpioSetMode(curchip->scl, PI_INPUT);
        clock_gettime(CLOCK_MONOTONIC,&ttime);
        clock_wait();
        gpioSetMode(curchip->scl, PI_OUTPUT);
    }
    clock_gettime(CLOCK_MONOTONIC,&ttime);
}

unsigned i2cgetack()
{
    unsigned result;

    gpioSetMode(curchip->sda, PI_INPUT);
    /* slave sends bit */
    clock_gettime(CLOCK_MONOTONIC,&ttime);
    clock_wait();
    gpioSetMode(curchip->scl, PI_INPUT);
    clock_gettime(CLOCK_MONOTONIC,&ttime);
    clock_wait();
    if (gpioRead(curchip->sda) == 0) result=1;
    else result=0;
    /* read SDA */
    gpioSetMode(curchip->scl, PI_OUTPUT);
    clock_gettime(CLOCK_MONOTONIC,&ttime);
    return(result);
}

void i2csendack()
{
    gpioSetMode(curchip->sda, PI_OUTPUT);
    clock_gettime(CLOCK_MONOTONIC,&ttime);
    clock_wait();
    gpioSetMode(curchip->scl, PI_INPUT);
    clock_gettime(CLOCK_MONOTONIC,&ttime);
    clock_wait();
    gpioSetMode(curchip->scl, PI_OUTPUT);
}

void i2csendnack()
{
    gpioSetMode(curchip->sda, PI_INPUT);
    clock_gettime(CLOCK_MONOTONIC,&ttime);
    clock_wait();
    gpioSetMode(curchip->scl, PI_INPUT);
    clock_gettime(CLOCK_MONOTONIC,&ttime);
    clock_wait();
    gpioSetMode(curchip->scl, PI_OUTPUT);
}

/* LOCAL I2C METHODS */

/* set I2C address */
int address_i2c()
{
    int file_i2c;

    if (curchip->bus == 0) file_i2c=file_i2c_0;
    if (curchip->bus == 1) file_i2c=file_i2c_1;

    if(ioctl(file_i2c, I2C_SLAVE, curchip->address) < 0) return(-1);
    return(0);
}

/* write multiple raw values to the specified DAC channels - channels 1 to 4, EEPROM not affected */
int multiple_raw(unsigned size, unsigned channels[], unsigned reference, unsigned gains[], unsigned values[])
{
    unsigned i;
    unsigned char buffer[3*size];
    int file_i2c;

    if (curchip->bus == 0) file_i2c=file_i2c_0;
    if (curchip->bus == 1) file_i2c=file_i2c_1;

    for (i = 0; i < size; i++)
    {
        /* first byte is 0b01000XXU, where XX is channel address (0-3) */
        buffer[3*i] = 0x40 | ((channels[i]-1) << 1);
        /* second word is XPPYDDDD DDDDDDDD, where X is reference, Y is gain && D is data */
        buffer[3*i+1] = (((values[i] >> 8) & 0x0F) | (reference << 7) | (gains[i] << 4));
        buffer[3*i+2] = values[i] & 0xFF;
    }
    if (write(file_i2c, buffer, 3*size) != 3*size) return(-1);
    return(0);
}

/* write four sequential raw values to the all DAC channels - channels 1 to 4, EEPROM affected */
int sequential_raw(unsigned reference, unsigned gains[], unsigned values[])
{
    unsigned i;
    unsigned char buffer[9];
    int file_i2c;

    if (curchip->bus == 0) file_i2c=file_i2c_0;
    if (curchip->bus == 1) file_i2c=file_i2c_1;

    /* first byte is 0101000U for writing starting at channel */
    buffer[0]=0x50;
    for (i = 0; i < 4; i++)
    {
        /* second word is XPPYDDDD DDDDDDDD, where X is reference, Y is gain && D is data */
        buffer[2*i+1] = (((values[i] >> 8) & 0x0F) | (reference << 7) | (gains[i] << 4));
        buffer[2*i+2] = values[i] & 0xFF;
    }
    if (write(file_i2c, buffer, 9) != 9) return(-1);
    return(0);
}

/* write single raw value to the selected DAC channel - channels 1 to 4, EEPROM affected */
int single_raw(unsigned channel, unsigned reference, unsigned gain, unsigned value)
{
    unsigned char buffer[3];
    int file_i2c;

    if (curchip->bus == 0) file_i2c=file_i2c_0;
    if (curchip->bus == 1) file_i2c=file_i2c_1;

    /* first byte is 01011XXU, where XX is channel address (0-3) */
    buffer[0]=0x58 | ((channel-1) << 1);
    /* second word is XPPYDDDD DDDDDDDD, where X is reference, Y is gain && D is data */
    buffer[1] = (((value >> 8) & 0x0F) | (reference << 7) | (gain << 4));
    buffer[2] = value & 0xFF;
    if (write(file_i2c, buffer, 3) != 3) return(-1);
    return(0);
}

/* GLOBAL INITIALIZATION METHODS */

/* initialise communications */
struct chip *initialise(int sda, int scl, int ldac, int address)
{
    struct chip *tempchip = malloc(sizeof(struct chip));

    if((sda>27) || (sda<0) || (scl>27) || (scl<0)) fprintf(stderr, "SDA and SCL out of range\n");
    if((address>0x07) || (address<0x00)) address = 0x60 | address;
    if((address>0x67) || (address<0x60)) address=UNDEFINED;
    if((ldac>27) || (ldac<0)) ldac=UNDEFINED;
    tempchip->sda=(unsigned)sda;
    tempchip->scl=(unsigned)scl;
    tempchip->ldac=(unsigned)ldac;
    tempchip->address=(unsigned)address;
    tempchip->bus=UNDEFINED;

    if ((ldac !=UNDEFINED) && !init_gpio)
    {
        if (gpioInitialise() < 0) return(NULL);
        init_gpio = true;
    }

    if ((sda == 0) && (scl == 1))
    {
        tempchip->bus=0;
        count_i2c_0 = count_i2c_0 + 1;
        if (!init_i2c_0)
        {
             char *filename = (char*)"/dev/i2c-0";
             file_i2c_0 = open(filename, O_RDWR);
             if (file_i2c_0 < 0) fprintf(stderr, "Failed to open the i2c-0 bus\n");
             else init_i2c_0=true;
        }
    }

    if ((sda == 2) && (scl == 3))
    {
        tempchip->bus=1;
        count_i2c_1 = count_i2c_1 + 1;
        if (!init_i2c_1)
        {
             char *filename = (char*)"/dev/i2c-1";
             file_i2c_1 = open(filename, O_RDWR);
             if (file_i2c_1 < 0) fprintf(stderr, "Failed to open the i2c-1 bus\n");
             else init_i2c_1=true;
        }
    }
    return(tempchip);
}

/* deinitialise communications */
int deinitialise(struct chip *tempchip)
{
    if (tempchip->bus == 0) count_i2c_0 = count_i2c_0 - 1;
    if (tempchip->bus == 1) count_i2c_1 = count_i2c_1 - 1;

    if ((init_i2c_0) && (count_i2c_0 == 0))
    {
        close(file_i2c_0);
        init_i2c_0 = false;
    }
    if ((init_i2c_1) && (count_i2c_1 == 0))
    {
        close(file_i2c_1);
        init_i2c_1 = false;
    }

    free(tempchip);
    return(0);
}

/* GLOBAL GPIO METHODS */

/* get the DAC address */
int getaddress(struct chip *tempchip)
{
    unsigned i;
    int ret;
    unsigned errs[4], addr[2], res, err=0;

    curchip=tempchip;
    if (!init_gpio || (curchip->ldac==0)) return(0x1000);
    clock_gettime(CLOCK_MONOTONIC,&ttime);
    start_gpio();
    i2cstart();
    i2csendbyte(0x00);
    errs[0]=i2cgetack();
    i2csendbyte(0x0C);
    gpioSetMode(curchip->ldac, PI_OUTPUT);
    errs[1]=i2cgetack();
    i2crestart();
    i2csendbyte(0xC1);
    gpioSetMode(curchip->ldac, PI_INPUT);
    errs[2]=i2cgetack();
    res=i2cgetbyte();
    i2csendnack();
    i2cstop();

    addr[0] = (res & 0xE0) >> 5;
    addr[1] = (res & 0x0E) >> 1;

    if ((addr[0] != addr[1]) || ((res & 0x11) != 0x10)) errs[3] = 0;
    else errs[3] = 1;
    for (i=0; i<=3; i++)
    {
        if (errs[i]==0) err = err | (0x08 >> i);
    }
    if (err>0)
    {
        ret=-(int)err;
        curchip->address = UNDEFINED;
    }
    else
    {
        res = 0x60 | addr[0];
        curchip->address = res;
        ret=(int)res;
    }
    stop_gpio();

    return(ret);
}

/* set the DAC address */
int setaddress(struct chip *tempchip, unsigned addr)
{
    unsigned i;
    int ret;
    unsigned errs[4], err=0;
    unsigned addr_cur, addr_new;

    curchip=tempchip;
    if (!init_gpio || (curchip->ldac==0) || (curchip->address==0)) return(0x1000);
    start_gpio();
    clock_gettime(CLOCK_MONOTONIC,&ttime);
    addr_cur = curchip->address & 0x07;
    addr_new = addr & 0x07;

    i2cstart();
    i2csendbyte(0xC0 | (addr_cur << 1));
    errs[0]=i2cgetack();
    i2csendbyte(0x61 | (addr_cur << 2));
    gpioSetMode(curchip->ldac, PI_OUTPUT);
    errs[1]=i2cgetack();
    i2csendbyte(0x62 | (addr_new << 2));
    errs[2]=i2cgetack();
    i2csendbyte(0x63 | (addr_new << 2));
    gpioSetMode(curchip->ldac, PI_INPUT);
    errs[3]=i2cgetack();
    i2cstop();

    for (i=0; i<=3; i++)
    {
        if (errs[i]==0) err = err | (0x08 >> i);
    }
    if (err>0) ret=-(int)err;
    else
    {
        curchip->address = addr;
        ret = 0;
    }
    stop_gpio();

    return(ret);
}

/* GLOBAL I2C METHODS */

/* write single value to the selected DAC channel using internal reference - channels 1 to 4 */
int singleinternal(struct chip *tempchip, int channel, float volt, bool eeprom)
{
    unsigned gain=1;
    unsigned value;

    curchip=tempchip;
    if (curchip->bus == UNDEFINED) return(-1);
    if (address_i2c() == -1) return(-2);

    if(volt>2) gain=2;
    value=(unsigned)(0x1000 * volt/2.048/gain);
    if(eeprom) return(single_raw((unsigned)channel,1,gain-1,value));
    else return(multiple_raw(1,(unsigned[]){(unsigned)channel},1,(unsigned[]){gain-1},(unsigned[]){value}));
}

/* write single value to the selected DAC channel using external reference - channels 1 to 4 */
int singleexternal(struct chip *tempchip, int channel, float rel, bool eeprom)
{
    unsigned value;

    curchip=tempchip;
    if (curchip->bus == UNDEFINED) return(-1);
    if (address_i2c() == -1) return(-2);

    value=(unsigned)(0x1000 * rel);
    if(eeprom) return(single_raw((unsigned)channel,0,0,value));
    else return(multiple_raw(1,(unsigned[]){(unsigned)channel},0,(unsigned[]){0},(unsigned[]){value}));
}

/* write four values to the DAC channels using internal reference */
int multipleinternal(struct chip *tempchip, float volts[], bool eeprom)
{
    unsigned i;
    unsigned gain;
    unsigned values[4];
    unsigned gains[4];

    curchip=tempchip;
    if (curchip->bus == UNDEFINED) return(-1);
    if (address_i2c() == -1) return(-2);

    for (i = 0; i < 4; i++)
    {
        gain=1;
        if(volts[i]>2) gain=2;
        gains[i] = gain-1;
        values[i] = (unsigned)(0x1000 * volts[i]/2.048/gain);
    }
    if(eeprom) return(sequential_raw(1,gains,values));
    else return(multiple_raw(4,(unsigned[]){1,2,3,4},1,gains,values));
}

/* write four values to DAC channels using external reference */
int multipleexternal(struct chip *tempchip, float rels[], bool eeprom)
{
    unsigned i;
    unsigned values[4];

    curchip=tempchip;
    if (curchip->bus == UNDEFINED) return(-1);
    if (address_i2c() == -1) return(-2);

    for (i = 0; i < 4; i++)
    {
        values[i]=(unsigned)(0x1000 * rels[i]);
    }
    if(eeprom) return(sequential_raw(0,(unsigned[]){0,0,0,0},values));
    else return(multiple_raw(4,(unsigned[]){1,2,3,4},0,(unsigned[]){0,0,0,0},values));
}
