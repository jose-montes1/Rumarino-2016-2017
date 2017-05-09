#include <msp430.h>
#include "Serial_JMPv2.2.h"
#include "typecast.h"
#include "MS5837-30BA.h"
#include "Razor_AHRS.h"
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
#define invalidCommand() conditions.validCommand = 0; conditions.clearInput = 1


//Struct utilized to keep tract of the conditions of the controller
struct serialConditions {
	uint8 depthControllerEnabled : 1;       //Determines whether the depth controller is enabled
	uint8 alignControllerEnabled : 1;       //Determines whether the align controller is enabled
	uint8 forwardsDeadreckoning : 1;        //Determines if you go forwards by time or not

	uint8 systemRunning : 1;                //Determines whether the system starts or not
	// 0 - not running | 1 - running

	uint8 terminalIO : 1;                   //Determines if the system receives input from the script or from the user
	/* 0 - input from script | 1 - input from terminal */


	uint8 firstCommand : 1;                 //Determines if it is the first input
	uint8 secondCommand : 1;                //Determines if it is the second command
	uint8 thirdCommand : 1;                 //Determines if it is the third command

	uint8 clearInput : 1;                   //Determines when to clear inputs (complete or invalid command)
	uint8 validCommand : 1;                 //Determines whether it is a valid command or not

	//  uint8 printFeedbackFrame : 1;           //Determines when to print an output frame from the status
	//  uint8 printContinousFrames : 1;         // TODO - maybe
	//  uint8 printStatusFrame : 1;             //Determines when to print a system status frame
};


