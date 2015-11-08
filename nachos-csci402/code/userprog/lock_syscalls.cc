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

// +++++++++++++++++++++++++ UTILITY +++++++++++++++++++++++++
// generic function that allows us to send any message to the server
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

// generic function that allows us to receive any message to the server
string getFromServer(PacketHeader &pktHdr, MailHeader &mailHdr) {
	char inBuffer[64];
    postOffice->Receive(0, &pktHdr, &mailHdr, inBuffer);
    stringstream ss;
    ss << inBuffer;
    return ss.str();
}

// generic send and recieve paradigm for program
// this function takes the syscallCode, name of entity to be created, and the entity indexes
// if it is not create, pass name as ""
// if entity indexe(s) are needed, pass -1
string sendAndRecieveMessage(char* sysCode, char* name, int entityIndex1, int entityIndex2) {
	PacketHeader pktHdr;
	MailHeader mailHdr;

	sendToServer(pktHdr, mailHdr, sysCode, name, entityIndex1, entityIndex2);

	return getFromServer(pktHdr, mailHdr);
}

// create lock syscall
int CreateLock_sys(int vaddr, int size, int appendNum) {
	char* name = new char[size + 1]; //allocate new char array
	name[size] = '\0'; //end the char array with a null character

	if (copyin(vaddr, size, name) == -1){
		DEBUG('l',"%s"," COPYIN FAILED\n");
		delete[] name;
		currentThread->space->locksLock->Release();
		return -1;
	}; //copy contents of the virtual addr (ReadRegister(4)) to the name

    string receivedString = sendAndRecieveMessage("L C ", name, -1, -1);
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
	return currentLockIndex;
}

// acquire lock syscall
void Acquire_sys(int lockIndex) {
	string receivedString = sendAndRecieveMessage("L A ", "", lockIndex, -1);
    cout << "Acquire::receivedString: " << receivedString << endl;
}

// release lock syscall
void Release_sys(int lockIndex) {
	string receivedString = sendAndRecieveMessage("L R ", "", lockIndex, -1);
    cout << "Release::receivedString: " << receivedString << endl;
}

void DestroyLock_sys(int lockIndex) {
	string receivedString = sendAndRecieveMessage("L D ", "", lockIndex, -1);
    cout << "DestroyLock::receivedString: " << receivedString << endl;
}

// ++++++++++++++++++++ MONITORS ++++++++++++++++++++++++

// create monitor syscall
int CreateMonitor_sys(int vaddr, int size, int appendNum) {
	char* name = new char[size + 1]; //allocate new char array
	name[size] = '\0'; //end the char array with a null character

	if (copyin(vaddr, size, name) == -1){
		DEBUG('l',"%s"," COPYIN FAILED\n");
		delete[] name;
		currentThread->space->locksLock->Release();
		return -1;
	}; //copy contents of the virtual addr (ReadRegister(4)) to the name

    string receivedString = sendAndRecieveMessage("M C ", name, -1, -1);
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
	return currentLockIndex;
}

// get monitor syscall
void GetMonitor_sys(int monitorIndex) {
	string receivedString = sendAndRecieveMessage("M G ", "", monitorIndex, -1);
    cout << "Client::GetMonitor::receivedString: " << receivedString << endl;
}

// set monitor syscall
void SetMonitor_sys(int monitorIndex) {
	string receivedString = sendAndRecieveMessage("M S ", "", monitorIndex, -1);
    cout << "Client::SetMonitor::receivedString: " << receivedString << endl;
}

// destroy monitor syscall
void DestroyMonitor_sys(int monitorIndex) {
	string receivedString = sendAndRecieveMessage("M D ", "", monitorIndex, -1);
    cout << "Client::DestroyMonitor::receivedString: " << receivedString << endl;
}
