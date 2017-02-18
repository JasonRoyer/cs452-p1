#include "phase1.h"
#include <assert.h>
#include <stdio.h>

int Grandchild(void *arg) {
    return 0;
}

int Child(void *arg) {
    int     pid;
    int     state;
    int     status = 0;
    pid = P1_Fork("Grandchild", Grandchild, NULL, USLOSS_MIN_STACK, 2, 0);
    USLOSS_Console("P1_Fork[%d] >= 0\n", pid);
    assert(pid >= 0);
    /*
     * Grandchild runs at priority 2, which is higher than ours. the grandchild should already have quit before
     * we get here.
     */
    state = P1_GetState(pid);
    assert(state == 3 || state == -1);
    return status;
}

int P2_Startup(void *notused)
{
    int pid;
    int status = 0;

    pid = P1_Fork("Child", Child, NULL, USLOSS_MIN_STACK, 3, 0);

    assert(pid >= 0);
    return status;
}

void test_setup(int argc, char **argv) {
    // Do nothing.
}

void test_cleanup(int argc, char **argv) {
    // Do nothing.
}
