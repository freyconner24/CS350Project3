// nettest.cc 
//	Test out message delivery between two "Nachos" machines,
//	using the Post Office to coordinate delivery.
//
//	Two caveats:
//	  1. Two copies of Nachos must be running, with machine ID's 0 and 1:
//		./nachos -m 0 -o 1 &
//		./nachos -m 1 -o 0 &
//
//	  2. You need an implementation of condition variables,
//	     which is *not* provided as part of the baseline threads 
//	     implementation.  The Post Office won't work without
//	     a correct implementation of condition variables.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "network.h"
#include "post.h"
#include "interrupt.h"
#include "list.h"
#include <sstream>

// Test out message delivery, by doing the following:
//	1. send a message to the machine with ID "farAddr", at mail box #0
//	2. wait for the other machine's message to arrive (in our mailbox #0)
//	3. send an acknowledgment for the other machine's message
//	4. wait for an acknowledgement from the other machine to our 
//	    original message

void
MailTest(int farAddr)
{
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char *data = "Hello there!";
    char *ack = "Got it!";
    char buffer[MaxMailSize];

    // construct packet, mail header for original message
    // To: destination machine, mailbox 0
    // From: our machine, reply to: mailbox 1
    outPktHdr.to = farAddr;		
    outMailHdr.to = 0;
    outMailHdr.from = 1;
    outMailHdr.length = strlen(data) + 1;

    // Send the first message
    bool success = postOffice->Send(outPktHdr, outMailHdr, data);

    if ( !success ) {
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }

    // Wait for the first message from the other machine
    postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer);
    printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
    fflush(stdout);

    // Send acknowledgement to the other machine (using "reply to" mailbox
    // in the message that just arrived
    outPktHdr.to = inPktHdr.from;
    outMailHdr.to = inMailHdr.from;
    outMailHdr.length = strlen(ack) + 1;
    success = postOffice->Send(outPktHdr, outMailHdr, ack); 

    if ( !success ) {
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }

    // Wait for the ack from the other machine to the first message we sent.
    postOffice->Receive(1, &inPktHdr, &inMailHdr, buffer);
    printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
    fflush(stdout);

    // Then we're done!
    interrupt->Halt();
}

// ++++++++++++++++++++++++++++ Declarations ++++++++++++++++++++++++++++

struct ServerLock {
    bool deleteFlag;
    bool isDeleted;

    enum LockStatus {FREE, BUSY};
    LockStatus lockStatus;
    char* name;
    List* waitQueue;
    int machineId;
    int clientId;

};

struct ServerMon {
    bool deleteFlag;
    bool isDeleted;

    int machineId;
    int clientId;
};

struct ServerCond {
    bool deleteFlag;
    bool isDeleted;

    char* name;
    struct ServerLock waitingLock;
    List *waitQueue;
    int machineId;
    int clientId;
};

struct ServerLock serverLocks[MAX_LOCK_COUNT];
struct ServerMon serverMons[MAX_COND_COUNT];
struct ServerCond serverConds[MAX_MON_COUNT];

int serverLockCount = 0;
int serverMonCount = 0;
int serverCondCount = 0;

// ++++++++++++++++++++++++++++ Validation ++++++++++++++++++++++++++++

bool validateLockIndex(int lockIndex) {

}

bool validateMonitorIndex(int monitorIndex) {

}

bool validateConditionIndex(int conditionIndex) {

}

// ++++++++++++++++++++++++++++ Locks ++++++++++++++++++++++++++++

