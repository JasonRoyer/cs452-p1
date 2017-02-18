#include "phase1.h"
#include <assert.h>
#include <stdio.h>

int Grandchild(void *arg) {
    return 0;
}

int doNothing(void *arg) {
    int i;
    for (i = 0; i < 1000000; i+=2) {
        i--;
    }
    return 0;
}

int Child(void *arg) {
    int     pid,i;
    int     status = 0;
    for (i = 0; i < 1000; i++)
    pid = P1_Fork("Grandchild", doNothing, NULL, USLOSS_MIN_STACK, 2, 0);
    return status;
}

int P2_Startup(void *notused)
{
    int pid;
    int status = 0;
    pid = P1_Fork("Child", Child, NULL, USLOSS_MIN_STACK, 3, 0);
    return status;
}

void test_setup(int argc, char **argv) {
    // Do nothing.
}

void test_cleanup(int argc, char **argv) {
    // Do nothing.
}
