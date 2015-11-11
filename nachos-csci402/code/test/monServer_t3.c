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
	PrintString("Acquiring and Signaling.\n", 25);

	Acquire(lock2);
  Signal(lock2, cond2);

	Exit(1);
}