int CreateLock_server(string name, int appendNum) {
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

void Acquire_server(int lockIndex) {
    if(!validateLockIndex(lockIndex)) {
        return;
    }

    if(currentThread == lockOwner) //current thread is lock owner
    {
        return;
    }

    if(serverLocks[lockIndex].lockStatus == FREE) //lock is available
    {
        //I can have the lock
        serverLocks[lockIndex].lockStatus = BUSY; //make state BUSY
        lockOwner = currentThread; //make myself the owner
    }
    else //lock is busy
    {
        serverLocks[lockIndex].waitQueue->Append(currentThread); //Put current thread on the lock’s waitQueue
        currentThread->Sleep(); // this Receive
    }
}

void Release_server(int lockIndex) {
    if(!validateLockIndex(lockIndex)) {
        return;
    }

    if(currentThread != lockOwner) //current thread is not lock owner
    {
        return;
    }

    if(!waitQueue->IsEmpty()) //lock waitQueue is not empty
    {
        Thread* thread = (Thread*)waitQueue->Remove(); //remove 1 waiting thread
        serverLocks[lockIndex].lockOwner = thread; //make them lock owner
        scheduler->ReadyToRun(thread); //this Send
    }
    else
    {
        serverLocks[lockIndex].lockStatus = FREE; //make lock available
        serverLocks[lockIndex].lockOwner = NULL; //unset ownership
    }
}

void DestroyLock_server(int lockIndex) {
    if(!validateLockIndex(lockIndex)) {
        return;
    }
}

// ++++++++++++++++++++++++++++ MVs ++++++++++++++++++++++++++++

int CreateMonitor_server(string name, int appendNum) {

}

void GetMonitor_server(int monitorIndex) {
    if(!validateMonitorIndex(monitorIndex)) {
        return;
    }
}

void SetMonitor_server(int monitorIndex) {
    if(!validateMonitorIndex(monitorIndex)) {
        return;
    }
}

void DestroyMonitor_server(int monitorIndex) {
    if(!validateMonitorIndex(monitorIndex)) {
        return;
    }
}

// ++++++++++++++++++++++++++++ CVs ++++++++++++++++++++++++++++

int CreateCondition_server(string name, int appendNum) {

}

void Wait_server(int lockIndex, int conditionIndex) {
    if(!validateLockIndex(lockIndex)) {
        return;
    }
    if(validateConditionIndex(conditionIndex)) {
        return;
    }

    if(serverLocks[lockIndex] == NULL)
    {
        return;
    }
    if(serverConds[conditionIndex] == NULL)
    {
        //no one waiting
        serverConds[conditionIndex] = serverLocks[lockIndex];
    }
    if(serverConds[conditionIndex] != serverLocks[lockIndex])
    {
        return;
    }
    //OK to wait
    serverConds[conditionIndex].waitQueue->Append(currentThread);//Hung: add myself to Condition Variable waitQueue
    serverLocks[lockIndex].->Release();
    currentThread->Sleep(); //currentThread is put on the waitQueue
    serverLocks[lockIndex].->Acquire();
}

void Signal_server(int lockIndex, int conditionIndex) {
    if(!validateLockIndex(lockIndex)) {
        return;
    }
    if(validateConditionIndex(conditionIndex)) {
        return;
    }

    if(serverConds[conditionIndex].waitQueue->IsEmpty()) //no thread waiting
    {
        return;
    }

    if(serverConds[conditionIndex] != serverLocks[lockIndex])
    {
        return;
    }

    //Wake up one waiting thread
    Thread* thread = (Thread*)waitQueue->Remove();//Remove one thread from waitQueue

    scheduler->ReadyToRun(thread); //Put on readyQueue

    if(serverConds[conditionIndex].waitQueue->IsEmpty()) //waitQueue is empty
    {
        serverConds[conditionIndex] = NULL;
    }
}

void Broadcast_server(int lockIndex, int conditionIndex) {
    if(!validateLockIndex(lockIndex)) {
        return;
    }
    if(validateConditionIndex(conditionIndex)) {
        return;
    }

    if(serverLocks[lockIndex] == NULL)
    {
        return;
    }

    if(serverLocks[lockIndex] != serverConds[conditionIndex].waitingLock)
    {
        return;
    }

    while(!serverConds[conditionIndex].waitQueue->IsEmpty()) //waitQueue is not empty
    {
        Signal(serverLocks[lockIndex]);
    }
}

void DestroyCondition_server(int conditionIndex) {
    if(validateConditionIndex(conditionIndex)) {
        return;
    }
}

// int CreateLock_sys(int vaddr, int size, int appendNum); LC
// void Acquire_sys(int lockIndex); LA
// void Release_sys(int lockIndex); LR
// void DestroyLock_sys(int destroyValue); LD

// int CreateMonitor_sys(int vaddr, int size, int appendNum); MC
// void GetMonitor_sys(int lockIndex); MG
// void SetMonitor_sys(int lockIndex); MS
// void DestroyMonitor_sys(int destroyValue); MD

// int CreateCondition_sys(int vaddr, int size, int appendNum); CC
// void Wait_sys(int lockIndex, int conditionIndex); CW
// void Signal_sys(int lockIndex, int conditionIndex); CS
// void Broadcast_sys(int lockIndex, int conditionIndex); CB
// void DestroyCondition_sys(int destroyValue); CD

void parseMessage(char* buffer) {

}

// [SysCode1|SysCode2|results|entityId1|entityId2|entityId3]

// CreateLock:       "L C name"
// Acquire:          "L A 32"
// Release:          "L R 2"
// DestroyLock:      "L D 21"

// CreateMonitor:    "M C name"
// GetMonitor:       "M G 32"
// SetMonitor:       "M S 2"
// DestroyMonitor:   "M D 21"

// CreateCondition:  "C C name"
// Wait:             "C W 32 2"
// Signal:           "C S 2 46"
// Broadcast:        "C B 21 36"
// DestroyCondition: "C D 21"

void Server(int farAddr) {
    int sysCode1 = 0, sysCode2 = 1;

    PacketHeader outPktHdr, inPktHdr; // Pkt is hardware level // just need to know the machine->Id at command line
    MailHeader outMailHdr, inMailHdr; // Mail 
    char *data = "Hello there!";
    char *ack = "Got it!";
    char buffer[MaxMailSize];
    stringstream ss;

    outPktHdr.to = farAddr; 
    outPktHdr.to = inPktHdr.from;
    outMailHdr.to = inMailHdr.from;
    //outMailHdr.to = 0; //mailbox 0 TODO: might need
    //outMailHdr.from = 0; //server 0 //ClientId is in Header From field
    outMailHdr.length = strlen(data) + 1;

    // has to have a waitQueue of replies
    // -m 0
    while(true) {
        //Recieve the message
        postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer);
        printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
        fflush(stdout);
        //Parse the message
        int entityId = -1;
        int entityIndex1 = -1;
        int entityIndex2 = -1;
        ss << buffer;

        ss >> sysCode1 >> sysCode2;
        if(sysCode2 == 'C') {
            ss >> name;
        } else {
            ss >> entityIndex1;
        }

        switch(sysCode1) {
            case 'L':
                switch(buffer[sysCode2]) {
                    case 'C':
                        entityId = CreateLock_server(name, serverLockCount);
                    break;
                    case 'A':
                        // only send reply when they can Acquire
                        Acquire_server(entityIndex1);
                    break;
                    case 'R':
                        Release_server(entityIndex1);
                    break;
                    case 'D':
                        DestroyLock_server(entityIndex1);
                    break;
                }
            break;
            case 'M':
                switch(buffer[sysCode2]) {
                    case 'C':
                        entityId = CreateMonitor_server(name, serverMonitorCount);
                    break;
                    case 'G':
                        GetMonitor_server(int entityIndex1);
                    break;
                    case 'S':
                        SetMonitor_server(int entityIndex1); 
                    break;
                    case 'D':
                        DestroyMonitor_server(int entityIndex1);
                    break;
                }
            break;
            case 'C':
                switch(buffer[sysCode2]) {
                    case 'C':
                        entityId = CreateCondition_server(name, serverConditionCount);
                    break;
                    case 'W':
                        ss >> entityIndex2;
                        Wait_server(entityIndex1, entityIndex2); //lock then CV
                    break;
                    case 'S':
                        ss >> entityIndex2;
                        Signal_server(entityIndex1, entityIndex2); //lock then CV
                    break;
                    case 'B':
                        ss >> entityIndex2;
                        Broadcast_server(entityIndex1, entityIndex2); //lock then CV
                    break;
                    case 'D':
                        DestroyCondition_server(entityIndex2);
                    break;
                }
            break;
        }

        //Process the message
        //Send a reply (maybe)
    }
}





