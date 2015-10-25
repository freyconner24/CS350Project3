// threadtest.cc
//  Simple test case for the threads assignment.
//
//  Create two threads, and have them context switch
//  back and forth between themselves by calling Thread::Yield,
//  to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include <iostream>
#include <sstream>
#include <string>
#include <stdlib.h>
#include "copyright.h"
#include "system.h"
#ifdef CHANGED
#include "synch.h"
#endif

using namespace std;


void ApplicationClerk(int myLine);
void createClerkThreads(Thread* t);
void clerkFactory();
void clerkSignalsNextCustomer(int myLine);
int chooseCustomerFromLine(int myLine);
void Part2();
void PassportClerk(int myLine);
void PictureClerk(int myLine);
void Cashier(int myLine);
void Customer(int custNumber);
void Senator(int custNumber);
void wakeUpClerks();
void printMoney();
void Manager();
bool customersAreAllDone();

class CustomerAttribute {
public:
    CustomerAttribute() {}
    CustomerAttribute(int ssn) {
        SSN = ssn;
        applicationIsFiled = false;
        likesPicture = false;
        hasCertification = false;
        isDone = false;

        int moneyArray[4] = {100, 600, 1100, 1600};
        int randomIndex = rand() % 4; //0 to 3
        money = moneyArray[randomIndex];
    }
    ~CustomerAttribute() {}

    int SSN;
    bool likesPicture;
    bool applicationIsFiled;
    bool hasCertification;
    bool isDone;
    int money;
    int currentLine;
};

//----------------------------------------------------------------------
// SimpleThread
//  Loop 5 times, yielding the CPU to another ready thread
//  each iteration.
//
//  "which" is simply a number identifying the thread, for debugging
//  purposes.
//----------------------------------------------------------------------

void
SimpleThread(int which)
{
    int num;

    for (num = 0; num < 5; num++) {
    printf("*** thread %d looped %d times\n", which, num);
        currentThread->Yield();
    }
}

//----------------------------------------------------------OUR STUFF-------------------------------------

#ifdef CHANGED
// --------------------------------------------------
// Test Suite
// --------------------------------------------------


// --------------------------------------------------
// Test 1 - see TestSuite() for details
// --------------------------------------------------
Semaphore t1_s1("t1_s1",0);       // To make sure t1_t1 acquires the
                                  // lock before t1_t2
Semaphore t1_s2("t1_s2",0);       // To make sure t1_t2 Is waiting on the
                                  // lock before t1_t3 releases it
Semaphore t1_s3("t1_s3",0);       // To make sure t1_t1 does not release the
                                  // lock before t1_t3 tries to acquire it
Semaphore t1_done("t1_done",0);   // So that TestSuite knows when Test 1 is
                                  // done
Lock t1_l1("t1_l1");          // the lock tested in Test 1

// --------------------------------------------------
// t1_t1() -- test1 thread 1
//     This is the rightful lock owner
// --------------------------------------------------
void t1_t1() {
    t1_l1.Acquire();
    t1_s1.V();  // Allow t1_t2 to try to Acquire Lock

    printf ("%s: Acquired Lock %s, waiting for t3\n",currentThread->getName(),
        t1_l1.getName());
    t1_s3.P();
    printf ("%s: working in CS\n",currentThread->getName());
    for (int i = 0; i < 1000000; i++) ;
    printf ("%s: Releasing Lock %s\n",currentThread->getName(),
        t1_l1.getName());
    t1_l1.Release();
    t1_done.V();
}

// --------------------------------------------------
// t1_t2() -- test1 thread 2
//     This thread will wait on the held lock.
// --------------------------------------------------
void t1_t2() {

    t1_s1.P();  // Wait until t1 has the lock
    t1_s2.V();  // Let t3 try to acquire the lock

    printf("%s: trying to acquire lock %s\n",currentThread->getName(),
        t1_l1.getName());
    t1_l1.Acquire();

    printf ("%s: Acquired Lock %s, working in CS\n",currentThread->getName(),
        t1_l1.getName());
    for (int i = 0; i < 10; i++)
    ;
    printf ("%s: Releasing Lock %s\n",currentThread->getName(),
        t1_l1.getName());
    t1_l1.Release();
    t1_done.V();
}

