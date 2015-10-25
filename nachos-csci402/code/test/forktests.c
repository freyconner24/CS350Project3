/*
This file tests that forks work properly.
Invalid input into fork does not even compile.
*/

#include "syscall.h"


void helloWorld(){
	Write("Hello World!\n", 13, ConsoleOutput);
	Exit(0);
}

void addOneAndPrintNumber();

int someNumber = 0;
int main(){
	/*Testing that these two threads fork */
	Fork(helloWorld, 0);
	Fork(helloWorld, 0);

	Fork(helloWorld, "awdaw");
	/*Testing that threads can access the same variables in the process*/
	Fork(addOneAndPrintNumber, 0);
	Fork(addOneAndPrintNumber, 0);
	Exit(0);
}

void addOneAndPrintNumber(){
	PrintString("someNumber is currently: ", 25);
	PrintNum(someNumber);
	PrintNl();
	someNumber++;
	PrintString("Adding one to someNumber: ", 26);
	PrintNum(someNumber);
	PrintNl();
	Exit(0);
}
