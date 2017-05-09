#include "CMD_JMP.h"

void initializeCommandList(struct commandList *this, int totalCmmds){
	this->amountOfCommands = 0;
	this->totalCommands = totalCmmds;
	this->cmmdArr = (comparator *) calloc((size_t) totalCmmds, sizeof(comparator));
}



void addCommand(struct commandList *this, comparator funct){
	if(this->amountOfCommands < this->totalCommands){
		*(this->cmmdArr + this->amountOfCommands) = funct;
		this->amountOfCommands++;
	}
}


int verifyCommand(struct commandList *this, char *command){
	int i = this->amountOfCommands;
	char validCommand = 0;
	for(; i > 0; i--){
		if(((*(*(this->cmmdArr + i - 1)))(command)) > 0){
			validCommand = 1; 			
			break;		
		}
	}
	return validCommand; 
}

