#include "syscall.h"

int mon1;
int mon2;
int value1;
int value2;
int lock1;
int cond1;
int cond2;
int lock2;

int main(){
	PrintString("Creating monitors and locks\n", 28);
	mon1 = CreateMonitor("Monitor1", 8, 5);
	mon1 = CreateMonitor("Monitor2", 8, 10);
	lock1 = CreateLock("Lock1", 5, 0);
	lock2 = CreateLock("Lock2", 5, 0);
	cond1 = CreateCondition("Cond1", 5, 0);
	cond2 = CreateCondition("Cond2", 5, 0);

	PrintString("Setting Monitors 1 from 0-4, should be sucessful.\n", 50);
	SetMonitor(mon1, 0, 0);
	SetMonitor(mon1, 1, 1);
	SetMonitor(mon1, 2, 2);
	SetMonitor(mon1, 3, 3);
	SetMonitor(mon1, 4, 4);

	PrintString("Setting Monitors 2 from 5-9, with acquire and release, should be successful.\n", 77);
	Acquire(lock1);
	SetMonitor(mon2, 5, 5);
	SetMonitor(mon2, 6, 6);
	SetMonitor(mon2, 7, 7);
	SetMonitor(mon2, 8, 8);
	SetMonitor(mon2, 9, 9);
	Release(lock1);

	value1 = GetMonitor(mon1, 3);
	PrintString("Printing monitor value. Should have 3.\n", 39);
	PrintNum(value1);PrintNl();
	value2 = GetMonitor(mon2, 5);
	PrintString("Printing monitor value. Should have 5.\n", 39);
	PrintNum(value2);PrintNl();


	Exit(1);
}
