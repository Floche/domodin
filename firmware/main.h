#ifndef MAIN_H
#define MAIN_H
// Here shall lie only the declaration of functions and defines
#ifndef bool
#define bool int1
#define false 0
#define true 1
#endif

#define SIZE_TELEINFO   64
#define END_OF_LABEL    0x0D

#use delay(clock=48M)
/* RS232 configuration */
#use rs232(baud=115200,xmit=PIN_D3,rcv=PIN_D2)
#use i2c(SLAVE, SCL=PIN_B4, SDA=PIN_B5, address=0xA0)
#fuses WDT512, INTRC_PLL_IO, PLL2, PLLEN, NOCPUDIV

void Init_hard();

#endif
