// system.h
//	All global variables used in Nachos are defined here.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#ifndef SYSTEM_H
#define SYSTEM_H

#include "copyright.h"
#include "utility.h"
#include "thread.h"
#include "scheduler.h"
#include "interrupt.h"
#include "synchlist.h"
#include "stats.h"
#include "timer.h"
#include "syscall.h"
#include "addrspace.h"

#define MAX_LOCK_COUNT 50
#define MAX_COND_COUNT 50
#define ADDRESS_SPACE_COUNT 500

// Initialization and cleanup routines
extern void Initialize(int argc, char **argv); 	// Initialization,
						// called before anything else
extern void Cleanup();				// Cleanup, called when
						// Nachos is done.

struct UserLock {
	bool deleteFlag;
	Lock* userLock;
	bool isDeleted;
};

struct UserCond {
	bool deleteFlag;
	Condition* userCond;
	bool isDeleted;
};

class ProcessEntry {
	public:
		AddrSpace* space;
		SpaceId spaceId;
		int sleepThreadCount;
		int awakeThreadCount;
		int stackLocations[ADDRESS_SPACE_COUNT];
};

class ProcessTable {
	public:
		ProcessTable() {
			runningProcessCount = 0;
		}
		ProcessEntry* processEntries[ADDRESS_SPACE_COUNT];
		int runningProcessCount;
};

extern Thread *currentThread;			// the thread holding the CPU
extern Thread *threadToBeDestroyed;  		// the thread that just finished
extern Scheduler *scheduler;			// the ready list
extern Interrupt *interrupt;			// interrupt status
extern Statistics *stats;			// performance metrics
extern Timer *timer;				// the hardware alarm clock

extern Lock* kernelLock;
extern int processCount;
extern int totalThreadCount;
extern BitMap* bitmap;
extern ProcessTable* processTable;
extern int threadArgs[500];

#ifdef USER_PROGRAM
#include "machine.h"
extern Machine* machine;	// user program memory and registers
#endif

#ifdef FILESYS_NEEDED 		// FILESYS or FILESYS_STUB
#include "filesys.h"
extern FileSystem  *fileSystem;
#endif

#ifdef FILESYS
#include "synchdisk.h"
extern SynchDisk   *synchDisk;
#endif

#ifdef NETWORK
#include "post.h"
extern PostOffice* postOffice;
#endif

#endif // SYSTEM_H
