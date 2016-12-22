#include <msp430.h> 
#include "Motors_JMP.h"
#include "Serial_JMP.h"

/*
 * main.c
 */
void main(void) {
	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	USB_setup(9600);
	MOTOR_full_setup();
	_BIS_SR(GIE);
	char command;

	int m1_speed = 0;
	int m2_speed = 0;
	int m3_speed = 0;
	int m4_speed = 0;

	while(1){
		USB_receive_byte(&command);
		/* For Up and Down */
		if(command == 'o'){
			if(m1_speed < 100){
				m1_speed+=5;
				m2_speed+=5;
				m3_speed+=5;
				m4_speed+=5;
			}
		}else if(command == 'l'){
			if(m1_speed > -100){
				m1_speed-=5;
				m2_speed-=5;
				m3_speed-=5;
				m4_speed-=5;
			}
		} else if(command == 'x'){
			MOTOR_speed(0, 5);
			m1_speed = 0;
			m2_speed = 0;
			m3_speed = 0;
			m4_speed = 0;
		}
		MOTOR_speed(m1_speed, 5);


		/* For front and back
		if(command == 'w'){
			if(m1_speed != m2_speed){
				m1_speed = 0;
				m2_speed = 0;
			}if(m1_speed < 100){
				m1_speed += 5;
			}
		} else if(command == 's'){
			if(m1_speed != m2_speed){
				m1_speed = 0;
				m2_speed = 0;
			}if(m1_speed > -100){
				m1_speed -= 5;
			}
		} else if(command == 'a'){
			if(m1_speed == m2_speed){
				m1_speed = 0;
				m2_speed = 0;
			}if(m1_speed < 100){
				m1_speed += 5;
				m2_speed -= 5;
			}
		} else if(command == 'd'){
			if(m1_speed == m2_speed){
				m1_speed = 0;
				m2_speed = 0;
			}if(m2_speed < 100){
				m1_speed -= 5;
				m2_speed += 5;
			}
		} else if(command == 'x '){
			MOTOR_speed(0, 5);
			m1_speed = 0;
			m2_speed = 0;
			m3_speed = 0;
			m4_speed = 0;
		}
		MOTOR_speed(m1_speed, 1);
		MOTOR_speed(m2_speed, 2);
		MOTOR_speed(m3_speed, 3);
		MOTOR_speed(m4_speed, 4);
		*/
		command = ' ';
	}
}
