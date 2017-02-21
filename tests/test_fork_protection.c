#include "phase1.h"
#include <stdlib.h>
#include <assert.h>

int bad_process() {
    // should never be runned
    assert( 0 );
    return 0;
}

void quiter(int interupt, void *arg) {
    USLOSS_Console("Quiter invoked!\n");
    assert( 1 );
    USLOSS_Halt(0);
}

int bad_child() {
    // set to user mode
    int i = USLOSS_PsrSet(USLOSS_PsrGet() | USLOSS_PSR_CURRENT_INT);
    if (USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE)
      i=   USLOSS_PsrSet(USLOSS_PsrGet() ^ USLOSS_PSR_CURRENT_MODE);
    P1_Fork("bad process", bad_process, NULL, USLOSS_MIN_STACK, 2 , 0);
    USLOSS_Syscall(NULL);
    return 0;
}

int P2_Startup(void *notused)
{
    USLOSS_IntVec[USLOSS_SYSCALL_INT] = quiter;
    P1_Fork("bad child", bad_child, NULL, USLOSS_MIN_STACK, 3 , 0);
    return 0;
}

void test_setup(int argc, char **argv) {
    // Do nothing.
}

void test_cleanup(int argc, char **argv) {
    // Do nothing.
}
