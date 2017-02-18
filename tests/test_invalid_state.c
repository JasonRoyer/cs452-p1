#include "phase1.h"
#include <assert.h>
#include <stdio.h>

//tests that GetState returns -1 if the pid is invalid
int P2_Startup(void *notused) 
{
    int status = 0;
    int state = 0;

    USLOSS_Console("P2_Startup\n");
    state = P1_GetState( -1 );
    USLOSS_Console("State of pid -1: %d\n", state );

    return status;
}

void test_setup(int argc, char **argv) {
    // Do nothing.
}

void test_cleanup(int argc, char **argv) {
    // Do nothing.
}
