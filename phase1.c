/* ------------------------------------------------------------------------
   phase1.c

   Skeleton file for Phase 1. These routines are very incomplete and are
   intended to give you a starting point. Feel free to use this or not.


   ------------------------------------------------------------------------ */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "usloss.h"
#include "phase1.h"

/* -------------------------- Globals ------------------------------------- */

typedef struct PCB {
    USLOSS_Context      context;
    int                 (*startFunc)(void *);   /* Starting function */
    void                 *startArg;             /* Arg to starting function */
    int                  priority;
    int                  tag;
    int                  ppid;  // parent pid
    int                  status; 
} PCB;


/* the process table */
PCB procTable[P1_MAXPROC];

/* current process ID */
int pid = -1;

/* number of processes */

int numProcs = 0;

static int sentinel(void *arg);
static void launch(void);

int checkMode() {
	return USLOSS_PsrGet() && USLOSS_PSR_CURRENT_MODE;
	}



/* -------------------------- Functions ----------------------------------- */
/* ------------------------------------------------------------------------
   Name - dispatcher
   Purpose - runs the highest priority runnable process
   Parameters - none
   Returns - nothing
   Side Effects - runs a process
   ----------------------------------------------------------------------- */

void dispatcher()
{
  /*
   * Run the highest priority runnable process. There is guaranteed to be one
   * because the sentinel is always runnable.
   */
	printf("running pid: %d\n", pid);
   int i; for (i = 0; i < P1_MAXPROC ; i++ )
   {
     if (procTable[i].priority != 0 && (procTable[i].priority < procTable[pid].priority)){ 
		// save old context, load new context, set pid to new current pid, i. 
		// doesn't check priority? may have implicit priority with PID
		USLOSS_ContextSwitch(&procTable[pid].context,&procTable[i].context);		
		 pid = i;
		 printf("switched to pid: %d\n", pid);
		 break;
	 }
   }
}

void wraperFunc(){
	// change this argument
	P1_Quit(procTable[pid].startFunc((procTable[pid].startArg)));
	dispatcher();
}
/* ------------------------------------------------------------------------
   Name - startup
   Purpose - Initializes semaphores, process lists and interrupt vector.
             Start up sentinel process and the P2_Startup process.
   Parameters - none, called by USLOSS
   Returns - nothing
   Side Effects - lots, starts the whole thing
   ----------------------------------------------------------------------- */
void startup(int argc, char **argv)
{

  /* initialize the process table here */
  // it's already initialized
 // int i; for(i=0 ; i<P1_MAXPROC ; i++) {
	  //procTable[i] = (PCB *) malloc(sizeof(PCB));
	  //}

  /* Initialize the Ready list, Blocked list, etc. here */

  /* Initialize the interrupt vector here */

  /* Initialize the semaphores here */
	printf("it started and i am running\n");
  /* startup a sentinel process */
  /* HINT: you don't want any forked processes to run until startup is finished.
   * You'll need to do something  to prevent that from happening.
   * Otherwise your sentinel will start running as soon as you fork it and 
   * it will call P1_Halt because it is the only running process.
   */
   launch();
  pid = P1_Fork("sentinel", sentinel, NULL, USLOSS_MIN_STACK, 6, 0);

  /* start the P2_Startup process */
  P1_Fork("P2_Startup", P2_Startup, NULL, 4 * USLOSS_MIN_STACK, 1, 0);

  dispatcher();
  /* Should never get here (sentinel will call USLOSS_Halt) */

  return;
} /* End of startup */

/* ------------------------------------------------------------------------
   Name - finish
   Purpose - Required by USLOSS
   Parameters - none
   Returns - nothing
   Side Effects - none
   ----------------------------------------------------------------------- */
void finish(int argc, char **argv)
{
  USLOSS_Console("Goodbye.\n");
} /* End of finish */


int getNewPid()
{
  // finds the first available PID used by fork to find pid for new process
  int i = 1;
  while (procTable[i].startFunc != NULL) {
	  i++;
	  if (i > P1_MAXPROC) {
		  return 0;
		  }
	  }
  return i;
}

/* ------------------------------------------------------------------------
   Name - P1_Fork
   Purpose - Gets a new process from the process table and initializes
             information of the process.  Updates information in the
             parent process to reflect this child process creation.
   Parameters - the process procedure address, the size of the stack and
                the priority to be assigned to the child process.
   Returns - the process id of the created child or an error code.
   Side Effects - ReadyList is changed, procTable is changed, Current
                  process information changed
   ------------------------------------------------------------------------ */
