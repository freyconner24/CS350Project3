/* exectests.c
 *	Simple program to test exec syscalls
 */

#include "syscall.h"

int main(){
  Write("Testing Exec Calls\n", 19, ConsoleOutput);
  /*Testing that exec works */
  Exec("../test/matmult");
  Exec("../test/testfiles");
  /*Testing that program doesn't execute these invalid inputs into Execs. */
  Exec("../awdawd");
  Exec("../awdawd");
  Exec("");
  Exit(0);
}
