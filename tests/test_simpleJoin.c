#include "phase1.h"
#include <assert.h>
#include <stdio.h>

int Child(void *arg) {
    USLOSS_Console("Child %d\n", (int) arg);
    return (int) arg;
}

int P2_Startup(void *notused)
{
    #define NUM 10
    int status = 0;
    int pid;
	int input = 1;
    pid = P1_Fork("Child", Child, (void *) &input, USLOSS_MIN_STACK, 3, 0);
    assert(pid >= 0);
    int returnedPID;
	returnedPID = P1_Join(0, &status);
	assert(returnedPID >= 0);
	assert(returnedPID == pid);
	assert(status == 1);
    return status;
}

void test_setup(int argc, char **argv) {
    // Do nothing.
}

void test_cleanup(int argc, char **argv) {
    // Do nothing.
}
