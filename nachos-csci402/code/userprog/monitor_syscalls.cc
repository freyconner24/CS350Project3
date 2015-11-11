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


// create monitor syscall
int CreateMonitor_sys(int vaddr, int size, int appendNum) {
	char* name = new char[size + 1];
	name[size] = '\0'; //end the char array with a null character

	if(copyin(vaddr, size, name) == -1) { //copy contents of the virtual addr (ReadRegister(4)) to the buffer
		printf("    CreateMonitor::copyin failed\n");
		delete [] name;
		return -1;
	}

    string receivedString = sendAndRecieveMessage("M C ", name, -1, -1);
    stringstream ss;
    ss << receivedString;

    int currentMonIndex = -1;
    ss >> currentMonIndex;

    if(currentMonIndex == -1) {
        cout << "Client::currentMonIndex == -1" << endl;
        interrupt->Halt();
    }

	//currentThread->space->lockCount += 1;
    cout << "currentMonIndex: " << currentMonIndex << endl;

    cout << "Client::CreateMonitor::receivedString: " << receivedString << endl;
	return currentMonIndex;
}

// monitor wait syscall
void GetMonitor_sys(int lockIndex, int monitorIndex) {
	string receivedString = sendAndRecieveMessage("M G ", "", lockIndex, monitorIndex);
    cout << "Client::GetMonitor::receivedString: " << receivedString << endl;
}

// monitor signal syscall
void SetMonitor_sys(int lockIndex, int monitorIndex) {
	string receivedString = sendAndRecieveMessage("M S ", "", lockIndex, monitorIndex);
    cout << "Client::SetMonitor::receivedString: " << receivedString << endl;
}

// monitor destroy syscall
void DestroyMonitor_sys(int monitorIndex) {
	string receivedString = sendAndRecieveMessage("M D ", "", monitorIndex, -1);
    cout << "DestroyMonitor::receivedString: " << receivedString << endl;

}
