#include "phase1.h"
#include <stdlib.h>
#include <assert.h>
#include <limits.h>

//Tests invalid priority, fork should return -3

int childPid;

int child() {
    USLOSS_Console("Should not see this\n" );
    assert( 0 );
    return 0;
}

int P2_Startup(void *notused)
{
    int pid;
    USLOSS_Console("Attempt to fork child with invalid priority\n" );
    pid = P1_Fork("child", child, NULL, USLOSS_MIN_STACK , -1 , 0);
    assert( pid == -3 );
    return 0;
}

void test_setup(int argc, char **argv) {
    // Do nothing.
}

void test_cleanup(int argc, char **argv) {
    // Do nothing.
}
