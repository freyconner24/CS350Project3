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
  Write("Waiting with wrong Lock2, should give error\n", 44, ConsoleOutput);
  Wait(lock2, cond1);
  Write("Signaling with wrong lock2, should give error\n", 46, ConsoleOutput);
  Signal(lock2, cond1);
  Write("Broadcasting with wrong lock2, should give error\n", 49, ConsoleOutput);
  Broadcast(lock2, cond1);
  PrintNl();
  Write("Signaling with lock1, should be successful\n", 43, ConsoleOutput);
  Acquire(lock1);
  Signal(lock1, cond1);

	Write("Finshing condServer_t2\n", 23, ConsoleOutput);
	Exit(0);
}
