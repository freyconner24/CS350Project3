#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "custom_syscalls.h"
#include "addrspace.h"
#include <stdio.h>
#include <iostream>

#define BUFFER_SIZE 32

int CreateCondition_sys(int vaddr, int size, int appendNum) {
	currentThread->space->condsLock->Acquire(); //CL: acquire kernelLock so that no other thread is running on kernel mode
	if (currentThread->space->condCount == MAX_COND_COUNT){
		printf("    CreateCondition::");
		printf(currentThread->getName());
		printf(" has too many conditions!\n");
		currentThread->space->condsLock->Release();
		return -1;
	}

	if(size < 0 || size > 32) {
		printf("    CreateCondition::");
		printf(currentThread->getName());
		printf(" size must be: (size < 0 || size > 32) ? %d\n", size);
		currentThread->space->condsLock->Release();
		return -1;
	}

	char* buffer = new char[size + 1];
	buffer[size] = '\0'; //end the char array with a null character

	if(copyin(vaddr, size, buffer) == -1) { //copy contents of the virtual addr (ReadRegister(4)) to the buffer
		printf("    CreateCondition::copyin failed\n");
		delete [] buffer;
		currentThread->space->condsLock->Release();
		return -1;
	}
	if (currentThread->space->condCount < 0 || currentThread->space->condCount >= MAX_COND_COUNT){
		DEBUG('b',"ERROR IN CONDCOUNT\n");
		printf("    CreateCondition::condCount is neg or greater than max count: %d\n", currentThread->space->condCount);
		currentThread->space->condsLock->Release();
		return -1;
	}

	currentThread->space->userConds[currentThread->space->condCount].userCond = new Condition(buffer); // instantiate new lock
	currentThread->space->userConds[currentThread->space->condCount].deleteFlag = FALSE; // indicate the lock is not to be deleted
	currentThread->space->userConds[currentThread->space->condCount].isDeleted = FALSE; // indicate the lock is not in use
	int currentCondIndex = currentThread->space->condCount; // save the currentlockcount to be returned later
	currentThread->space->condCount++;
	printf("    CreateCondition::Condition number: %d || name: %s is created by %s\n", currentCondIndex, currentThread->space->userConds[currentCondIndex].userCond->getName(), currentThread->getName());

	currentThread->space->condsLock->Release(); //release kernel lock
	return currentCondIndex;
}

void Wait_sys(int lockIndex, int conditionIndex) {
	currentThread->space->condsLock->Acquire(); // CL: acquire kernelLock so that no other thread is running on kernel mode
	if (conditionIndex < 0 || conditionIndex >= currentThread->space->condCount){
		printf("    Wait::invalid cond\n");
		currentThread->space->condsLock->Release();
		return;
	}
	// printf("    LockIndex: %d\n", lockIndex);
	// printf("    LockCount: %d\n", currentThread->space->lockCount);
	// printf("    ConditionIndex: %d\n", conditionIndex);
	// printf("    ConditionCount: %d\n", currentThread->space->condCount);
	if (lockIndex < 0 || lockIndex >= currentThread->space->lockCount){
		printf("    Wait::invalid lock\n");
		currentThread->space->condsLock->Release();
		return;
	}
	if (currentThread->space->userConds[conditionIndex].isDeleted == TRUE){
		printf("    Wait::destroyed cond\n");
		currentThread->space->condsLock->Release();
		return;
	}
	if (currentThread->space->userLocks[lockIndex].isDeleted == TRUE){
		printf("    Wait::destroyed lock\n");
		currentThread->space->condsLock->Release();
		return;
	}

	// how does a lock use a wait?
	currentThread->space->condsLock->Release();//release kernel lock
	printf("    Wait::Condition  number %d, name %s is waited on by %s \n", conditionIndex, currentThread->space->userConds[conditionIndex].userCond->getName(), currentThread->getName());

	//updateProcessThreadCounts(currentThread->space, SLEEP);
	currentThread->space->userConds[conditionIndex].userCond->Wait(currentThread->space->userLocks[lockIndex].userLock); // acquire userlock at index
}

