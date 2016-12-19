#include <msp430f5529.h>
#include "Pwm_JMP.h"
#include "Serial_JMP.h"
/*
 * Pwm_JMP.c
 *
 *  Created on: Oct 26, 2016
 *      Author: jose.montes1
 *
 *  This file is utilized as a generic PWM
 *  generator with added servo functionality
 *
 */

//int main(void){
//	pwm_setup();
//	UART_setup(9600);
//	char command;
//	int theta = 0;
//	while(1){
//		UART_receive();
//		if(command == "a" ){
//			theta -= 10;
//		}
//		else if(command == "d"){
//			theta += 10;
//		}
//		pwm_write_angle(theta);
//	}
//}




void pwm_setup(){
	P1DIR |= BIT5;								// Set as ouput
	P1SEL |= BIT5;								// Select timer module


	TA0CCR0 = ServoMS;

	TA0CCTL4 = OUTMOD_7;                        // CCR4 reset/set
	TA0CCR4 = 0;                                 // CCR1 PWM duty cycle

	TA0CTL = TASSEL_1 + MC_1 + TACLR;           // ACLK, up mode, clear T


}
void pwm_write_angle(int speed){
	int microseconds;
	if(speed < 90 & speed > -90){
		microseconds = DEADBAND + (speed*RANGE)/90;
		pwm_write_microseconds(microseconds);
		UART_print_value("Angle: ", microseconds);

	}else{
		UART_print_status(FAIL, "Invalid ESC speed");
	}
}
void pwm_write_speed(int speed){
	int microseconds;
	if(speed < 100 & speed > -100){
		microseconds = DEADBAND + (speed*RANGE)/100;
		pwm_write_microseconds(microseconds);
		UART_print_value("Period: ", microseconds);

	}else{
		UART_print_status(FAIL, "Invalid ESC speed");
	}
}

void pwm_write_microseconds(int microseconds){
	float dutyCycle = microseconds*100/20000;
	pwm_write_duty_cycle(dutyCycle);
}

void pwm_write_duty_cycle(float dutyCycle){
	int value = ((float) (dutyCycle*ServoMS))/100;	// Calculate the appropriate register value
	TA0R = 0;
	TA0CCR4 = value;							// Send register value

}
