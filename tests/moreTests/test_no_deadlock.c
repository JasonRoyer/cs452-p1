/*
  test case for pseudo clock
  output should be a list of 10 numbers, all close to 100000 like

  ---------Starting Test 11 ----------
wait period : 90008
wait period : 99991
wait period : 99998
wait period : 99995
wait period : 99991
wait period : 99992
wait period : 99994
wait period : 99996
wait period : 99994
wait period : 99994
 ---------Ending Test 11 ----------
*/

#include "phase1.h"
#include <stdio.h>
#include <assert.h>

int child(void *arg){

  USLOSS_Console(" \n---------Starting Test CLOCK ----------\n");
  int begin = -1;
  int now;
  int rc = 1;
  int i;
  for (i = 0; i < 11; i++) {
    rc = P1_WaitDevice(USLOSS_CLOCK_DEV, 0, &now);
    assert(rc == 0);
    if (begin != -1) {
      USLOSS_Console("wait period : %d\n", now - begin);
    }
    begin = now;
  }

  USLOSS_Console(" ---------Ending Test CLOCK ----------\n");
  return 0;
}

int P2_Startup(void *arg){

  USLOSS_Console(" P2 will now wait on child, who is waiting on clock device\n");
  P1_Fork("Child", child, NULL, USLOSS_MIN_STACK, 4, 0);
  int r;
  P1_Join(0, &r);
  return 0;
}


void test_setup(int argc, char **argv) {
    // Do nothing.
}

void test_cleanup(int argc, char **argv) {
    // Do nothing.
}
