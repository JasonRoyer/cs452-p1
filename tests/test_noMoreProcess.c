#include "phase1.h"
#include <stdlib.h>
#include <assert.h>
#include <limits.h>

//Tests forking too many processes

int childPid;

int child() {
    return 0;
}

int bad_child() {
    USLOSS_Console("Should not see this\n" );
    assert( 0 );
    return 0;
}

int P2_Startup(void *notused)
{
    int i;
    for (i = 0; i < 48; i++) {
        USLOSS_Console("Attempt %d\n", i );
        P1_Fork("child", child, NULL, USLOSS_MIN_STACK, 3 , 0);
    }
    childPid = P1_Fork("bad child", bad_child, NULL, USLOSS_MIN_STACK, 3 , 0);
    assert(childPid == -1);
    return 0;
}

void test_setup(int argc, char **argv) {
    // Do nothing.
}

void test_cleanup(int argc, char **argv) {
    // Do nothing.
}
