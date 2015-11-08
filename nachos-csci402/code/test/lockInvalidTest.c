/* lock_test.c
 *	Simple program to test the file handling system calls
 */

#include "syscall.h"

int lock1;
int lock2;
int theLockThatDoesntExist;
int check = 0;
int i = 0;
int deadLock1;
int deadLock2;
int lockToBeDestroyed;

int main() {
  lock1 = CreateLock("Lock1", 5, 0);
	lock2 = CreateLock("Lock2", 5, 0);
	deadLock1 = CreateLock("deadLock1", 9, 0);
	deadLock2 = CreateLock("deadLock2", 9, 0);
	lockToBeDestroyed = CreateLock("lockToBeDestroyed", 17, 0);

  theLockThatDoesntExist = lock1+10;
	Write("Testing invalid actions for locks\n", 34, ConsoleOutput);
	Write("Acquiring theLockThatDoesntExist, should give error\n", 52, ConsoleOutput);
	Acquire(theLockThatDoesntExist);
	Write("Releasing theLockThatDoesntExist, should give error\n", 52, ConsoleOutput);
	Release(theLockThatDoesntExist);
	Write("Destroying theLockThatDoesntExist, should give error\n", 53, ConsoleOutput);
	DestroyLock(theLockThatDoesntExist);
	Write("Destroying lockToBeDestroyed, should be successful\n", 51, ConsoleOutput);
	DestroyLock(lockToBeDestroyed);
	Write("Finshing invalid test\n", 22, ConsoleOutput);
	Exit(0);
}