// --------------------------------------------------
// t1_t3() -- test1 thread 3
//     This thread will try to release the lock illegally
// --------------------------------------------------
void t1_t3() {

    t1_s2.P();  // Wait until t2 is ready to try to acquire the lock

    t1_s3.V();  // Let t1 do it's stuff
    for ( int i = 0; i < 3; i++ ) {
    printf("%s: Trying to release Lock %s\n",currentThread->getName(),
           t1_l1.getName());
    t1_l1.Release();
    }
}

// --------------------------------------------------
// Test 2 - see TestSuite() for details
// --------------------------------------------------
Lock t2_l1("t2_l1");        // For mutual exclusion
Condition t2_c1("t2_c1");   // The condition variable to test
Semaphore t2_s1("t2_s1",0); // To ensure the Signal comes before the wait
Semaphore t2_done("t2_done",0);     // So that TestSuite knows when Test 2 is
                                  // done

// --------------------------------------------------
// t2_t1() -- test 2 thread 1
//     This thread will signal a variable with nothing waiting
// --------------------------------------------------
void t2_t1() {
    t2_l1.Acquire();
    printf("%s: Lock %s acquired, signalling %s\n",currentThread->getName(),
       t2_l1.getName(), t2_c1.getName());
    t2_c1.Signal(&t2_l1);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
       t2_l1.getName());
    t2_l1.Release();
    t2_s1.V();  // release t2_t2
    t2_done.V();
}

// --------------------------------------------------
// t2_t2() -- test 2 thread 2
//     This thread will wait on a pre-signalled variable
// --------------------------------------------------
void t2_t2() {
    t2_s1.P();  // Wait for t2_t1 to be done with the lock
    t2_l1.Acquire();
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
       t2_l1.getName(), t2_c1.getName());
    t2_c1.Wait(&t2_l1);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
       t2_l1.getName());
    t2_l1.Release();
}
// --------------------------------------------------
// Test 3 - see TestSuite() for details
// --------------------------------------------------
Lock t3_l1("t3_l1");        // For mutual exclusion
Condition t3_c1("t3_c1");   // The condition variable to test
Semaphore t3_s1("t3_s1",0); // To ensure the Signal comes before the wait
Semaphore t3_done("t3_done",0); // So that TestSuite knows when Test 3 is
                                // done

// --------------------------------------------------
// t3_waiter()
//     These threads will wait on the t3_c1 condition variable.  Only
//     one t3_waiter will be released
// --------------------------------------------------
void t3_waiter() {
    t3_l1.Acquire();
    t3_s1.V();      // Let the signaller know we're ready to wait
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
       t3_l1.getName(), t3_c1.getName());
    t3_c1.Wait(&t3_l1);
    printf("%s: freed from %s\n",currentThread->getName(), t3_c1.getName());
    t3_l1.Release();
    t3_done.V();
}


// --------------------------------------------------
// t3_signaller()
//     This threads will signal the t3_c1 condition variable.  Only
//     one t3_signaller will be released
// --------------------------------------------------
void t3_signaller() {

    // Don't signal until someone's waiting

    for ( int i = 0; i < 5 ; i++ )
    t3_s1.P();
    t3_l1.Acquire();
    printf("%s: Lock %s acquired, signalling %s\n",currentThread->getName(),
       t3_l1.getName(), t3_c1.getName());
    t3_c1.Signal(&t3_l1);
    printf("%s: Releasing %s\n",currentThread->getName(), t3_l1.getName());
    t3_l1.Release();
    t3_done.V();
}

// --------------------------------------------------
// Test 4 - see TestSuite() for details
// --------------------------------------------------
Lock t4_l1("t4_l1");        // For mutual exclusion
Condition t4_c1("t4_c1");   // The condition variable to test
Semaphore t4_s1("t4_s1",0); // To ensure the Signal comes before the wait
Semaphore t4_done("t4_done",0); // So that TestSuite knows when Test 4 is
                                // done

