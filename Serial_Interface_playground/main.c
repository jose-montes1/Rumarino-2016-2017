#include <msp430.h> 
#include "Serial_JMP.h"
#include <stdlib.h>
/*
 * main.c
 */

//UART Interrupt Vector

void main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    UART_setup(9600);
    USB_setup(57600);
	_BIS_SR(GIE);

	char buffer[500];
	//buffer = (char *) malloc(100);


	UART_print("#o0");
	__delay_cycles(500000);
	while(1){
		UART_print("#f");
		UART_receive_line(buffer);
		//UART_print(buffer);
		//UART_transmit_byte('\0');
		//UART_transmit_byte('\n');
		//UART_transmit_byte('\r');
		USB_print(buffer);


	}


}
