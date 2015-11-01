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
        clerkMessedUp = false;

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
    bool clerkMessedUp;
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
    for (i = 0; i < 2; i++ )
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
        if(t3_l1.getLockOwner() != NULL) {
            printf("This is the current lockOwner: %s\n", t3_l1.getLockOwner()->getName());
        } else {
            printf("The lock is NULL\n");
        }

        t3_done.P();
        scheduler->Print();
        t3_l1.Print();
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
int testChosen = 1; // CL: indicate test number (1-7) or full program (0)
const int CLERK_NUMBER = 20; // CL: default clerknumbers for the full case
const int CUSTOMER_NUMBER = 60; // CL: default customer numbers
const int CLERK_TYPES = 4; // CL: 4 types of clerks so that static info/ string can be extracted by array access later
const int SENATOR_NUMBER = 10; // CL: number of senators
int clerkCount = 0;  // CL: number of total clerks of the 4 types that can be modified
int customerCount = 0; // CL: number of customers that can be modified
int senatorCount = 0; // CL: number of senators that can be modified
int senatorLineCount = 0; // CL: number of senators in a line at any given time
bool senatorDone = false;

// initialize locks and arrays for linecount, clerk, customer, manager, senator information
Lock* clerkLineLock = new Lock("ClerkLineLock");
int clerkLineCount[CLERK_NUMBER]; // CL: number of customers in a clerk's regular line
int clerkBribeLineCount[CLERK_NUMBER]; // CL: number of customers in a clerk's bribe line
enum ClerkState {AVAILABLE, BUSY, ONBREAK}; // CL: enum for clerk's conditions
ClerkState clerkStates[CLERK_NUMBER] = {AVAILABLE}; //state of each clerk is available in the beginning
Condition* clerkLineCV[CLERK_NUMBER];
Condition* clerkBribeLineCV[CLERK_NUMBER];
Condition* clerkSenatorLineCV = new Condition("ClerkSenatorLineCV");
Lock* clerkSenatorCVLock[CLERK_NUMBER];

Condition* outsideLineCV = new Condition("OutsideLineCV");
Lock* outsideLock = new Lock("OutsideLock");
int outsideLineCount = 0; // CL: an outside line for customers to line up if senator is here, or other rare situations

Condition* breakCV[CLERK_NUMBER];
Lock* senatorLock = new Lock("SenatorLock");
// Condition* senatorCV = new Condition("SenatorCV");

CustomerAttribute customerAttributes[CUSTOMER_NUMBER]; // CL: customer attributes, accessed by custNumber
int clerkMoney[CLERK_NUMBER] = {0}; // CL: every clerk has no bribe money in the beginning
//Senator control variables
Condition* senatorLineCV = new Condition("SenatorLineCV");
Condition* clerkSenatorCV[CLERK_NUMBER];
//Second monitor
Lock* clerkLock[CLERK_NUMBER];
Condition* clerkCV[CLERK_NUMBER];
int customerData[CLERK_NUMBER]; //HUNG: Every clerk will use this to get the customer's customerAttribute index
string clerkTypesStatic[CLERK_TYPES] = { "ApplicationClerks : ", "PictureClerks     : ", "PassportClerks    : ", "Cashiers          : " };
string clerkTypes[CLERK_NUMBER];
int clerkArray[CLERK_TYPES];

Lock* breakLock[CLERK_NUMBER];

// CL: parameter: A string to be parsed into a char*
//     return value: char*

char* cStringDeepCopy(string str) {
    char *cstr = new char[str.length() + 1];
    strcpy(cstr, str.c_str());
    return cstr;
}

// CL: parameter: an int array that contains numbers of each clerk type
//     Summary: gets input from user or test, initialize and print out clerk numbers
//     return value: void

void clerkFactory(int countOfEachClerkType[]) {
    int tempClerkCount = 0;
    for(int i = 0; i < CLERK_TYPES; ++i) {
        if(testChosen == 0) { // gets input from user if running full program, does not get input if test
            do {
                cout << clerkTypesStatic[i];
                cin >> tempClerkCount;
                if(tempClerkCount <= 0 || tempClerkCount > 5) {
                    cout << "    The number of clerks must be between 1 and 5 inclusive!" << endl;
                }
            } while(tempClerkCount <= 0 || tempClerkCount > 5);
            clerkCount += tempClerkCount;
            clerkArray[i] = tempClerkCount;
        } else {
            clerkArray[i] = countOfEachClerkType[i];
        }
    }
    // CL: print statements
    cout << "Number of ApplicationClerks = " <<  clerkArray[0] << endl;
    cout << "Number of PictureClerks = " << clerkArray[1] << endl;
    cout << "Number of PassportClerks = " << clerkArray[2] << endl;
    cout << "Number of CashiersClerks = " << clerkArray[3] << endl;
    cout << "Number of Senators = " << senatorCount << endl;
}

