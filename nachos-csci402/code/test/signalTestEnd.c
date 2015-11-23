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
  Write("Signall cond1 with lock1 again, should be successful\n", 53, ConsoleOutput);
  Acquire(lock1);
  Signal(lock1,cond1);
  Release(lock1);

  for (i = 0; i < 20000; ++i){
    Yield();
  }
  Acquire(lock1);
  Signal(lock1,cond1);
  Release(lock1);

  for (i = 0; i < 20000; ++i){
    Yield();
  }
  Acquire(lock1);
  Signal(lock1,cond1);
  Release(lock1);

  for (i = 0; i < 20000; ++i){
    Yield();
  }
  Acquire(lock1);
  Signal(lock1,cond1);
  Release(lock1);
  /*Wait(lock1, cond1);*/

  Write("Finshing signalTestEnd\n", 23, ConsoleOutput);
  Exit(0);
}
