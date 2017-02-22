/*
  Test case for basic Semaphores. Ensures that P1_P blocks, then child executes and prints 0, then child unblocks parent and parent prints 1

  should print :

  ---------Starting Test SEM ----------
 0
 1
 ---------Ending Test SEM ----------

*/

#include "phase1.h"
#include <stdio.h>

P1_Semaphore sem1;

int Child(){
  int result = P1_SemFree(sem1);
  if ( result == -2 )
  {
      USLOSS_Console("Passed not freeing a blocked semaphore\n");
  } else {
      USLOSS_Console("Failed: freed a blocked semaphore\n");
  }
  P1_V(sem1);
  return(0);
}

int P2_Startup(void *arg){

    int result = P1_SemCreate("semaphore" , 0, &sem1 );
    if ( result == 0 )
    {
        USLOSS_Console("Passed creating semaphore\n");
    } else {
        USLOSS_Console("Failed creating semaphore\n");
    }

    result = P1_SemCreate("semaphore" , 0, &sem1 );
    if ( result == -1 )
    {
        USLOSS_Console("Passed creating semaphore with duplicate name\n");
    } else {
        USLOSS_Console("Failed: created duplicate semaphore\n");
    }

    result = P1_SemCreate(NULL, 0, &sem1 );
    if ( result == -3 )
    {
        USLOSS_Console("Passed creating semaphore with null name\n");
    } else {
        USLOSS_Console("Failed: created semaphore with NULL name\n");
    }

    P1_Fork("Child", Child, NULL, USLOSS_MIN_STACK, 4, 0);
    P1_P(sem1);

    result = P1_SemFree(sem1);
    if ( result == 0 )
    {
        USLOSS_Console("Passed freeing valid semaphore\n");
    } else {
        USLOSS_Console("Failed: failed to free semaphore\n");
    }

    result = P1_SemFree(sem1);
    if ( result == -1 )
    {
        USLOSS_Console("Passed freeing invalid semaphore\n");
    } else {
        USLOSS_Console("Failed: freed invalid semaphore\n");
    }
    
    result = P1_P(sem1);
    if ( result == -1 )
    {
        USLOSS_Console("Passed P'ing semaphore with invalid state\n");
    } else {
        USLOSS_Console("Failed: P'd invalid semaphore\n");
    }

    result = P1_V(sem1);
    if ( result == -1 )
    {
        USLOSS_Console("Passed V'ing semaphore with invalid state\n");
    } else {
        USLOSS_Console("Failed: V'd invalid semaphore\n");
    }
    return 0;
}

void test_setup(int argc, char **argv) {
    // Do nothing.
}

void test_cleanup(int argc, char **argv) {
    // Do nothing.
}
