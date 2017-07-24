#include <msp430.h>
#include "Serial_JMPv2.1.h"
#include "typecast.h"
#include "MS5837-30BA.h"
#include "Motors_JMP.h"
#include "General_JMP.h"
/*
 * main.c
 * General skeleton for controller implementation
 * Jose Montes
 *
 *
 */

#define FEET_TO_PWM 6.25



//Struct utilized to keep tract of the conditions of the controller
struct serialConditions {
	uint8 depthControllerEnabled : 1;		//Determines whether the depth controller is enabled
	uint8 alignControllerEnabled : 1;		//Determines whether the align controller is enabled
	uint8 forwardsDeadreckoning : 1;		//Determines if you go forwards by time or not

	uint8 systemRunning : 1;				//Determines whether the system starts or not
	// 0 - not running | 1 - running

	uint8 terminalIO : 1;					//Determines if the system receives input from the script or from the user
	/* 0 - input from script | 1 - input from terminal */


	uint8 firstCommand : 1;					//Determines if it is the first input
	uint8 secondCommand : 1;				//Determines if it is the second command
	uint8 thirdCommand : 1;					//Determines if it is the third command

	uint8 clearInput : 1; 					//Determines when to clear inputs (complete or invalid command)
	uint8 validCommand : 1;					//Determines whether it is a valid command or not

	//	uint8 printFeedbackFrame : 1;			//Determines when to print an output frame from the status
	//	uint8 printContinousFrames : 1;			// TODO - maybe
	//	uint8 printStatusFrame : 1;				//Determines when to print a system status frame
};


void forwards(unsigned int time, int speed, unsigned char *done){
	MOTOR_speed(speed, H_MOTORS);	//Sends speed to front two motors
	timeExceed(time, done);
}


//Use this to output information about the current stage
void printInputFeedback(int operatorNumber, char command){
	switch(operatorNumber){
	case 1: USB_print("Entered first operator: "); break;
	case 2: USB_print("Entered second operator: "); break;
	case 3: USB_print("Entered third operator: "); break;
	default: break;
	}
	USB_putchar(command);
	USB_println(" ");
}


#define invalidCommand() conditions.validCommand = 0; conditions.clearInput = 1


