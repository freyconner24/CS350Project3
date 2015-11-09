/* condServer_t2.c
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
  PrintNl();
  Write("Operations with wrong lock in t2, should not be successful\n", 59, ConsoleOutput);
  Write("Waiting cond1 with lock1, should be successful\n", 47, ConsoleOutput);
  Acquire(lock1);
  Wait(lock1, cond1);

  PrintNl();
  Write("A working set of operations, should be successful\n", 50, ConsoleOutput);
  PrintNl();
  Write("Broadcasting with lock1, should be successful\n", 47, ConsoleOutput);
  Acquire(lock1);
  Broadcast(lock1, cond1);
  Write("Destroying cond1, should be successful\n", 39, ConsoleOutput);
  DestroyCondition(cond1);

  Write("Destroying cond2, should be successful\n", 39, ConsoleOutput);
  DestroyCondition(cond2);

  Write("Finshing condServer_t1\n", 23, ConsoleOutput);
  Exit(0);
}
