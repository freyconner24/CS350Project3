/* forkTwoSorts.c 
 *    Test that forking two threads work.

 *    Intended to test having two threads running in the same address
 *    space for the MMU.
 *    sortOne and sortTwo are copies of the sorting algorithm
 *    in the sort.c program.
 *    These forks are more memory swapping intensive.
 */

#include "syscall.h"

int A[1024];	/* size of physical memory; with code, we'll run out of space!*/
int B[1024];

int
sortOne()
{
    int i, j, tmp;

    /* first initialize the array, in reverse sorted order */
    for (i = 0; i < 1024; i++)		
        A[i] = 1024 - i;

    /* then sort! */
    for (i = 0; i < 1023; i++)
        for (j = i; j < (1023 - i); j++)
	   if (A[j] > A[j + 1]) {	/* out of order -> need to swap ! */
	      tmp = A[j];
	      A[j] = A[j + 1];
	      A[j + 1] = tmp;
    	   }
    Exit(A[0]);		/* and then we're done -- should be 0! */
}

int
sortTwo()
{
    int i, j, tmp;

    /* first initialize the array, in reverse sorted order */
    for (i = 0; i < 1024; i++)      
        B[i] = 1024 - i;

    /* then sort! */
    for (i = 0; i < 1023; i++)
        for (j = i; j < (1023 - i); j++)
       if (B[j] > B[j + 1]) {   /* out of order -> need to swap ! */
          tmp = B[j];
          B[j] = B[j + 1];
          B[j + 1] = tmp;
           }
    Exit(B[0]);     /* and then we're done -- should be 0! */
}

int 
main()
{
    Fork(sortOne, 1);
    Fork(sortTwo, 2);
    Exit(0);
}