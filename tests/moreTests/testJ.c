#include "phase1.h"
#include <assert.h>
#include <stdio.h>

int Child(void *arg) {
    USLOSS_Console("Child has run, should print after P2_Startup but before the join message\n");
    return 12;
}

int P2_Startup(void *notused) 
{
    int pid;
    int state;
    int status = 0;

    USLOSS_Console("P2_Startup\n");
    int go;
    int t=0;
    for(go = 0; go < 100000000; go++){
      t=t+1;
    }
    USLOSS_Console("Should still be in P2\n");
    pid = P1_Fork("Child", Child, NULL, USLOSS_MIN_STACK, 3, 0);
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
    USLOSS_Console("Join message: In P2 after join, should print after child since parent called join, t should equal 100000000, t = %d\n", t);
    return status;
}

void test_setup(int argc, char **argv) {
    // Do nothing.
}

void test_cleanup(int argc, char **argv) {
    // Do nothing.
}
