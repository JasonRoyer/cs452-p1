/* ------------------------------------------------------------------------
   phase1.c

   Skeleton file for Phase 1. These routines are very incomplete and are
   intended to give you a starting point. Feel free to use this or not.


   ------------------------------------------------------------------------ */

#include <stddef.h>
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



/* Here is a simple priority queue to be used by the dispatcher. 
 */
typedef struct p_node
{
  int pid;
  int priority;
  struct p_node * next;
} p_node;

typedef struct priority_queue
{
  p_node * head;

} priority_queue;


priority_queue * pq_create()
{
  // creates and returns a priority queue struct. 
  priority_queue * retq =  malloc(sizeof(priority_queue));
  retq->head = NULL;
  return retq;
}

void pq_push(priority_queue * pq, int pid, int priority)
{
  // THis function takes a pointer to the priority queue
  // and integers representing the pid and priority
  // creates and pushes the new value on our queue. 

  // create the new node. 
  p_node * new = malloc(sizeof(p_node));
  new -> pid = pid;
  new -> priority = priority;
  new -> next = NULL;

  // now we insert it into the queue. 
  // high priority is closer to the head. 
  // head node is the highest priority. 
  if (pq->head == NULL) {pq->head = new;}
  else
  {
    p_node * curr = head;
    while (new->priority > curr->priority) { curr = curr->next; }
    new -> next = curr->next;
    curr->next = new;
  }
  return;
} 

int pq_pop(priority_queue * pq)
{
  if (pq->head == NULL) { return 0; }
  int retVal = pq->head->pid;
  pq->head = pq->head->next;
  return retVal;
}




/* the process table */
PCB procTable[P1_MAXPROC];


/* current process ID */
int pid = -1;

/* number of processes */

int numProcs = 0;

static int sentinel(void *arg);
static void launch(void);


int checkMode() { return USLOSS_PSRGet() && USLOSS_PSR_CURRENT_MODE; }



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
   int i; for (i = 0; i < P1_MAXPROC ; i++ )
   {
     if (i < pid) 
     { USLOSS_ContextSwitch(&procTable[pid].context,&procTable[i].context); pid = i; break;}
   }
   return;
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
  int i; for(i=0 ; i<P1_MAXPROC ; i++) { procTable[i] = malloc(sizeOf(PCB));}

  /* Initialize the Ready list, Blocked list, etc. here */

  /* Initialize the interrupt vector here */

  /* Initialize the semaphores here */

  /* startup a sentinel process */
  /* HINT: you don't want any forked processes to run until startup is finished.
   * You'll need to do something  to prevent that from happening.
   * Otherwise your sentinel will start running as soon as you fork it and 
   * it will call P1_Halt because it is the only running process.
   */
  P1_Fork("sentinel", sentinel, NULL, USLOSS_MIN_STACK, 6, 0);

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
  while (procTable[i] != NULL) { i++; if (i > P1_MAXPROC) {return 0;}}
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
    if (!checkmode()) {USLOSS_IllegalInstruction();}
    // first lets do some checks. 
    // make sure the parameters are valid. 
    int newPid = getNewPid();
    if (newPid == 0) { return -1; }
    if (stacksize < USLOSS_MIN_STACK) { return -2; }
    if (priority < 1 || priority > 5) { return -3;} 
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
    USLOSS_PTE pt = P3_AllocatePageTable(newPid);
    USLOSS_ContextInit(USLOSS_Context procTable[newPid].context, stack, stacksize, pt, f);
    dispatcher(); // call the dispatcher. 
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
  if (!checkmode()) {USLOSS_IllegalInstruction();}
  dispatcher();
}

int P1_GetPID(void)
{
  // returns pid of currently running process.
  if (!checkmode()) {USLOSS_IllegalInstruction();}

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
  if (!checkmode()) {USLOSS_IllegalInstruction();}
  if (PID > 50 || PID < 1) { return -1;}  // invalid PID
  if (PID = P1_GetPID()) { return 0; }            // process is currently running. 

  // TODO: the conditions are associated with the return values. 
  //       implement the condition checks
  int isReady, isKilled, isQuit, isWait;
  // TODO: check for ready process
  if (isReady)   { return 1; }
  // TODO: check for killed process
  if (isKilled)  { return 2; }
  // TODO: check for quit process
  if (isQuit)    { return 3; }
  // TODO: check for process waiting on semaphore
  if (isWait)    { return 4; }
}

void P1_DumpProcesses(void)
{

  if (!checkmode()) {USLOSS_IllegalInstruction();}
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
    while (numProcs > 1)
    {
        /* Check for deadlock here */
        USLOSS_WaitInt();
    }
    USLOSS_Halt(0);
    /* Never gets here. */
    return 0;
} /* End of sentinel */
