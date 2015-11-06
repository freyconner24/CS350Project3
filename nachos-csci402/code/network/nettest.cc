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
#include <sstream>
#include <string>

// Test out message delivery, by doing the following:
//	1. send a message to the machine with ID "farAddr", at mail box #0
//	2. wait for the other machine's message to arrive (in our mailbox #0)
//	3. send an acknowledgment for the other machine's message
//	4. wait for an acknowledgement from the other machine to our
//	    original message

#define MAX_MON_COUNT 50

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

struct ServerThread{
  int machineId;
  int mailboxNum;
};

bool operator==(const ServerThread& t1, const ServerThread& t2) {
    if(t1.machineId != t2.machineId) {
        return false;
    }

    if(t1.mailboxNum != t2.mailboxNum) {
        return false;
    }

    return true;
}

struct ServerLock {
    bool deleteFlag;
    bool isDeleted;

    enum LockStatus {FREE, BUSY};
    LockStatus lockStatus;
    char* name;
    List* waitQueue;
    ServerThread lockOwner;
};

struct ServerMon {
    bool deleteFlag;
    bool isDeleted;
};

struct ServerCond {
    bool deleteFlag;
    bool isDeleted;

    char* name;
    int waitingLockIndex;
    List *waitQueue;
};

ServerLock serverLocks[MAX_MON_COUNT];
ServerMon serverMons[MAX_MON_COUNT];
ServerCond serverConds[MAX_MON_COUNT];

int serverLockCount = 0;
int serverMonCount = 0;
int serverCondCount = 0;

// ++++++++++++++++++++++++++++ Validation ++++++++++++++++++++++++++++

bool validateLockIndex(int lockIndex) {
    if (lockIndex < 0 || lockIndex >= serverLockCount){ // check if index is in valid range
      DEBUG('l',"    Lock::Lock number %d invalid, thread can't acquire-----------------------\n", lockIndex);
      return false;
    }
    if (serverLocks[lockIndex].isDeleted == TRUE){ // check if lock is deleted
  		DEBUG('l',"    Lock::Lock number %d already destroyed, thread can't acquire-----------------------\n", lockIndex);
  		return false;
  	}
    return true;
}

bool validateMonitorIndex(int monitorIndex) {
    if (monitorIndex < 0 || monitorIndex >= serverMonCount){ // check if index is in valid range
      DEBUG('l',"    Mon::Mon number %d invalid\n", monitorIndex);
      return false;
    }
    if (serverMons[monitorIndex].isDeleted == TRUE){ // check if lock is deleted
      DEBUG('l',"    Mon::Mon number %d already destroyed\n", monitorIndex);
      return false;
    }
    return true;
}

bool validateConditionIndex(int conditionIndex) {
    if (conditionIndex < 0 || conditionIndex >= serverCondCount){ // check if index is in valid range
      DEBUG('l',"    Cond::Cond number %d invalid\n", conditionIndex);
      return false;
    }
    if (serverConds[conditionIndex].isDeleted == TRUE){ // check if lock is deleted
      DEBUG('l',"    Cond::Cond number %d already destroyed\n", conditionIndex);
      return false;
    }
    return true;
}

// ++++++++++++++++++++++++++++ Locks ++++++++++++++++++++++++++++

int CreateLock_server(char* name, int appendNum, PacketHeader pktHdr, MailHeader mailHdr) {
    serverLocks[serverLockCount].deleteFlag = FALSE;
    serverLocks[serverLockCount].isDeleted = FALSE;
    serverLocks[serverLockCount].lockStatus = serverLocks[serverLockCount].FREE;
    serverLocks[serverLockCount].name = name;
    serverLocks[serverLockCount].lockOwner.machineId = pktHdr.from;
    serverLocks[serverLockCount].lockOwner.mailboxNum = mailHdr.from;

    int currentLockIndex = serverLockCount;
    ++serverLockCount;

    return currentLockIndex;
}



