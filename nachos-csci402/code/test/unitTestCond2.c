/* condServer_t4.c
 * make sure conditions work with 5 clients
 */

#include "syscall.h"

int lock3 = 2;
int lock4 = 3;
int theLockThatDoesntExist;
int theCondThatDoesntExist;
int i = 0;
int cond3 = 2;
int cond4 = 3;

int main() {
  Write("Acquire and Wait 3, should be successful\n", 41, ConsoleOutput);
  Acquire(lock3);
  Wait(lock3, cond3);

  Write("Finshing unit_1\n", 16, ConsoleOutput);
  Exit(0);
}
