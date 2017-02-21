#include "phase1.h"
#include <stdlib.h>
#include <assert.h>

void quiter(int interupt, void *arg) 
{
    USLOSS_Console("Quiter invoked!\n");
    assert( 1 );
    USLOSS_Halt(0);
}

int P2_Startup(void *notused)
{
    USLOSS_IntVec[USLOSS_SYSCALL_INT] = quiter;
    int i = USLOSS_PsrSet(USLOSS_PsrGet() | USLOSS_PSR_CURRENT_INT);
    if (USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE)
      i =   USLOSS_PsrSet(USLOSS_PsrGet() ^ USLOSS_PSR_CURRENT_MODE);

    P1_Quit( 1 );
    USLOSS_Syscall(NULL);
    return 1;
}

void test_setup(int argc, char **argv) {
    // Do nothing.
}

void test_cleanup(int argc, char **argv) {
    // Do nothing.
}
