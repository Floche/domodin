#include <18f47j53.h>
#device ADC=12
#include "hardware.h"
#include "outputs.h"
#include "main.h"

extern bool Delestage_enable;
extern unsigned long l_ADC1, l_ADC2;

bool flagValidFrame = false;
char FrameTeleinfo[SIZE_TELEINFO] = {0};
char FrameTeleinfoStored[SIZE_TELEINFO] = {0};
char FrameIndex = 0;
char FrameIndexStored = 0;
unsigned long l_ADC1_MAX = 0, l_ADC2_MAX = 0;
int8 iNbrADC = 0;

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
        delay_ms(10);
    }
}

void Init_hard()
{
    output_high(LED1);
    output_low(LED2);

    setup_timer_0(T0_DIV_256 | T0_INTERNAL);
    setup_timer_1(T1_FOSC);
    set_timer0(TIMER0_1SEC);
    set_timer1(TIMER1_100US);

    setup_adc(ADC_CLOCK_INTERNAL);
    setup_adc_ports(RELAIS1_ADC | RELAIS2_ADC);

    // setup_wd(WDT_ON);
    enable_interrupts(INT_RDA);
    enable_interrupts(INT_SSP);
    enable_interrupts(INT_TIMER0);
    disable_interrupts(INT_TIMER1);
    enable_interrupts(INT_AD);
    enable_interrupts(PERIPH);
    enable_interrupts(GLOBAL);
    
    input(ZCD);
    PORT_D_PULLUPS(true);
    
    Init_outputs();
}

/*timer interrupt toutes les 1s -> 
    lance timer interrupt toutes les 100us, reset valeur max -> 
    lance adc
interrupt adc -> read et stocke valeur max, compte 100 fois, si 100e fois, stock valeur max dans tableau i2c et desactive timer 100us, 
*/
#INT_TIMER0
void timer0_int()
{
    enable_interrupts(INT_TIMER1);
    set_timer1(TIMER1_100US);
    l_ADC1_MAX = 0;
    l_ADC2_MAX = 0;
    set_adc_channel(RELAIS1_ADC);

    set_timer0(TIMER0_1SEC);
}

#INT_TIMER1
void timer1_int()
{
    read_adc(ADC_START_ONLY);
    set_timer1(TIMER1_100US);
}

#INT_AD
void adc_int()
{
    unsigned long val = read_adc(ADC_READ_ONLY);
    if(iNbrADC < 100)
    {
        iNbrADC++;
        if(val > l_ADC1_MAX)
            l_ADC1_MAX = val;
    }
    else if(iNbrADC < 200)
    {
        iNbrADC++;
        set_adc_channel(RELAIS2_ADC);
        if(val > l_ADC2_MAX)
            l_ADC2_MAX = val;        
    }
    else
    {
        l_ADC1 = l_ADC1_MAX;
        l_ADC2 = l_ADC2_MAX;

        disable_interrupts(INT_TIMER1);
        iNbrADC = 0;
    }
}

#INT_RDA
void serial_isr()
{
    FrameTeleinfo[FrameIndex] = getc(); 
    putc(FrameTeleinfo[FrameIndex]);

    if(FrameTeleinfo[FrameIndex] == END_OF_LABEL)
    {
        memcpy(FrameTeleinfoStored, FrameTeleinfo, FrameIndex+1);
        FrameIndexStored = FrameIndex;
        FrameIndex = 0;
        flagValidFrame = true;
    }

    FrameIndex = (FrameIndex == (SIZE_TELEINFO-1)) ? 0 : FrameIndex+1;
}

#INT_SSP
void ssp_interupt()
{
    BYTE incoming, state;

    state = i2c_isr_state();

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
