#include <msp430.h> 
#include "Serial_JMP.h"
#include "General_JMP.h"
#include "typecast.h"
/*
 * Created by Jose A. Montes Perez
 * Algorithm taken from  [1] MS5837-30BA datasheet - page 7
 * [1] http://www.mouser.com/ds/2/418/MS5837-30BA-736494.pdf
 *
 */

//Data sheet variables
//Variable names taken straight from datasheet
uint16 C1; //Pressure Sensitivity
uint16 C2; //Pressure Offset
uint16 C3; //Temperature coefficient of pressure sensitivity
uint16 C4; //Temperature coefficient of pressure offset
uint16 C5; //Reference temperature
uint16 C6; //Temperature coefficient of the temperature

uint32 D1; //Digital pressure value
uint32 D2; //Digital temperature value

int32 dT; //Difference between actual and reference value
int32 TEMP; //Actual temperature

int32 long OFF; //Offset actual temperature
int32 long SENS; //Sensitivity at actual temperature
int32 P; //Temperature compensated pressure



//I2C Addresses
#define PSENSOR 0x76


//Commands list
#define RESET 0x1E
#define CONVERT_D1 0x48
#define CONVERT_D2 0x58
#define ADC_READ 0x00
#define PROM_READ_C1 0xA2
#define PROM_READ_C2 0xA4
#define PROM_READ_C3 0xA6
#define PROM_READ_C4 0xA8
#define PROM_READ_C5 0xAA
#define PROM_READ_C6 0xAC

//Local dependencies
unsigned char I2C_buffer[8];


void reset_sequence(){
	I2C_write(PSENSOR, RESET, 0, 0);	//Send reset command to the pressure sensor
	__delay_cycles(2000);
}
void read_factory_calibration(){
	I2C_read(PSENSOR, PROM_READ_C1, &I2C_buffer[0], 2);
	C1 = (I2C_buffer[0] << 8) | I2C_buffer[1];
	I2C_read(PSENSOR, PROM_READ_C2, &I2C_buffer[0], 2);
	C2 = (I2C_buffer[0] << 8) | I2C_buffer[1];
	I2C_read(PSENSOR, PROM_READ_C3, &I2C_buffer[0], 2);
	C3 = (I2C_buffer[0] << 8) | I2C_buffer[1];
	I2C_read(PSENSOR, PROM_READ_C4, &I2C_buffer[0], 2);
	C4 = (I2C_buffer[0] << 8) | I2C_buffer[1];
	I2C_read(PSENSOR, PROM_READ_C5, &I2C_buffer[0], 2);
	C5 = (I2C_buffer[0] << 8) | I2C_buffer[1];
	I2C_read(PSENSOR, PROM_READ_C6, &I2C_buffer[0], 2);
	C6 = (I2C_buffer[0] << 8) | I2C_buffer[1];
}

void read_conversions(){
	I2C_write(PSENSOR, CONVERT_D1, 0, 0);
	delay(20);												//Wait 20ms (worst case) for conversion to end
	I2C_read(PSENSOR, ADC_READ, &I2C_buffer[0], 3);
	D1 = ((uint32) I2C_buffer[0] << 16) | ((uint32) I2C_buffer[1] << 8) | I2C_buffer[3];
	I2C_write(PSENSOR, CONVERT_D2, 0, 0);
	delay(20);												//Wait 20ms (worst case) for conversion to end
	I2C_read(PSENSOR, ADC_READ, &I2C_buffer[0], 3);
	D2 = ((uint32) I2C_buffer[0] << 16) | ((uint32) I2C_buffer[1] << 8) | I2C_buffer[3];
}






int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
//    _BIS_SR(GIE);
	I2C_setup(400000);			//Start I2C communication at 100k bps

	reset_sequence();
	read_factory_calibration();
	read_conversions();


	dT = D2 - (uint32) (C5 << 8);
	TEMP = 2000 + ((dT*C6) >> 23);

	OFF = ((uint32) C2 << 16) + ((C4*dT) >> 7);
	SENS = (uint32) (C1 << 15) + ((C3*dT) >> 8);
	P = ((D1*SENS)>>21) - (OFF >> 13);
	_no_operation();


	return 0;
}
