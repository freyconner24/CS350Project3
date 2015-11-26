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
#define MAX_ARR_COUNT 10

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

// abstract concept.  Even thought there are no actual threads, it makes it easier
// to port from the synch.cc file
struct ServerThread{
  int machineId;
  int mailboxNum;
};

string stringArr[10];

// check if ServerThread is "null".  again, this is an abstract concept
// in order to accommodate a direct mapping from the synch.cc file
bool threadIsNull(ServerThread thread) {
    if(thread.machineId != -1) {
        return FALSE;
    }

    if(thread.mailboxNum != -1) {
        return FALSE;
    }

    return TRUE;
}

// operator overload for == on ServerThread
// since we can't compare structs easily
bool operator==(const ServerThread& t1, const ServerThread& t2) {
    if(t1.machineId != t2.machineId) {
        return false;
    }

    if(t1.mailboxNum != t2.mailboxNum) {
        return false;
    }

    return true;
}

// lock implementation for server
struct ServerLock {
    int num;
    bool deleteFlag;
    bool isDeleted;

    enum LockStatus {FREE, BUSY};
    LockStatus lockStatus;
    char* name;
    List* waitQueue;
    int queueSize;
    ServerThread lockOwner;
};

// operator overload for == on ServerLock
// since we can't compare structs easily
bool operator==(const ServerLock& l1, const ServerLock& l2) {

    if(l1.num != l2.num) {
      cout << "NOT EQUAL\n";
        return false;
    }
    return true;
}

// check if ServerLock is "null".  again, this is an abstract concept
// in order to accommodate a direct mapping from the synch.cc file
bool lockIsNull(ServerLock lock) {
    if(lock.deleteFlag != FALSE) {
        return FALSE;
    }

    if(lock.isDeleted != FALSE) {
        return FALSE;
    }

    if(lock.lockStatus != lock.FREE) {
        return FALSE;
    }

    if(lock.waitQueue != NULL) {
        return FALSE;
    }

    if(!threadIsNull(lock.lockOwner)) {
        return FALSE;
    }

    return TRUE;
}

// set ServerLock to "null".  again, this is an abstract concept
// in order to accommodate a direct mapping from the synch.cc file
void setLockToNull(ServerLock& lock) {
    lock.name = "";
    lock.num = -1;
    lock.deleteFlag = FALSE;
    lock.isDeleted = FALSE;

    lock.lockStatus = lock.FREE;
    lock.waitQueue = NULL;
    lock.lockOwner.machineId = -1;
    lock.lockOwner.mailboxNum = -1;
}

// condition implementation on the server
struct ServerCond {
    bool deleteFlag;
    bool isDeleted;

    char* name;
    int waitingLockIndex;
    List* waitQueue;
    int queueSize;
    bool hasWaitingLock;
};

// monitor implementation on the server
struct ServerMon {
    bool deleteFlag;
    bool isDeleted;
    int* values;
    char* name;
};

// a message struct that holds the pktHdr mailHdr and message

struct Msg{
    Msg(PacketHeader p, MailHeader m, char* d){
      pktHdr = p;
      mailHdr = m;
      data = d;
    };
    Msg(){};
    PacketHeader pktHdr;
    MailHeader mailHdr;
    char* data;
};

// arrays of all of the monitor variables
ServerLock serverLocks[MAX_MON_COUNT];
ServerMon serverMons[MAX_MON_COUNT];
ServerCond serverConds[MAX_MON_COUNT];

// server counts of the locks, mons, and conds
int serverLockCount = 0;
int serverMonCount = 0;
int serverCondCount = 0;

// ++++++++++++++++++++++++++++ Validation ++++++++++++++++++++++++++++

// make sure that we were handed a valid lock
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

// make sure that we were handed a valid monitor
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

bool validateArrayIndex(int arrayIndex) {
  // cout << "arrayindex " << arrayIndex << endl;
    if (arrayIndex < 0 || arrayIndex >= MAX_ARR_COUNT){ // check if index is in valid range
      DEBUG('l',"    Mon::Array index %d invalid\n", arrayIndex);
      return false;
    }
    return true;
}
// make sure that we were handed a valid condition
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

