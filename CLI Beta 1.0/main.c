#include <msp430.h> 
#include "Serial_JMPv2.2.h"
#include "CMD_JMP.c"

#include <stdlib.h>



/*
 * main.c
 */

 
 
 
 
 
 int testCLI(char *this){
	int returnVal = -1;
	if(strCmp(this, "test1", 5) > 0){
		USB_print("test case succesfull: 1\n");
		returnVal = 1;
	}else{
		USB_print("test case failed: 1\n");
	}
	return returnVal;
}
 
 
 
 int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	USB_setup(9600);
	
	struct commandList commands;
	
	initializeCommandList(&commands, 5);
	addCommand(&commands, &testCLI);
	
	char cmd[COMMAND_BUFFER_SIZE], *transactionComplete = 0;
	transactionComplete = malloc(1);


	USB_getline_a(cmd, transactionComplete);


	while(1){
		if(transactionComplete){
			verifyCommand(&commands, cmd);
		}
	}	
	//Run commands
}












