/* lock_test.c
 *	Simple program to test the file handling system calls
 */

#include "syscall.h"

int lock3 = 2;
int lock4 = 3;
int i;
int check = 0;
int j;

int main() {
  Write("t3 releasing lock1 before acquiring\n", 36, ConsoleOutput);
	Release(lock3);
	Write("t3 acquiring lock1, should be successful\n", 41, ConsoleOutput);
	Acquire(lock3);
  Acquire(lock3);
	for(j = 0; j < 50000; ++j){
    Yield();
  }
	Write("t3 releasing lock3\n", 19, ConsoleOutput);
	Release(lock3);
  Acquire(lock4);
  DestroyLock(lock4);
  Release(lock4);
	Write("Finshing thread 3\n", 18, ConsoleOutput);
	Exit(0);
}
