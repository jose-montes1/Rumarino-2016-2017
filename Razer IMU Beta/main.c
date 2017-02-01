#include <msp430.h> 
#include <stdlib.h>
#include "Serial_JMP.h"



/*
 * Created by Jose A. Montes Perez
 * 	On Dec 21, 2016
 *
 * This code is designed to interface the
 * sparkfun RAzor 9dog IMU[1] with the msp430f559lp.
 * The code is inspired via the firmware found in [2].
 *
 * Depends on Serial_JMP.h
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
 *
 */


char inBuffer[36];

char buff1[50];
char buff2[50];
char buff3[50];

char *sensorTextInput[] = {buff1, buff2, buff3};

float acc[3];	// Holds the accelerometer values in  x,y,z format
float mag[3];	// Holds the magnetometer values in   x,y,z format
float gyr[3];	// Holds the gyroscope values in      x,y,z format


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
	__bic_SR_register_on_exit(LPM0_bits);
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

	//UART_print("#o0");			//Setup the razor for non-continuous stream
	UART_print("#o0");			//Setup the razor for continuous stream
	UART_print("#oscb");		//Set the output to raw data in text format
	//UART_print("#osrb");		//Set the output to raw data in binary format
	
}

/****************************************************************************************************
 * Function description:
 * 	This function reads one frame of information and writes it into the global variables
 *
 ***************************************************************************************************/


void calibration_read(){
	//UART_print("#f");


	int i;
	/*
	for(i = 0; i < 3; i++){
		UART_receive_line(*(sensorTextInput+i));
	}
	char **buff = sensorTextInput;
	USB_print(*buff++);
	USB_print(*buff++);
	USB_print(*buff);

}*/

	//Reading floating point values (RAW)
	for(i = 0; i < 36; i++){
		UART_receive_byte(&inBuffer[i]);
	}
	for(i = 0; i < 3; i++){
		acc[i] = *(float *)&(inBuffer+4*i);
		mag[i] = *(float *)&(inBuffer+4*i+12);
		gyr[i] = *(float *)&(inBuffer+4*i+24);
	}
	unsigned char *buff;
	snprintf(buff, 300, "acc: x-%f, y-%f, z-%f\n\rmag: x-%f, y-%f, z-%f\n\rgyr: x-%f, y-%f, z-%f", acc[0], acc[1], acc[2], mag[0], mag[1], mag[2], gyr[0], gyr[1], gyr[2]);
	USB_print(buff);
	

}

/*
TODO - Test this area
void read_sensors_mode(){

	UART_print("#o0");			//Setup the razor for non-continuous stream
	UART_print("#osct");		//Set the output to calibrated data in text format
	//UART_print("#oscb");		//Set the output to calibrated data in binary format TODO - have to make binary parser
	int i;
	for(i = 0; i < 36; i++){
		UART_receive_byte(inBuffer[i]);
	}
	USB_print(inBuffer);
}

void read_angles_mode(){

	UART_print("#o0");			//Setup the razor for non-continuous stream
	UART_prinnt("#oc");			//Setup the razor for calibrated output
	UART_print("#ot");			//Set the output for angles in text format
	//UART_print("#ob");		//Set the output for angles in binary format
	int i;
	for(i = 0; i < 9; i++){
		UART_receive_byte(inBuffer[i]);
	}
	inBuffer[9] = 0x0A;			// Hardcore new line character
	USB_print(inBuffer);
}


 **********************************************{}****************************************************
 * Current main configuration
 * This main is setup to control the output of the Razor IMU and read the raw values to
 * calibrate it.
 *
 *
 */

void main(void) {
	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	UART_setup(57600);
	USB_setup(9600);
	__delay_cycles(50000);
	TIMER_setup(300);
	_BIS_SR(GIE);
	calibration_mode();
	while(1){
		calibration_read();
		__bis_SR_register(LPM0_bits);
	}
}