// --------------------------------------------------
// t4_waiter()
//     These threads will wait on the t4_c1 condition variable.  All
//     t4_waiters will be released
// --------------------------------------------------
void t4_waiter() {
    t4_l1.Acquire();
    t4_s1.V();      // Let the signaller know we're ready to wait
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
       t4_l1.getName(), t4_c1.getName());
    t4_c1.Wait(&t4_l1);
    printf("%s: freed from %s\n",currentThread->getName(), t4_c1.getName());
    t4_l1.Release();
    t4_done.V();
}


// --------------------------------------------------
// t2_signaller()
//     This thread will broadcast to the t4_c1 condition variable.
//     All t4_waiters will be released
// --------------------------------------------------
void t4_signaller() {

    // Don't broadcast until someone's waiting

    for ( int i = 0; i < 5 ; i++ )
    t4_s1.P();
    t4_l1.Acquire();
    printf("%s: Lock %s acquired, broadcasting %s\n",currentThread->getName(),
       t4_l1.getName(), t4_c1.getName());
    t4_c1.Broadcast(&t4_l1);
    printf("%s: Releasing %s\n",currentThread->getName(), t4_l1.getName());
    t4_l1.Release();
    t4_done.V();
}
// --------------------------------------------------
// Test 5 - see TestSuite() for details
// --------------------------------------------------
Lock t5_l1("t5_l1");        // For mutual exclusion
Lock t5_l2("t5_l2");        // Second lock for the bad behavior
Condition t5_c1("t5_c1");   // The condition variable to test
Semaphore t5_s1("t5_s1",0); // To make sure t5_t2 acquires the lock after
                                // t5_t1

// --------------------------------------------------
// t5_t1() -- test 5 thread 1
//     This thread will wait on a condition under t5_l1
// --------------------------------------------------
void t5_t1() {
    t5_l1.Acquire();
    t5_s1.V();  // release t5_t2
    printf("%s: Lock %s acquired, waiting on %s\n",currentThread->getName(),
       t5_l1.getName(), t5_c1.getName());
    t5_c1.Wait(&t5_l1);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
       t5_l1.getName());
    t5_l1.Release();
}

// --------------------------------------------------
// t5_t1() -- test 5 thread 1
//     This thread will wait on a t5_c1 condition under t5_l2, which is
//     a Fatal error
// --------------------------------------------------
void t5_t2() {
    t5_s1.P();  // Wait for t5_t1 to get into the monitor
    t5_l1.Acquire();
    t5_l2.Acquire();
    printf("%s: Lock %s acquired, signalling %s\n",currentThread->getName(),
       t5_l2.getName(), t5_c1.getName());
    t5_c1.Signal(&t5_l2);
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
       t5_l2.getName());
    t5_l2.Release();
    printf("%s: Releasing Lock %s\n",currentThread->getName(),
       t5_l1.getName());
    t5_l1.Release();
}

