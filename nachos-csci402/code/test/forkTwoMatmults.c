/* forkTwoMatmults.c 
 *    Test that forking two threads work.

 *    Intended to test having two threads running in the same address
 *	  space for the MMU.
 *    matmultOne and matmultTwo are copies of the matrix multiplication
 *	  in the matmult.c program.
 */
#include "syscall.h"



#define Dim 	20	/* sum total of the arrays doesn't fit in 
			 * physical memory 
			 */


int A[Dim][Dim];
int B[Dim][Dim];
int C[Dim][Dim];

int D[Dim][Dim];
int E[Dim][Dim];
int F[Dim][Dim];

int
matmultOne()
{
    int i, j, k;

    for (i = 0; i < Dim; i++)		/* first initialize the matrices */
	for (j = 0; j < Dim; j++) {
	     A[i][j] = i;
	     B[i][j] = j;
	     C[i][j] = 0;
	}

    for (i = 0; i < Dim; i++)		/* then multiply them together */
	for (j = 0; j < Dim; j++)
            for (k = 0; k < Dim; k++)
		 C[i][j] += A[i][k] * B[k][j];

    Exit(C[Dim-1][Dim-1]);		/* and then we're done */
}

int
matmultTwo()
{
    int i, j, k;

    for (i = 0; i < Dim; i++)		/* first initialize the matrices */
	for (j = 0; j < Dim; j++) {
	     D[i][j] = i;
	     E[i][j] = j;
	     F[i][j] = 0;
	}

    for (i = 0; i < Dim; i++)		/* then multiply them together */
	for (j = 0; j < Dim; j++)
            for (k = 0; k < Dim; k++)
		 F[i][j] += D[i][k] * E[k][j];

    Exit(F[Dim-1][Dim-1]);		/* and then we're done */
}

int 
main()
{
	Fork(matmultOne, 0);
	Fork(matmultTwo, 1);
	Exit(0);
}