int P1_Fork(char *name, int (*f)(void *), void *arg, int stacksize, int priority, int tag)
{
    if (!checkMode()) {USLOSS_IllegalInstruction();}
    // first lets do some checks. 
    // make sure the parameters are valid. 
    int newPid = getNewPid();
	// first PID may need to be 0, will test later
    if (newPid == 0) { return -1; }
    if (stacksize < USLOSS_MIN_STACK) { return -2; }
    if (priority < 0 || priority > 6) { return -3;} 
    if (tag != 0 && tag != 1) { return -4; }
    // now we know we have 
    procTable[newPid].startFunc = f;
    procTable[newPid].startArg = arg;
    procTable[newPid].priority = priority;
    procTable[newPid].tag = tag;
    procTable[newPid].ppid = pid;
    // now we initialize the context. 
    // more stuff here, e.g. allocate stack, page table, initialize context, etc.
    char * stack = malloc(stacksize*sizeof(char)); // allocating stack
    USLOSS_PTE *pt = P3_AllocatePageTable(newPid);
	// add wrapper here?
    USLOSS_ContextInit(&procTable[newPid].context, stack, stacksize, pt, wraperFunc);
    //dispatcher(); // call the dispatcher. 
    return newPid;
} /* End of fork */

/* ------------------------------------------------------------------------
   Name - launch
   Purpose - Dummy function to enable interrupts and launch a given process
             upon startup.
   Parameters - none
   Returns - nothing
   Side Effects - enable interrupts
   ------------------------------------------------------------------------ */
void launch(void)
{
  printf("launch was called\n");
  int  rc;
  int  status;
  status = USLOSS_PsrSet(USLOSS_PsrGet() | USLOSS_PSR_CURRENT_INT);
  if (status != 0) {
      USLOSS_Console("USLOSS_PsrSet failed: %d\n", status);
      USLOSS_Halt(1);
  }
  rc = procTable[pid].startFunc(procTable[pid].startArg);
  /* quit if we ever come back */
  P1_Quit(rc);
} /* End of launch */

/* ------------------------------------------------------------------------
   Name - P1_Quit
   Purpose - Causes the process to quit and wait for its parent to call P1_Join.
   Parameters - quit status
   Returns - nothing
   Side Effects - the currently running process quits
   ------------------------------------------------------------------------ */
void P1_Quit(int status) {
  // Do something here.
  if (!checkMode()) {USLOSS_IllegalInstruction();}
  // clean up current Proccess
  
  dispatcher();
}

int P1_GetPID(void)
{
  // returns pid of currently running process.
  if (!checkMode()) {USLOSS_IllegalInstruction();}

  return pid;
}

/* ------------------------------------------------------------------------
   Name - P1_GetState
   Purpose - gets the state of the process
   Parameters - process PID
   Returns - process state
   Side Effects - none
   ------------------------------------------------------------------------ */
int P1_GetState(int PID) {
  if (!checkMode()) {USLOSS_IllegalInstruction();}
  if (PID > P1_MAXPROC || PID < 1) { return -1;}  // invalid PID
  if (PID == P1_GetPID()) { return 0; }            // process is currently running. 

  // TODO: the conditions are associated with the return values. 
  //       implement the condition checks
  //int isReady, isKilled, isQuit, isWait;
  // TODO: check for ready process
  //if (isReady)   { return 1; }
  // TODO: check for killed process
  //if (isKilled)  { return 2; }
  // TODO: check for quit process
  //if (isQuit)    { return 3; }
  // TODO: check for process waiting on semaphore
 // if (isWait)    { return 4; }
  return procTable[PID].status;
}

void P1_DumpProcesses(void)
{

  if (!checkMode()) {USLOSS_IllegalInstruction();}
}

/* ------------------------------------------------------------------------
   Name - sentinel
   Purpose - The purpose of the sentinel routine is two-fold.  One
             responsibility is to keep the system going when all other
             processes are blocked.  The other is to detect and report
             simple deadlock states.
   Parameters - none
   Returns - nothing
   Side Effects -  if system is in deadlock, print appropriate error
                   and halt.
   ----------------------------------------------------------------------- */
int sentinel (void *notused)
{
    while (1)
    {
        /* Check for deadlock here */
        USLOSS_WaitInt();
		dispatcher();
    }
	printf("ending sentinel\n");
    USLOSS_Halt(0);
    /* Never gets here. */
    return 0;
} /* End of sentinel */
