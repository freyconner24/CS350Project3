/* condtest.c
 *  Simple program to test the file handling system calls
 */

#include "syscall.h"

int lock1;
int cond1;
int lock2;
int cond2;
int cond3;

int theLockThatDoesntExist;

void t1(){
  PrintString(">>>>>>>>>>>>>", 13);
  PrintString("t1 thread started\n", 18);
  PrintString(">>>>>>>>>>>>>", 13);
  PrintString("t1 trying to acquire lock2\n", 27);
  Acquire(lock2);
  PrintString(">>>>>>>>>>>>>", 13);
  PrintString("t1 acquired lock2\n", 18);
  PrintString(">>>>>>>>>>>>>", 13);
  PrintString("t1 Waiting for lock2\n", 21);
  Wait(lock2, cond2);
  PrintString(">>>>>>>>>>>>>", 13);
  PrintString("t1 Signaled by lock2\n", 21);
  Signal(lock2, cond3);
  Release(lock2);
  Exit(0);
}

void t2(){
  PrintString(">>>>>>>>>>>>>", 13);
  PrintString("t2 thread started\n", 18);
  PrintString(">>>>>>>>>>>>>", 13);
  PrintString("t2 trying to acquire lock2\n", 27);
  Acquire(lock2);
  PrintString(">>>>>>>>>>>>>", 13);
  PrintString("t2 acquired lock1\n", 18);
  PrintString(">>>>>>>>>>>>>", 13);
  PrintString("t2 trying to signal lock2 (t1)\n", 31);
  Signal(lock2, cond2);
  PrintString(">>>>>>>>>>>>>", 13);
  PrintString("t2 signaled lock2 (t1)\n", 23);
  PrintString(">>>>>>>>>>>>>", 13);
  PrintString("t2 releasing lock2\n", 19);
  Release(lock2);
  Exit(0);
}

int condId = 0;
int lockId = 0;

int main() {
  PrintString("++++++++ Starting the condition variable testsuite ++++++++\n", 60);
    PrintString("Creating Condition and Lock with bad string lengths\n", 52);
    cond1 = CreateCondition("Condition1", -1, 0);
    lock1 = CreateLock("Lock1", -1, 0);
    cond2 = CreateCondition("Condition2", 10, 0);
    lock2 = CreateLock("Lock2", 5, 0);

    if(cond1 == -1 && cond1 == -1) {
      PrintString("Success!  Both cond1 and lock1 equal -1\n", 40);
    }
    
  PrintString("Creating valid Condition and Lock\n", 34);
    cond1 = CreateCondition("Condition1", 10, 0);
    lock1 = CreateLock("Lock1", 5, 0);


    PrintString("Wait with bad condition\n", 24);
    Wait(lock1, cond1 + 1);

    PrintString("Wait with bad lock\n", 19);
    Wait(lock1 + 1, cond1);

    PrintString("Signalling with bad condition\n", 30);
    Signal(lock1, cond1 + 1);

    PrintString("Signalling with bad lock\n", 25);
    Signal(lock1 + 1, cond1);

    PrintString("Broadcast with bad condition\n", 29);
    Broadcast(lock1, cond1 + 1);

    PrintString("Broadcast with bad lock\n", 24);
    Broadcast(lock1 + 1, cond1);

    PrintString("Destroying bad condition\n", 25);
    DestroyCondition(cond1 + 1);

    PrintString("Destroying valid condition\n", 27);
    DestroyCondition(cond1);

    PrintString("++++++++++++ starting actual testing conditions ++++++++++++", 60);
    cond3 = CreateCondition("Condition3", 10, 0);
    PrintString(">>>>>>>>>>>>>", 13);
    PrintString("main thread trying to acquire lock2\n", 36);
    Acquire(lock2);
    PrintString(">>>>>>>>>>>>>", 13);
    PrintString("main thread acquired lock2\n", 27);
    PrintString(">>>>>>>>>>>>>", 13);
    PrintString("Forked thread 1\n", 16);
    Fork(t1,0);
    PrintString(">>>>>>>>>>>>>", 13);
    PrintString("Forked thread 2\n", 16);
    Fork(t2,0);
    PrintString(">>>>>>>>>>>>>", 13);
    PrintString("main thread waiting on lock2\n", 29);
    Wait(lock2, cond3);
    PrintString(">>>>>>>>>>>>>", 13);
    PrintString("main thread signaled by t1\n", 27);
    Release(lock2);
    PrintString(">>>>>>>>>>>>>", 13);
    PrintString("main thread released lock2\n", 27);

  /*Write("Testing Locks\n", 14, ConsoleOutput)
  lockNum = CreateLock("nameLock");
  Acquire(lockNum);
  condNum = CreateCondition("someCondition");
  Signal(lockNum, condNum);
  Broadcast(lockNum, condNum);
  Release(lockNum);
  DestroyLock(lockNum);
  Write("Locks complete\n", 15, ConsoleOutput);
  Exec("../test/halt");
  Exec("../test/halt");
  */

  PrintString("-------- Finshing Condition test suite --------\n", 48);
  Exit(0);
}
