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

  PrintNl();
	Write("Testing invalid actions for conds\n", 34, ConsoleOutput);
	Write("Waiting with invalid lock and valid condition, should give error\n", 65, ConsoleOutput);
	Wait(theLockThatDoesntExist, cond1);
  Write("Waiting with valid lock and invalid condition, should give error\n", 65, ConsoleOutput);
	Wait(lock1, theCondThatDoesntExist);
	Write("Signaling theCondThatDoesntExist, should give error\n", 52, ConsoleOutput);
	Signal(lock1, theCondThatDoesntExist);
	Write("Destroying theCondThatDoesntExist, should give error\n", 53, ConsoleOutput);
	DestroyCondition(theCondThatDoesntExist);
	Write("Destroying condToBeDestroyed, should be successful\n", 51, ConsoleOutput);
	DestroyCondition(condToBeDestroyed);
  Write("Signaling condToBeDestroyed, should give error\n", 47, ConsoleOutput);
	Signal(lock1, condToBeDestroyed);
  Write("Broadcasting condToBeDestroyed, should give error\n", 50, ConsoleOutput);
  Broadcast(lock1, condToBeDestroyed);
  Write("Destroying condToBeDestroyed, should give error\n", 48, ConsoleOutput);
	DestroyCondition(condToBeDestroyed);
  Write("Waiting before acquring, should give error\n", 43, ConsoleOutput);
  Wait(lock1, cond1);
  Write("Signaling before acquring, should give error\n", 45, ConsoleOutput);
  Signal(lock1, cond1);
  Write("Broadcasting before acquring, should give error\n", 48, ConsoleOutput);
  Broadcast(lock1, cond1);


	Write("Finshing condServerInitTest\n", 28, ConsoleOutput);
	Exit(0);
}
