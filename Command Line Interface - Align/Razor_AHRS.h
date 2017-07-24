/*
 * RAZOR_AHRS.h
 *
 *  Created on: Feb 11, 2017
 *      Author: RathK
 */

#ifndef RAZOR_AHRS_H_
#define RAZOR_AHRS_H_


#define IMU_BAUD_RATE 9600      //Baud Rate for communicating with the sensor
#define IMU_SAMPLE_RATE 60      //Suggested sample rate in milliseconds
#define IMU_BUFFER 60           //Malloc buffer size with slight overshoot


void RAZOR_get_yaw(float *yaw);
void RAZOR_get_pitch(float *pitch);
void RAZOR_get_roll(float *roll);

void RAZOR_sngl_setup();
void RAZOR_cont_setup();

void RAZOR_out_angles();
void RAZOR_out_angles_t();
void RAZOR_out_acc();
void RAZOR_out_gyr();
void RAZOR_out_mag();
void RAZOR_refresh_value();
void RAZOR_refresh_atvalue();


#endif /* RAZOR_AHRS_H_ */