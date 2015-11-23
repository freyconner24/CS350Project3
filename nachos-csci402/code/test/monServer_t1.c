#include "syscall.h"

int mon1 = 0;
int mon2 = 1;
int value1 = 0;
int value2 = 1;
int lock1 = 0;
int lock2 = 1;
int cond1 = 0;
int cond2 = 1;
int i = 0;


int main(){
	PrintString("Setting monitors\n", 17);
  Acquire(lock1);
	SetMonitor(mon1, 0, 9);
	for (i = 0; i < 30000; ++i){
		Yield();
	}
	Release(lock1);
	PrintString("Waiting on lock2\n", 17);
	Acquire(lock1);
  Wait(lock1, cond1);
	Release(lock1);

	PrintString("Destroying Monitors 1 and 2\n", 28);
	DestroyMonitor(mon1);
	DestroyMonitor(mon2);

	Exit(1);
}
