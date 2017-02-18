#include "phase1.h"
#include <stdlib.h>
#include <assert.h>
int childPid;

int child() {
    USLOSS_Console("Current PID [%d] == return value of P1_Fork [%d]\n", P1_GetPID(), childPid);
    assert(P1_GetPID() == childPid);
    return 0;
}

int P2_Startup(void *notused)
{
    childPid = P1_Fork("child", child, NULL, USLOSS_MIN_STACK, 3, 0);
    USLOSS_Console("Child status [%d] == 1 (ready)\n", P1_GetState(childPid));
    assert(P1_GetState(childPid) == 1);
    return 0;
}

void test_setup(int argc, char **argv) {
    // Do nothing.
}

void test_cleanup(int argc, char **argv) {
    // Do nothing.
}