// --------------------------------------------------
// TestSuite()
//     This is the main thread of the test suite.  It runs the
//     following tests:
//
//       1.  Show that a thread trying to release a lock it does not
//       hold does not work
//
//       2.  Show that Signals are not stored -- a Signal with no
//       thread waiting is ignored
//
//       3.  Show that Signal only wakes 1 thread
//
//   4.  Show that Broadcast wakes all waiting threads
//
//       5.  Show that Signalling a thread waiting under one lock
//       while holding another is a Fatal error
//
//     Fatal errors terminate the thread in question.
// --------------------------------------------------
void TestSuite() {
    Thread *t;
    char *name;
    int i;

    // Test 1
    printf("Starting Test 1\n");

    t = new Thread("t1_t1");
    t->Fork((VoidFunctionPtr)t1_t1,0);

    t = new Thread("t1_t2");
    t->Fork((VoidFunctionPtr)t1_t2,0);

    t = new Thread("t1_t3");
    t->Fork((VoidFunctionPtr)t1_t3,0);

    // Wait for Test 1 to complete
    for (  i = 0; i < 2; i++ )
    t1_done.P();

    // Test 2

    printf("Starting Test 2.  Note that it is an error if thread t2_t2\n");
    printf("completes\n");

    t = new Thread("t2_t1");
    t->Fork((VoidFunctionPtr)t2_t1,0);

    t = new Thread("t2_t2");
    t->Fork((VoidFunctionPtr)t2_t2,0);

    // Wait for Test 2 to complete
    t2_done.P();

    // Test 3

    printf("Starting Test 3\n");

    for (  i = 0 ; i < 5 ; i++ ) {
    name = new char [20];
    sprintf(name,"t3_waiter%d",i);
    t = new Thread(name);
    t->Fork((VoidFunctionPtr)t3_waiter,0);
    }
    t = new Thread("t3_signaller");
    t->Fork((VoidFunctionPtr)t3_signaller,0);

    // Wait for Test 3 to complete
    for (  i = 0; i < 2; i++ ) {
        printf("***************BEFORE****************\n");
        if(t3_l1.getLockOwner() != NULL) {
            printf("This is the current lockOwner: %s\n", t3_l1.getLockOwner()->getName());
        } else {printf("The lock is NULL\n");}

        t3_done.P();
        scheduler->Print();
        printf("^^^^^^^^^^^^^^");
        t3_l1.Print();


        printf("***************AFTER*****************\n");
    }

    // Test 4

    printf("Starting Test 4\n");

    for (  i = 0 ; i < 5 ; i++ ) {
    name = new char [20];
    sprintf(name,"t4_waiter%d",i);
    t = new Thread(name);
    t->Fork((VoidFunctionPtr)t4_waiter,0);
    }
    t = new Thread("t4_signaller");
    t->Fork((VoidFunctionPtr)t4_signaller,0);

    // Wait for Test 4 to complete
    for (  i = 0; i < 6; i++ )
    t4_done.P();

    // Test 5

    printf("Starting Test 5.  Note that it is an error if thread t5_t1\n");
    printf("completes\n");

    t = new Thread("t5_t1");
    t->Fork((VoidFunctionPtr)t5_t1,0);

    t = new Thread("t5_t2");
    t->Fork((VoidFunctionPtr)t5_t2,0);
}

#endif

//----------------------------------------------------------------------
// ThreadTest
//  Set up a ping-pong between two threads, by forking a thread
//  to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

//HUNG: Some constants, since we are writing our own test cases,
//the number of clerks and customers is totally up to us
const int CLERK_NUMBER = 25;
const int CUSTOMER_NUMBER = 50;
const int CLERK_TYPES = 4;
const int clerkCount = 5; //TODO make this NOT const and change to user input with cin >>

Lock* clerkLineLock = new Lock("ClerkLineLock");
int clerkLineCount[CLERK_NUMBER];
int clerkBribeLineCount[CLERK_NUMBER];
enum ClerkState {AVAILABLE, BUSY, ONBREAK};
ClerkState clerkStates[CLERK_NUMBER];
Condition* clerkLineCV[CLERK_NUMBER];
Condition* clerkBribeLineCV[CLERK_NUMBER];
Condition* outsideLineCV = new Condition("OutsideLineCV");
//TODO: finish up break condition variables
Condition* breakCV[CLERK_NUMBER];

int outsideLineCount = 0;
CustomerAttribute customerAttributes[CUSTOMER_NUMBER];
int clerkMoney[CLERK_NUMBER] = {0};
//HUNG: senator control variables
int senatorCount = 0;
Condition* senatorLineCV;
//Second monitor
Lock* clerkLock[CLERK_NUMBER];
Condition* clerkCV[CLERK_NUMBER];
int customerData[CLERK_NUMBER]; //HUNG: Every clerk will use this to get the customer's customerAttribute index
string clerkTypesStatic[CLERK_TYPES] = { "ApplicationClerks : ", "PictureClerks     : ", "PassportClerks    : ", "Cashiers          : " };
string clerkTypes[CLERK_NUMBER]; 
int clerkArray[CLERK_TYPES];

char* cStringDeepCopy(string str) {
    char *cstr = new char[str.length() + 1];
    strcpy(cstr, str.c_str());
    // do stuff
    //delete [] cstr;
    return cstr;
}

