#include "phase1.h"
#include <assert.h>
#include <stdio.h>

int Child(void *arg) {
    USLOSS_Console("Child\n");
    return 12;
}

int P2_Startup(void *notused) 
{
    USLOSS_Console("P2_Startup\n");
    int x;
    P1_Join(0, &x);
    USLOSS_Console("Passed calling join when no children present. Now attempting to fork child and then call join\n");
    P1_Fork("Child", Child, NULL, USLOSS_MIN_STACK, 3, 0);
    P1_Join(0, &x);
    USLOSS_Console("Passed calling join when we fork a lower priority process\n");
    P1_Join(0, &x);
    USLOSS_Console("Passed calling join when no children avaiable (last child has already been joined)\n");
    return 0;
}

void test_setup(int argc, char **argv) {
    // Do nothing.
}

void test_cleanup(int argc, char **argv) {
    // Do nothing.
}
