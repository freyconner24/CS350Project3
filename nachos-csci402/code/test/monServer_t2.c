#include "syscall.h"

int mon1 = 0;
int mon2 = 1;
int value1;
int value2;
int lock1 = 0;
int lock2 = 1;
int cond1 = 0;
int cond2 = 1;

int main(){
PrintString("Acquiring and printing monitor value. Should have 9.\n", 54);
  Acquire(lock1);
  value1 = GetMonitor(mon1, 0);
  PrintNum(value1);PrintNl();
  Release(lock1);

	Exit(1);
}
