#include <msp430.h> 
#include "../MSP430\ -\ Libs/Serial_JMP.h"



/*
 * Created by Jose A. Montes Perez
 * 	On Dec 21, 2016
 *
 * This code is designed to interface the
 * sparkfun RAzor 9dog IMU[1] with the msp430f559lp.
 * The code is inspired via the firmware found in [2].
 *
 *
 * Wiring diagram:
 * P3.3 -> TX@Razor
 * P3.4 -> RX@Razor
 *
 * USB -> PC COM PORT
 *
 * [1]https://www.sparkfun.com/products/retired/10736
 * [2]https://github.com/Razor-AHRS/razor-9dof-ahrs
 *
 *
 * main.c
 */




unsigned char *inputBuffer[36];

float acc[3];
float mag[3];
float gyr[3];


/****************************************************************************************************
 *Function Description
 * 	This Function allows you to manually control the output rate of the IMU in non-continuous mode.
 * The parameter milliseconds defines the refresh rate and has a minimum of 20ms and a maximum of
 * 10000ms.
 *
 *
 *
 *****************************************************************************************************/
void TIMER_setup(int milliseconds){
	if(milliseconds > 20 && milliseconds < 1000){
		int operations = ((long) milliseconds << 16)/1000;	// Conversion from milliseconds to discrete operations
		TA0CCR0 = operations;								// Set frequency in CCR0 register
		TA0CTL = TASSEL_2 + MC_1 + TACLR;					// Set clock to ACLK, Up mode, Clear counter
		TA0CCTL0 = CCIE;									// Enable interrupts
	}
}
// Complimentary ISR to timer. This sends the request for the data to the IMU
#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void){
	__bic_SR_register_on_exit(LPMO_bits);
}

/****************************************************************************************************
 * Function description:
 * 	This function outputs the raw data in csv format to the microcontroller which then
 * sends it to the computer via USB. Once the calibration parameters have been obtained
 * we can locally filter them or we can place them in the firmware.
 *
 * 	Note that you will need something to capture the CSV output of the microcontroller
 * on the attached PC.
 *
 *
 ***************************************************************************************************/

void calibration_mode(){

	UART_print("#o0");			//Setup the razor for non-continuous stream
	UART_print("#osrt");		//Set the output to raw data in text format

}



void main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	UART_setup(57600);
    USB_setup(9600);
    calibration_mode();
    TIMER_setup(30);
    while(1){
    	UART_print("#f");
    	int i;
    	for(i = 0; i < 36; i++){
    		UART_receive_byte(inBuffer[i]);
    	}
    	USB_print(inBuffer);
    	__bis_SR_register(LPM0_bits);
    }
}