int main(void) {
	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

	/***** SETUP AREA *****/
	//Initialize modules
	USB_setup(9600);
	I2C_setup(400000);
	MOTOR_ultra_setup();
	_BIS_SR(GIE);
	PRESSURE_calibrate();


	/***** Variable Declarations *****/
	//Motor Variables
	int16 v_motors;
	int16 right_motor;
	int16 left_motor;


	//Controller variables
	int16 pressure = 0;
	int depth_gain = 0;
	float reference = 0;
	float actual_depth_bars = 0;
	float actual_depth_milibars = 0;
	float actual_depth_feet = 0;
	float error = 0;
	float output = 0;
	long actual_depth_pwm;

	//Forwards
	unsigned char tiemDone;



	struct serialConditions conditions = {0,0,0,0,0,0,0,0,0,1};




	int8 commands[4];	//Command buffer
	commands[0] = 0;	//First operator
	commands[1] = 0;	//Second operator
	commands[2] = 0;	//Third operator
	commands[3] = 0;	//Trash collector


	while(!conditions.systemRunning){ 							//Verify that the system is enabled
		USB_println("Press 's' to start the system");			//Print usage to user
		USB_getchar(&commands[0], 'b');								//Get input from user in blocking mode
		if(*commands == 's'){									//Error check input
			conditions.systemRunning = 1;
		}
	}
	USB_println("Awaiting command");
	commands[0] = 0;
	USB_getchar(&commands[0], 'a');
	conditions.clearInput = 1;

	while(1){

		/* Enter command management /
		 * o - depth controller - required more operands
		 * a* - alignment controller - requires more operands
		 * x - exit all
		 * f - go forwards a certain amount of time - required more operands
		 * b - go backwards a certain amount of time - required more operands
		 * wasd - horizontal control keys
		 * t - print user mode
		 * y - print script mode
		 *  (* - means not implemented)
		 */
		if(commands[0] != 0  && conditions.systemRunning){
			/* Print feedback frame and set operator to second command */
			if(conditions.firstCommand){				//Verify so you only print once (to avoid spam)
				printInputFeedback(1,commands[0]);		//Print feedback to the user
				USB_getchar(&commands[1], 'a');			//Set UART receive to next value
				conditions.firstCommand = 0;			//Disable so you don't enter this if anymore
			}
			/* Enter depth controller Commands */
			if(commands[0] == 'o'){

				/* Depth controller options
				 * 0-9 Give set point corresponding to feet
				 * q - Place the set point at 10 feet
				 * w - Place the set point at 11 feet
				 * e - Place the set point at 12 feet
				 * r - Place the set point at 13 feet
				 * t - Place the set point at 14 feet
				 * y - Place the set point at 15 feet
				 * u - Place the set point at 16 feet
				 *
				 * g - set the gain of the controller - requires other operand
				 *
				 * x - exit vertical controller and resets vertical motors
				 *
				 */

				/* if command arrives */
				if(commands[1] != 0){
					/* Print feedback frame and set operator to second command */
					if(conditions.secondCommand){				//Verify so you only print once (to avoid spam)
						printInputFeedback(2,commands[1]);		//Print feedback to the user
						USB_getchar(&commands[2], 'a');			//Set UART receive to next value
						conditions.secondCommand = 0;			//Disable so you don't enter this if anymore
					}

					/* Set point and enable controller commands */
					if(commands[1] >= '0' && commands[1] <= '9'){
						reference = FEET_TO_PWM*(commands[1] - '0');
						conditions.clearInput = 1;
						conditions.depthControllerEnabled = 1;
					}else if(commands[1] == 'q'){
						reference = FEET_TO_PWM*(10);
						conditions.clearInput = 1;
						conditions.depthControllerEnabled = 1;
					}else if(commands[1] == 'w'){
						reference = FEET_TO_PWM*(11);
						conditions.clearInput = 1;
						conditions.depthControllerEnabled = 1;
					}else if(commands[1] == 'e'){
						reference = FEET_TO_PWM*(12);
						conditions.clearInput = 1;
						conditions.depthControllerEnabled = 1;
					}else if(commands[1] == 'r'){
						reference = FEET_TO_PWM*(13);
						conditions.clearInput = 1;
						conditions.depthControllerEnabled = 1;
					}else if(commands[1] == 't'){
						reference = FEET_TO_PWM*(14);
						conditions.clearInput = 1;
						conditions.depthControllerEnabled = 1;
					}else if(commands[1] == 'y'){
						reference = FEET_TO_PWM*(15);
						conditions.clearInput = 1;
						conditions.depthControllerEnabled = 1;
					}else if(commands[1] == 'u'){
						reference = FEET_TO_PWM*(16);
						conditions.clearInput = 1;
						conditions.depthControllerEnabled = 1;
					}

					/* Turn of controller command */
					else if(commands[1] == 'x'){
						MOTOR_speed(0, V_MOTORS);
						conditions.clearInput = 1;
						conditions.depthControllerEnabled = 0;
					}

					/* Set Gain command */
					else if(commands[1] == 'g'){
						/* Set gain command
						 * 0 - 9 Sets the gain of the controller to the input value
						 */
						if(commands[2] != 0){
							/* Print feedback to user */
							if(conditions.thirdCommand){				//Verify so you only print once (to avoid spam)
								printInputFeedback(3,commands[2]);		//Print feedback to the user
								USB_getchar(&commands[3], 'a');			//Set UART receive to next value
								conditions.thirdCommand = 0;			//Disable so you don't enter this if anymore
							}
							/*Verify proper command */
							if(commands[2] >= '0' && commands[2] <= '9'){
								depth_gain = commands[2] - '0' + 1;
								conditions.clearInput = 1;
							}
							/* Invalid third Command */
							else{
								invalidCommand();
							}
						}
					}
					/*Invalid second command */
					else{
						invalidCommand();
					}
				}
			}


			/* Enter Align Controller Commands */
			//else if(commands[0] == 'a'){
			//TODO - Leave this until the controller is implemented
			//}


			/* Enter forwards by a certain amount */
			else if(commands[0] == 'f'){
				/* Forwards input
				 * 0-9 Receive the amount of time you want to wait
				 * 0 means 1 second, 9 means 10
				 */
				if(commands[1] != 0){
					/* Print feedback to user */
					if(conditions.secondCommand){				//Verify so you only print once (to avoid spam)
						printInputFeedback(2,commands[1]);		//Print feedback to the user
						USB_getchar(&commands[2], 'a');			//Set UART receive to next value
						conditions.secondCommand = 0;			//Disable so you don't enter this if anymore
					}

					/* Set the time variable and go on */
					if(commands[1] >= '0' && commands[1] <= '9'){
						forwards(commands[1]-'0' + 1, MOTOR_CAP, &tiemDone);
						left_motor = 0;
						right_motor = 0;
						conditions.forwardsDeadreckoning = 1;
						conditions.clearInput = 1;
					}
					/*Invalid second command */
					else{
						invalidCommand();
					}
				}
			}

			/* Enter backwards by a certain amount */
			else if(commands[0] == 'b'){
				/* Backwards input
				 * 0-9 Receive the amount of time you want to wait
				 * 0 means 1 second, 9 means 10
				 */
				if(commands[1] != 0){
					/* Print feedback to user */
					if(conditions.secondCommand){				//Verify so you only print once (to avoid spam)
						printInputFeedback(2,commands[1]);		//Print feedback to the user
						USB_getchar(&commands[2], 'a');			//Set UART receive to next value
						conditions.secondCommand = 0;			//Disable so you don't enter this if anymore
					}

					/* Set the time variable and go on */
					if(commands[1] >= '0' && commands[1] <= '9'){
						forwards(commands[0] -'0'+1, -1*MOTOR_CAP, &tiemDone);
						left_motor = 0;
						right_motor = 0;
						conditions.forwardsDeadreckoning = 1;
						conditions.clearInput = 1;
					}
					/*Invalid second command */
					else{
						invalidCommand();
					}
				}
			}


			/* Manual control operation - go forwards - if tilting stop*/
			else if(commands[0] == 'w'){
				if(right_motor != left_motor){
					right_motor = 0;
					left_motor = 0;
				}if(right_motor < MOTOR_CAP){
					right_motor += 5;
					left_motor += 5;
				}
				conditions.clearInput = 1;
				/* Manual control operation */
			}else if(commands[0] == 's'){
				if(right_motor != left_motor){
					right_motor = 0;
					left_motor = 0;
				}if(right_motor > -1*MOTOR_CAP){
					right_motor -= 5;
					left_motor -= 5;
				}
				conditions.clearInput = 1;
			}else if(commands[0] == 'a'){
				if(right_motor == left_motor){
					right_motor = 0;
					left_motor = 0;
				}if(right_motor < MOTOR_CAP){
					right_motor += 5;
					left_motor -= 5;
				}
				conditions.clearInput = 1;
			}else if(commands[0] == 'd'){
				if(right_motor == left_motor){
					right_motor = 0;
					left_motor = 0;
				}if(left_motor < MOTOR_CAP){
					right_motor -= 5;
					left_motor += 5;
				}
				conditions.clearInput = 1;
			}

			/* Change the input setting to User mode */
			else if(commands[0] == 'u'){
				conditions.terminalIO = 1;
				conditions.clearInput = 1;
			}

			/*Changes the input settings to the Script mode */
			else if(commands[0] == 'y'){
				conditions.terminalIO = 0;
				conditions.clearInput = 1;

			}


			/* Halt everything */
			else if(commands[0] == 'x'){
				MOTOR_speed(0,V_MOTORS);
				MOTOR_speed(0,H_MOTORS);
				conditions.alignControllerEnabled = 0;
				conditions.depthControllerEnabled = 0;
				conditions.systemRunning = 0;
				conditions.clearInput = 1;
			}
			/*Invalid first operand */
			else{
				invalidCommand();
			}
		}

		if(commands[0] != 0 && !conditions.systemRunning){
			if(commands[0] == 's'){
				USB_println("Restarting the system");
				conditions.systemRunning = 1;
				conditions.clearInput = 1;
			}
		}

		/* Reset the input state if you have a valid or invalid command */
		if(conditions.clearInput){

			if(conditions.validCommand){
				USB_println("Entered valid command");
			}else{
				USB_println("Entered invalid command");
			}
			conditions.validCommand = 1;					//Asume a valid command

			commands[0] = 0;
			commands[1] = 0;
			commands[2] = 0;

			conditions.firstCommand = 1;
			conditions.secondCommand = 1;
			conditions.thirdCommand  = 1;

			USB_getchar(&commands[0], 'a');
			conditions.clearInput = 0;

		}

		if(conditions.systemRunning){

			/* Depth controller section*/
			if(conditions.depthControllerEnabled){
				PRESSURE_start_conversion();
				pressure = PRESSURE_get_pressure();
				actual_depth_bars = (float)33.4552565551477*(pressure);       // convert bars to milibars
				actual_depth_milibars = (float)(actual_depth_bars/10000); //convert milibars to feet
				actual_depth_feet = (float)(actual_depth_milibars - 29);  // calibrate depending on altitude above sea
				actual_depth_pwm = (actual_depth_feet*6.25);
				error = (reference - actual_depth_pwm);      //calculate error
				output = depth_gain*error;
				if(output > MOTOR_CAP){
					v_motors = MOTOR_CAP;
				}else if(output < -1*MOTOR_CAP){
					v_motors = -1*MOTOR_CAP;
				}else{
					v_motors = output;
				}
				MOTOR_speed(v_motors, V_MOTORS);
			}

			/* Align Controller section*/
			if(conditions.alignControllerEnabled){
			}

			/* WASD control section*/
			if(!conditions.forwardsDeadreckoning){
				MOTOR_speed(left_motor, L_MOTOR);
				MOTOR_speed(right_motor, R_MOTOR);
			}

			/* Forwards and backwards section */
			if(conditions.forwardsDeadreckoning){
				if(tiemDone){
					conditions.forwardsDeadreckoning = 0;
					USB_println("Done forwards");
				}
			}

			/* Print usage to user */
			if (conditions.terminalIO){
				USB_print_value("set point is: ", reference);
				USB_print_value("  pressure is: ", pressure);
				USB_print_value(" depth gain: ", depth_gain);
				USB_print_float("  depth is: ", actual_depth_bars);
				USB_print_float("  depth is: ", actual_depth_milibars);
				USB_print_float("  depth is: ", actual_depth_feet);
				USB_print_value("  vert motors: ", v_motors);
				USB_println(" ");
				conditions.terminalIO = 0;
			}

			/* Prints usage to the script */
			if (!conditions.terminalIO){
			}

		}
	}
}