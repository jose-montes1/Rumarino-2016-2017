
/***********************************************************************************
 *This is the first attempt to implement an align for proteus
 *Author: Omar Gonzalez
 *Date: march 17, 2017
 *
 ***********************************************************************************/




#include <msp430.h> 
#include "Motors_JMP.h"
#include "Serial_JMPv2.2.h"
#include "General_JMP.h"
#include "typecast.h"
#include "Razor_AHRS.h"
#include "MS5837-30BA.h"

#define MSDELAY 50
#define abss(a) a >= 0 ? a :-1*a

int16 pressure;
int32 temp;
int samp;

#define Controller_Gain_a 2.0




/*
 * main.c
 */

int main(void) {
	WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer
	RAZOR_sngl_setup();
	RAZOR_out_angles();
	USB_setup(9600);
	USB_println("Initializing setup");
	_BIS_SR(GIE);
	MOTOR_ultra_setup();

	char command;


	int m5_speed = 0;
	int m6_speed = 0;

	char controller_initialized = 0;
	float align_reference = 0;
	float align_actual_point = 0;
	float align_error = 0;
	float align_output = 0;
	float n_yaw;
	int delta_input;
	__delay_cycles(5000);
	RAZOR_refresh_value();
	RAZOR_get_yaw(&align_reference);
	if(align_reference < 0){
		align_reference += 360;
	}

	USB_getchar(&command, 'a');
	USB_print("System initialized");


	while(1){
		if(command != 0){

			if(command >= '0' && command <= '9'){
				align_reference += (command - '0')*20;      // converts to decimal value of each number pressed
			}else if(command == 'q'){
				align_reference += -20;
			}else if(command == 'w'){
				align_reference += -40;
			}else if(command == 'e'){
				align_reference += -60;
			}else if(command == 'r'){
				align_reference += -80;
			}else if(command == 't'){
				align_reference += -100;
			}else if(command == 'y'){
				align_reference += -120;
			}else if(command == 'u'){
				align_reference += -180;
			}

			else if(command == 'm'){
				RAZOR_refresh_value();
				RAZOR_get_yaw(&align_reference);
			}

			else if(command == 'x'){
				controller_initialized = 0;
			}else if(command == 'h'){
				controller_initialized = 1;
			}



			if(align_reference > 360){
				align_reference -= 360;
			}
			if(align_reference < 0){
				align_reference += 360;
			}
			command = 0;
		}
		if(controller_initialized){

			//USB_println("Controller looping");

			RAZOR_refresh_value();
			RAZOR_get_yaw(&align_actual_point);


			//Cambiar a 360
			if(align_actual_point < 0){
				align_actual_point +=  360;

			}

			USB_print_float("Actual point is ", align_actual_point);
			USB_print_float(" setpoint for controller loop: ", align_reference);

			// equation of differences for proportional controller
			align_error = (align_reference - align_actual_point);      //calculate error

			if(align_error > 0){
				if(align_error>180){
					align_error = align_error - 360;
				}else{                                  //si no es juan es pablo
					align_error = align_error;
				}
			}else{
				if(abss(align_error) > 180){
					align_error = 360 - abss(align_error);
				}else{                                //si no es juan es pedro
					align_error = align_error;
				}
			}

			// Proportional Controller
			USB_print_float(" entering into controller: ", align_error);
			align_output = Controller_Gain_a*align_error;
			USB_print_float(" controller align_output: ", align_output);

			// send controller align_output to motors
			m5_speed = align_output;
			m6_speed = -align_output;

			// m5_speed debe ser motor derecho --> pin 2.4
			/// m6_speed debe ser motor izquierdo --> pin 2.5

			/* Satutration*/

			if((m5_speed) > MOTOR_CAP)
				m5_speed = MOTOR_CAP;
			if((m6_speed) > MOTOR_CAP)
				m6_speed = MOTOR_CAP;
			if((m5_speed) < -1*MOTOR_CAP)
				m5_speed = -1*MOTOR_CAP;
			if((m6_speed) < -1*MOTOR_CAP)
				m6_speed = -1*MOTOR_CAP;


			USB_print_value("  motor5: ", m5_speed);
			USB_print_value("  motor6: ", m6_speed);

			USB_println(" ");
			MOTOR_speed(m5_speed, R_MOTOR);
			MOTOR_speed(m6_speed, L_MOTOR);

		}
		else{
			USB_println("Not doing anything");
			MOTOR_speed(0, R_MOTOR);
			MOTOR_speed(0, L_MOTOR);

			__delay_cycles(20000);
		}

		//--------------------------------------------------------------------------------------------------------------------
	}
}