// a function to release the lock on the server side without actually sending any messages out to the postOffice
// identical to Release_server aside from messages.
void serverReleaseLock(int lockIndex, PacketHeader &pktHdr, MailHeader &mailHdr){
  cout << "Releasing lock in server Release\n";
  if(!serverLocks[lockIndex].waitQueue->IsEmpty()) //lock waitQueue is not empty
  {
    string* msg;
    int stringIndex;
    stringstream ss;
    msg = (string*) serverLocks[lockIndex].waitQueue->Remove();
    ss << *msg;
    cout << *msg << endl;
    ss >> pktHdr.to;
    ss >> mailHdr.to;
    ss >> mailHdr.from;
    serverLocks[lockIndex].lockOwner.machineId = pktHdr.to; //unset ownership
    serverLocks[lockIndex].lockOwner.mailboxNum = mailHdr.to; //unset ownership
    cout << stringArr[stringIndex];
    cout << stringArr[stringIndex].length();
    mailHdr.length = stringArr[stringIndex].length();
    char buf[stringArr[stringIndex].length() + 1];
    for(unsigned int i = 0; i < stringArr[stringIndex].length(); ++i) {
      buf[i] = stringArr[stringIndex][i];
    }
    buf[stringArr[stringIndex].length()]= '\0';
    postOffice->Send(pktHdr, mailHdr, buf);
  }
  else
  {
      serverLocks[lockIndex].lockStatus = serverLocks[lockIndex].FREE; //make lock available
      serverLocks[lockIndex].lockOwner.machineId = -1; //unset ownership
      serverLocks[lockIndex].lockOwner.mailboxNum = -1; //unset ownership
      cout << "Lock released after \n";
  }
}

// a helper function to place messages into lock waitqueues
// we swap the header to froms and parse the data, encoded into strings and append the message to lock waitqueues
void putMsgLock(PacketHeader &pktHdr, MailHeader &mailHdr, char* data, int lockIndex){
  pktHdr.to = pktHdr.from;
  int temp = mailHdr.to;
  mailHdr.to = mailHdr.from;
  mailHdr.from = temp;
  string *msg = new string();
  stringstream ss;
  ss << pktHdr.to << ' ' << mailHdr.to << ' ' << mailHdr.from << ' ' << 0;
  *msg = ss.str();
  serverLocks[lockIndex].waitQueue->Append(msg); //Put current thread on the lock’s waitQueue
}

// a helper function to place messages into condition waitqueues
// we swap the header to froms and parse the data, encoded into strings and append the message to the queue
void putMsgCond(PacketHeader &pktHdr, MailHeader &mailHdr, char* data, int conditionIndex){
  pktHdr.to = pktHdr.from;
  int temp = mailHdr.to;
  mailHdr.to = mailHdr.from;
  mailHdr.from = temp;
  string *msg = new string();
  stringstream ss;
  ss << pktHdr.to << ' ' << mailHdr.to << ' ' << mailHdr.from << ' ' << 1;
  *msg = ss.str();

  serverConds[conditionIndex].waitQueue->Append(msg); //Put current thread on the lock’s waitQueue
}


// +++++++++++++++++ UTILITY SERVER METHODS +++++++++++++++++