void Acquire_server(int lockIndex, PacketHeader pktHdr, MailHeader mailHdr) {
    if(!validateLockIndex(lockIndex)) {
        return;
    }

    ServerThread serverCurrentThread;
    serverCurrentThread.machineId = pktHdr.from; // this is essentailly the server machineId
    serverCurrentThread.mailboxNum = mailHdr.from; // this is the mailbox that the mail came from since it's equal to client mailbox

    if(serverCurrentThread == serverLocks[lockIndex].lockOwner) //current thread is lock owner
    {
        return;
    }

    if(serverLocks[lockIndex].lockStatus == serverLocks[lockIndex].FREE) //lock is available
    {
        //I can have the lock
        serverLocks[lockIndex].lockStatus = serverLocks[lockIndex].BUSY; //make state BUSY
        serverLocks[lockIndex].lockOwner = serverCurrentThread; //make myself the owner
    }
    else //lock is busy
    {
        serverLocks[lockIndex].waitQueue->Append(&serverCurrentThread); //Put current thread on the lock’s waitQueue
    }
}

void Release_server(int lockIndex, PacketHeader pktHdr, MailHeader mailHdr) {
    if(!validateLockIndex(lockIndex)) {
        return;
    }

    ServerThread serverCurrentThread;
    serverCurrentThread.machineId = 0;
    serverCurrentThread.mailboxNum = mailHdr.from;

    if(!(serverCurrentThread == serverLocks[lockIndex].lockOwner)) //current thread is not lock owner
    {
        return;
    }

    if(!serverLocks[lockIndex].waitQueue->IsEmpty()) //lock waitQueue is not empty
    {
        ServerThread thread = *(ServerThread*) (serverLocks[lockIndex].waitQueue->Remove()); //remove 1 waiting thread
        serverLocks[lockIndex].lockOwner = thread; //make them lock owner
        char* data = "You got the lock!";
        int clientId = pktHdr.from;
        pktHdr.from = 0;
        pktHdr.to = clientId;
        int clientMailbox = mailHdr.to;
        mailHdr.to = mailHdr.from;
        mailHdr.from = clientMailbox;

        bool success = postOffice->Send(pktHdr, mailHdr, data);

        if ( !success ) {
            printf("Release::The first postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
            interrupt->Halt();
        }

        pktHdr.to = serverLocks[lockIndex].lockOwner.machineId;
        mailHdr.to = serverLocks[lockIndex].lockOwner.mailboxNum;
        success = postOffice->Send(pktHdr, mailHdr, data);

        if ( !success ) {
            printf("Release::The second postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
            interrupt->Halt();
        }
    }
    else
    {
        serverLocks[lockIndex].lockStatus = serverLocks[lockIndex].FREE; //make lock available
        serverLocks[lockIndex].lockOwner.machineId = -1; //unset ownership
        serverLocks[lockIndex].lockOwner.mailboxNum = -1; //unset ownership
    }
}

void DestroyLock_server(int lockIndex) {
    if(!validateLockIndex(lockIndex)) {
        return;
    }
}

// ++++++++++++++++++++++++++++ MVs ++++++++++++++++++++++++++++

int CreateMonitor_server(string name, int appendNum) {
    int currentMonIndex = 0;
    return currentMonIndex;
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
    int currentCondIndex = 0;
    return currentCondIndex;
}

void Wait_server(int lockIndex, int conditionIndex) {
    if(!validateLockIndex(lockIndex)) {
        return;
    }
    if(!validateConditionIndex(conditionIndex)) {
        return;
    }

}

void Signal_server(int lockIndex, int conditionIndex) {
    if(!validateLockIndex(lockIndex)) {
        return;
    }
    if(!validateConditionIndex(conditionIndex)) {
        return;
    }

}

void Broadcast_server(int lockIndex, int conditionIndex) {
    if(!validateLockIndex(lockIndex)) {
        return;
    }
    if(!validateConditionIndex(conditionIndex)) {
        return;
    }

}

void DestroyCondition_server(int conditionIndex) {
    if(!validateConditionIndex(conditionIndex)) {
        return;
    }
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

void Server() {
    cout << "Server()" << endl;
    char sysCode1, sysCode2;

    PacketHeader pktHdr; // Pkt is hardware level // just need to know the machine->Id at command line
    MailHeader mailHdr; // Mail
    char buffer[MaxMailSize];
    stringstream ss;

    char* name;

    while(true) {
        //Recieve the message
        cout << "Recieve()" << endl;
        postOffice->Receive(0, &pktHdr, &mailHdr, buffer);
        cout << "Recieved()" << endl;
        printf("Got \"%s\" from %d, box %d\n",buffer,pktHdr.from,mailHdr.from);
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
                switch(sysCode2) {
                    case 'C':
                        ss.str("");
                        entityId = CreateLock_server(name, serverLockCount, pktHdr, mailHdr);
                        ss << entityId;
                        ss << " CreateLock_server";
                    break;
                    case 'A':
                        // only send reply when they can Acquire
                        Acquire_server(entityIndex1, pktHdr, mailHdr);
                        ss.str("");
                        ss << "Acquire_server";
                    break;
                    case 'R':
                        Release_server(entityIndex1, pktHdr, mailHdr);
                        ss.str("");
                        ss << "Release_server";
                    break;
                    case 'D':
                        DestroyLock_server(entityIndex1);
                        ss.str("");
                        ss << "DestroyLock_server";
                    break;
                }
            break;
            case 'M':
                switch(sysCode2) {
                    case 'C':
                        ss.str("");
                        entityId = CreateMonitor_server(name, serverMonCount);
                        ss << entityId;
                        ss << " CreateMonitor_server";
                    break;
                    case 'G':
                        GetMonitor_server(entityIndex1);
                        ss.str("");
                        ss << "GetMonitor_server";
                    break;
                    case 'S':
                        SetMonitor_server(entityIndex1);
                        ss.str("");
                        ss << "SetMonitor_server";
                    break;
                    case 'D':
                        DestroyMonitor_server(entityIndex1);
                        ss.str("");
                        ss << "DestroyMonitor_server";
                    break;
                }
            break;
            case 'C':
                switch(sysCode2) {
                    case 'C':
                        ss.str("");
                        entityId = CreateCondition_server(name, serverCondCount);
                        ss << entityId;
                        ss << " CreateCondition_server";
                    break;
                    case 'W':
                        ss >> entityIndex2;
                        Wait_server(entityIndex1, entityIndex2); //lock then CV
                        ss.str("");
                        ss << "Wait_server";
                    break;
                    case 'S':
                        ss >> entityIndex2;
                        Signal_server(entityIndex1, entityIndex2); //lock then CV
                        ss.str("");
                        ss << "Signal_server";
                    break;
                    case 'B':
                        ss >> entityIndex2;
                        Broadcast_server(entityIndex1, entityIndex2); //lock then CV
                        ss.str("");
                        ss << "Broadcast_server";
                    break;
                    case 'D':
                        DestroyCondition_server(entityIndex2);
                        ss.str("");
                        ss << "DestroyCondition_server";
                    break;
                }
            break;
        }

        //Process the message
        const char* tempChar = ss.str().c_str();
        char replyBuffer[MaxMailSize];
        for(unsigned int i = 0; i < strlen(tempChar); ++i) {
            replyBuffer[i] = tempChar[i];
        }

        //Send a reply (maybe)
        int clientId = pktHdr.from;
        pktHdr.to = clientId;
        int clientMailbox = mailHdr.to;
        mailHdr.to = mailHdr.from;
        mailHdr.from = clientMailbox;

        bool success = postOffice->Send(pktHdr, mailHdr, replyBuffer);

        if ( !success ) {
            printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
            interrupt->Halt();
        }

    }
}
