#include <18f47j53.h>
#use delay(clock=48M)
#fuses NOWDT
#fuses INTRC_PLL_IO, PLL2, PLLEN, NOCPUDIV
#fuses NOPROTECT
#fuses NODEBUG
#fuses STVREN

/* RS232 configuration */
#use rs232(baud=115200,parity=N,xmit=PIN_C6,rcv=PIN_D2,bits=8)
#use i2c(SLAVE, SCL=PIN_B4, SDA=PIN_B5, address=0xA0)