void clerkFactory() {
    int tempClerkCount = 0;
    int array[4] = {1,1,1,2};
    for(int i = 0; i < CLERK_TYPES; ++i) {
        cout << clerkTypesStatic[i] << array[i] << endl;
        //cin >> tempClerkCount;
        //clerkArray[i] = tempClerkCount;
        clerkArray[i] = array[i];
    }
}


void createClerkThreads(Thread* t) {
    int clerkNumber = 0;
    for(int i = 0; i < 4; ++i) {
        if(i == 0) {
            for(int j = 0; j < clerkArray[i]; ++j) {
                stringstream sstm;
                sstm << "ApplicationClerk_" << clerkNumber;
                t = new Thread(cStringDeepCopy(sstm.str()));
                t->Fork((VoidFunctionPtr)ApplicationClerk,clerkNumber);
                clerkTypes[clerkNumber] = "ApplicationClerk";
                ++clerkNumber;
            }
        } else if(i == 1) {
            for(int j = 0; j < clerkArray[i]; ++j) {
                stringstream sstm;
                sstm << "PictureClerk_" << clerkNumber;
                t = new Thread(cStringDeepCopy(sstm.str()));
                t->Fork((VoidFunctionPtr)PictureClerk,clerkNumber);
                clerkTypes[clerkNumber] = "PictureClerk";
                ++clerkNumber;
            }
        } else if(i == 2) {
            for(int j = 0; j < clerkArray[i]; ++j) {
                stringstream sstm;
                sstm << "PassportClerk_" << clerkNumber;
                t = new Thread(cStringDeepCopy(sstm.str()));
                t->Fork((VoidFunctionPtr)PassportClerk,clerkNumber);
                clerkTypes[clerkNumber] = "PassportClerk";
                ++clerkNumber;
            }
        } else { // i == 3
            for(int j = 0; j < clerkArray[i]; ++j) {
                stringstream sstm;
                sstm << "Cashier_" << clerkNumber;
                t = new Thread(cStringDeepCopy(sstm.str()));
                t->Fork((VoidFunctionPtr)Cashier,clerkNumber);
                clerkTypes[clerkNumber] = "Cashier";
                ++clerkNumber;
            }
        }
    }
}

//Monitor variables
//First monitor

void createClerkLocksAndConditions() {
    for(int i = 0; i < clerkCount; ++i) {
        stringstream sstm;
        sstm << "ClerkLock_" << i;
        clerkLock[i] = new Lock(cStringDeepCopy(sstm.str()));
        stringstream sstm2;
        sstm2 << "ClerkCV_" << i;
        clerkCV[i] = new Condition(cStringDeepCopy(sstm2.str()));
        stringstream sstm3;
        sstm3 << "ClerkLineCV_" << i;
        clerkLineCV[i] = new Condition(cStringDeepCopy(sstm3.str()));
        stringstream sstm4;
        sstm4 << "ClerkBribeLineCV_" << i;
        clerkBribeLineCV[i] = new Condition(cStringDeepCopy(sstm4.str()));
        stringstream sstm5;
        sstm4 << "ClerkBribeLineCV_" << i;
        breakCV[i] = new Condition(cStringDeepCopy(sstm5.str()));

    }
}

void createCustomerThreads(Thread *t) {
    for(int i = 0 ; i < 10 ; i++){
        stringstream sstm;
        sstm << "Customer_" << i;
        t = new Thread(cStringDeepCopy(sstm.str()));
        t->Fork((VoidFunctionPtr)Customer,i);
    }    
}

void Part2() {
    Thread *t;
    char *name;

    printf("Starting Part 2\n");

    createClerkLocksAndConditions();
    clerkFactory();
    createClerkThreads(t);
    createCustomerThreads(t);
    t->Fork((VoidFunctionPtr)Manager,0); // TODO: start Manager thread
    //t->Fork((VoidFunctionPtr)Senator,i); // TODO: start Senator thread

    printf("Starting Test 1\n");

}