void forwards(unsigned int time, int speed, unsigned char *done){
	MOTOR_speed(speed, H_MOTORS);   //Sends speed to front two motors
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





int main(void) {
	WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer

	/***** SETUP AREA *****/
	//Initialize modules
	USB_setup(9600);
	I2C_setup(400000);
	MOTOR_ultra_setup();
	_BIS_SR(GIE);
	RAZOR_sngl_setup();
	RAZOR_out_angles();
	//PRESSURE_calibrate();


	/***** Variable Declarations *****/
	//Motor Variables
	//int16 v_motors;
	int16 right_motor;
	int16 left_motor;


	//Depth Controller variables
	//int16 //PRESSURE = 0;
	int depth_gain = 0;
	float depth_reference = 0;
	float actual_depth_bars = 0;
	float actual_depth_milibars = 0;
	float actual_depth_feet = 2;
	float depth_error = 0;
	float depth_output = 0;
	long actual_depth_pwm;


	//Align controller variable
	float align_reference = 0;
	float align_actual_point = 0;
	float align_error = 0;
	float align_output = 0;
	int align_gain = 0;
	__delay_cycles(5000);
	RAZOR_refresh_value();
	RAZOR_get_yaw(&align_reference);
	if(align_reference < 0){
		align_reference += 360;
	}




	int m1_speed = 0;
	int m2_speed = 0;
	int m3_speed = 0;
	int m4_speed = 0;
	int m5_speed = 0;
	int m6_speed = 0;

	//Forwards
	unsigned char tiemDone;



	struct serialConditions conditions = {0,0,0,0,0,0,0,0,0,1};




	int8 commands[4];   //Command buffer
	commands[0] = 0;    //First operator
	commands[1] = 0;    //Second operator
	commands[2] = 0;    //Third operator
	commands[3] = 0;    //Trash collector


	while(!conditions.systemRunning){                           //Verify that the system is enabled
		USB_println("Press 's' to start the system");           //Print usage to user
		USB_getchar(&commands[0], 'b');                             //Get input from user in blocking mode
		if(*commands == 's'){                                   //Error check input
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
			if(conditions.firstCommand){                //Verify so you only print once (to avoid spam)
				printInputFeedback(1,commands[0]);      //Print feedback to the user
				USB_getchar(&commands[1], 'a');         //Set UART receive to next value
				conditions.firstCommand = 0;            //Disable so you don't enter this if anymore
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
					if(conditions.secondCommand){               //Verify so you only print once (to avoid spam)
						printInputFeedback(2,commands[1]);      //Print feedback to the user
						USB_getchar(&commands[2], 'a');         //Set UART receive to next value
						conditions.secondCommand = 0;           //Disable so you don't enter this if anymore
					}

					/* Set point and enable controller commands */
					if(commands[1] >= '0' && commands[1] <= '9'){
						depth_reference = FEET_TO_PWM*(commands[1] - '0');
						conditions.clearInput = 1;
						conditions.depthControllerEnabled = 1;
					}else if(commands[1] == 'q'){
						depth_reference = FEET_TO_PWM*(10);
						conditions.clearInput = 1;
						conditions.depthControllerEnabled = 1;
					}else if(commands[1] == 'w'){
						depth_reference = FEET_TO_PWM*(11);
						conditions.clearInput = 1;
						conditions.depthControllerEnabled = 1;
					}else if(commands[1] == 'e'){
						depth_reference = FEET_TO_PWM*(12);
						conditions.clearInput = 1;
						conditions.depthControllerEnabled = 1;
					}else if(commands[1] == 'r'){
						depth_reference = FEET_TO_PWM*(13);
						conditions.clearInput = 1;
						conditions.depthControllerEnabled = 1;
					}else if(commands[1] == 't'){
						depth_reference = FEET_TO_PWM*(14);
						conditions.clearInput = 1;
						conditions.depthControllerEnabled = 1;
					}else if(commands[1] == 'y'){
						depth_reference = FEET_TO_PWM*(15);
						conditions.clearInput = 1;
						conditions.depthControllerEnabled = 1;
					}else if(commands[1] == 'u'){
						depth_reference = FEET_TO_PWM*(16);
						conditions.clearInput = 1;
						conditions.depthControllerEnabled = 1;
					}

					/* Turn of controller command */
					else if(commands[1] == 'x'){
						MOTOR_speed(0, V_MOTORS);
						USB_println("exiting controller ");
						USB_println(" ");

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
							if(conditions.thirdCommand){                //Verify so you only print once (to avoid spam)
								printInputFeedback(3,commands[2]);      //Print feedback to the user
								USB_getchar(&commands[3], 'a');         //Set UART receive to next value
								conditions.thirdCommand = 0;            //Disable so you don't enter this if anymore
							}
							/*Verify proper command */
							if(commands[2] >= '0' && commands[2] <= '9'){
								depth_gain = commands[2] - '0' + 1;
								// USB_print_value("depth gain: ", depth_gain);
								// USB_println(" ");

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
			else if(commands[0] == 'a'){

				/* align controller options
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
					if(conditions.secondCommand){               //Verify so you only print once (to avoid spam)
						printInputFeedback(2,commands[1]);      //Print feedback to the user
						USB_getchar(&commands[2], 'a');         //Set UART receive to next value
						conditions.secondCommand = 0;           //Disable so you don't enter this if anymore
					}

					/* Set point and enable controller commands */
					if(commands[1] >= '0' && commands[1] <= '9'){
						align_reference += (commands[1] - '0')*20;      // converts to decimal value of each number pressed
						conditions.clearInput = 1;
						conditions.alignControllerEnabled = 1;
					}else if(commands[1] == 'q'){
						align_reference += -20;
						conditions.clearInput = 1;
						conditions.alignControllerEnabled = 1;
					}else if(commands[1] == 'w'){
						align_reference += -40;
						conditions.clearInput = 1;
						conditions.alignControllerEnabled = 1;
					}else if(commands[1] == 'e'){
						align_reference += -60;
						conditions.clearInput = 1;
						conditions.alignControllerEnabled = 1;
					}else if(commands[1] == 'r'){
						align_reference += -80;
						conditions.clearInput = 1;
						conditions.alignControllerEnabled = 1;
					}else if(commands[1] == 't'){
						align_reference += -100;
						conditions.clearInput = 1;
						conditions.alignControllerEnabled = 1;
					}else if(commands[1] == 'y'){
						align_reference += -120;
						conditions.clearInput = 1;
						conditions.alignControllerEnabled = 1;
					}else if(commands[1] == 'u'){
						align_reference += -180;
						conditions.clearInput = 1;
						conditions.alignControllerEnabled = 1;
					}

					else if(commands[1] == 'm'){
						RAZOR_refresh_value();
						RAZOR_get_yaw(&align_reference);
						conditions.clearInput = 1;
					}

					/* Turn of controller command */
					else if(commands[1] == 'x'){
						MOTOR_speed(0, H_MOTORS);
						USB_println("exiting controller ");
						USB_println(" ");

						conditions.clearInput = 1;
						conditions.alignControllerEnabled = 0;
					}

					/* Set Gain command */
					else if(commands[1] == 'g'){
						/* Set gain command
						 * 0 - 9 Sets the gain of the controller to the input value
						 */
						if(commands[2] != 0){
							/* Print feedback to user */
							if(conditions.thirdCommand){                //Verify so you only print once (to avoid spam)
								printInputFeedback(3,commands[2]);      //Print feedback to the user
								USB_getchar(&commands[3], 'a');         //Set UART receive to next value
								conditions.thirdCommand = 0;            //Disable so you don't enter this if anymore
							}
							/*Verify proper command */
							if(commands[2] >= '0' && commands[2] <= '9'){
								align_gain = commands[2] - '0' + 1;
								// USB_print_value("align gain: ", align_gain);
								// USB_println(" ");

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


			/* Enter forwards by a certain amount */
			else if(commands[0] == 'f'){
				/* Forwards input
				 * 0-9 Receive the amount of time you want to wait
				 * 0 means 1 second, 9 means 10
				 */
				if(commands[1] != 0){
					/* Print feedback to user */
					if(conditions.secondCommand){               //Verify so you only print once (to avoid spam)
						printInputFeedback(2,commands[1]);      //Print feedback to the user
						USB_getchar(&commands[2], 'a');         //Set UART receive to next value
						conditions.secondCommand = 0;           //Disable so you don't enter this if anymore
					}

					/* Set the time variable and go on */
					if(commands[1] >= '0' && commands[1] <= '9'){
						forwards(commands[1]-'0' + 1, MOTOR_CAP, &tiemDone);
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
					if(conditions.secondCommand){               //Verify so you only print once (to avoid spam)
						printInputFeedback(2,commands[1]);      //Print feedback to the user
						USB_getchar(&commands[2], 'a');         //Set UART receive to next value
						conditions.secondCommand = 0;           //Disable so you don't enter this if anymore
					}

					/* Set the time variable and go on */
					if(commands[1] >= '0' && commands[1] <= '9'){
						forwards(commands[1]-'0' + 1, -MOTOR_CAP, &tiemDone);
						conditions.forwardsDeadreckoning = 1;
						conditions.clearInput = 1;
					}
					/*Invalid second command */
					else{
						invalidCommand();
					}
				}
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
				USB_println("exiting all ");
				USB_println(" ");
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
			conditions.validCommand = 1;                    //Asume a valid command

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
				//				PRESSURE_start_conversion();
				//				PRESSURE = PRESSURE_get_PRESSURE();
				//				actual_depth_bars = (float)33.4552565551477;//*(//PRESSURE);       // convert bars to milibars
				//				actual_depth_milibars = (float)(actual_depth_bars/10000); //convert milibars to feet
				//				actual_depth_feet = (float)(actual_depth_milibars - 29);  // calibrate depending on altitude above sea
				//				actual_depth_pwm = (actual_depth_feet*6.25);
				depth_error = (depth_reference - actual_depth_pwm);      //calculate error
				depth_output = depth_gain*depth_error;
				m1_speed = depth_output;
				m2_speed = depth_output;
				m3_speed = depth_output;
				m4_speed = depth_output;


				/* Satutration*/

				if((m1_speed) > MOTOR_CAP)
					m1_speed = MOTOR_CAP;
				if((m2_speed) > MOTOR_CAP)
					m2_speed = MOTOR_CAP;
				if(m3_speed > MOTOR_CAP)
					m3_speed = MOTOR_CAP;
				if(m4_speed > MOTOR_CAP)
					m4_speed = MOTOR_CAP;
				if((m1_speed) < -1*MOTOR_CAP)
					m1_speed = -1*MOTOR_CAP;
				if((m2_speed) < -1*MOTOR_CAP)
					m2_speed = -1*MOTOR_CAP;
				if(m3_speed < -1*MOTOR_CAP)
					m3_speed = -1*MOTOR_CAP;
				if(m4_speed < -1*MOTOR_CAP)
					m4_speed = -1*MOTOR_CAP;


				MOTOR_speed(m1_speed, 1);
				MOTOR_speed(m2_speed, 2);
				MOTOR_speed(m3_speed, 3);
				MOTOR_speed(m4_speed, 4);
			}

			/* Align Controller section*/
			if(conditions.alignControllerEnabled){

				RAZOR_refresh_value();
				RAZOR_get_yaw(&align_actual_point);


				//Cambiar a 360
				if(align_actual_point < 0){
					align_actual_point +=  360;

				}


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
				align_output = align_gain*align_error;

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




				MOTOR_speed(m5_speed, R_MOTOR);
				MOTOR_speed(m6_speed, L_MOTOR);

			}

			/* WASD control section*/
//			if(!conditions.forwardsDeadreckoning){
//				MOTOR_speed(left_motor, L_MOTOR);
//				MOTOR_speed(right_motor, R_MOTOR);
//			}

			/* Forwards and backwards section */
			if(conditions.forwardsDeadreckoning){
				if(tiemDone){
					MOTOR_speed(0, H_MOTORS);
					conditions.forwardsDeadreckoning = 0;
					USB_println("Done forwards");
				}
			}

			/* Print usage to user */
			if (conditions.terminalIO){
				USB_print_value(" depth gain: ", depth_gain);
				USB_print_value(" align gain: ", align_gain);

				//USB_print_float("  presurre: ", PRESSURE);
				USB_print_value(" depth set point: ", depth_reference/6.25);
				USB_print_float("  depth: ", actual_depth_feet);
				USB_print_float("  depth error: ", depth_error);

				USB_print_float(" align setpoint: ", align_reference);
				USB_print_float(" orientation: ", align_actual_point);
				USB_print_float(" align error: ", align_error);


				USB_print_value("  motor1: ", m1_speed);
				USB_print_value("  motor2: ", m2_speed);
				USB_print_value("  motor3: ", m3_speed);
				USB_print_value("  motor4: ", m4_speed);
				USB_print_value("  motor5: ", m5_speed);
				USB_print_value("  motor6: ", m6_speed);

				USB_println(" ");

				conditions.terminalIO = 0;
			}

			/* Prints usage to the script */
			if (!conditions.terminalIO){
			}
		}
	}
}

