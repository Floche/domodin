#include <wiringPi.h>
#include <stdio.h>
#include <wiringSerial.h>

#define PIN_RST 7
#define BOOTLOADER_PIN1	9
#define BOOTLOADER_PIN2	8

int main (void)
{
	int fSerial = 0;
	FILE * filehex;
	char hexBuf[256];
	int line = 0;

	wiringPiSetup();
	fSerial = serialOpen("/dev/ttyAMA0",115200);
	if(fSerial == -1)
	{
		printf("error while opening tty\r\n");
		return 1;
	}

	filehex = fopen("domodin.hex","r");
	if(filehex == NULL)
	{
		printf("error while opening hex file\r\n");
		return 2;
	}

	pinMode (PIN_RST, OUTPUT);
	pinMode (BOOTLOADER_PIN1, OUTPUT);
  	pinMode (BOOTLOADER_PIN2, OUTPUT);

	// Set IO to reset the PIC and to enable its bootloader
	digitalWrite (BOOTLOADER_PIN2,  LOW);
	digitalWrite (BOOTLOADER_PIN1,  HIGH);

	// reset PIC
	digitalWrite (PIN_RST, HIGH);
	delay(10);
	digitalWrite(PIN_RST, LOW);

	// disable the bootloader mode
	delay(10);
	digitalWrite(BOOTLOADER_PIN1, LOW);

	serialFlush(fSerial);
	while(fgets(hexBuf, sizeof(hexBuf), filehex) != NULL)
  	{
		// only transmit HEX file lines with proper start code (':' character)
		if(hexBuf[0] != ':') continue;

		// transmit
		serialPuts(fSerial, hexBuf);

		if(serialGetchar(fSerial) != 0x06)
		{
			printf("Error line %d !\r\n", line);
			break;
		}
		line++;
	}

	serialClose(fSerial);
	fclose(filehex);
	printf("New firmware loaded !\r\n");
	return 0 ;
}

