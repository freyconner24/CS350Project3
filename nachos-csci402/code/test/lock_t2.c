/* lock_test.c
 *	Simple program to test the file handling system calls
 */

#include "syscall.h"

int lock1 = 0;
int lock2 = 1;
int i;
int check = 0;
int j;

int main() {
  Write("t2 releasing lock1 before acquiring\n", 36, ConsoleOutput);
	Release(lock1);
  PrintString("t2 trying to destroy lock2\n",27);
	Write("t2 acquiring lock1, should be successful\n", 41, ConsoleOutput);
	Acquire(lock1);
	for(j = 0; j < 70000; ++j){
    Yield();
  }
	Write("t2 releasing lock1\n", 19, ConsoleOutput);
	Release(lock1);
  Acquire(lock2);
  DestroyLock(lock2);
  Release(lock2);
	Write("Finshing thread 2\n", 18, ConsoleOutput);
	Exit(0);
}
