#include "phase1.h"
#include <assert.h>
#include <stdio.h>

int Grandchild(void *arg) {
    return 1;
}

int Child(void *arg) {
    P1_Fork("Grandchild", Grandchild, NULL, USLOSS_MIN_STACK, 4, 0);
    int y;
    P1_Join(0, &y);
    USLOSS_Console("%d\n", y);
    return 2;
}

int P2_Startup(void *notused) 
{
    P1_Fork("Child", Child, NULL, USLOSS_MIN_STACK, 3, 0);
    int x;
    P1_Join(0, &x);
    USLOSS_Console("%d\n", x);
    USLOSS_Console("3\n");
    return 0;
}

void test_setup(int argc, char **argv) {
    // Do nothing.
}

void test_cleanup(int argc, char **argv) {
    // Do nothing.
}
