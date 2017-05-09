/*
 * RAZOR_AHRS.c
 *
 * 	This code works to communicate the Sparkfun Razor AHRS[1] to the MSP430F5529 micro-controller[2].
 * 	Contains the implementation to the API defined in the RAZOR_AHRS.h header.
 *  Code is based on the firnmware found at [3]
 *
 * Depends on Serial_JMP.h
 *
 * Wiring diagram:
 * P3.3 -> TX@Razor
 * P3.4 -> RX@Razor
 *
 * USB -> PC COM PORT
 *
 *
 *  Created on: Feb 11, 2017
 *      Author: Jose A. Montes Perez
 *
 *  References:
 *  [1] https://www.sparkfun.com/products/retired/10736 - Sparkfun Razor IMU
 *  [2] http://www.ti.com/product/MSP430F5529 - MSP430F5529
 *  [3] https://github.com/Razor-AHRS/razor-9dof-ahrs
 *
 */

#include <msp430.h>
#include <stdlib.h>
#include "RAZOR_AHRS.h"
#include "Serial_JMPv2.2.h"


char outputMode;
char refreshMode;
float temp1, temp2, temp3;

//Variables that store the values for a particular
//angle
float yaw,pitch,roll;
//Variables were the values for the particular
//sensor are sotred
float acc_x,acc_y,acc_z;
float mag_x,mag_y,mag_z;
float gyr_x,gyr_y,gyr_z;


void RAZOR_get_yaw(float *out_yaw){
	*out_yaw = yaw;
}

void Razor_get_pitch(float *out_pitch){
	*out_pitch = pitch;
}

void RAZOR_get_roll(float *out_roll){
	*out_roll = roll;
}


void RAZOR_sngl_setup(){
	UART_setup(IMU_BAUD_RATE);
	UART_print("#o0");
	refreshMode = 0;
}

void RAZOR_cont_setup(){
	UART_setup(IMU_BAUD_RATE);
	UART_print("#o1");
	refreshMode = 1;
}

void RAZOR_out_angles(){
	UART_print("#ob");
	outputMode = 1;
}

void RAZOR_out_acc(){
	UART_print("#osca");
	outputMode = 2;
}


void RAZOR_out_mag(){
	UART_print("#oscm");
	outputMode = 3;
}

void RAZOR_out_gyr(){
	UART_print("#oscg");
	outputMode = 4;
}



void RAZOR_out_angles_t(){
	UART_print("#ot");
	outputMode = 5;
}

void RAZOR_refresh_value(){
	char *inBuffer;
	inBuffer = (char *) malloc(12);
	switch(outputMode){
	case 1:
		UART_print("#f");
		UART_getchar_n(inBuffer, 12, 'l');
		temp1 = *(float *)(inBuffer);
		temp2 = *(float *)(inBuffer + 4);
		temp3 = *(float *)(inBuffer + 8);
		while ((temp1 > 180) || (temp1 < 0.0001 && temp1 > -0.0001)){
			UART_print("#o0#ob");
			__delay_cycles(50);
			UART_print("#f");
			UART_getchar_n(inBuffer, 12, 'l');
			UART_print("#f");
			UART_getchar_n(inBuffer, 12, 'l');
			temp1 = *(float *)(inBuffer);
			temp2 = *(float *)(inBuffer + 4);
			temp3 = *(float *)(inBuffer + 8);
		}
		yaw = temp1;
		pitch = temp2;
		roll = temp3;
		break;
	case 2:
		UART_print("#f");
		UART_getchar_n(inBuffer, 12, 'l');
		acc_x = *(float *) (inBuffer);
		acc_y = *(float *) (inBuffer + 4);
		acc_z = *(float *) (inBuffer + 8);
		break;
	case 3:
		UART_print("#f");
		UART_getchar_n(inBuffer, 12, 'l');
		mag_x = *(float *) (inBuffer + 12);
		mag_y = *(float *) (inBuffer + 16);
		mag_z = *(float *) (inBuffer + 20);
		break;
	case 4:
		UART_print("#f");
		UART_getchar_n(inBuffer, 12, 'l');
		gyr_x = *(float *) (inBuffer + 24);
		gyr_y = *(float *) (inBuffer + 28);
		gyr_z = *(float *) (inBuffer + 32);
		break;
	default: break;
	}
	free(inBuffer);
}

