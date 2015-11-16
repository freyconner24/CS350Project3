/* signalTestEnd.c
 *	Simple program to test the file handling system calls
 */

#include "syscall.h"

int lock1 = 0;
int lock2 = 1;
int theLockThatDoesntExist;
int theCondThatDoesntExist;
int i = 0;
int cond1 = 0;
int cond2 = 1;
int condToBeDestroyed = 2;

int main() {
  Write("Waiting cond1 with lock1, should be successful\n", 47, ConsoleOutput);
  Acquire(lock1);
  Wait(lock1,cond1);
  Release(lock1);

  Write("Finshing signalTest\n", 20, ConsoleOutput);
  Exit(0);
}
