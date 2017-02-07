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
#include <string.h>

/* -------------------------- Globals ------------------------------------- */


typedef struct PCB {
    USLOSS_Context      context;
    int                 (*startFunc)(void *);   /* Starting function */
    void                 *startArg;             /* Arg to starting function */
	int	state; // the state of the process IE  running, ready to run, not used, quit, is waiting. 
	int priority;
	int tag;
	int isUsed;
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
	  p_node * curr = pq->head;
	 if(new->priority < curr->priority){
		// insert at head
		new->next = pq->head;
		pq->head = new;
	}else {
		// insert after head
		while (curr->next != NULL) {
			if(new->priority > curr->next->priority){
				curr = curr->next;
			}else {
				// insert it after curr
				new->next = curr->next;
				curr->next = new;
				return;
			}
			
		}
	}
	
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

void pq_print(priority_queue * pq){
	p_node * curr = pq-> head;
	while(curr != NULL){
		//printf("%d : ",curr->pid);
		curr = curr->next;
	}
	//printf("\n");
}

void pq_remove(priority_queue * pq, int thePID){
	if (pq->head->pid == thePID){
		pq->head = pq->head->next;
	}else {
		 p_node * curr = pq->head;
		while (curr->next != NULL) {
			if(curr->next->pid == thePID){
				curr->next = curr->next->next;
				return;
			}else {
				curr = curr->next;
			}
			
		}
	}
}



/* the process table */
PCB procTable[P1_MAXPROC];

// the priority queue
priority_queue * procQueue;

/* current process ID */
int pid = -1;

/* number of processes */

int numProcs = 0;

static int sentinel(void *arg);
static void launch(void);

int checkMode() {
	return USLOSS_PsrGet() && USLOSS_PSR_CURRENT_MODE;
	}
	
int getNewPid()
{
  // finds the first available PID used by fork to find pid for new process
  
  for(int i=0; i < 50; i++){
	  if(procTable[i].isUsed == 0){
		  // 2 is the state for not used
		  return i;
	  }
  }
  // 50 processes have run if it reaches here. 
  // so return -1 and deal with it later
  return -1;
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
   pq_print(procQueue);
  int oldPID = pid;
  pid = pq_pop(procQueue);
  //printf("Dispatcher switched PID from:%d to %d\n", oldPID,pid);
  USLOSS_ContextSwitch(&procTable[oldPID].context,&procTable[pid].context);
}

void wrapperFunc(){
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
  
  // initializing process queue
  procQueue = pq_create();

  /* initialize the process table here */
	//for(int i=0; i < 50; i++){
	//	procTable[i].state = 2;
	//  }
  
  /* Initialize the Ready list, Blocked list, etc. here */

  /* Initialize the interrupt vector here */

  /* Initialize the semaphores here */

  /* startup a sentinel process */
  /* HINT: you don't want any forked processes to run until startup is finished.
  
   * You'll need to do something  to prevent that from happening.
   * Otherwise your sentinel will start running as soon as you fork it and 
   * it will call P1_Halt because it is the only running process.
   */
   
   //printf("Stated up\n");
   P1_Fork("sentinel", sentinel, NULL, USLOSS_MIN_STACK, 6, 0);

  /* start the P2_Startup process */
  pid = P1_Fork("P2_Startup", P2_Startup, NULL, 4 * USLOSS_MIN_STACK, 1, 0);

	launch();
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
	// check input peramaters
	if (!checkMode()){
		USLOSS_IllegalInstruction();
	}	
	
    int newPid = getNewPid();
	if(newPid == -1){
		// we already have 50 process do something
		return -1;
	}
	if(stacksize < USLOSS_MIN_STACK){
		return -2;
	}
	if(priority < 0 || priority > 6){
		return -3;
	}
	if (tag != 0 && tag !=1){
		return -4;
	}
	
	//printf("forked %s with PID %d\n", name,newPid);
	// TODO: procTable[pid].child = newPid something like this to keep track of children may need later
	numProcs++;
  procTable[newPid].startFunc = f;
  procTable[newPid].startArg = arg;
	procTable[newPid].state = 1;
	procTable[newPid].isUsed = 1;
	procTable[newPid].priority = priority;
  procTable[newPid].tag = tag;
	
	
    // more stuff here, e.g. allocate stack, page table, initialize context, etc.
	char * stack = malloc(stacksize*sizeof(char)); // allocating stack
  USLOSS_PTE *pt = P3_AllocatePageTable(newPid);
	USLOSS_ContextInit(&procTable[newPid].context, stack, stacksize, pt, wrapperFunc);
	pq_push(procQueue,newPid,priority);
  if (priority < procTable[pid].priority) { 
		pq_push(procQueue,pid,procTable[pid].priority);
		dispatcher(); }
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
	//printf("launched\n");
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
  if (!checkMode()) {USLOSS_IllegalInstruction();}
  // clean up current PID
  // TODO: update as things get added to PCB
  // remove from Q this happens right now because of launch
  pq_remove(procQueue, pid);
  //printf("Quitting PID %d\n", pid);
  numProcs--;
  procTable[pid].state =3;
 // memset(&procTable[pid],0,sizeof(PCB));
  //procTable[pid].state = 2;
}

/* ------------------------------------------------------------------------
   Name - P1_GetState
   Purpose - gets the state of the process
   Parameters - process PID
   Returns - process state
   Side Effects - none
   ------------------------------------------------------------------------ */
int P1_GetState(int PID) {
	if(PID < 0 || PID > 49){
		return -1;
	}
  return procTable[PID].state;
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
		//printf("running sent %d\n",numProcs);
        USLOSS_WaitInt();
		dispatcher();
		// may not need to call dispatcher here? we will see
		
    }
    USLOSS_Halt(0);
    /* Never gets here. */
    return 0;
} /* End of sentinel */