void Signal_sys(int lockIndex, int conditionIndex) {
	currentThread->space->condsLock->Acquire(); // CL: acquire kernelLock so that no other thread is running on kernel mode
	if(conditionIndex < 0 || conditionIndex >= currentThread->space->condCount){
		printf("    Signal::invalid cond\n");
		currentThread->space->condsLock->Release();
		return;
	}
	if(lockIndex < 0 || lockIndex >= currentThread->space->lockCount){
		printf("    Signal::invalid lock\n");
		currentThread->space->condsLock->Release();
		return;
	}
	if(currentThread->space->userConds[conditionIndex].isDeleted == TRUE){
		printf("    Signal::destroyed cond\n");
		currentThread->space->condsLock->Release();
		return;
	}
	if(currentThread->space->userLocks[lockIndex].isDeleted == TRUE){
		printf("    Signal::destroyed lock\n");
		currentThread->space->condsLock->Release();
		return;
	}

	currentThread->space->condsLock->Release(); //release kernel lock
	printf("    Condition  number %d, name %s is signalled by %s\n", conditionIndex, currentThread->space->userConds[conditionIndex].userCond->getName(), currentThread->getName());

	// if(currentThread->space->userLocks[lockIndex].userLock->waitQueueIsEmpty()) {
	// 	//updateProcessThreadCounts(currentThread->space, AWAKE);
	// }
	currentThread->space->userConds[conditionIndex].userCond->Signal(currentThread->space->userLocks[lockIndex].userLock); // acquire userlock at index

	if (currentThread->space->userConds[conditionIndex].deleteFlag &&
		currentThread->space->userConds[conditionIndex].userCond->waitQueueIsEmpty()){
		printf("    DestroyCondition::Condition  number %d, name %s is destroyed by %s \n", conditionIndex, currentThread->space->userConds[conditionIndex].userCond->getName(), currentThread->getName());
		delete currentThread->space->userConds[conditionIndex].userCond;
		currentThread->space->userConds[conditionIndex].isDeleted = TRUE;
	}
}

void Broadcast_sys(int lockIndex, int conditionIndex) {
	currentThread->space->condsLock->Acquire(); // CL: acquire kernelLock so that no other thread is running on kernel mode

	if(conditionIndex < 0 || conditionIndex >= currentThread->space->condCount){
		printf("    Broadcast::invalid cond\n");
		currentThread->space->condsLock->Release();
		return;
	}
	if(lockIndex < 0 || lockIndex >= currentThread->space->lockCount){
		printf("    Broadcast::invalid lock\n");
		currentThread->space->condsLock->Release();
		return;
	}
	if(currentThread->space->userConds[conditionIndex].isDeleted == TRUE){
		printf("    Broadcast::signaling destroyed cond\n");
		currentThread->space->condsLock->Release();
		return;
	}
	if(currentThread->space->userLocks[lockIndex].isDeleted == TRUE){
		printf("    Broadcast::signaling destroyed lock\n");
		currentThread->space->condsLock->Release();
		return;
	}

	currentThread->space->condsLock->Release();//release kernel lock
	printf("    Broadcast::Condition  number %d, name %s is broadcast by %s \n", conditionIndex, currentThread->space->userConds[conditionIndex].userCond->getName(), currentThread->getName());

	currentThread->space->userConds[conditionIndex].userCond->Broadcast(currentThread->space->userLocks[lockIndex].userLock); // acquire userlock at index

	if (currentThread->space->userConds[conditionIndex].deleteFlag){
		printf("    DestroyCondition::Condition  number %d, name %s is destroyed by %s \n", conditionIndex, currentThread->space->userConds[conditionIndex].userCond->getName(), currentThread->getName());
		delete currentThread->space->userConds[conditionIndex].userCond;
		currentThread->space->userConds[conditionIndex].isDeleted = TRUE;
	}
}

void DestroyCondition_sys(int index) {
	currentThread->space->condsLock->Acquire(); // CL: acquire kernelLock so that no other thread is running on kernel mode
	if (index < 0 || index >= currentThread->space->condCount){
		printf("    DestroyCondition::destroying invalid cond\n");
		currentThread->space->condsLock->Release();
		return;
	}
	if (currentThread->space->userConds[index].isDeleted == TRUE){
		printf("    DestroyCondition::condition is already destroyed\n");
		currentThread->space->condsLock->Release();
		return;
	}
	if (!currentThread->space->userConds[index].userCond->waitQueueIsEmpty()){
		printf("    DestroyCondition::cond still in use\n");
		currentThread->space->userConds[index].deleteFlag = TRUE; // s
		currentThread->space->condsLock->Release();
		return;
	}else{
		currentThread->space->userConds[index].isDeleted = TRUE;
		delete currentThread->space->userConds[index].userCond;
	}

	currentThread->space->condsLock->Release();//release kernel lock
}