int chooseCustomerFromLine(int myLine) {
    clerkLineLock->Acquire();
    //if(senatorCount > 0) {//HUNG: senator stuff

    //} else
    //do{
    bool testFlag = true;
    while(testFlag) {
        clerkLineLock->Acquire();
        if(clerkBribeLineCount[myLine] > 0) {
            cout << currentThread->getName() << ": I'm taking from bribe line" << endl;
            clerkBribeLineCV[myLine]->Signal(clerkLineLock);
            clerkStates[myLine] = BUSY; //redundant setting
            testFlag = false;
        } else if(clerkLineCount[myLine] > 0) {
            cout << currentThread->getName() << ": I'm taking from reg line"  <<  endl;
            clerkLineCV[myLine]->Signal(clerkLineLock);
            clerkStates[myLine] = BUSY; //redundant setting
            testFlag = false;
        } else {
            //TODO: ONBREAK CODE
            // clerkLock[myLine]->Acquire();
            // cout << currentThread->getName() << ": I'm going on break" <<clerkBribeLineCount[myLine] +clerkLineCount[myLine]<< endl;
            // clerkStates[myLine] = ONBREAK;
            
            clerkLineLock->Release();
            // breakCV[myLine]->Wait(clerkLock[myLine]);
            // clerkLineLock->Acquire();
        }
    }
    //}while(clerkStates[myLine] != BUSY); //TODO: must be a better way of doing this
    
    clerkLock[myLine]->Acquire();
    clerkLineLock->Release();
    //wait for customer
    clerkCV[myLine]->Wait(clerkLock[myLine]);
    //Do my job -> customer waiting
    return customerData[myLine]; //HUNG: Customer would have set the customerData[myLine] to be their custNumber
}

void clerkSignalsNextCustomer(int myLine) {
    clerkCV[myLine]->Signal(clerkLock[myLine]);
    clerkCV[myLine]->Wait(clerkLock[myLine]);
    clerkLock[myLine]->Release();
}

void ApplicationClerk(int myLine) {
    while(true) {
        int custNumber = chooseCustomerFromLine(myLine);
        clerkStates[myLine] = BUSY;
        cout << currentThread->getName() << "::starting to process application for Customer_" << custNumber << endl;
        int numYields = rand() % 80 + 20;
        for(int i = 0; i < numYields; ++i) {
            currentThread->Yield();
        }
        customerAttributes[custNumber].applicationIsFiled = true;
        // clerkStates[myLine] = AVAILABLE; //TODO: the manager should do this
        clerkSignalsNextCustomer(myLine);
    }
}

void PictureClerk(int myLine) {
    while(true) {
        int custNumber = chooseCustomerFromLine(myLine);
        clerkStates[myLine] = BUSY;
        cout << currentThread->getName() << "::starting to process picture for Customer_" << custNumber << endl;
        int numYields = rand() % 80 + 20;

        while(!customerAttributes[custNumber].likesPicture) {
            for(int i = 0; i < numYields; ++i) {
                currentThread->Yield();
            }
            //int probability = rand() % 100;
            //if(probability >= 25) {
                customerAttributes[custNumber].likesPicture = true;
            //}
        }
        // clerkStates[myLine] = AVAILABLE;
        clerkSignalsNextCustomer(myLine);
    }
}

void PassportClerk(int myLine) {
    while(true) {
        int custNumber = chooseCustomerFromLine(myLine);
        if(customerAttributes[custNumber].likesPicture && customerAttributes[custNumber].applicationIsFiled) {
            clerkStates[myLine] = BUSY;
            cout << currentThread->getName() << "::starting to process passport for Customer_" << custNumber << endl;
            int numYields = rand() % 80 + 20;
            for(int i = 0; i < numYields; ++i) {
                currentThread->Yield();
            }
            // clerkStates[myLine] = AVAILABLE;
            customerAttributes[custNumber].hasCertification = true;
        } else { //customer gets sent to back of line
            cout << "SENT TO BACK OF LINE &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&" << endl;
            int waitTime = rand() % 900 + 100;
            for(int i = 0; i < waitTime; ++i) {
                currentThread->Yield();
            }
            //TODO send to back of PassportLine

        }
        clerkSignalsNextCustomer(myLine);
    }
}

