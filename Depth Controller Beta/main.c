#include <msp430.h> 
#include "Serial_JMP.h"
#include "General_JMP.h"
#include "typecast.h"
#include "MS5837-30BA.h"


/*
 * main.c
 */
int main(void) {
	 WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	_BIS_SR(GIE);
	I2C_setup(400000);			//Start I2C communication at 100k bps
	USB_setup(9600);

	reset_sequence();
	read_factory_calibration();


	while(1){
		read_conversions();
		//test_case();


		calculate_pressure();
		_no_operation();
		USB_print_value("Pressure is: ", P);
		USB_print_value("     Temperature is: ", TEMP);
		USB_print("\r\n");

	return 0;
}
