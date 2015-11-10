// addrspace.cc
//  Routines to manage address spaces (executing user programs).
//
//  In order to run a user program, you must:
//
//  1. link with the -N -T 0 option
//  2. run coff2noff to convert the object file to Nachos format
//    (Nachos object code format is essentially just a simpler
//    version of the UNIX executable object code format)
//  3. load the NOFF file into the Nachos file system
//    (if you haven't implemented the file system yet, you
//    don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#include "noff.h"
#include "table.h"
#include "synch.h"

extern "C" { int bzero(char *, int); };

Table::Table(int s) : map(s), table(0), lock(0), size(s) {
    table = new void *[size];
    lock = new Lock("TableLock");
}

Table::~Table() {
    if (table) {
      delete table;
      table = 0;
    }
    if (lock) {
      delete lock;
      lock = 0;
    }
}

void *Table::Get(int i) {
    // Return the element associated with the given if, or 0 if
    // there is none.

    return (i >=0 && i < size && map.Test(i)) ? table[i] : 0;
}

int Table::Put(void *f) {
    // Put the element in the table and return the slot it used.  Use a
    // lock so 2 files don't get the same space.
    int i;  // to find the next slot

    lock->Acquire();
    i = map.Find();
    lock->Release();
    if ( i != -1)
  table[i] = f;
    return i;
}

void *Table::Remove(int i) {
    // Remove the element associated with identifier i from the table,
    // and return it.

    void *f =0;

    if ( i >= 0 && i < size ) {
  lock->Acquire();
  if ( map.Test(i) ) {
      map.Clear(i);
      f = table[i];
      table[i] = 0;
  }
  lock->Release();
    }
    return f;
}

//----------------------------------------------------------------------
// SwapHeader
//  Do little endian to big endian conversion on the bytes in the
//  object file header, in case the file was generated on a little
//  endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void
SwapHeader (NoffHeader *noffH)
{
  noffH->noffMagic = WordToHost(noffH->noffMagic);
  noffH->code.size = WordToHost(noffH->code.size);
  noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
  noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
  noffH->initData.size = WordToHost(noffH->initData.size);
  noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
  noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
  noffH->uninitData.size = WordToHost(noffH->uninitData.size);
  noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
  noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
//  Create an address space to run a user program.
//  Load the program from a file "executable", and set everything
//  up so that we can start executing user instructions.
//
//  Assumes that the object code file is in NOFF format.
//
//  "executable" is the file containing the object code to load into memory
//
//      It's possible to fail to fully construct the address space for
//      several reasons, including being unable to allocate memory,
//      and being unable to read key parts of the executable.
//      Incompletely consretucted address spaces have the member
//      constructed set to false.
//----------------------------------------------------------------------

AddrSpace::AddrSpace(OpenFile *filename) : fileTable(MaxOpenFiles) {
  pageTableLock = new Lock("PageTableLock");
  pageTableLock->Acquire();
  lockCount = 0;
  condCount = 0;
  condsLock = new Lock ("CondsLock");
  locksLock = new Lock("LocksLock");
  userLocks = new UserLock[MAX_LOCK_COUNT];
  userConds = new UserCond[MAX_COND_COUNT];
    NoffHeader noffH;
    unsigned int i, size;
    threadCount = 1;
    // Don't allocate the input or output to disk files
    fileTable.Put(0);
    fileTable.Put(0);

//Saving exectuable to addrspace
executable = filename;
if (executable == NULL) {
printf("Unable to open file\n");
return; 
}

    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) &&
    (WordToHost(noffH.noffMagic) == NOFFMAGIC))
      SwapHeader(&noffH);
    ASSERT(noffH.noffMagic == NOFFMAGIC);

    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size ;
      numPages = divRoundUp(size, PageSize) + divRoundUp(UserStackSize,PageSize);


    DEBUG('a', "Initializing address space, num pages %d, size %d\n",
          numPages, size);
    int tempIndex = 0;
