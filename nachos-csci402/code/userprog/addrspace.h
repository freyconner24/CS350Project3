// addrspace.h
//  Data structures to keep track of executing user programs
//  (address spaces).
//
//  For now, we don't keep any information about address spaces.
//  The user level CPU state is saved and restored in the thread
//  executing the user program (see thread.h).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "filesys.h"
#include "table.h"

#define UserStackSize       1024    // increase this as necessary!

#define MaxOpenFiles 256
#define MaxChildSpaces 256

#define MAX_THREADS_IN_PROCESS 100
#define MAX_PROCESSES_IN_TABLE 100


struct UserLock;
struct UserCond;
class ProcessEntry;
enum DiskLocation {SWAP, EXECUTABLE, NEITHER}; // 0 - SWAP, 1 - EXECUTABLE, 2 - NEITHER
                                                //An ENUM to indicate the location of the instruction

//An extended translation entry used for the page table, extra fields needed for MMU
class ExtendedTranslationEntry : public TranslationEntry{
  public:
    int byteOffset; //Byteoffset for instruction in the executable or swapfile, stored so we don't need to recalculate it everytime
    DiskLocation diskLocation; //A variable to indicate where the page entry is located on the disk
};

class AddrSpace {
  public:
    AddrSpace(OpenFile *filename);  // Create an address space,
                    // initializing it with the program
                    // stored in the file "executable"
    ~AddrSpace();           // De-allocate an address space

    void InitRegisters();       // Initialize user-level CPU registers,
                    // before jumping to user code

    void SaveState();           // Save/restore address space-specific
    void RestoreState();        // info on a context switch
    Table fileTable;            // Table of openfiles
    unsigned int getNumPages() { return numPages; }
    int processId;
    int spaceId;
    int NewPageTable();
    void DeleteCurrentThread();
    void PrintPageTable();

    int currentPCReg_space;
    int nextPCReg_space;
    int stackReg_space;
    int threadCount;
    int lockCount;
    int condCount;
    UserLock* userLocks;
    UserCond* userConds;
    Lock* locksLock;
    Lock* condsLock;
    int StackTopForMain;
    ExtendedTranslationEntry  *pageTable;   // Assume linear page table translation
    OpenFile *executable; //A handler for the open file associated with the address space
 private:
    Lock *pageTableLock;
    unsigned int numPages;      // Number of pages in the virtual address space 
    ProcessEntry* processEntry;

};

#endif // ADDRSPACE_H
