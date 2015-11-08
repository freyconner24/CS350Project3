#include "syscall.h"

int main(){
	int lock1, lock2;
	lock1 = CreateLock("SharedLock", 10, 1);
	Acquire(lock1);
	lock2 = CreateLock("SoloLock", 8, 2);
	Acquire(lock2);
	DestroyLock(lock2);
	Acquire(lock2);
	Release(lock1);
	Exit(1);
}