// Initializing and reading into process' pagetable
    pageTableLock->Acquire();
    pageTable = new ExtendedTranslationEntry[numPages]; 
    processCount++;
    processId = processCount;
    StackTopForMain =  divRoundUp(size, PageSize);
    //Populating page table
    for (i = 0; i < numPages; i++) {
      pageTable[i].virtualPage = i; 
      pageTable[i].valid = FALSE;
      pageTable[i].use = FALSE;
      pageTable[i].dirty = FALSE;
      pageTable[i].readOnly = FALSE;  
      pageTable[i].byteOffset = 40 + pageTable[i].virtualPage * PageSize;
      //If the page is in the code segment or initialized data, it will be read from the executable
      if(i < divRoundUp(noffH.code.size + noffH.initData.size, PageSize) ) {
        pageTable[i].diskLocation = EXECUTABLE;
      }else{
        //Page is not in code or initalized data segment, therefore it's not going to be load from disk
        pageTable[i].diskLocation = NEITHER;
      }


    }

 pageTableLock->Release();
}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
//
//  Dealloate an address space.  release pages, page tables, files
//  and file tables
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
  //Deleteing the pagetable and closing the executable
  delete executable;
  delete pageTable;
  //Checking the IPT and clearing the entries from this process
  for (int i = 0; i < NumPhysPages; i++){
    if(ipt[i].spaceOwner == processId){
      bitmap->Clear(ipt[i].physicalPage);
    }
  }
  // delete [] userLocks;
  // delete [] userConds;
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
//  Set the initial values for the user-level register set.
//
//  We write these directly into the "machine" registers, so
//  that we can immediately jump to user code.  Note that these
//  will be saved/restored into the currentThread->userRegisters
//  when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++)
  machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

   // Set the stack register to the end of the address space, where we
   // allocated the stack; but subtract off a bit, to make sure we don't
   // accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
    DEBUG('a', "Initializing stack register to %x\n", numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
//  On a context switch, save any machine state, specific
//  to this address space, that needs saving.
//
//  For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState()
{
    //Invalidating TLB on context switch
    for(int i = 0; i < TLBSize; ++i) {
        if(machine->tlb[i].valid){
          ipt[machine->tlb[i].physicalPage].dirty = machine->tlb[i].dirty;
        }
        machine->tlb[i].valid = FALSE;
    }

}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
//  On a context switch, restore the machine state so that
//  this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState()
{
    //Invalidating TLB on context switch
    for(int i = 0; i < TLBSize; ++i) {
        if(machine->tlb[i].valid){
          ipt[machine->tlb[i].physicalPage].dirty = machine->tlb[i].dirty;
        }
        machine->tlb[i].valid = FALSE;
    }


}

int AddrSpace::NewPageTable(){
    //Expanding the pagetable, gets called on Fork
    pageTableLock->Acquire();
    ExtendedTranslationEntry* newTable = new ExtendedTranslationEntry [numPages+8];
    //Copying the old pagetable into new pagetable
    for (unsigned int i = 0; i < numPages; i++) {
      newTable[i].virtualPage = pageTable[i].virtualPage; // for now, virtual page # = phys page #
      newTable[i].physicalPage = pageTable[i].physicalPage;
      newTable[i].valid = pageTable[i].valid;
      newTable[i].use = pageTable[i].use;
      newTable[i].dirty = pageTable[i].dirty;
      newTable[i].readOnly = pageTable[i].readOnly;  // if the code segment was entirely on
      newTable[i].byteOffset = pageTable[i].byteOffset;
      newTable[i].diskLocation = pageTable[i].diskLocation;
    }
    //Populating the new stack added to the pagetable
    for (unsigned int i = numPages; i < numPages+8; i++) {

      newTable[i].virtualPage = i;
      newTable[i].physicalPage = -1;
      newTable[i].valid = FALSE;
      newTable[i].use = FALSE;
      newTable[i].dirty = FALSE;
      newTable[i].readOnly = FALSE;  // if the code segment was entirely on
      newTable[i].diskLocation = NEITHER;
      newTable[i].byteOffset = -1;
    }
    
    delete[] pageTable;
    pageTable = newTable;
    numPages = numPages+8;
    //RestoreState();
    int tempNum = numPages - 8 ; 
    pageTableLock->Release();
    return tempNum; 
}

//Removing a thread from a process
void AddrSpace::DeleteCurrentThread(){
  //IntStatus oldLevel = interrupt->SetLevel(IntOff);
  pageTableLock->Acquire();
  --threadCount;
  int stackLocation = processTable->processEntries[processId]->stackLocations[currentThread->id];
  //Clearing stack's entries in the IPT if there are any, and likewise with the TLB
  for (int i = 0; i < UserStackSize / PageSize; ++i){ // UserStackSize / PageSize 's gonna be 8 for ass2
      //Return physical page
    if(pageTable[stackLocation + i].physicalPage != -1){
      bitmap->Clear(pageTable[stackLocation + i].physicalPage);
      pageTable[stackLocation + i].physicalPage = -1;
      for(int j = 0; j < TLBSize; j++){
        if(machine->tlb[j].physicalPage = stackLocation + i){
            machine->tlb[j].valid = FALSE;    
        }
      }
    }

    pageTable[stackLocation + i].valid = FALSE;
    
    //interrupt->SetLevel(oldLevel);
  }
    pageTableLock->Release();
  
}
//Helper function for development
void AddrSpace::PrintPageTable(){
  for(unsigned int i = 0 ; i < numPages ; i++){
    DEBUG('a', " PageTable virtual address: %d, physical address  %d!  isValid: %d\n",
    pageTable[i].virtualPage, pageTable[i].physicalPage, pageTable[i].valid);

  }
}
