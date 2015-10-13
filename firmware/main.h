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

#define TIMER0_1SEC		65535-46875
#define TIMER1_100US	65535-4800

#use delay(clock=48M)
/* RS232 configuration */
#use rs232(baud=1200,bits=7,xmit=PIN_C6,rcv=PIN_C7, PARITY=E, STOP=1)
#use i2c(SLAVE, SCL=PIN_B4, SDA=PIN_B5, address=0xA0)
#fuses WDT512, INTRC_PLL_IO, PLL2, PLLEN, NOCPUDIV

#build (reset=0x400, interrupt=0x408)
#org 0, 0x3FF {}

void Init_hard();

#endif
