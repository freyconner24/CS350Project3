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
  Write("Acquire and Signal 3, no permission\n", 36, ConsoleOutput);
  Acquire(lock3);
  Signal(lock3, cond3);

  Write("Acquire and Wait 3, should be successful\n", 41, ConsoleOutput);
  Acquire(lock3);
  Wait(lock3, cond3);

  Write("Acquire and Broadcast 4, should be successful\n", 46, ConsoleOutput);
  Acquire(lock4);
  Broadcast(lock4, cond4);

  Write("Acquire and Broadcast 3, should be successful\n", 46, ConsoleOutput);

  Acquire(lock3);
  Broadcast(lock3, cond3);

  Write("Destroy cond3 cond4, should be successful\n", 42, ConsoleOutput);

  DestroyCondition(cond3);
  DestroyCondition(cond4);

  Write("Finshing condServer_t4\n", 23, ConsoleOutput);
  Exit(0);
}
