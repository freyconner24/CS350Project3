/* lock_test.c
 *	Simple program to test the file handling system calls
 */

#include "syscall.h"

int lock1;
int lock2;
int lock3;
int lock4;
int theLockThatDoesntExist;
int check = 0;
int i = 0;
int deadLock1;
int deadLock2;
int lockToBeDestroyed;

int main() {
  Acquire(lock1);
  for(i = 0; i < 40000; ++i){
    Yield();
  }
  Release(lock1);
  for(i = 0; i < 40000; ++i){
    Yield();
  }

	Write("Finshing acquireTest\n", 21, ConsoleOutput);
	Exit(0);
}
