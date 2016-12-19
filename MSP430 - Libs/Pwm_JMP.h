/*
 * Pwm_JMP.h
 *
 *  Created on: Oct 26, 2016
 *      Author: jose.montes1
 */

#ifndef PWM_JMP_H_
#define PWM_JMP_H_

#define ServoMS 655
#define DEADBAND 1500
#define RANGE 500
#define ESCRANGE 400
void pwm_setup();
void pwm_write_speed(int speed);
void pwm_write_microseconds(int microseconds);
void pwm_write_duty_cycle(float dutyCycle);




#endif /* PWM_JMP_H_ */
