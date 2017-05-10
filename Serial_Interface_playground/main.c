#include <msp430.h> 
#include "Serial_JMPv2.2.h"
#include <stdlib.h>
/*
 * main.c
 */

//UART Interrupt Vector


#define picAddr 0x50

void main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
    I2C_setup(400000);
    USB_setup(9600);
	_BIS_SR(GIE);

	unsigned char picOutput;
	char keystroke;

	while(1){
		USB_getchar(&keystroke, 'b');
		if(keystroke == 'r'){
			USB_println("Reading byte from pic...");
			I2C_read(picAddr, 0, &picOutput, 1);
			USB_putchar(picOutput);
			USB_println(" <--- Read this");
		}
		keystroke = 0;
	}
}
