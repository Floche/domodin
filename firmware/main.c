#include <18f47j53.h>
#include "hardware.h"
#include "outputs.h"
#include "main.h"

#build (reset=0x400, interrupt=0x408)

#define BOOTLOADER_PIN      PIN_B4
#define BOOTLOADER_PIN_BIS  PIN_B5

#byte PIC_SSPSTAT=0xFC7
extern bool Delestage_enable;

bool flagValidFrame = false;
char FrameTeleinfo[SIZE_TELEINFO] = {0};
char FrameTeleinfoStored[SIZE_TELEINFO] = {0};
char FrameIndex = 0;
char FrameIndexStored = 0;

BYTE I2C_table_index, Registers_table[16] = {0};


void main()
{
	flagValidFrame = 0;
    Init_hard();
    while(1)
    {
    	// restart_wd();
	    if(flagValidFrame && Delestage_enable)
	    {
	    	Detection_delestage();
	    	flagValidFrame = false;
	    }
        Update_outputs();
            
        delay_ms(200);
    }
}

void Init_hard()
{
    output_high(LED1);
    output_low(LED2);
	// setup_wd(WDT_ON);
    // enable_interrupts(PERIPH);
    enable_interrupts(INT_RDA);
    enable_interrupts(INT_SSP);
    // enable_interrupts(INT_TIMER0);
    // enable_interrupts(INT_AD);
	enable_interrupts(GLOBAL);
    
    input(ZCD);
    PORT_D_PULLUPS(true);
    
    Init_outputs();
}

#INT_RDA
void serial_isr()
{ 
    FrameTeleinfo[FrameIndex] = getc(); 
    putc(FrameTeleinfo[FrameIndex]);
    FrameIndex++;

    if(FrameTeleinfo[FrameIndex-1] == END_OF_LABEL)
    {
        memcpy(FrameTeleinfoStored, FrameTeleinfo, FrameIndex);
        FrameIndexStored = FrameIndex;
        FrameIndex = 0;
        flagValidFrame = true;
    }
}

#INT_SSP
void ssp_interupt()
{
    BYTE incoming, state;

    state = i2c_isr_state();
    //printf("state = %X, statebis = %X\n", state, PIC_SSPSTAT);

    if(state < 0x80)                     //Master is sending data
    {
        // state is the index of the I2C byte received
        //First received byte is meant to be the index in the table
        //then it is the data of the table
        incoming = i2c_read();
        if(state == 1)                     
        {
            I2C_table_index = incoming;
        }
        else if(state > 1 && I2C_table_index < 16)
        {
            Registers_table[I2C_table_index] = incoming;
            I2C_table_index++;
        }
    }
    if(state >= 0x80)                     //Master is requesting data
    {
        if(I2C_table_index < 16)
        {
            i2c_write(Registers_table[I2C_table_index]);
            I2C_table_index++;
        }
        else
            i2c_write(0xFF);
    }
}

#org 0, 0x3FF {}