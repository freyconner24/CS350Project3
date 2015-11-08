#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "custom_syscalls.h"
#include "synchlist.h"
#include "addrspace.h"
#include "network.h"
#include "post.h"
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <string>

#define BUFFER_SIZE 32

void sendToServer(PacketHeader &pktHdr, MailHeader &mailHdr, char* serverCode, char name[], int entityIndex1, int entityIndex2) {
    mailHdr.to = 0;
    mailHdr.from = 0;
    pktHdr.to = 0;

    stringstream ss;
	ss << serverCode;

	if(serverCode[2] == 'C') { // it is a Create and needs a name
		ss << name;
	} else {
		ss << entityIndex1 << ' ' << entityIndex2;
	}

	string str = ss.str();
	char sendBuffer[64];

	for(unsigned int i = 0; i < str.size(); ++i) {
		sendBuffer[i] = str.at(i);
	}
	sendBuffer[str.size()] = '\0';
	mailHdr.length = str.size() + 1;

	cout << "Client::sendBuffer: " << sendBuffer << endl;
    bool success = postOffice->Send(pktHdr, mailHdr, sendBuffer);

    if ( !success ) {
    	cout << serverCode << "::";
		printf("Client::The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
		interrupt->Halt();
	}
}

string getFromServer(PacketHeader &pktHdr, MailHeader &mailHdr) {
	char inBuffer[64];
    postOffice->Receive(0, &pktHdr, &mailHdr, inBuffer);
    stringstream ss;
    ss << inBuffer;
    return ss.str();
}

int CreateLock_sys(int vaddr, int size, int appendNum) {
	char* name = new char[size + 1]; //allocate new char array
	name[size] = '\0'; //end the char array with a null character

	if (copyin(vaddr, size, name) == -1){
		DEBUG('l',"%s"," COPYIN FAILED\n");
		delete[] name;
		currentThread->space->locksLock->Release();
		return -1;
	}; //copy contents of the virtual addr (ReadRegister(4)) to the name

	// set attributes of new lock

    PacketHeader pktHdr;
	MailHeader mailHdr;

    sendToServer(pktHdr, mailHdr, "L C ", name, -1, -1);

    string receivedString = getFromServer(pktHdr, mailHdr);
    stringstream ss;
    ss << receivedString;

    int currentLockIndex = -1;
    ss >> currentLockIndex;

    if(currentLockIndex == -1) {
        cout << "Client::currentLockIndex == -1" << endl;
        interrupt->Halt();
    }

	currentThread->space->lockCount += 1;
    cout << "currentLockIndex: " << currentLockIndex << endl;
	//DEBUG('a', "Lock has number %d and name %s\n", currentLockIndex, buffer);
	//DEBUG('l',"    Lock::Lock number: %d || name: %s created by %s\n", currentLockIndex, currentThread->space->userLocks[currentLockIndex].userLock->getName(), currentThread->getName());
	return currentLockIndex;
}

void Acquire_sys(int lockIndex) {
	PacketHeader pktHdr;
	MailHeader mailHdr;

	sendToServer(pktHdr, mailHdr, "L A ", "", lockIndex, -1);

	string receivedString = getFromServer(pktHdr, mailHdr);

    cout << "Acquire::receivedString: " << receivedString << endl;

	// Lock* userLock = currentThread->space->userLocks[lockIndex].userLock;
	// if(userLock->lockStatus != userLock->FREE) {
	// 	updateProcessThreadCounts(currentThread->space, SLEEP);
	// }
	// currentThread->space->locksLock->Release();//release kernel lock
	// currentThread->space->userLocks[lockIndex].userLock->Acquire(); // acquire userlock at lockIndex
	// currentThread->space->locksLock->Release();
}

void Release_sys(int lockIndex) {
	PacketHeader pktHdr;
	MailHeader mailHdr;

    sendToServer(pktHdr, mailHdr, "L R ", "", lockIndex, -1);

	string receivedString = getFromServer(pktHdr, mailHdr);

    cout << "Release::receivedString: " << receivedString << endl;

	// currentThread->space->locksLock->Release();//release kernel lock
	// currentThread->space->userLocks[lockIndex].userLock->Release(); // release userlock at index
	// // destroys lock if lock is free and delete flag is true
	// if(currentThread->space->userLocks[lockIndex].userLock->lockStatus == currentThread->space->userLocks[index].userLock->FREE && currentThread->space->userLocks[index].deleteFlag == TRUE) {
	// 	DEBUG('l'," Lock  number %d  and name %s is destroyed by %s \n", lockIndex, currentThread->space->userLocks[index].userLock->getName(), currentThread->getName());
	// 	currentThread->space->userLocks[lockIndex].isDeleted = TRUE;
	// 	delete currentThread->space->userLocks[lockIndex].userLock;
	// }
}

void DestroyLock_sys(int lockIndex) {
	PacketHeader pktHdr;
	MailHeader mailHdr;

    sendToServer(pktHdr, mailHdr, "L D ", "", lockIndex, -1);

	string receivedString = getFromServer(pktHdr, mailHdr);

    cout << "DestroyLock::receivedString: " << receivedString << endl;
}

// ++++++++++++++++++++ MONITORS ++++++++++++++++++++++++

int CreateMonitor_sys(int vaddr, int size, int appendNum) {
	char* name = new char[size + 1]; //allocate new char array
	name[size] = '\0'; //end the char array with a null character

	if (copyin(vaddr, size, name) == -1){
		DEBUG('l',"%s"," COPYIN FAILED\n");
		delete[] name;
		currentThread->space->locksLock->Release();
		return -1;
	}; //copy contents of the virtual addr (ReadRegister(4)) to the name

	// set attributes of new lock

    PacketHeader pktHdr;
	MailHeader mailHdr;

    sendToServer(pktHdr, mailHdr, "M C ", name, -1, -1);

    string receivedString = getFromServer(pktHdr, mailHdr);
    stringstream ss;
    ss << receivedString;

    int currentLockIndex = -1;
    ss >> currentLockIndex;

    if(currentLockIndex == -1) {
        cout << "Client::currentLockIndex == -1" << endl;
        interrupt->Halt();
    }

	currentThread->space->lockCount += 1;
    cout << "currentLockIndex: " << currentLockIndex << endl;
	//DEBUG('a', "Lock has number %d and name %s\n", currentLockIndex, buffer);
	//DEBUG('l',"    Lock::Lock number: %d || name: %s created by %s\n", currentLockIndex, currentThread->space->userLocks[currentLockIndex].userLock->getName(), currentThread->getName());
	return currentLockIndex;
}

void GetMonitor_sys(int monitorIndex) {
	//DEBUG('a', "Lock  number %d and name %s\n", monitorIndex, currentThread->space->userLocks[index].userLock->getName());
	//DEBUG('l',"    Lock::Lock number: %d || name:  %s acquired by %s\n", monitorIndex, currentThread->space->userLocks[index].userLock->getName(), currentThread->getName());

	PacketHeader pktHdr;
	MailHeader mailHdr;

	sendToServer(pktHdr, mailHdr, "M G ", "", monitorIndex, -1);

	string receivedString = getFromServer(pktHdr, mailHdr);

    cout << "Client::GetMonitor::receivedString: " << receivedString << endl;

	// Lock* userLock = currentThread->space->userLocks[monitorIndex].userLock;
	// if(userLock->lockStatus != userLock->FREE) {
	// 	updateProcessThreadCounts(currentThread->space, SLEEP);
	// }
	// currentThread->space->locksLock->Release();//release kernel lock
	// currentThread->space->userLocks[monitorIndex].userLock->Acquire(); // acquire userlock at index
	// currentThread->space->locksLock->Release();
}

void SetMonitor_sys(int monitorIndex) {
	// DEBUG('l',"    Lock::Lock number: %d || and name: %s released by %s\n", monitorIndex, currentThread->space->userLocks[index].userLock->getName(), currentThread->getName());
	PacketHeader pktHdr;
	MailHeader mailHdr;

    sendToServer(pktHdr, mailHdr, "M S ", "", monitorIndex, -1);

	string receivedString = getFromServer(pktHdr, mailHdr);

    cout << "Client::SetMonitor::receivedString: " << receivedString << endl;

	// currentThread->space->locksLock->Release();//release kernel lock
	// currentThread->space->userLocks[index].userLock->Release(); // release userlock at index
	// // destroys lock if lock is free and delete flag is true
	// if(currentThread->space->userLocks[index].userLock->lockStatus == currentThread->space->userLocks[index].userLock->FREE && currentThread->space->userLocks[index].deleteFlag == TRUE) {
	// 	DEBUG('l'," Lock  number %d  and name %s is destroyed by %s \n", index, currentThread->space->userLocks[index].userLock->getName(), currentThread->getName());
	// 	currentThread->space->userLocks[index].isDeleted = TRUE;
	// 	delete currentThread->space->userLocks[index].userLock;
	// }
}

void DestroyMonitor_sys(int monitorIndex) {
	PacketHeader pktHdr;
	MailHeader mailHdr;

    sendToServer(pktHdr, mailHdr, "M D ", "", monitorIndex, -1);

	string receivedString = getFromServer(pktHdr, mailHdr);

    cout << "Client::DestroyMonitor::receivedString: " << receivedString << endl;
}
