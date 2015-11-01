/* testfiles.c
 *	Simple program to test the file handling system calls
 */

#include "syscall.h"


void helloWorld(){
	Write("Hello World!\n", 13, ConsoleOutput);
	Exit(0);
}

int main() {
	OpenFileId fd;
	int bytesread, lockNum, condNum;
 	char buf[20];
	/*Write("Testing Locks\n", 14, ConsoleOutput);

	lockNum = CreateLock("nameLock");
	Acquire(lockNum);
	condNum = CreateCondition("someCondition");
	Signal(lockNum, condNum);
	Broadcast(lockNum, condNum);
	Release(lockNum);
	DestroyLock(lockNum);
	Write("Locks complete\n", 15, ConsoleOutput);
	Exec("../test/halt");
	Exec("../test/halt");
	*/
/*	Exec('halt');*/

	Write("Forking helloWorld\n", 19, ConsoleOutput);
	Fork(helloWorld, 0);
		Fork(helloWorld, 0);
			Fork(helloWorld, 0);
				Fork(helloWorld, 0);
					Fork(helloWorld, 0);
						Fork(helloWorld, 0);
							Fork(helloWorld, 0);
								Fork(helloWorld, 0);
									Fork(helloWorld, 0);
										Fork(helloWorld, 0);

										Fork(helloWorld, 0);
	Write("Finshing testfiles.c\n", 21, ConsoleOutput);

}
