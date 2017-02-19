#include "phase1.h"
#include <stdlib.h>
#include <assert.h>
#include <limits.h>

//Tests invalid tag, fork should return -4

int childPid;

int child() {
    USLOSS_Console("Should not see this\n" );
    assert( 0 );
    return 0;
}

int P2_Startup(void *notused)
{
    int pid;
    USLOSS_Console("Attempt to fork child with invalid tag\n" );
    pid = P1_Fork("child", child, NULL, USLOSS_MIN_STACK , 3 , -1 );
    assert( pid == -4 );
    return 0;
}

void test_setup(int argc, char **argv) {
    // Do nothing.
}

void test_cleanup(int argc, char **argv) {
    // Do nothing.
}
