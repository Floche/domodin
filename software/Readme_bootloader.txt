To compile bootloader.c on RaspberryPi, you need
the library WiringPi, and then :

gcc -Wall -o bootloader bootloader.c -lwiringPi

To use bootloader, move domodin.hex next to it, and launch it.
