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

int CreateCondition_sys(int vaddr, int size, int appendNum) {
	char* name = new char[size + 1];
	name[size] = '\0'; //end the char array with a null character

	if(copyin(vaddr, size, name) == -1) { //copy contents of the virtual addr (ReadRegister(4)) to the buffer
		printf("    CreateCondition::copyin failed\n");
		delete [] name;
		currentThread->space->condsLock->Release();
		return -1;
	}

	PacketHeader pktHdr;
	MailHeader mailHdr;

    sendToServer(pktHdr, mailHdr, "C C ", name, -1, -1);

    string receivedString = getFromServer(pktHdr, mailHdr);
    stringstream ss;
    ss << receivedString;

    int currentCondIndex = -1;
    ss >> currentCondIndex;

    if(currentCondIndex == -1) {
        cout << "Client::currentCondIndex == -1" << endl;
        interrupt->Halt();
    }

	currentThread->space->lockCount += 1;
    cout << "currentCondIndex: " << currentCondIndex << endl;

    cout << "Client::CreateCondition::receivedString: " << receivedString << endl;
	return currentCondIndex;
}

void Wait_sys(int lockIndex, int conditionIndex) {
	PacketHeader pktHdr;
	MailHeader mailHdr;
	sendToServer(pktHdr, mailHdr, "C W ", "", lockIndex, conditionIndex);

	string receivedString = getFromServer(pktHdr, mailHdr);

    cout << "Client::Wait::receivedString: " << receivedString << endl;
}

void Signal_sys(int lockIndex, int conditionIndex) {
	PacketHeader pktHdr;
	MailHeader mailHdr;

    sendToServer(pktHdr, mailHdr, "C S ", "", lockIndex, conditionIndex);

	string receivedString = getFromServer(pktHdr, mailHdr);

    cout << "Client::Signal::receivedString: " << receivedString << endl;
}

void Broadcast_sys(int lockIndex, int conditionIndex) {
	PacketHeader pktHdr;
	MailHeader mailHdr;

    sendToServer(pktHdr, mailHdr, "C B ", "", lockIndex, conditionIndex);

	string receivedString = getFromServer(pktHdr, mailHdr);

    cout << "Client::Broadcast::receivedString: " << receivedString << endl;
}

void DestroyCondition_sys(int index) {
	PacketHeader pktHdr;
	MailHeader mailHdr;

    sendToServer(pktHdr, mailHdr, "C D ", "", index, -1);

	string receivedString = getFromServer(pktHdr, mailHdr);

    cout << "DestroyLock::receivedString: " << receivedString << endl;

}
