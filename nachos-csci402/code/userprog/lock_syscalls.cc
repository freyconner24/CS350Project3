#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "custom_syscalls.h"
#include "synchlist.h"
#include "addrspace.h"
#include <stdio.h>
#include <iostream>

#define BUFFER_SIZE 32

int CreateLock_sys(int vaddr, int size, int appendNum) {
	currentThread->space->locksLock->Acquire(); //CL: acquire kernelLock so that no other thread is running on kernel mode
	if (currentThread->space->lockCount >= MAX_LOCK_COUNT){ //check if over max size of lock count
		DEBUG('l',currentThread->getName());
		DEBUG('l'," has too many locks!-----------------------\n");
		currentThread->space->locksLock->Release();
		return -1;
	}
	if (size < 0 || size >= 32){ // check if size of char array is valid
		DEBUG('l',currentThread->getName());
		DEBUG('l'," Size must be between 0 and 32!-----------------------\n");
		currentThread->space->locksLock->Release();
		return -1;
	}
	char* buffer = new char[size + 1]; //allocate new char array
	char* empty = new char[size + 1];
	buffer[size] = '\0'; //end the char array with a null character
	empty[size] = '\0';

	if (copyin(vaddr, size, buffer) == -1){
		DEBUG('l',"%s"," COPYIN FAILED\n");
		delete[] buffer;
		currentThread->space->locksLock->Release();
		return -1;
	}; //copy contents of the virtual addr (ReadRegister(4)) to the buffer

	// set attributes of new lock
	currentThread->space->userLocks[currentThread->space->lockCount].userLock = new Lock(buffer); // instantiate new lock
	currentThread->space->userLocks[currentThread->space->lockCount].deleteFlag = FALSE; // indicate the lock is not to be deleted
	currentThread->space->userLocks[currentThread->space->lockCount].isDeleted = FALSE; // indicate the lock is not in use
	int currentLockIndex = currentThread->space->lockCount; // save the currentlockcount to be returned later
	++(currentThread->space->lockCount); // increase lock count
	DEBUG('a', "Lock has number %d and name %s\n", currentLockIndex, buffer);
	DEBUG('l',"    Lock::Lock number: %d || name: %s created by %s\n", currentLockIndex, currentThread->space->userLocks[currentLockIndex].userLock->getName(), currentThread->getName());
	currentThread->space->locksLock->Release(); //release kernel lock
	return currentLockIndex;
}

void Acquire_sys(int index) {
	currentThread->space->locksLock->Acquire();
	if (index < 0 || index >= currentThread->space->lockCount){ // check if index is in valid range
		DEBUG('l',"    Lock::Lock number %d invalid, thread %s can't acquire-----------------------\n", index, currentThread->getName());
		currentThread->space->locksLock->Release();
		return;
	}

	if (currentThread->space->userLocks[index].isDeleted == TRUE){ // check if lock is deleted
		DEBUG('l',"    Lock::Lock number %d already destroyed, thread %s can't acquire-----------------------\n", index, currentThread->getName());
		currentThread->space->locksLock->Release();
		interrupt->Halt();
	}

	// check if lock is busy
	if(currentThread->space->userLocks[index].userLock->lockStatus == currentThread->space->userLocks[index].userLock->BUSY){
		DEBUG('l',"    Lock::Lock number %d and name %s already in use, adding to queue-----------------------\n", index, currentThread->space->userLocks[index].userLock->getName());
		currentThread->space->locksLock->Release();
		currentThread->space->userLocks[index].userLock->Acquire(); // acquire userlock at index
		return;
	}

	DEBUG('a', "Lock  number %d and name %s\n", index, currentThread->space->userLocks[index].userLock->getName());
	DEBUG('l',"    Lock::Lock number: %d || name:  %s acquired by %s\n", index, currentThread->space->userLocks[index].userLock->getName(), currentThread->getName());

	Lock* userLock = currentThread->space->userLocks[index].userLock;
	if(userLock->lockStatus != userLock->FREE) {
		updateProcessThreadCounts(currentThread->space, SLEEP);
	}
	currentThread->space->locksLock->Release();//release kernel lock
	currentThread->space->userLocks[index].userLock->Acquire(); // acquire userlock at index
}

void Release_sys(int index) {
	currentThread->space->locksLock->Acquire(); // CL: acquire kernelLock so that no other thread is running on kernel mode
	Lock* userLock = currentThread->space->userLocks[index].userLock;
	if (index < 0 || index >= currentThread->space->lockCount){ //check if index is valid
		DEBUG('l',"Lock number %d invalid, thread %s can't release-----------------------\n", index, currentThread->getName());
		currentThread->space->locksLock->Release();
		return;
	}
	if (currentThread->space->userLocks[index].isDeleted == TRUE){ // check if lock is deleted
		DEBUG('l'," Lock number %d already destroyed, %s can't release-----------------------\n", index, currentThread->getName());
		currentThread->space->locksLock->Release();
		return;
	}
	if(userLock->lockStatus == userLock->FREE){ // check if lock is already free
		DEBUG('l'," lock not in use, nothing is done-----------------------\n");
		currentThread->space->locksLock->Release();
		return;
	}
	DEBUG('l',"    Lock::Lock number: %d || and name: %s released by %s\n", index, currentThread->space->userLocks[index].userLock->getName(), currentThread->getName());

	currentThread->space->locksLock->Release();//release kernel lock
	currentThread->space->userLocks[index].userLock->Release(); // release userlock at index
	// destroys lock if lock is free and delete flag is true
	if(currentThread->space->userLocks[index].userLock->lockStatus == currentThread->space->userLocks[index].userLock->FREE && currentThread->space->userLocks[index].deleteFlag == TRUE) {
		DEBUG('l'," Lock  number %d  and name %s is destroyed by %s \n", index, currentThread->space->userLocks[index].userLock->getName(), currentThread->getName());
		currentThread->space->userLocks[index].isDeleted = TRUE;
		delete currentThread->space->userLocks[index].userLock;
	}
}

void DestroyLock_sys(int index) {
	currentThread->space->locksLock->Acquire();; // CL: acquire locksLock so that no other thread is running on kernel mode
	if (index < 0 || index >= currentThread->space->lockCount){ // check if lock index is valid
		DEBUG('l'," Lock number %d invalid, thread %s can't destroy-----------------------\n", index, currentThread->getName());
		currentThread->space->locksLock->Release();
		return;
	}
	if (currentThread->space->userLocks[index].isDeleted == TRUE){ // check if lock is already destroyed
		DEBUG('l'," Lock number %d already destroyed, thread %s can't destroy-----------------------\n", index, currentThread->getName());
		currentThread->space->locksLock->Release();
		return;
	}

	currentThread->space->userLocks[index].deleteFlag = TRUE;
	if (currentThread->space->userLocks[index].userLock->lockStatus == currentThread->space->userLocks[index].userLock->BUSY){
		DEBUG('l'," DestroyLock::Lock number %d and name %s still in use, delete later-----------------------\n", index, currentThread->space->userLocks[index].userLock->getName());
	}else{
		currentThread->space->userLocks[index].isDeleted = TRUE;
		delete currentThread->space->userLocks[index].userLock;
		DEBUG('l'," Lock number %d and name %s deleted-----------------------\n", index, currentThread->space->userLocks[index].userLock->getName());

	}
	currentThread->space->locksLock->Release();//release kernel lock
}
