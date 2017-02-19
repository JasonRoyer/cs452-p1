#include "phase1.h"
#include <stdlib.h>
#include <assert.h>

int processCount = 2;

int randomFork(int (*f)(void*)) {
    processCount++;
    int r = rand();
    int *priority = malloc(sizeof(int));
    *priority = (r % 5 + 1);
    P1_Fork("child", f, priority, USLOSS_MIN_STACK, *priority , 0);
    return 0;
}

int doNothing(void *arg) {
    int myPriority = *((int*)arg);
    USLOSS_Console("%d",myPriority);
    free(arg);
    processCount--;
    return 0;
}

int P2_Startup(void *notused)
{
    srand(1);
    while(processCount < 50) {
        randomFork(doNothing);
    }
    processCount--;
    return 0;
}

void test_setup(int argc, char **argv) {
    // Do nothing.
}

void test_cleanup(int argc, char **argv) {
    // Do nothing.
}
