#include <18f47j53.h>
#include "outputs.h"
#include "main.h"
#include "hardware.h"

extern char FrameTeleinfoStored[SIZE_TELEINFO];
extern char FrameIndexStored;
extern BYTE Registers_table[16];

bool Delestage_enable = true;
unsigned long lADC_1 = 0, lADC_2 = 0;
t_Radiator Rads[6];

void Init_outputs()
{
    Rads[0].Positive = FP1A;
    Rads[0].Negative = FP1B;
    Rads[1].Positive = FP2A;
    Rads[1].Negative = FP2B;
    Rads[2].Positive = FP3A;
    Rads[2].Negative = FP3B;
    Rads[3].Positive = FP4A;
    Rads[3].Negative = FP4B;
    Rads[4].Positive = FP5A;
    Rads[4].Negative = FP5B;
    Rads[5].Positive = FP6A;
    Rads[5].Negative = FP6B;

    int i;
    for(i = 0; i < 16; i++)
        Registers_table[i] = 0;

    Update_outputs();
}

void Radiator(t_Order Order, int8 id)
{
    if(Rads[id].CurrentOrder != Order)
    {
        switch(Order)
        {
            case ARRET:
                output_high(Rads[id].Positive);
                output_low(Rads[id].Negative);
            break;
            case HORS_GEL:
                output_low(Rads[id].Positive);
                output_high(Rads[id].Negative); 
            break;
            case ECO:
                output_high(Rads[id].Positive);
                output_high(Rads[id].Negative);            
            break;
            case CONFORT:
                output_low(Rads[id].Positive);
                output_low(Rads[id].Negative);            
            break;
            default:
                break;
        }
    Rads[id].CurrentOrder = Order;        
    }
}

void Detection_delestage()
{
    // First check the Frame (size, crc...)
    if(FrameTeleinfoStored[0] == 0x0A && FrameIndexStored > 2)
    {
        int8 i;
        int16 checksum = 0;
        // last is CR, -1 is CRC
        for(i=1; i < (FrameIndexStored-1); i++)
        {
            checksum += FrameTeleinfoStored[i];
        }
        checksum &= 0x3F;
        checksum += 0x20;
        if(FrameTeleinfoStored[FrameIndexStored-1] == checksum) // valid frame
        {
            if(FrameTeleinfoStored[1] == 'A' &&
               FrameTeleinfoStored[2] == 'D' &&
               FrameTeleinfoStored[3] == 'P' &&
               FrameTeleinfoStored[4] == 'S' &&
               FrameTeleinfoStored[5] == 0x20)
            {
                Registers_table[0] = 0;
                Registers_table[1] = 0;
                Registers_table[2] = 0;
                Registers_table[3] = 0;
                Registers_table[4] = 0;
                Registers_table[5] = 0;
                Update_outputs();
            }
        }
    }
}

/*Table de registres :

@0 = FP0 (1/2/3/4)
........
@5 = FP5
@6 = relai 1 (bit 0)
@7 = relai 2 (bit 0)

@8 = bit7 = delestage enable
     bit1:0 = ordre de delestage = 00 tout en meme temps
                                   01 FP puis relais
                                   10 Relais puis FP
@9/10 = ADC1 Read only
@11/12 = ADC2 Read only
@13 toggle led
*/
// Update_output() : Update the output according to Registers_Table
void Update_outputs()
{
    bool bRelai_1 = false, bRelai_2 = false;
    int8 i = 0;
    
    for(i = 0; i < 6; i++)
    {
        Radiator(Registers_table[i], i);
    }
    bRelai_1 = Registers_table[6] & 0x01 ? true : false;
    bRelai_2 = Registers_table[7] & 0x01 ? true : false;
    Update_Relais(bRelai_1, bRelai_2);

    Delestage_enable = Registers_table[8] & 0x80 ? true : false;
    // ordre de delestage = Registers_table[8] & 0x03;

    Registers_table[9] = lADC_1 >> 8;
    Registers_table[10] = lADC_1 & 0xFF;
    Registers_table[11] = lADC_2 >> 8;
    Registers_table[12] = lADC_2 & 0xFF;

    if((Registers_table[13]&0x0F) == 0)
        output_low(LED1);
    else if((Registers_table[13]&0x0F) == 0x01)
        output_high(LED1);
    else
        output_toggle(LED1);

    if((Registers_table[13]&0xF0) == 0)
        output_low(LED2);
    else if((Registers_table[13]&0xF0) == 0x10)
        output_high(LED2);
    else
        output_toggle(LED2);
}

// Update the relais state : because they are bistable, we need to check if the desired state changed before changing the output
void Update_Relais(bool bRelai_1, bool bRelai_2)
{
    static bool bLastRelai_1 = true, bLastRelai_2 = true; // true allow to disable the relais at startup :)

    if(bLastRelai_1 != bRelai_1)
    {
        if(bRelai_1)
        {
            output_high(RELAIS1_ON);
            output_low(RELAIS1_OFF);
        }
        else
        {
            output_low(RELAIS1_ON);
            output_high(RELAIS1_OFF);
        }
        delay_ms(DELAIS_RELAIS_MS);
        output_low(RELAIS1_ON);
        output_low(RELAIS1_OFF);
    }
    if(bLastRelai_2 != bRelai_2)
    {
        if(bRelai_2)
        {
            output_high(RELAIS2_ON);
            output_low(RELAIS2_OFF);
        }
        else
        {
            output_low(RELAIS2_ON);
            output_high(RELAIS2_OFF);
        }
        delay_ms(DELAIS_RELAIS_MS);    
        output_low(RELAIS2_ON);
        output_low(RELAIS2_OFF);    
    }

    bLastRelai_1 = bRelai_1;
    bLastRelai_2 = bRelai_2;
}
