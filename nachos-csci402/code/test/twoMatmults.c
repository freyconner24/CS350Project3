/* twoMatmults.c 
 *    Test to prove that two exec calls work.
*
 *    Intended to test having two executables run and share the IPT
 *		and simulated NACHOS memory.
 *    Executing the matmult.c program twice.
 *    
 */

#include "syscall.h"

int main(){
	Exec("../test/matmult");
	Exec("../test/matmult");
	Exit(0);

	return 0;
}