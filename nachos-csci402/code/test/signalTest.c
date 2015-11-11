/* lock_test.c
 *	Simple program to test the file handling system calls
 */

#include "syscall.h"

int lock1 = 0;
int lock2 = 1;
int cond1 = 0;
int cond2 = 1;
int theLockThatDoesntExist;
int check = 0;
int i = 0;
int deadLock1;
int deadLock2;
int lockToBeDestroyed;

int main() {
  Acquire(lock1);
  Wait(lock1, cond1);
  for(i = 0; i < 40000; ++i){
    Yield();
  }
  Acquire(lock1);
  Signal(lock1, cond1);
  for(i = 0; i < 40000; ++i){
    Yield();
  }
  Release(lock1);
	Write("Finshing signalTest\n", 21, ConsoleOutput);
	Exit(0);
}