// CL: Parameter: Thread*
//     Summary: create and fire off threads with designated names
//     Return value: void

void createClerkThreads(Thread* t) {
    int clerkNumber = 0;
    for(int i = 0; i < 4; ++i) {
        if(i == 0) {
            for(int j = 0; j < clerkArray[i]; ++j) {
                stringstream sstm;
                sstm << "ApplicationClerk_" << clerkNumber;
                char* clerkName = cStringDeepCopy(sstm.str());
                t = new Thread(clerkName);
                t->Fork((VoidFunctionPtr)ApplicationClerk,clerkNumber);
                clerkTypes[clerkNumber] = "ApplicationClerk";
                ++clerkNumber;
            }
        } else if(i == 1) {
            for(int j = 0; j < clerkArray[i]; ++j) {
                stringstream sstm;
                sstm << "PictureClerk_" << clerkNumber;
                char* clerkName = cStringDeepCopy(sstm.str());
                t = new Thread(clerkName);
                t->Fork((VoidFunctionPtr)PictureClerk,clerkNumber);
                clerkTypes[clerkNumber] = "PictureClerk";
                ++clerkNumber;
            }
        } else if(i == 2) {
            for(int j = 0; j < clerkArray[i]; ++j) {
                stringstream sstm;
                sstm << "PassportClerk_" << clerkNumber;
                char* clerkName = cStringDeepCopy(sstm.str());
                t = new Thread(clerkName);
                t->Fork((VoidFunctionPtr)PassportClerk,clerkNumber);
                clerkTypes[clerkNumber] = "PassportClerk";
                ++clerkNumber;
            }
        } else { // i == 3
            for(int j = 0; j < clerkArray[i]; ++j) {
                stringstream sstm;
                sstm << "Cashier_" << clerkNumber;
                char* clerkName = cStringDeepCopy(sstm.str());
                t = new Thread(clerkName);
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
        char* name = cStringDeepCopy(sstm.str());
        clerkLock[i] = new Lock(name);

        sstm.str(string());
        sstm << "ClerkCV_" << i;
        name = cStringDeepCopy(sstm.str());
        clerkCV[i] = new Condition(name);

        sstm.str(string());
        sstm << "ClerkLineCV_" << i;
        name = cStringDeepCopy(sstm.str());
        clerkLineCV[i] = new Condition(name);

        sstm.str(string());
        sstm << "ClerkBribeLineCV_" << i;
        name = cStringDeepCopy(sstm.str());
        clerkBribeLineCV[i] = new Condition(name);

        sstm.str(string());
        sstm << "BreakLock_" << i;
        name = cStringDeepCopy(sstm.str());
        breakLock[i] = new Lock(name);

        sstm.str(string());
        sstm << "BreakCV_" << i;
        name = cStringDeepCopy(sstm.str());
        breakCV[i] = new Condition(name);

        sstm.str(string());
        sstm << "ClerkSenatorCV_" << i;
        name = cStringDeepCopy(sstm.str());
        clerkSenatorCV[i] = new Condition(name);

        sstm.str(string());
        sstm << "ClerkSenatorCV_" << i;
        name = cStringDeepCopy(sstm.str());
        clerkSenatorCVLock[i] = new Lock(name);
    }
}

// CL: Parameter: Thread*
//     Summary: create and fire off customer threads with designated names
//     Return value: void
void createCustomerThreads(Thread *t) {
    for(int i = 0 ; i < customerCount; i++){
        stringstream sstm;
        sstm << "Customer_" << i;
        t = new Thread(cStringDeepCopy(sstm.str()));
        t->Fork((VoidFunctionPtr)Customer,i);
    }
}

// CL: Parameter: Thread*
//     Summary: create and fire off senator threads with designated names
//     Return value: void
void createSenatorThreads(Thread *t){
    for(int i = 0 ; i < senatorCount; i++){
        stringstream sstm;
        sstm << "Senator_" << i + 50;
        t = new Thread(cStringDeepCopy(sstm.str()));
        t->Fork((VoidFunctionPtr)Senator,i + 50);
    }
}

// CL: Parameter: Thread*, int []
//     Summary: group all create thread/ lock/ condition functions together and fire off manager
//     Return value: void
void createTestVariables(Thread* t, int countOfEachClerkType[]) {
    clerkFactory(countOfEachClerkType);
    createClerkLocksAndConditions();
    createClerkThreads(t);
    createCustomerThreads(t);
    createSenatorThreads(t);
    t = new Thread("Manager");
    t->Fork((VoidFunctionPtr)Manager,0);
}

// CL: Parameter: -
//     Summary: the main function for part2 of assignment, includes if else test statements, how to run the tests, and create all variables
//     Return value: void

void Part2() {
    Thread *t;
    char *name;
    int countOfEachClerkType[CLERK_TYPES] = {0,0,0,0};

    printf("Starting Part 2\n");
    cout << "Test to run (put 0 for full program): ";
    cin >> testChosen;

    if(testChosen == 1) {
        printf("Starting Test 1\n"); //Customers always take the shortest line, but no 2 customers ever choose the same shortest line at the same time

        customerCount = 10;
        clerkCount = 6;
        senatorCount = 0;
        countOfEachClerkType[0] = 2; countOfEachClerkType[1] = 2; countOfEachClerkType[2] = 1; countOfEachClerkType[3] = 1;

        createTestVariables(t, countOfEachClerkType);
    } else if(testChosen == 2) {
        printf("Starting Test 2\n"); //Managers only read one from one Clerk's total money received, at a time
        customerCount = 5;
        clerkCount = 4;
        senatorCount = 0;
        countOfEachClerkType[0] = 1; countOfEachClerkType[1] = 1; countOfEachClerkType[2] = 1; countOfEachClerkType[3] = 1;

        createTestVariables(t, countOfEachClerkType);
    } else if(testChosen == 3) {
        printf("Starting Test 3\n"); //Customers do not leave until they are given their passport by the Cashier.
                                     //The Cashier does not start on another customer until they know that the last Customer has left their area
        customerCount = 5;
        clerkCount = 4;
        senatorCount = 0;
        countOfEachClerkType[0] = 1; countOfEachClerkType[1] = 1; countOfEachClerkType[2] = 1; countOfEachClerkType[3] = 1;

        createTestVariables(t, countOfEachClerkType);
    } else if(testChosen == 4) {
        printf("Starting Test 4\n"); //Clerks go on break when they have no one waiting in their line
        customerCount = 5;
        clerkCount = 4;
        senatorCount = 0;
        countOfEachClerkType[0] = 1; countOfEachClerkType[1] = 1; countOfEachClerkType[2] = 1; countOfEachClerkType[3] = 1;

        createTestVariables(t, countOfEachClerkType);
    } else if(testChosen == 5) {
        printf("Starting Test 5\n"); //Managers get Clerks off their break when lines get too long
        customerCount = 7;
        clerkCount = 4;
        senatorCount = 0;
        countOfEachClerkType[0] = 1; countOfEachClerkType[1] = 1; countOfEachClerkType[2] = 1; countOfEachClerkType[3] = 1;

        createTestVariables(t, countOfEachClerkType);
    } else if(testChosen == 6) {
        printf("Starting Test 6\n"); //Total sales never suffers from a race condition
        customerCount = 25;
        clerkCount = 4;
        senatorCount = 0;
        countOfEachClerkType[0] = 1; countOfEachClerkType[1] = 1; countOfEachClerkType[2] = 1; countOfEachClerkType[3] = 1;

        createTestVariables(t, countOfEachClerkType);
    } else if(testChosen == 7) {
        printf("Starting Test 7\n"); //Total sales never suffers from a race condition
        customerCount = 7;
        clerkCount = 4;
        senatorCount = 1;
        countOfEachClerkType[0] = 1; countOfEachClerkType[1] = 1; countOfEachClerkType[2] = 1; countOfEachClerkType[3] = 1;

        createTestVariables(t, countOfEachClerkType);
    } else if(testChosen == 0) {
        do {
            cout << "Number of customers: ";
            cin >> customerCount;
            if(customerCount <= 0 || customerCount > 50) {
                cout << "    The number of customers must be between 1 and 50 inclusive!" << endl;
            }
        } while(customerCount <= 0 || customerCount > 50);

        do {
            cout << "Number of Senators: ";
            cin >> senatorCount;
            if(senatorCount < 0 || senatorCount > 10) {
                cout << "    The number of senators must be between 1 and 10 inclusive!" << endl;
            }
        } while(senatorCount < 0 || senatorCount > 10);

        createTestVariables(t, countOfEachClerkType);
    }
}

// CL: Parameter: int myLine (line number of current clerk)
//     Summary: chooses customer from the line, or decides if clerks go on break
//     Return value: int (the customer number)

int chooseCustomerFromLine(int myLine) {
        bool testFlag = false;
        do {
            testFlag = false;
            if((senatorLineCount > 0  && clerkSenatorCVLock[myLine] != NULL) || (senatorLineCount > 0 && senatorDone)){// && clerkLock[myLine]->getLockOwner() == NULL){
                // CL: chooses senator line first

                clerkSenatorCVLock[myLine]->Acquire();
                clerkSenatorCV[myLine]->Signal(clerkSenatorCVLock[myLine]);
                //Wait for senator here, if they need me
                clerkSenatorCV[myLine]->Wait(clerkSenatorCVLock[myLine]);

                if(senatorLineCount ==0){
                  clerkLineLock->Acquire();
                  clerkStates[myLine] = AVAILABLE;
                  clerkSenatorCVLock[myLine]->Release();
                }else if(senatorLineCount > 0 && senatorDone){
                  clerkStates[myLine] = AVAILABLE;
                  clerkSenatorCVLock[myLine]->Release();

                }else{
                  clerkStates[myLine] = BUSY;
                  testFlag = true;
                }
            }else{
                clerkLineLock->Acquire();
                if(clerkBribeLineCount[myLine] > 0) {
                    clerkBribeLineCV[myLine]->Signal(clerkLineLock);
                    clerkStates[myLine] = BUSY; //redundant setting
                } else if(clerkLineCount[myLine] > 0) {
                    cout << currentThread->getName() << " is servicing a customer from regular line"  <<  endl;
                    clerkLineCV[myLine]->Signal(clerkLineLock);
                    clerkStates[myLine] = BUSY; //redundant setting
                }else{
                    breakLock[myLine]->Acquire();
                    cout << currentThread->getName() << " is going on break" << endl;
                    clerkStates[myLine] = ONBREAK;
                    clerkLineLock->Release();
                    breakCV[myLine]->Wait(breakLock[myLine]);
                    clerkStates[myLine] = AVAILABLE;
                    breakLock[myLine]->Release();
                }
            }

        } while(clerkStates[myLine] != BUSY);

    clerkLock[myLine]->Acquire();
    clerkLineLock->Release();
    //wait for customer
    if(testFlag){
      clerkSenatorCV[myLine]->Signal(clerkSenatorCVLock[myLine]);
      clerkSenatorCVLock[myLine]->Release();
    }
    clerkCV[myLine]->Wait(clerkLock[myLine]);
    //Do my job -> customer waiting
    return customerData[myLine];
}

// CL: Parameter: int myLine (line number of clerk)
//     Summary: completes the final signal wait communication after doing all the logic in choosing customer
//     Return value: void

void clerkSignalsNextCustomer(int myLine) {
    clerkCV[myLine]->Signal(clerkLock[myLine]);
    // If there is a senator, here is where the clerk waits, after senator is done with them
    clerkCV[myLine]->Wait(clerkLock[myLine]);
    clerkLock[myLine]->Release();
}

// CL: Parameter: int myLine (line number of clerk)
//     Summary: logics for application clerk
//     Return value: void

void ApplicationClerk(int myLine) {

    while(true) {
        int custNumber = chooseCustomerFromLine(myLine);
        string personName = "Customer_";
        if(custNumber >= 50) {
            personName = "Senator_";
        }
        clerkStates[myLine] = BUSY;
        cout << currentThread->getName() << " has signalled a " << personName << " to come to their counter. " << "(" << personName << custNumber << ")" << endl;
        currentThread->Yield();
        cout << personName << custNumber << " has given SSN "<< custNumber << " to " << currentThread->getName() << endl;
        currentThread->Yield();

        cout << currentThread->getName() << " has received SSN " << custNumber << " from " << personName << custNumber << endl;
// CL: random time for applicationclerk to process data
        int numYields = rand() % 80 + 20;
        for(int i = 0; i < numYields; ++i) {
            currentThread->Yield();
        }

        customerAttributes[custNumber].applicationIsFiled = true;
        cout << currentThread->getName() << " has recorded a completed application for " << personName << custNumber << endl;

        clerkSignalsNextCustomer(myLine);
    }
}

// CL: Parameter: int myLine (line number of clerk)
//     Summary: logics for picture clerk
//     Return value: void

void PictureClerk(int myLine) {
    while(true) {
        int custNumber = chooseCustomerFromLine(myLine);
        string personName = "Customer_";
        if(custNumber >= 50) {
            personName = "Senator_";
        }
        clerkStates[myLine] = BUSY;
        cout << currentThread->getName() << " has signalled a " << personName << " to come to their counter. " << "(" << personName << custNumber << ")" << endl;
        currentThread->Yield();
        cout << personName << custNumber << " has given SSN " << custNumber << " to " << currentThread->getName() << endl;
        currentThread->Yield();
        cout << currentThread->getName() << " has received SSN " << custNumber << " from " << personName << custNumber << endl;

        int numYields = rand() % 80 + 20;

        while(!customerAttributes[custNumber].likesPicture) {
            cout << currentThread->getName() << " has taken a picture of " << personName << custNumber << endl;

            int probability = rand() % 100;
            if(probability >= 25) {
                customerAttributes[custNumber].likesPicture = true;
                cout << "Customer_" << custNumber << " does like their picture from " << currentThread->getName() << endl;
                currentThread->Yield();
                cout << currentThread->getName() << " has been told that Customer_" << custNumber << " does like their picture" << endl;
                // CL: random time for pictureclerk to process data

                for(int i = 0; i < numYields; ++i) {
                    currentThread->Yield();
                }
            }else{
                cout << personName << custNumber << " does not like their picture from " << currentThread->getName() << endl;
                cout << currentThread->getName() << " has been told that " << personName << custNumber << " does not like their picture" << endl;
            }
        }
        clerkSignalsNextCustomer(myLine);
    }
}

// CL: Parameter: int myLine (line number of clerk)
//     Summary: logics for passport clerk
//     Return value: void

void PassportClerk(int myLine) {
    while(true) {
        int custNumber = chooseCustomerFromLine(myLine);
        string personName = "Customer_";
        if(custNumber >= 50) {
            personName = "Senator_";
        }
        cout << currentThread->getName() << " has signalled a " << personName << " to come to their counter. " << "(" << personName << custNumber << ")" << endl;
        currentThread->Yield();
        cout << personName << custNumber << " has given SSN "<< custNumber << " to " << currentThread->getName() << endl;
        currentThread->Yield();
        cout << currentThread->getName() << " has received SSN " << custNumber << "from " << personName << custNumber << endl;
        if(customerAttributes[custNumber].likesPicture && customerAttributes[custNumber].applicationIsFiled) {
            // CL: only determine that customer has app and pic completed by the boolean
            cout << currentThread->getName() << " has determined that " << personName << custNumber << " has both their application and picture completed" << endl;
            clerkStates[myLine] = BUSY;

            int numYields = rand() % 80 + 20;

            int clerkMessedUp = rand() % 100;
			if(custNumber > 49){
              clerkMessedUp = 100;
            }
            if(clerkMessedUp <= 5) { //Send to back of line
                cout << currentThread->getName() << ": Messed up for " << personName << custNumber<< ". Sending customer to back of line."<<endl;
                customerAttributes[custNumber].clerkMessedUp = true; //TODO: customer uses this to know which back line to go to
            } else {
                cout << currentThread->getName() << " has recorded " << personName << custNumber << " passport documentation" << endl;
                for(int i = 0; i < numYields; ++i) {
                    currentThread->Yield();
                }
                customerAttributes[custNumber].clerkMessedUp = false;
                customerAttributes[custNumber].hasCertification = true;
            }
        } else {
            cout << currentThread->getName() << " has determined that " << personName << custNumber << " does not have both their application and picture completed" << endl;
        }
        clerkSignalsNextCustomer(myLine);
    }
}

// CL: Parameter: int myLine (line number of clerk)
//     Summary: logics for cashier
//     Return value: void

void Cashier(int myLine) {
    while(true) {
        int custNumber = chooseCustomerFromLine(myLine);
        string personName = "Customer_";
        if(custNumber >= 50) {
            personName = "Senator_";
        }
        cout << currentThread->getName() << " has signalled a " << personName << " to come to their counter. " << "(" << personName << custNumber << ")" << endl;
        currentThread->Yield();
        cout << personName << custNumber << " has given SSN "<< custNumber << " to " << currentThread->getName() << endl;
        currentThread->Yield();
        cout << currentThread->getName() << " has received SSN " << custNumber << "from " << personName << custNumber << endl;
        if(customerAttributes[custNumber].hasCertification) {
            cout << currentThread->getName() << " has verified that " << personName << custNumber << "has been certified by a PassportClerk" << endl;
            customerAttributes[custNumber].money -= 100;// CL: cashier takes money from customer
            cout << currentThread->getName() << " has received the $100 from " << personName << custNumber << "after certification" << endl;
            cout << personName << custNumber << " has given " << currentThread->getName() << " $100" << endl;

            clerkMoney[myLine] += 100;
            clerkStates[myLine] = BUSY;
            int numYields = rand() % 80 + 20;
            // CL: yields after processing money
            for(int i = 0; i < numYields; ++i) {
                currentThread->Yield();
            }
            int clerkMessedUp = rand() % 100;
			   if(custNumber > 49){
              clerkMessedUp = 100;
            }
            if(clerkMessedUp <= 5) { //Send to back of line
                cout << currentThread->getName() << ": Messed up for " << personName << custNumber<< ". Sending customer to back of line."<< endl;
                customerAttributes[custNumber].clerkMessedUp = true; //TODO: customer uses this to know which back line to go to
            } else {
                cout << currentThread->getName() << " has provided " << personName << custNumber << "their completed passport" << endl;
                currentThread->Yield();
                cout << currentThread->getName() << " has recorded that " << personName << custNumber << " has been given their completed passport" << endl;
                customerAttributes[custNumber].clerkMessedUp = false;
                customerAttributes[custNumber].isDone = true;
            }
        }
        clerkSignalsNextCustomer(myLine);
    }
}

// CL: Parameter: int custNumber
//     Summary: logics for customer, includes logic to choose lines to go to and to bribe or not
//     Return value: void

void Customer(int custNumber) {
    CustomerAttribute myCustAtt = CustomerAttribute(custNumber); //Hung: Creating a CustomerAttribute for each new customer
    customerAttributes[custNumber] = myCustAtt;
    while(!customerAttributes[custNumber].isDone) {
		if(senatorLineCount >0){
			outsideLock->Acquire();
			outsideLineCV->Wait(outsideLock);
			outsideLock->Release();
        }
        clerkLineLock->Acquire(); // CL: acquire lock so that only this customer can access and get into the lines
        bool bribe = false; //HUNG: flag to know whether the customer has paid the bribe, can't be arsed to think of a more elegant way of doing this
        int myLine = -1;
        int lineSize = 1000;
        bool pickedApplication;
        bool pickedPicture;
        if(!customerAttributes[custNumber].applicationIsFiled && !customerAttributes[custNumber].likesPicture) { // check conditions if application and picture are done
            pickedApplication = (bool) (rand() % 2);
            pickedPicture = !pickedApplication;
        } else {
            pickedApplication = true;
            pickedPicture = true;
        }
        for(int i = 0; i < clerkCount; i++) {
            int totalLineCount = clerkLineCount[i] + clerkBribeLineCount[i];

            // CL: if else pairs for customer to choose clerk based on their attributes
            if(pickedApplication &&
                !customerAttributes[custNumber].applicationIsFiled &&
                !customerAttributes[custNumber].hasCertification &&
                !customerAttributes[custNumber].isDone &&
                clerkTypes[i] == "ApplicationClerk") {

                if(totalLineCount < lineSize ) {
                    myLine = i;
                    lineSize = totalLineCount;
                }
            } else if(pickedPicture &&
                      !customerAttributes[custNumber].likesPicture &&
                      !customerAttributes[custNumber].hasCertification &&
                      !customerAttributes[custNumber].isDone &&
                      clerkTypes[i] == "PictureClerk") {
                if(totalLineCount < lineSize) {
                    myLine = i;
                    lineSize = totalLineCount;
                }
            } else if(customerAttributes[custNumber].applicationIsFiled &&
                      customerAttributes[custNumber].likesPicture &&
                      !customerAttributes[custNumber].hasCertification &&
                      !customerAttributes[custNumber].isDone &&
                      clerkTypes[i] == "PassportClerk") {
                if(totalLineCount < lineSize) {
                    myLine = i;
                    lineSize = totalLineCount;
                }
            } else if(customerAttributes[custNumber].applicationIsFiled &&
                      customerAttributes[custNumber].likesPicture &&
                      customerAttributes[custNumber].hasCertification &&
                      !customerAttributes[custNumber].isDone &&
                      clerkTypes[i] == "Cashier") {
                if(totalLineCount < lineSize) {
                    myLine = i;
                    lineSize = totalLineCount;
                }
            }
        }

        if(clerkStates[myLine] != AVAILABLE ) { //clerkStates[myLine] == BUSY
            //I must wait in line
            if(customerAttributes[custNumber].money > 100){
                cout << currentThread->getName() << " has gotten in bribe line for " << clerkTypes[myLine] << "_" << myLine << endl;
                // CL: takes bribe money
                customerAttributes[custNumber].money -= 500;
                clerkMoney[myLine] += 500;
                clerkBribeLineCount[myLine]++;
                bribe = true;
                clerkBribeLineCV[myLine]->Wait(clerkLineLock);
            } else {
                cout << currentThread->getName() << " has gotten in regular line for " << clerkTypes[myLine] << "_" << myLine << endl;
                clerkLineCount[myLine]++;
                clerkLineCV[myLine]->Wait(clerkLineLock);
            }

            int totalLineCount = 0;
            for(int i = 0; i < clerkCount; ++i) {
                totalLineCount = totalLineCount + clerkBribeLineCount[i] + clerkLineCount[i];
            }

            if(bribe) {
                clerkBribeLineCount[myLine]--;
            } else {
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

        if(customerAttributes[custNumber].clerkMessedUp) {
            cout << "Clerk messed up.  Customer is going to the back of the line." << endl;
            int yieldTime = rand() % 900 + 100;
            for(int i = 0; i < yieldTime; ++i) {
                currentThread->Yield();
            }
            customerAttributes[custNumber].clerkMessedUp = false;
        }
    }
    // CL: CUSTOMER IS DONE! YAY!
    cout << currentThread->getName() << " is leaving the Passport Office." << endl;
}

// CL: Parameter: int custNumber (because we treat senators as customers)
//     Summary: logics for senators, includes logic to choose lines to go to, very similar to customer but a lot more conditions and locks
//     Return value: void

void Senator(int custNumber){
    CustomerAttribute myCustAtt = CustomerAttribute(custNumber); //Hung: custNumber == 50 to 59
    customerAttributes[custNumber] = myCustAtt;
    senatorLock->Acquire();
    if(senatorLineCount > 0){
        senatorLineCount++;
        senatorLineCV->Wait(senatorLock);
        senatorDone = true;
        for(int i =0; i < clerkCount; i++){
            clerkLock[i]->Acquire();
            clerkSenatorCVLock[i]->Acquire();
        }
        for(int i =0; i < clerkCount; i++){
            clerkCV[i]->Signal(clerkLock[i]);
            clerkLock[i]->Release();
        }
        for(int i = 0; i < clerkCount; i++){

            cout << "Waiting for clerk "<< i << endl;
            clerkSenatorCV[i]->Signal(clerkSenatorCVLock[i]);

            clerkSenatorCV[i]->Wait(clerkSenatorCVLock[i]);
            cout << "Getting confirmation from clerk "<< i << endl;
        }
        senatorDone=false;
    }else{
        for(int i = 0; i < clerkCount; i++){
            clerkSenatorCVLock[i]->Acquire();
        }

        senatorLineCount++;
        senatorLock->Release();
        senatorDone = false;

        for(int i = 0; i < clerkCount; i++){
            cout << "Waiting for clerk "<< i << endl;
            clerkSenatorCV[i]->Wait(clerkSenatorCVLock[i]);
            cout << "Getting confirmation from clerk "<< i << endl;
        }
    }

    int myLine = 0;
    while(!customerAttributes[custNumber].isDone) {
        for(int i = 0; i < clerkCount; i++) {
            if(!customerAttributes[custNumber].applicationIsFiled &&
                //customerAttributes[custNumber].likesPicture &&
                !customerAttributes[custNumber].hasCertification &&
                !customerAttributes[custNumber].isDone &&
                clerkTypes[i] == "ApplicationClerk") {
                cout << "    " << currentThread->getName() << "::: ApplicationClerk chosen" << endl;
                myLine = i;
            } else if(//customerAttributes[custNumber].applicationIsFiled &&
                      !customerAttributes[custNumber].likesPicture &&
                      !customerAttributes[custNumber].hasCertification &&
                      !customerAttributes[custNumber].isDone &&
                      clerkTypes[i] == "PictureClerk") {
                cout << "    " << currentThread->getName() << "::: PictureClerk chosen" << endl;
            myLine = i;
            } else if(customerAttributes[custNumber].applicationIsFiled &&
                      customerAttributes[custNumber].likesPicture &&
                      !customerAttributes[custNumber].hasCertification &&
                      !customerAttributes[custNumber].isDone &&
                      clerkTypes[i] == "PassportClerk") {
                cout << "    " << currentThread->getName() << "::: PassportClerk chosen" << endl;
            myLine = i;
            } else if(customerAttributes[custNumber].applicationIsFiled &&
                      customerAttributes[custNumber].likesPicture &&
                      customerAttributes[custNumber].hasCertification &&
                      !customerAttributes[custNumber].isDone &&
                      clerkTypes[i] == "Cashier") {
                cout << "    " << currentThread->getName() << "::: Cashier chosen" << endl;
                myLine = i;
            }
        }
        clerkSenatorCV[myLine]->Signal(clerkSenatorCVLock[myLine]);
        cout << currentThread->getName() << "has gotten in regular line for " << clerkTypes[myLine] << "_" << myLine << endl;
        clerkSenatorCV[myLine]->Wait(clerkSenatorCVLock[myLine]);

        clerkSenatorCVLock[myLine]->Release();
        clerkLock[myLine]->Acquire();
        //Give my data to my clerk
        customerData[myLine] = custNumber;
        clerkCV[myLine]->Signal(clerkLock[myLine]);

        //wait for clerk to do their job
        clerkCV[myLine]->Wait(clerkLock[myLine]);
        //Read my data
    }

    senatorLock->Acquire();
    senatorLineCount--;
    if(senatorLineCount == 0){
        for(int i = 0 ; i < clerkCount ; i++){
            //Free up clerks that worked with me
            clerkCV[i]->Signal(clerkLock[i]);
            //Frees all locks I hold
            clerkLock[i]->Release();
            //Free up clerks that I didn't work with
            clerkSenatorCV[i]->Signal(clerkSenatorCVLock[i]);
            //Frees up all clerkSenatorCVLocks
            clerkSenatorCVLock[i]->Release();
        }
        outsideLock->Acquire();
        outsideLineCV->Broadcast(outsideLock);
        outsideLock->Release();
        senatorLock->Release();
    }else{
        for(int i = 0 ; i < clerkCount ; i++){
            //Free up clerkLocks for other senator
            clerkLock[i]->Release();
            //Frees up clerkSenatorCVLocks for other senator
            clerkSenatorCVLock[i]->Release();
        }
        senatorLineCV->Signal(senatorLock);
        senatorLock->Release();
    }
    cout << currentThread->getName() << " is leaving the Passport Office." << endl;
}

void wakeUpClerks() {
    for(int i = 0; i < clerkCount; ++i) {
        if(clerkStates[i] == ONBREAK) {
            cout << "Manager has woken up a " << clerkTypes[i] << "_" << i << endl;
            breakLock[i]->Acquire();
            breakCV[i]->Signal(breakLock[i]);
            breakLock[i]->Release();
            cout << clerkTypes[i] << "_" << i << " is coming off break" << endl;
        }
    }
}

// CL: Parameter: -
//     Summary: print all the money as manager checks the money earned by each clerk
//     Return value: void

void printMoney() {
    int totalMoney = 0;
    int applicationMoney = 0;
    int pictureMoney = 0;
    int passportMoney = 0;
    int cashierMoney = 0;

    for(int i = 0; i < clerkCount; ++i) {
        if (i < clerkArray[0]){ //ApplicationClerk index
            applicationMoney += clerkMoney[i];
        } else if (i < clerkArray[0] + clerkArray[1]){ //PictureClerk index
            pictureMoney += clerkMoney[i];
        } else if (i < clerkArray[0] + clerkArray[1] + clerkArray[2]){ //PassportClerk index
            passportMoney += clerkMoney[i];
        } else if (i < clerkArray[0] + clerkArray[1] + clerkArray[2] + clerkArray[3]){ //Cashier index
            cashierMoney += clerkMoney[i];
        }
        totalMoney += clerkMoney[i];
    }

    cout << "Manager has counted a total of " << applicationMoney << " for ApplicationClerks" << endl;
    cout << "Manager has counted a total of " << pictureMoney << " for PictureClerks" << endl;
    cout << "Manager has counted a total of " << passportMoney << " for PassportClerks" << endl;
    cout << "Manager has counted a total of " << cashierMoney << " for Cashiers" << endl;
    cout << "Manager has counted a total of " << totalMoney << " for the passport office" << endl << endl;
}

int prevTotalBoolCount = 0;
int currentTotalBoolCount = 0;

// CL: Parameter: -
//     Summary: manager code, interrupts are disabled
//     Return value: void

void Manager() {
     do {
        IntStatus oldLevel = interrupt->SetLevel(IntOff); //disable interrupts
        int totalLineCount = 0;
        for(int i = 0; i < clerkCount; ++i) {
            totalLineCount += clerkLineCount[i] + clerkBribeLineCount[i];
            if(totalLineCount > 2 || senatorLineCount > 0 ) {
                wakeUpClerks();
                break;
            }
        }
        printMoney();
        (void) interrupt->SetLevel(oldLevel); //restore interrupts

        int waitTime = 100000;
        for(int i = 0; i < waitTime; ++i) {
            currentThread->Yield();
        }
    } while(!customersAreAllDone());
}

// CL: Parameter: -
//     Summary: check if customers are done, i.e. are all attributes set to true
//     Return value: void

bool customersAreAllDone() {
    int boolCount = 0;
    for(int i = 0; i < customerCount; ++i) {

        boolCount += customerAttributes[i].isDone;
    }

    for(int i = 0; i < senatorCount; ++i) {
        i += 50;
        boolCount += customerAttributes[i].isDone;
        i -= 50;
    }

    prevTotalBoolCount = currentTotalBoolCount;
    currentTotalBoolCount = 0;
    for(int i = 0; i < customerCount; ++i) {
        currentTotalBoolCount += customerAttributes[i].applicationIsFiled + customerAttributes[i].likesPicture + customerAttributes[i].hasCertification + customerAttributes[i].isDone;
    }
    if(prevTotalBoolCount == currentTotalBoolCount) {
        wakeUpClerks();
    }

    if(boolCount == customerCount + senatorCount) {
        return true;
    }
    return false;
}
