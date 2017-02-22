#include "phase1.h"
#include <assert.h>
#include <stdio.h>

int Child(void *arg) {
    USLOSS_Console("Child\n");
    return 1;
}

int Child2(void *arg) {
    USLOSS_Console("Child2\n");
    return 2;
}


int P2_Startup(void *notused) 
{
    int pid;
    int state;
    int status = 0;

    USLOSS_Console("P2_Startup\n");
    pid = P1_Fork("Child", Child, NULL, USLOSS_MIN_STACK, 1, 0);
    pid = P1_Fork("Child", Child2, NULL, USLOSS_MIN_STACK, 3, 0);
    if (pid < 0) {
        USLOSS_Console("Unable to fork child: %d\n", pid);
        status = 1;
    } else {
        /*
         * Child runs at priority 3, which is lower than ours. Part A is
         * run-to-completion, so we should continue to run while the 
         * child waits.
         */

        state = P1_GetState(pid);
        if (state != 1) { // child should be ready
            USLOSS_Console("Child is in invalid state: %d\n", state);
            status = 1;
        }
    }
    int x;
    P1_Join(0, &x);
    USLOSS_Console("Done with Join %d\n", x);
    P1_Join(0, &x);
    USLOSS_Console("Done with Join %d\n", x);
    return status;
}

void test_setup(int argc, char **argv) {
    // Do nothing.
}

void test_cleanup(int argc, char **argv) {
    // Do nothing.
}
