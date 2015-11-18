#include <18f47j53.h>
#device ADC=12
#include "main.h"
#include "hardware.h"
#include "outputs.h"

extern bool Delestage_enable;
extern unsigned int16 l_ADC1_max, l_ADC1_min, l_ADC2_max, l_ADC2_min;

unsigned int16 l_ADC1_max_acc=0, l_ADC2_max_acc=0, l_ADC1_min_acc=0, l_ADC2_min_acc=0;

bool flagValidFrame = false;
char FrameTeleinfo[SIZE_TELEINFO] = {0};
char FrameTeleinfoStored[SIZE_TELEINFO] = {0};
char FrameIndex = 0;
char FrameIndexStored = 0;
int16 lNbrADC = 0;

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

    setup_timer_1(T1_FOSC);
    set_timer1(TIMER1_100US);

    setup_adc(ADC_CLOCK_DIV_64);
    setup_adc_ports(sAN10 | sAN12);
    set_adc_channel(RELAIS1_ADC);

    // setup_wd(WDT_ON);
    enable_interrupts(INT_RDA);
    enable_interrupts(INT_SSP);
    enable_interrupts(INT_TIMER1);
    enable_interrupts(INT_AD);
    enable_interrupts(PERIPH);
    enable_interrupts(GLOBAL);
    
    input(ZCD);
    input(CONTACT_IO);
    input(ONE_WIRE);
    PORT_D_PULLUPS(true);
    // PORT_A_PULLUPS(true);
    
    Init_outputs();
}

void average_and_store(unsigned int16 ADCmax1, unsigned int16 ADCmax2, unsigned int16 ADCmin1, unsigned int16 ADCmin2)
{
    static int8 iNbr_acc = 0;

    if(iNbr_acc < 16)
    {
        iNbr_acc ++;
        l_ADC1_max_acc += ADCmax1;
        l_ADC2_max_acc += ADCmax2;
        l_ADC1_min_acc += ADCmin1;
        l_ADC2_min_acc += ADCmin2;
    }
    else
    {
        l_ADC1_max = l_ADC1_max_acc>>4;
        l_ADC1_min = l_ADC1_min_acc>>4;
        l_ADC2_max = l_ADC2_max_acc>>4;
        l_ADC2_min = l_ADC2_min_acc>>4;
        
        iNbr_acc = 0;
        l_ADC1_max_acc = 0;
        l_ADC2_max_acc = 0;
        l_ADC1_min_acc = 0;
        l_ADC2_min_acc = 0;
    }
}

void max_min(int8 id, unsigned int16 val)
{
    static unsigned int16 ADCmax1 = 0, ADCmax2 = 0, ADCmin1 = 0xFFFF, ADCmin2 = 0xFFFF;
    if(id == 1)
    {
        if(val > ADCmax1)
        {
            ADCmax1 = val;
        }
        if(val < ADCmin1)
        {
            ADCmin1 = val;
        }
    }
    else if(id == 2)
    {
        if(val > ADCmax2)
        {
            ADCmax2 = val;
        }
        if(val < ADCmin2)
        {
            ADCmin2 = val;
        }
    }
    else
    {
        average_and_store(ADCmax1, ADCmax2, ADCmin1, ADCmin2);
        ADCmax1 = 0;
        ADCmax2 = 0;
        ADCmin1 = 0xFFFF;
        ADCmin2 = 0xFFFF;
    }
}

#INT_TIMER1
void timer1_int()
{
    set_timer1(TIMER1_100US);
    read_adc(ADC_START_ONLY);
}

#INT_AD
void adc_int()
{
    unsigned int16 val = read_adc(ADC_READ_ONLY);
    
    // Read 200 times on ADC 1, interval is 100us, so we read an entire 50Hz period
    if(lNbrADC < 200)
    {
        lNbrADC++;
        max_min(1, val);

        if(lNbrADC == 199)
        { // last iteration for ADC1
            set_adc_channel(RELAIS2_ADC);
        }
    }
    else
    {
        lNbrADC++;
        max_min(2, val);
        
        if(lNbrADC == 399) 
        {
            set_adc_channel(RELAIS1_ADC);
            max_min(0,0);
            lNbrADC = 0;
        }
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