void Cashier(int myLine) {
    while(true) {
        int custNumber = chooseCustomerFromLine(myLine);
        if(customerAttributes[custNumber].hasCertification) {
            customerAttributes[custNumber].money -= 100;
            clerkMoney[myLine] += 100;
            clerkStates[myLine] = BUSY;
            cout << currentThread->getName() << "::starting to process money for Customer_" << custNumber << endl;
            int numYields = rand() % 80 + 20;
            for(int i = 0; i < numYields; ++i) {
                currentThread->Yield();
            }
            // clerkStates[myLine] = AVAILABLE;
            cout << currentThread->getName() << ": I'm DONE ##############################################################" << endl;
            customerAttributes[custNumber].isDone = true;
        } else { //customer gets sent to back of line
            cout << "SENT TO BACK OF LINE ============================================================" << endl;
            int waitTime = rand() % 900 + 100;
            for(int i = 0; i < waitTime; ++i) {
                currentThread->Yield();
            }
            //TODO send to back of CashierLine
        }
        clerkSignalsNextCustomer(myLine);
    }
}

void Customer(int custNumber) {
    CustomerAttribute myCustAtt = CustomerAttribute(custNumber); //Hung: Creating a CustomerAttribute for each new customer
    customerAttributes[custNumber] = myCustAtt;
    while(!customerAttributes[custNumber].isDone) {
        clerkLineLock->Acquire();
        bool bribe = false; //HUNG: flag to know whether the customer has paid the bribe, can't be arsed to think of a more elegant way of doing this
        int myLine = -1;
        int lineSize = 1000;
        cout << currentThread->getName() << ": I'm choosing a line: ";
        for(int i = 0; i < clerkCount; i++) {
            if(customerAttributes[custNumber].hasCertification && clerkTypes[i] == "Cashier") {
                if(clerkLineCount[i] < lineSize ) { //&& clerkStates[i] != ONBREAK 
                    myLine = i;
                    lineSize = clerkLineCount[i];
                }
            } else if(customerAttributes[custNumber].likesPicture && 
                      customerAttributes[custNumber].applicationIsFiled && 
                      clerkTypes[i] == "PassportClerk") {
                if(clerkLineCount[i] < lineSize) {//&& clerkStates[i] != ONBREAK 
                    myLine = i;
                    lineSize = clerkLineCount[i];
                }
            } else if(!customerAttributes[custNumber].likesPicture && 
                      clerkTypes[i] == "PictureClerk") {
                if(clerkLineCount[i] < lineSize) {// && clerkStates[i] != ONBREAK 
                    myLine = i;
                    lineSize = clerkLineCount[i];
                }
            } else if(!customerAttributes[custNumber].applicationIsFiled && 
                      clerkTypes[i] == "ApplicationClerk") {
                if(clerkLineCount[i] < lineSize) {  //&& clerkStates[i] != ONBREAK 
                    myLine = i;
                    lineSize = clerkLineCount[i];
                }
            }
        }

        cout << myLine << endl;

        if(myLine == -1) {
            cout << "customerAttributes[custNumber].applicationIsFiled" << customerAttributes[custNumber].applicationIsFiled << endl;
        }

        // if(myLine == -1) {
        //     outsideLineCount++;
        //     outsideLineCV->Wait(clerkLineLock);
        // } else {
            if(clerkStates[myLine] != AVAILABLE ) {//clerkStates[myLine] == BUSY
                //I must wait in line
                cout << currentThread->getName() << ": I'm waiting for the clerk " << clerkTypes[myLine] << endl;
                //HUNG: adding code for line bribing TODO: check if all transactions are complete
                if(customerAttributes[custNumber].money > 100){
                    cout << currentThread->getName() << ": Has money to bribe with" << endl;
                    customerAttributes[custNumber].money -= 500;
                    clerkMoney[myLine] += 500;
                    clerkBribeLineCount[myLine]++;
                    bribe = true;
                    clerkBribeLineCV[myLine]->Wait(clerkLineLock);
                } else {
                    cout << currentThread->getName() << ": I'm poor" << endl;
                    clerkLineCount[myLine]++;
                    clerkLineCV[myLine]->Wait(clerkLineLock);
                }
                if(bribe) {
                    cout << currentThread->getName() << ": I'm bribing" << endl;
                    clerkLineCount[myLine]--;
                } else {
                    cout << currentThread->getName() << ": I'm not bribing" << endl;
                    clerkLineCount[myLine]--;
                }
            } else {
                clerkStates[myLine] = BUSY;
            }
            clerkLineLock->Release();

            clerkLock[myLine]->Acquire();
            //Give my data to my clerk
            customerData[myLine] = custNumber;
            clerkCV[myLine]->Signal(clerkLock[myLine]);
            //wait for clerk to do their job
            clerkCV[myLine]->Wait(clerkLock[myLine]);
           //Read my data
            clerkCV[myLine]->Signal(clerkLock[myLine]);
            clerkLock[myLine]->Release();
        // }
    }
}
/*
void Senator(int custNumber){
    clerkLineLock->Acquire();
    CustomerAttribute myCustAtt = CustomerAttribute(custNumber); //Hung: Creating a CustomerAttribute for each new senator
    customerAttributes[custNumber]=myCustAtt;
    senatorCount++;
    clerkLineLock->Release();
    for(int i = 0; i < CLERK_NUMBER; i++) {
        clerkLock[i]->Acquire();
    }
    //Give my data to my clerk
    //TODO: Need to define appClerk and picClerk
    customerData[appClerk] = custNumber;
    customerData[picClerk] = custNumber;
    //TODO: appClerk or picClerk first
    clerkCV[appClerk]->Signal(clerkLock[appClerk]);
    //wait for clerk to do their job
    clerkCV[appClerk]->Wait(clerkLock[appClerk]);
   //Read my data
    clerkCV[appClerk]->Signal(clerkLock[appClerk]);
    clerkCV[appClerk]->Release();

     clerkCV[appClerk]->Signal(clerkLock[appClerk]);
    //wait for clerk to do their job
    clerkCV[appClerk]->Wait(clerkLock[appClerk]);
   //Read my data
    clerkCV[myLine]->Signal(clerkLock[appClerk]);
    clerkLock[myLine]->Release();


}
*/
void wakeUpClerks() {
    // while(outsideLineCount > 0){
    //     clerkLineLock->Acquire();
    //     outsideLineCV->Broadcast(clerkLineLock);
    // }
    for(int i = 0; i < clerkCount; ++i) {
        if(clerkStates[i] == ONBREAK) {
            clerkStates[i] = AVAILABLE;
            clerkLock[i]->Acquire();
            breakCV[i]->Signal(clerkLock[i]);
            clerkLock[i]->Release();
        }
    }
    

}

