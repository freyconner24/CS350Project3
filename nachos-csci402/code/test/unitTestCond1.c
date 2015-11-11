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
  Write("Acquire and Signal 3\n", 21, ConsoleOutput);
  Acquire(lock3);
  Signal(lock3, cond3);

  Write("Finshing unit1\n", 15, ConsoleOutput);
  Exit(0);
}
