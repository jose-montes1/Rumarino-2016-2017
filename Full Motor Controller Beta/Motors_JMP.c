#include <msp430f5529.h>
#include "Motors_JMP.h"
/*
 * Pwm_JMP.c
 *
 *  Created on: Oct 26, 2016
 *      Author: Jose A. Montes Perez
 * Last Modification: Dec 21, 2016
 *
 * 	This file is utilized as a generic PWM generator
 * applied to ESC operations on the msp430f5529lp. 
 *	This supports up to four motor controller/servos 
 * at once. 
 * 
 * Wiring diagram:
 * Motor 1 -> P2.5
 * Motor 2 -> P2.4
 * Motor 3 -> P1.4
 * Motor 4 -> P1.3
 * 
 */

 
#define REFRESH_RATE 20972
#define DEADBAND 1573
#define RANGE 420

#define RANGE_MARRON 8

/***********************************************************************************
 *Function Description
 *		This function sets up all the motor specified in the wiring diagram to output ESC
 * compatible PWM signals. By default the will outup nothing (ground).
 *
 ***********************************************************************************/


void MOTOR_full_setup(){
	
	P1DIR |= BIT2 + BIT3 + BIT4 + BIT5;								// Setup motor 1 & 2 pins as output for pwm
	P1OUT |= BIT2 + BIT3 + BIT4 + BIT5;								// Set as pwm output
	P1SEL |= BIT2 + BIT3 + BIT4 + BIT5;								// Select timer module over gpio
	

	TA0CCR0 = REFRESH_RATE;								// Set the refresh rate of the pwm signal to 50Hz
	
	TA0CCTL1 = OUTMOD_7;            	         	    // Set Capture Compare Rregister 3 to reset/set mode
	TA0CCR1 = DEADBAND;                    	  	    // No ouptut signal
	TA0CCTL2 = OUTMOD_7;            	         	    // Set Capture Compare Rregister 3 to reset/set mode
	TA0CCR2 = DEADBAND;                     	    // No ouptut signal
	TA0CCTL3 = OUTMOD_7;                       	        // Set Capture Compare Rregister 3 to reset/set mode
	TA0CCR3 = DEADBAND;                             // No ouptut signal
	TA0CCTL4 = OUTMOD_7;		     		     		// Set Capture Compare Rregister 2 to reset/set mode
	TA0CCR4 = DEADBAND;					   			// No output signal


	TA0R = 0;
	TA0CTL = TASSEL_2 + MC_1;     	  			// ACLK, up mode, clear T

}

/***********************************************************************************
 *Function Description
 *		This function sets up half the motor specified in the wiring diagram to output ESC
 * compatible PWM signals. The pins configured are for motor 1 & 2. By default the will outup nothing (ground).
 *
 ***********************************************************************************/

void MOTOR_half_setup(){
	
	P1DIR |= BIT4 + BIT5;								// Setup motor 1 & 2 pins as output for pwm
	P1OUT |= BIT4 + BIT5;								// Set as pwm output
	P1SEL |= BIT4 + BIT5;								// Select timer module over gpio
	
	TA2CCR0 = REFRESH_RATE;								// Set the refresh rate of the pwm signal to 50Hz
	
	TA0CCTL4 = OUTMOD_7;            	             	// Set Capture Compare Rregister 3 to reset/set mode
	TA0CCR4 = DEADBAND;                       		// No ouptut signal
	TA0CCTL3 = OUTMOD_7;            	             	// Set Capture Compare Rregister 3 to reset/set mode
	TA0CCR3 = DEADBAND;                       		// No ouptut signal
	
	TA0R = 0;
	TA2CTL = TASSEL_2 + MC_1;       			// ACLK, up mode, clear T
}



/***********************************************************************************
 *Function Description
 * 	This function controls the velocity that the motors turn in. It has two parameters, one for speed.
 *  and one for motor selection. The speed ranges from -100 to 100. To select a motor simply put the 
 *  desirted motors number from the wiring diagram in the motor varible. If you want to change all 
 *  the motors place 5 as the input.
 *
 ***********************************************************************************/

void MOTOR_speed(int speed, unsigned int motor){
	if((speed < 101) && (speed > -101 ) && (motor < 6)){			// Verify correct parameters
		unsigned long operations = (speed*RANGE)/100 + DEADBAND;		// Calculate appropriate duty cycle
		TA0R = 0;													// Reset timer count
		switch (motor){												// Verify which motor to modify
			case 1: TA0CCR4=operations;break;						// Change speed of motor 1
			case 2: TA0CCR3=operations;break;						// Change speed of motor 2
			case 3: TA0CCR2=operations;break;						// Change speed of motor 3
			case 4: TA0CCR1=operations;break;						// Change speed of motor 4
			case 5: 												// Change speed of all motors
				TA0CCR1 = operations;
				TA0CCR2 = operations;
				TA0CCR3 = operations;
				TA0CCR4 = operations;
				break;
			default: break;											// Do nothing
		}
	}
}