void printMoney() {
    int totalMoney = 0;
    for(int i = 0; i < clerkCount; ++i) {
        totalMoney += clerkMoney[i];
        cout << "Money earned by clerk" << i << ": " << clerkMoney[i] << endl;
    }

    cout << "Total money earned: " << totalMoney << endl << endl;

}

void Manager() {
     do{
        IntStatus oldLevel = interrupt->SetLevel(IntOff); 
        int totalLineCount = 0;
        for(int i = 0; i < clerkCount; ++i) {
            totalLineCount += clerkLineCount[i] + clerkBribeLineCount[i];
            if(totalLineCount > 0) { // || outsideLineCount > 0
                //wakeUpClerks(); TODO uncomment
                break;
            }
        }
        printMoney();
        (void) interrupt->SetLevel(oldLevel); //restore interrupts

        int waitTime = 100000;
        for(int i = 0; i < waitTime; ++i) {
            currentThread->Yield();
        }
        cout << "++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
    }while(!customersAreAllDone());
}

bool customersAreAllDone() {
    cout << "***********************************************************" << endl;
    for(int i = 0; i < 10; ++i) {
        cout << "Customer " << i << " " << customerAttributes[i].likesPicture << 
                                           customerAttributes[i].applicationIsFiled << 
                                           customerAttributes[i].hasCertification << 
                                           customerAttributes[i].isDone << endl;
        //if(!customerAttributes[i].isDone) {
          //  return false;
        //}
    }
    
    return false;
}

void
ThreadTest()
{
    /*DEBUG('t', "Entering SimpleTest");

    Thread *t = new Thread("forked thread");

    t->Fork(SimpleThread, 1);
    SimpleThread(0);*/
    //TestSuite();
    Part2();
}
