//Lets give this a shot
//We need the following dependencies
//#include "Motors_JMP.c"
//#include "Razor_AHRS.c"
//#include "General_JMP.c"
/*
void backTime(int dragFix){
	TB0CTL &= ~MC_1;					//Stop Timer
	int oldTime = TB0R;					//Save old value of the counter register (we assume the value started at 0)
	int oldCounter = TB0CCR0;			//Save old timer value
	int timeEX = 0;						//Verify time exceed from delay seconds
	if(!delayC) timeEx = 1;
	delayC = 1;							//Change the interrupt operation so that it removes low power mode
	TB0CCR0 = dragFix*oldTime/1000;		//Put it the new, modified value
	TB0R = 0;							//Reset counter value
	TB0CTL = MC_1;						//Put the counter to run
	LPM0;								//Put this in low power mode
	TB0CCR0 = oldCounter - oldTime;		//Fix value to count to
	TB0R = 0;							//Reset counter register
	if(timeEX) delayC = 0;							//Fix interrupt functionallity
	TB0CTL = MC_1;						//Place the clock to run again
}
*/
