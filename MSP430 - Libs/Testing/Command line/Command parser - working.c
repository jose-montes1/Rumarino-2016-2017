#include<stdio.h>
#include<stdlib.h>


typedef int (*comparator)(char *);


struct commandList{
	unsigned char amountOfCommands;
	unsigned char totalCommands;	
	comparator *cmmdArr;
};


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


int strCmp(char *one, char *two, int positions){
	int i = positions, ret = 1;
	while(i > 0){
		printf("%c %c\n", *one, *two);
		if(*one != *two){
			ret = -1;
			break;
		}
		one++; 
		two++; 
		i--;
	}
	return ret;
}


int testCLI(char *this){
	int returnVal = -1;
	if(strCmp(this, "test1", 5) > 0){
		printf("test case succesfull: 1\n");
		returnVal = 1;
	}else{
		printf("test case failed: 1\n");
	}
	return returnVal;
}

int test2CLI(char *this){
	int returnVal = -1;
	if(strCmp(this, "test2", 5) > 0){
		printf("test case succesfull: 2\n");
		returnVal = 1;
	}else{
		printf("test case failed: 1\n");
	}
	return returnVal;
}



int main(void){

	struct commandList* commands;
	commands = (struct commandList *) malloc(sizeof(struct commandList));

	initializeCommandList(commands, 5);
	printf("Initialized shit\n");	
	addCommand(commands, &testCLI);
	addCommand(commands, &test2CLI);
	printf("Added Command\n");
	if(verifyCommand(commands, "this") > 0) printf("Noice1\n");	
	if(verifyCommand(commands, "test") > 0) printf("Noice2\n");
	if(verifyCommand(commands, "test1") > 0) printf("Noice3\n");
	if(verifyCommand(commands, "test2") > 0) printf("Noice4\n");

}
