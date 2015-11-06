/* lock_test.c
 *	Simple program to test the file handling system calls
 */

#include "syscall.h"

int lock1;

int main() {
    lock1 = CreateLock("Lock1", 5, 0);
	Exit(0);
}
