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

// create condition syscall
int CreateCondition_sys(int vaddr, int size, int appendNum) {
	char* name = new char[size + 1];
	name[size] = '\0'; //end the char array with a null character

	if(copyin(vaddr, size, name) == -1) { //copy contents of the virtual addr (ReadRegister(4)) to the buffer
		printf("    CreateCondition::copyin failed\n");
		delete [] name;
		currentThread->space->condsLock->Release();
		return -1;
	}

    string receivedString = sendAndRecieveMessage("C C ", name, -1, -1);
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

// condition wait syscall
void Wait_sys(int lockIndex, int conditionIndex) {
	string receivedString = sendAndRecieveMessage("C W ", "", lockIndex, conditionIndex);
    cout << "Client::Wait::receivedString: " << receivedString << endl;
}

// condition signal syscall
void Signal_sys(int lockIndex, int conditionIndex) {
	string receivedString = sendAndRecieveMessage("C S ", "", lockIndex, conditionIndex);
    cout << "Client::Signal::receivedString: " << receivedString << endl;
}

// condition broadcast syscall
void Broadcast_sys(int lockIndex, int conditionIndex) {
	string receivedString = sendAndRecieveMessage("C B ", "", lockIndex, conditionIndex);
    cout << "Client::Broadcast::receivedString: " << receivedString << endl;
}

// condition destroy syscall
void DestroyCondition_sys(int conditionIndex) {
	string receivedString = sendAndRecieveMessage("C D ", "", conditionIndex, -1);
    cout << "DestroyLock::receivedString: " << receivedString << endl;

}
