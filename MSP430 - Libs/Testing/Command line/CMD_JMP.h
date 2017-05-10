/* This is the header file to interface with a simple command line structure 
 *that receives commands as strings and excecutes them
 *
 *
 *
 * author: Jose A. Montes Perez
 * 1 - april - 2017
 * fools :_=p
 */


#ifndef CMD_JMP_H_
#define CMD_JMP_H_ 


typedef int (*comparator)(char *);


struct commandList{
	unsigned char amountOfCommands;
	unsigned char totalCommands;	
	comparator *cmmdArr;
};


void initializeCommandList(struct commandList *this, int totalCmmds);
void addCommand(struct commandList *this, comparator funct);
int verifyCommand(struct commandList *this, char *command);











#endif /* CMD_JMP_H_ */