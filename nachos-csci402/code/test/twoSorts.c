/* twoSorts.c 
 *    Test to prove that two exec calls work.
*
 *    Intended to test having two executables run and share the IPT
 *		and simulated NACHOS memory.
 *    Executing the sort.c program twice.
 *    These execs are more memory swapping intensive than matmult.
 */

#include "syscall.h"

int main(){
	Exec("../test/sort");
	Exec("../test/sort");
	Exit(0);

	return 0;
} 