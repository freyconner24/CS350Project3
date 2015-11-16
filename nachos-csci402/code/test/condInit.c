/* condServerInitTest.c
 *	Simple program to test the file handling system calls
 */

#include "syscall.h"

int lock1;
int lock2;
int lock3;
int lock4;
int theLockThatDoesntExist;
int theCondThatDoesntExist;
int i = 0;
int cond1;
int cond2;
int cond3;
int cond4;
int condToBeDestroyed;

int main() {
  Write("Creating test locks and CVs, should be successful\n", 50, ConsoleOutput);
  lock1 = CreateLock("Lock1", 5, 0);
	lock2 = CreateLock("Lock2", 5, 0);
	cond1 = CreateCondition("Condition1", 10, 0);
	cond2 = CreateCondition("Condition2", 10, 0);
  lock3 = CreateLock("Lock3", 5, 0);
  lock4 = CreateLock("Lock4", 5, 0);
  cond3 = CreateCondition("Condition3", 10, 0);
  cond4 = CreateCondition("Condition4", 10, 0);
	condToBeDestroyed = CreateCondition("condToBeDestroyed", 17, 0);

  theLockThatDoesntExist = lock1+10;
  theCondThatDoesntExist = cond1+10;
  Write("Destroying CV, should see both successful and unsuccesful attempts.\n", 68, ConsoleOutput);
  Write("Destroying condToBeDestroyed\n", 29, ConsoleOutput);
  DestroyCondition(condToBeDestroyed);
  Write("Destroying theCondThatDoesntExist\n", 34, ConsoleOutput);
  DestroyCondition(theCondThatDoesntExist);
  Write("Waiting theCondThatDoesntExist\n", 32, ConsoleOutput);
  Acquire(lock1);
  Wait(lock1, theCondThatDoesntExist);
  Write("Signaling theCondThatDoesntExist\n", 34, ConsoleOutput);
  Acquire(lock1);
  Signal(lock1, theCondThatDoesntExist);
  Release(lock1);
  Write("Broadcasting theCondThatDoesntExist\n", 36, ConsoleOutput);
  Broadcast(lock1,theCondThatDoesntExist);
	Write("Finshing condInit\n", 18, ConsoleOutput);
	Exit(0);
}