// abstract method to send message to the client from the server
// again, we swap the header info and send the data
void sendMessageToClient(char* data, PacketHeader &pktHdr, MailHeader &mailHdr) {
    pktHdr.to = pktHdr.from;
    int clientMailbox = mailHdr.to;
    mailHdr.to = mailHdr.from;
    mailHdr.from = clientMailbox;
    mailHdr.length = strlen(data) + 1;

    bool success = postOffice->Send(pktHdr, mailHdr, data);

    if ( !success ) {
        printf("Release::The first postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
        interrupt->Halt();
    }
}

// abstract method to send message to the client from the server
// another helper function to encode entity index messages
void sendCreateEntityMessage(stringstream &ss, PacketHeader &pktHdr, MailHeader &mailHdr) {
    const char* tempChar = ss.str().c_str();
    // cout << "tempChar: " << ss.str() << endl;
    char replyBuffer[MaxMailSize];
    for(unsigned int i = 0; i < strlen(tempChar); ++i) {
        replyBuffer[i] = tempChar[i];
    }

    replyBuffer[strlen(tempChar)] = '\0';

    //Send a reply (maybe)
    pktHdr.to = pktHdr.from;
    int clientMailbox = mailHdr.to;
    mailHdr.to = mailHdr.from;
    mailHdr.from = clientMailbox;
    mailHdr.length = strlen(tempChar) + 1;
    bool success = postOffice->Send(pktHdr, mailHdr, replyBuffer);

    if ( !success ) {
        printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
        interrupt->Halt();
    }
}

// ++++++++++++++++++++++++++++ Locks ++++++++++++++++++++++++++++

// create lock server call
int CreateLock_server(char* name, int appendNum, PacketHeader &pktHdr, MailHeader &mailHdr) {
    if (serverLockCount < 0 ||serverLockCount >= MAX_LOCK_COUNT){
      sendMessageToClient("Too many locks!", pktHdr, mailHdr);
      return -1;
    }

    bool deleteFlag;
    bool isDeleted;
    for (int i = 0; i < serverLockCount; ++i){
      deleteFlag = serverLocks[i].deleteFlag;
      isDeleted = serverLocks[i].isDeleted;
      if (strcmp(name, serverLocks[i].name) == 0 && !isDeleted){
        // cout << serverLocks[i].name << " returning 0?\n";
        // cout << "name == serverLocks[].name " << strcmp(name, serverLocks[i].name) << endl;
        return i;
      }
    }

    // initialize all the values
    serverLocks[serverLockCount].deleteFlag = FALSE;
    serverLocks[serverLockCount].isDeleted = FALSE;
    serverLocks[serverLockCount].lockStatus = serverLocks[serverLockCount].FREE;
    serverLocks[serverLockCount].name = new char [strlen(name) + 1];
    strncpy(serverLocks[serverLockCount].name, name, strlen(name));
    serverLocks[serverLockCount].name[strlen(name)] = '\0';
    serverLocks[serverLockCount].lockOwner.machineId = -1;
    serverLocks[serverLockCount].lockOwner.mailboxNum = -1;

    int currentLockIndex = serverLockCount;
    serverLocks[serverLockCount].num = currentLockIndex;
    ++serverLockCount; //increment count for lock

    //sendMessageToClient("Lock created!", pktHdr, mailHdr);
    return currentLockIndex;
}

// acquire lock server call
void Acquire_server(int lockIndex, PacketHeader &pktHdr, MailHeader &mailHdr) {
    if(!validateLockIndex(lockIndex)) {
        sendMessageToClient("Invalid lock index!", pktHdr, mailHdr);
        return;
    }
    ServerThread serverCurrentThread;
    serverCurrentThread.machineId = pktHdr.from; // this is essentailly the server machineId
    serverCurrentThread.mailboxNum = mailHdr.from; // this is the mailbox that the mail came from since it's equal to client mailbox

    if(serverCurrentThread == serverLocks[lockIndex].lockOwner) //current thread is lock owner
    {
        sendMessageToClient("Lock is yours. Done nothing.", pktHdr, mailHdr);
        return;
    }

    if(serverLocks[lockIndex].lockStatus == serverLocks[lockIndex].FREE) //lock is available
    {
        //I can have the lock
        //cout << "***********************" << endl;
        serverLocks[lockIndex].lockStatus = serverLocks[lockIndex].BUSY; //make state BUSY
        //serverLocks[lockIndex].lockOwner = serverCurrentThread; //make myself the owner
        serverLocks[lockIndex].lockOwner.machineId = pktHdr.from;
        serverLocks[lockIndex].lockOwner.mailboxNum = mailHdr.from;
        sendMessageToClient("You got the lock!", pktHdr, mailHdr); //send the message to the client
        return;
    }
    else //lock is busy
    {
      putMsgLock(pktHdr, mailHdr, "You got the lock!", lockIndex); //put the message on the waitqueue so that it can be sent when someone releases the lock
    }
}

// create release server call
void Release_server(int lockIndex, PacketHeader &pktHdr, MailHeader &mailHdr) {
  ServerThread serverCurrentThread;
  serverCurrentThread.machineId = pktHdr.from; // this is essentailly the server machineId
  serverCurrentThread.mailboxNum = mailHdr.from; // this is the mailbox that the mail came from since it's equal to client mailbox
    if(!validateLockIndex(lockIndex)) { //sanity checks
      sendMessageToClient("Invalid lock index!", pktHdr, mailHdr);
      return;
    }
    if (!(serverCurrentThread == serverLocks[lockIndex].lockOwner)) //current thread is not lock owner
    {
        sendMessageToClient("No permission to release!", pktHdr, mailHdr);
        return;
    }

    pktHdr.to = serverLocks[lockIndex].lockOwner.machineId;
    mailHdr.to = serverLocks[lockIndex].lockOwner.mailboxNum;

    if (serverLocks[lockIndex].deleteFlag == TRUE){
      serverLocks[lockIndex].isDeleted = TRUE;
      delete serverLocks[lockIndex].waitQueue;
      delete serverLocks[lockIndex].name;
      sendMessageToClient("Released, the lock is also deleted.", pktHdr, mailHdr);
      return;
    }
    if(!serverLocks[lockIndex].waitQueue->IsEmpty()) //lock waitQueue is not empty
    {
      // compliated code to get messages from the waitqueue, decode it and send it out
      sendMessageToClient("Released. Another thread took it.", pktHdr, mailHdr);
      string* msg;
      stringstream ss;
      int stringIndex;
      msg = (string*) (serverLocks[lockIndex].waitQueue->Remove());
      --(serverLocks[lockIndex].queueSize);
      ss << *msg;
      ss >> pktHdr.to;
      ss >> mailHdr.to;
      ss >> mailHdr.from;
      ss >> stringIndex;
      serverLocks[lockIndex].lockOwner.machineId = pktHdr.to; //unset ownership
      serverLocks[lockIndex].lockOwner.mailboxNum = mailHdr.to; //unset ownership

      mailHdr.length = stringArr[stringIndex].length()+1;
      char buf[stringArr[stringIndex].length() + 1];
      for(unsigned int i = 0; i < stringArr[stringIndex].length(); ++i) {
        buf[i] = stringArr[stringIndex][i];
      } //copy the char*
      buf[stringArr[stringIndex].length()]= '\0';
      postOffice->Send(pktHdr, mailHdr, buf);
    } else {
        // queue is empty
        serverLocks[lockIndex].lockStatus = serverLocks[lockIndex].FREE; //make lock available
        serverLocks[lockIndex].lockOwner.machineId = -1; //unset ownership
        serverLocks[lockIndex].lockOwner.mailboxNum = -1; //unset ownership
        sendMessageToClient("You released the lock!", pktHdr, mailHdr);
    }
}

// destroy lock server call
void DestroyLock_server(int lockIndex, PacketHeader &pktHdr, MailHeader &mailHdr) {
  ServerThread serverCurrentThread;
  serverCurrentThread.machineId = pktHdr.from; // this is essentailly the server machineId
  serverCurrentThread.mailboxNum = mailHdr.from; // this is the mailbox that the mail came from since it's equal to client mailbox

    if(!validateLockIndex(lockIndex)) {
      sendMessageToClient("Invalid lock index!", pktHdr, mailHdr);
        return;
    }
    if(serverLocks[lockIndex].lockStatus == serverLocks[lockIndex].BUSY) //lock waitQueue is not empty
    {
      serverLocks[lockIndex].deleteFlag = TRUE;
      pktHdr.to = serverLocks[lockIndex].lockOwner.machineId;
      mailHdr.from = mailHdr.to;
      mailHdr.to = serverLocks[lockIndex].lockOwner.mailboxNum;
      sendMessageToClient("Lock is in use will be destroyed later.", pktHdr, mailHdr);
    } else {
        serverLocks[lockIndex].lockStatus = serverLocks[lockIndex].FREE; //make lock available
        serverLocks[lockIndex].lockOwner.machineId = -1; //unset ownership
        serverLocks[lockIndex].lockOwner.mailboxNum = -1; //unset ownership
        serverLocks[lockIndex].isDeleted = TRUE; //unset ownership
        delete serverLocks[lockIndex].waitQueue;
        sendMessageToClient("You destroyed the lock!", pktHdr, mailHdr);
    }
}

// ++++++++++++++++++++++++++++ MVs ++++++++++++++++++++++++++++

// create monitor server call
int CreateMonitor_server(char* name, int appendNum, PacketHeader &pktHdr, MailHeader &mailHdr) {
  if (appendNum <= 0 || appendNum > 50){//sanity checks
    sendMessageToClient("Array invalid! Enter 1-50", pktHdr, mailHdr);
    return -1;
  }
  if (serverMonCount < 0 ||serverMonCount >= MAX_MON_COUNT){
    sendMessageToClient("Too many mons!", pktHdr, mailHdr);
    return -1;
  }
    bool deleteFlag;
    bool isDeleted;
    for (int i = 0; i < serverMonCount; ++i){
      deleteFlag = serverMons[i].deleteFlag;
      isDeleted = serverMons[i].isDeleted;
      if (strcmp(name, serverMons[i].name) == 0 && !isDeleted){
        // cout << serverLocks[i].name << " returning 0?\n";
        // cout << "name == serverLocks[].name " << strcmp(name, serverLocks[i].name) << endl;
        return i;
      }
    }

  serverMons[serverMonCount].deleteFlag = FALSE;
  serverMons[serverMonCount].isDeleted = FALSE;
  serverMons[serverMonCount].name = new char [strlen(name) + 1];
  strncpy(serverMons[serverMonCount].name, name, strlen(name));
  serverMons[serverMonCount].name[strlen(name)] = '\0';
  serverMons[serverMonCount].values = new int [appendNum];

  int currentMonIndex = serverMonCount;
  ++serverMonCount;
  //sendMessageToClient("Monitor created!", pktHdr, mailHdr);

  return currentMonIndex;
}

// get monitor server call
int GetMonitor_server(int monitorIndex, int arrayIndex,PacketHeader &pktHdr, MailHeader &mailHdr) {
    if(!validateMonitorIndex(monitorIndex)) {
      sendMessageToClient("Invalid monitor index!", pktHdr, mailHdr);
        return -1;
    }
    if (!validateArrayIndex(arrayIndex)){
      sendMessageToClient("Invalid array index!", pktHdr, mailHdr);
        return -1;
    }
    // we return the required value for monitor
    char temp;
    stringstream ss;
    ss << serverMons[monitorIndex].values[arrayIndex];
    ss >> temp;
    sendMessageToClient(&temp, pktHdr, mailHdr);
    return serverMons[monitorIndex].values[arrayIndex];
}

// set monitor server call
void SetMonitor_server(int monitorIndex, int arrayIndex, int value,PacketHeader &pktHdr, MailHeader &mailHdr) {
    // set the value and return the message
    if(!validateMonitorIndex(monitorIndex)) {
      sendMessageToClient("Invalid monitor index!", pktHdr, mailHdr);
        return;
    }
    if (!validateArrayIndex(arrayIndex)){
      sendMessageToClient("Invalid array index!", pktHdr, mailHdr);
        return;
    }
    serverMons[monitorIndex].values[arrayIndex] = value;
    sendMessageToClient("Set monitor successfully!", pktHdr, mailHdr);

}

// destroy monitor server call
void DestroyMonitor_server(int monitorIndex, PacketHeader &pktHdr, MailHeader &mailHdr) {
    if(!validateMonitorIndex(monitorIndex)) {
      sendMessageToClient("Invalid monitor index!", pktHdr, mailHdr);
        return;
    }
    serverMons[serverMonCount].isDeleted = TRUE;
    delete serverMons[serverMonCount].values;
    sendMessageToClient("Deleted monitor successfully!", pktHdr, mailHdr);
    return;
}

// ++++++++++++++++++++++++++++ CVs ++++++++++++++++++++++++++++

// create condition server call
int CreateCondition_server(char* name, int appendNum, PacketHeader &pktHdr, MailHeader &mailHdr) {
    if (serverCondCount < 0 ||serverCondCount >= MAX_COND_COUNT){
      sendMessageToClient("Too many conds!", pktHdr, mailHdr);
      return -1;
    }

    bool deleteFlag;
    bool isDeleted;
    for (int i = 0; i < serverCondCount; ++i){
      deleteFlag = serverConds[i].deleteFlag;
      isDeleted = serverConds[i].isDeleted;
      if (strcmp(name, serverConds[i].name) == 0 && !isDeleted){
        // cout << serverLocks[i].name << " returning 0?\n";
        // cout << "name == serverLocks[].name " << strcmp(name, serverLocks[i].name) << endl;
        return i;
      }
    }

    // initialize
    serverConds[serverCondCount].deleteFlag = FALSE;
    serverConds[serverCondCount].isDeleted = FALSE;
    serverConds[serverCondCount].name = new char [strlen(name) + 1];
    strncpy(serverConds[serverCondCount].name, name, strlen(name));
    serverConds[serverCondCount].name[strlen(name)] = '\0';
    serverConds[serverCondCount].waitingLockIndex = -1;
    serverConds[serverCondCount].hasWaitingLock == FALSE;
    int currentCondIndex = serverCondCount;
    ++serverCondCount;
    return currentCondIndex;
}

// wait condition server call
void Wait_server(int lockIndex, int conditionIndex, PacketHeader &pktHdr, MailHeader &mailHdr) {
  ServerThread thread;
  thread.machineId = pktHdr.from;
  thread.mailboxNum = mailHdr.from;
  ServerLock conditionLock = serverLocks[lockIndex];
  ServerLock *waitingLock = &(serverLocks[serverConds[conditionIndex].waitingLockIndex]);
  int tempPktTo =pktHdr.to;
  int tempMailTo =mailHdr.to;
  int tempMailFrom =mailHdr.from;
  if(!validateLockIndex(lockIndex)) { //long sanity checks with both locks and conditions
    sendMessageToClient("Invalid lock index!", pktHdr, mailHdr);
  }else if(!validateConditionIndex(conditionIndex)) {
    sendMessageToClient("Invalid cond index!", pktHdr, mailHdr);
  } else if (!(serverLocks[lockIndex].lockOwner == thread)){
    sendMessageToClient("Lock is not acquired!", pktHdr, mailHdr);
  }else if (serverConds[conditionIndex].deleteFlag){
    sendMessageToClient("Cond will be destroyed, can't wait!", pktHdr, mailHdr);
  }else if(!serverConds[conditionIndex].hasWaitingLock) {
      //no one waiting, set and wait directly
      waitingLock = &conditionLock;
      serverConds[conditionIndex].waitingLockIndex = lockIndex;
      serverConds[conditionIndex].hasWaitingLock = TRUE;
  }else if(!(*waitingLock == conditionLock)){
      sendMessageToClient("No permission to wait!", pktHdr, mailHdr);
      return;
  }
  // add the message so that we could pass it when we get signaled
  putMsgCond(pktHdr, mailHdr, "Finished Waiting!", conditionIndex);
  pktHdr.to = tempPktTo;
  mailHdr.to = tempMailTo;
  mailHdr.from = tempMailFrom;
  // use the saved headers to release the lock
  serverReleaseLock(lockIndex, pktHdr, mailHdr); // not the syscall, the helper function
}

// signal condition server call
void Signal_server(int lockIndex, int conditionIndex, PacketHeader &pktHdr, MailHeader &mailHdr) {
  int tempPktTo = pktHdr.from;
  int tempMailTo = mailHdr.from;
  int tempMailFrom = mailHdr.to;
  if(!validateConditionIndex(conditionIndex)) {
    sendMessageToClient("Invalid cond index!", pktHdr, mailHdr);
  }else if(serverConds[conditionIndex].waitQueue->IsEmpty()) //no thread waiting
  {
    sendMessageToClient("No thread waiting!", pktHdr, mailHdr);
  } else {
    // we remove the message from the queue, decode it and use it to acquire the lock after being signaled
    string* msg;
    stringstream ss;
    msg = (string*) (serverConds[conditionIndex].waitQueue->Remove());//Hung: add myself to Condition Variable waitQueue
    ss << *msg;
    ss >> pktHdr.from;
    ss >> mailHdr.from;
    ss >> mailHdr.to;
    //cout << "here?\n" << pktHdr.from << ' ' << mailHdr.from << ' ' << mailHdr.to << endl;
    Acquire_server(lockIndex, pktHdr, mailHdr);
    pktHdr.from = tempPktTo;
    mailHdr.from = tempMailTo;
    mailHdr.to = tempMailFrom;
    mailHdr.length = 9;
    //cout << "here?\n" << pktHdr.to << ' ' << mailHdr.to << ' ' << mailHdr.from << endl;

    sendMessageToClient("Signalled", pktHdr, mailHdr);
  }
  if(serverConds[conditionIndex].waitQueue->IsEmpty()){
    serverConds[conditionIndex].hasWaitingLock == FALSE; //reset if is empty
    serverConds[conditionIndex].waitingLockIndex == -1;
  }
}

// helper function to signal without postoffice functions
void Signal_without_send(int lockIndex, int conditionIndex, PacketHeader &pktHdr, MailHeader &mailHdr) {
  int tempPktTo = pktHdr.from;
  int tempMailTo = mailHdr.from;
  int tempMailFrom = mailHdr.to;
  if(!validateConditionIndex(conditionIndex)) {
    sendMessageToClient("Invalid cond index!", pktHdr, mailHdr);
  }else if(serverConds[conditionIndex].waitQueue->IsEmpty()) //no thread waiting
  {
    sendMessageToClient("No thread waiting!", pktHdr, mailHdr);
  } else {
    string* msg;
    stringstream ss;
    msg = (string*) (serverConds[conditionIndex].waitQueue->Remove());//Hung: add myself to Condition Variable waitQueue
    ss << *msg;
    ss >> pktHdr.from;
    ss >> mailHdr.from;
    ss >> mailHdr.to;
    //cout << "here?\n" << pktHdr.from << ' ' << mailHdr.from << ' ' << mailHdr.to << endl;
    Acquire_server(lockIndex, pktHdr, mailHdr);
    pktHdr.from = tempPktTo;
    mailHdr.from = tempMailTo;
    mailHdr.to = tempMailFrom;
    mailHdr.length = 9;
    //cout << "here?\n" << pktHdr.to << ' ' << mailHdr.to << ' ' << mailHdr.from << endl;

  }
  if(serverConds[conditionIndex].waitQueue->IsEmpty()){
    serverConds[conditionIndex].hasWaitingLock == FALSE;
    serverConds[conditionIndex].waitingLockIndex == -1;
  }
}


// broadcast condition server call
void Broadcast_server(int lockIndex, int conditionIndex, PacketHeader &pktHdr, MailHeader &mailHdr) {
  ServerThread thread;
  thread.machineId = pktHdr.from;
  thread.mailboxNum = mailHdr.from;
  ServerLock conditionLock = serverLocks[lockIndex];
  ServerLock *waitingLock = &(serverLocks[serverConds[conditionIndex].waitingLockIndex]);
  int tempPktTo = pktHdr.from;
  int tempMailTo = mailHdr.from;
  int tempMailFrom = mailHdr.to;
  char data[MaxMailSize];
  if(!validateLockIndex(lockIndex)) {
    sendMessageToClient("Invalid lock index!", pktHdr, mailHdr);
  }else if(!validateConditionIndex(conditionIndex)) {
    sendMessageToClient("Invalid cond index!", pktHdr, mailHdr);
  }else if(!(*waitingLock == conditionLock)) {
    sendMessageToClient("No permission to broadcast!", pktHdr, mailHdr);
  }else{
    string* msg;
    int stringIndex;
    stringstream ss;
    while(!serverConds[conditionIndex].waitQueue->IsEmpty()) //waitQueue is not empty
    {
      Signal_without_send(lockIndex, conditionIndex, pktHdr, mailHdr);
    }
    serverConds[conditionIndex].hasWaitingLock == FALSE;
    serverConds[conditionIndex].waitingLockIndex = -1;
  }
  pktHdr.from = tempPktTo;
  mailHdr.from = tempMailTo;
  mailHdr.to = tempMailFrom;
  mailHdr.length = 12;
  sendMessageToClient("Broadcasted!", pktHdr, mailHdr);
}

// destroy condition server call
void DestroyCondition_server(int conditionIndex, PacketHeader &pktHdr, MailHeader &mailHdr) {
  if(!validateConditionIndex(conditionIndex)) {
    sendMessageToClient("Invalid cond index!", pktHdr, mailHdr);
      return;
  }
  // can be destroyed
  if (serverConds[conditionIndex].waitQueue->IsEmpty()){
    serverConds[conditionIndex].isDeleted = TRUE;
    delete serverConds[conditionIndex].waitQueue;
    sendMessageToClient("Condition is destroyed.", pktHdr, mailHdr);
  }else {
    serverConds[conditionIndex].deleteFlag = TRUE;
    sendMessageToClient("Cond in use, destroy later.", pktHdr, mailHdr);
  }
}

// +++++++++++++++++ ENCODINGS +++++++++++++++++++

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
// Server polling and sending messages
void Server() {
    cout << "Server()" << endl;
    stringArr[0] = "You got the lock!";
    stringArr[1] = "Finished Waiting!";

    char sysCode1, sysCode2;
    for (int i = 0; i <MAX_MON_COUNT; ++i){
      serverLocks[i].waitQueue = new List();
      serverConds[i].waitQueue = new List();
      serverConds[i].hasWaitingLock = FALSE;
      serverLocks[i].lockStatus = serverLocks[i].FREE;
    }
    PacketHeader pktHdr; // Pkt is hardware level // just need to know the machine->Id at command line
    MailHeader mailHdr; // Mail
    char buffer[MaxMailSize];
    stringstream ss;

    char name[60];

    while(true) {
        //Recieve the message
        ss.str("");
        ss.clear();
        // cout << "Recieve()" << endl;
        postOffice->Receive(0, &pktHdr, &mailHdr, buffer);
        //printf("Got \"%s\" from %d, box %d\n",buffer,pktHdr.from,mailHdr.from);
        fflush(stdout);
        //Parse the message
        int entityId = -1;
        int entityIndex1 = -1;
        int entityIndex2 = -1;
        int entityIndex3 = -1;
        ss << buffer;
        ss >> sysCode1 >> sysCode2;
        if(sysCode2 == 'C') {
            ss >> name;
            cout << name << endl;
        } else {
            ss >> entityIndex1;
        }
        // big switch statement to determine syscalls
        switch(sysCode1) {
            case 'L': // lock server calls
                switch(sysCode2) {
                    case 'C': // create lock
                        cout << "Got to CreateLock_server" << endl;
                        ss.str("");
                        ss.clear();
                        entityId = CreateLock_server(name, serverLockCount, pktHdr, mailHdr);
                        ss << entityId;
                        //cout << "CreateLock_server::entityId: " << entityId << endl;
                        //Process the message
                        sendCreateEntityMessage(ss, pktHdr, mailHdr);
                    break;
                    case 'A': // acquire lock
                        // only send reply when they can Acquire
                        cout << "Got to Acquire_server" << endl;
                        Acquire_server(entityIndex1, pktHdr, mailHdr);
                        ss.str("");
                        ss.clear();
                        ss << "Acquire_server";
                    break;
                    case 'R': // release lock
                        Release_server(entityIndex1, pktHdr, mailHdr);
                        ss.str("");
                        ss.clear();
                        ss << "Release_server";
                    break;
                    case 'D': // destroy lock
                        DestroyLock_server(entityIndex1, pktHdr, mailHdr);
                        ss.str("");
                        ss.clear();
                        ss << "DestroyLock_server";
                    break;
                }
            break;
            case 'M': // monitor server calls
                switch(sysCode2) {
                    case 'C': // create monitor
                        ss >> entityIndex1;
                        ss.str("");
                        ss.clear();
                        entityId = CreateMonitor_server(name, entityIndex1, pktHdr, mailHdr);
                        ss << entityId;
                        //cout << "CreateMonitor_server::entityId: " << entityId << endl;
                        sendCreateEntityMessage(ss, pktHdr, mailHdr);
                    break;
                    case 'G': // get monitor
                        ss >> entityIndex2;
                        ss >> entityIndex3;
                        ss.str("");
                        ss.clear();
                        entityId = GetMonitor_server(entityIndex1, entityIndex2,pktHdr, mailHdr);
                    break;
                    case 'S': // set monitor
                    ss >> entityIndex2;
                    ss >> entityIndex3;
                        SetMonitor_server(entityIndex1, entityIndex2,entityIndex3,pktHdr, mailHdr);
                        ss.str("");
                        ss.clear();
                        ss << "SetMonitor_server";
                    break;
                    case 'D': // destroy monitor
                        DestroyMonitor_server(entityIndex1, pktHdr, mailHdr);
                        ss.str("");
                        ss.clear();
                        ss << "DestroyMonitor_server";
                    break;
                }
            break;
            case 'C': // condition server calls
                switch(sysCode2) {
                    case 'C': // create condition
                        ss.str("");
                        ss.clear();
                        entityId = CreateCondition_server(name, serverCondCount, pktHdr, mailHdr);
                        ss << entityId;
                        //cout << "CreateCondition_server::entityId: " << entityId << endl;
                        sendCreateEntityMessage(ss, pktHdr, mailHdr);
                    break;
                    case 'W': // condition wait
                        ss >> entityIndex2;
                        Wait_server(entityIndex1, entityIndex2, pktHdr, mailHdr); //lock then CV
                        ss.str("");
                        ss.clear();
                        ss << "Wait_server";
                    break;
                    case 'S': // condition signal
                        ss >> entityIndex2;
                        Signal_server(entityIndex1, entityIndex2, pktHdr, mailHdr); //lock then CV

                        ss.str("");
                        ss.clear();
                        ss << "Signal_server";
                    break;
                    case 'B': // create broadcast
                        ss >> entityIndex2;
                        Broadcast_server(entityIndex1, entityIndex2, pktHdr, mailHdr); //lock then CV
                        ss.str("");
                        ss.clear();
                        ss << "Broadcast_server";
                    break;
                    case 'D': // destroy condition
                        DestroyCondition_server(entityIndex1, pktHdr, mailHdr);
                        ss.str("");
                        ss.clear();
                        ss << "DestroyCondition_server";
                    break;
                }
            break;
        }
    }
}
