/* ------------------------------------------------------------------------
   phase1.c
   Version: 1.4

   Skeleton file for Phase 1. These routines are very incomplete and are
   intended to give you a starting point. Feel free to use this or not.


   ------------------------------------------------------------------------ */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "usloss.h"
#include "phase1.h"

/* -------------------------- Globals ------------------------------------- */

// semaphore struct along with global vars indicatin number of semaphors and a global semaphore table. 
int semCount =0;;
typedef struct priority_queue priority_queue;
typedef struct Semaphore Semaphore;

struct  Semaphore
{
  int   value;
  char*  name;
  priority_queue * q;
  
};

Semaphore* semTable[P1_MAXSEM];

typedef struct PCB {
    USLOSS_Context      context;
    int                 (*startFunc)(void *);   /* Starting function */
    void                 *startArg;             /* Arg to starting function */
	int		state;	/* The state the process is in
						-1:	invalid PID
						0:	the process is running
						1:	the process is ready
						2:	(not used)
						3: 	the process has quit
						4:      the process is waiting on a semaphore
					*/
	int 	isUsed;  /* Notes if the process space is free to be used. 0 = free 1 = being used*/
	int 	priority; /* The priority of the process (highest priority is 1,lowest priority is 6) */
	USLOSS_PTE *pagetable;
	int pagetableINT;
	int stateReady;
	int 	tag; /* The tag is either 0 or 1, and is used by P1_Join to wait for children with a matching tag. */
	int 	pPID; /* The PID of the parent of this procces*/ 
	int 	status; /* The status of the process once quit has been called*/
	Semaphore * childJoinSem0; /* Semaphore keeping track of how many children have tag 0 and have quit*/
	Semaphore* childJoinSem1;/* Semaphore keeping track of how many children have tag 1 and have quit*/
	Semaphore* semQuitable;/* Semaphore keeping track if this procces can finish quiting*/
} PCB;


/* the process table */
PCB procTable[P1_MAXPROC];


/* current process ID */
int pid = -1;

/* number of processes */
int numProcs = 0;

int startUP =1;

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



/* The Queue of process ready to run. Functions defined at bottom of file */
priority_queue * readyQueue;

static int sentinel(void *arg);
static void launch(void);
/* Declaration of function prototypes */

// queue stuff
void pq_push(priority_queue*, int, int);
int pq_pop(priority_queue*);
void pq_print(priority_queue*);
void pq_remove(priority_queue*, int );
int  pq_isEmpty(priority_queue*);
priority_queue * pq_create();


// OS stuff. 
void IllegalModeHandler(int, void*);
int  inKernelMode();
int  getNewPid();
void wrapperFunc();
// for interrupts. 
void modInterrupt(int);
void enableInterrupt();
void disableInterrupt();
int  currentInterruptStatus();

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
   
    /* if (priority < procTable[pid].priority){ 
			pq_push(readyQueue,pid,procTable[pid].priority);
			 
	}
	*/
	
	   //USLOSS_Console("PID %d coming into dispatcher with state %d\n",pid,procTable[pid].state);	
	if(pid != -1 && (procTable[pid].state == 0 || procTable[pid].state == 3) ){
		// place current back into queue	(procTable[pid].state == 0 || procTable[pid].state == 3)
		   //USLOSS_Console("pushing PID %d onto queue\n",pid);		
		pq_push(readyQueue,pid,procTable[pid].priority);
	}	
	pq_print(readyQueue);
	//save old and pop new
	int oldPID = pid;
	pid = pq_pop(readyQueue);
	// if pid.state is waiting pop another
	procTable[pid].state = 0; // mark new as running
	
	if(startUP == 1){
		// if it's first proccess launch it 
		startUP = 0;
		   //USLOSS_Console("Launching PID %d\n",pid);	
		launch();
	}else if(oldPID != pid){
		if(procTable[oldPID].state != 3){
			procTable[oldPID].state = 1; // state 1 means it's ready to run.		
		}
		// if it's not the same procces switch them.
		   //USLOSS_Console("Switching to PID %d\n",pid);		
		USLOSS_ContextSwitch(&procTable[oldPID].context,&procTable[pid].context);
		   //USLOSS_Console("i switched to new proccess\n");
	}
	// start here
	
	
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
  
  /* Initialize the Ready list, Blocked list, etc. here */
	readyQueue = pq_create();
  /* Initialize the interrupt vector here */
	USLOSS_IntVec[USLOSS_ILLEGAL_INT] = IllegalModeHandler;
  /* Initialize the semaphores here */
  /* startup a sentinel process */
  pid = P1_Fork("sentinel", sentinel, NULL, USLOSS_MIN_STACK, 6, 0);
	
  /* start the P2_Startup process */
  P1_Fork("P2_Startup", P2_Startup, NULL, 4 * USLOSS_MIN_STACK, 1, 0);

    //USLOSS_Console("calling dispatcher from startup\n");
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
     //USLOSS_Console("Goodbye.\n");
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
	// if in user mode throw illegal exception
	if(!inKernelMode()){
		USLOSS_IllegalInstruction();
		return -5;
	}
	
	/* newPid = pid of empty PCB here */
	int newPid = getNewPid();
	if(newPid == -1){
		// we already have 50 process 
		return -1;
	}
	
	if(stacksize < USLOSS_MIN_STACK){
		return -2;
	}
	
	if(priority < 1 || priority > 6){
		return -3;
	}
	
	if (tag != 0 && tag !=1){
		return -4;
	}
	
	// incriment procces counter
	numProcs++;
	
    /* Load PCB block at pid with correct information */
    procTable[newPid].startFunc = f;
    procTable[newPid].startArg = arg;
	procTable[newPid].state = 1;
	procTable[newPid].priority = priority;
	procTable[newPid].tag = tag;
	procTable[newPid].isUsed = 1;
	procTable[newPid].pPID = pid;
	
	char semName[7];
	sprintf(semName, "%dSem0",newPid);
	P1_SemCreate(semName,0, (P1_Semaphore *)&procTable[newPid].childJoinSem0);
	
	char semName1[7];
	sprintf(semName1, "%dSem1",newPid);
	P1_SemCreate(semName1,0,(P1_Semaphore *)&procTable[newPid].childJoinSem1);
	
	char semName2[17];
	sprintf(semName2, "%dSemQuitable",newPid);
	P1_SemCreate(semName2,0,(P1_Semaphore *)&procTable[newPid].semQuitable);
	
	
    // more stuff here, e.g. allocate stack, page table, initialize context, etc.
	//   //USLOSS_Console("Process %d, %s is in state %d during fork\n",newPid,name, procTable[newPid].state);
	// allocate stack
	char * stack = malloc(stacksize*sizeof(char));
	// allocate page table
	if (procTable[newPid].pagetableINT != 1){
		procTable[newPid].pagetableINT=1;
		procTable[newPid].pagetable= P3_AllocatePageTable(newPid);
	}
	// initialize context with wraper function 
	USLOSS_ContextInit(&procTable[newPid].context, stack, stacksize, procTable[newPid].pagetable, wrapperFunc);
	
	// push the new process into the readyQueue
	pq_push(readyQueue,newPid,priority);
	
	// call the dispatcher incase we need to switch to the new process
	
	if(numProcs >1){
		   //USLOSS_Console("calling dispatcher from fork\n");
		dispatcher();
	}
	
	
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
         //USLOSS_Console("USLOSS_PsrSet failed: %d\n", status);
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
	if(!inKernelMode()){
		USLOSS_IllegalInstruction();
	}else {
		
		     //USLOSS_Console("Quiting PID %d\n", pid);
		 
			 // set status before V
			 procTable[pid].status = status;
			 procTable[pid].state =3;
			 procTable[pid].stateReady = 1;
			 // P on the semaphore based on the tag
			 if(procTable[pid].tag ==0){
				  //  //USLOSS_Console("Child %d Ved parent %d joinSem0\n", pid,procTable[pid].pPID);		 
				P1_V(procTable[procTable[pid].pPID].childJoinSem0);
			} else{
				  // //USLOSS_Console("Child %d Ved parent %d joinSem1\n", pid,procTable[pid].pPID);	
				P1_V(procTable[procTable[pid].pPID].childJoinSem1);
			}
			 // P on semaphore allowing the finish of quit
		  // //USLOSS_Console(" Just above Quitable for pid %d\n", pid);	
		P1_P(procTable[pid].semQuitable);
		 //  //USLOSS_Console("Got past Quitable for pid %d\n", pid);		 
		
		// clear PCB at pid
		procTable[pid].startFunc = NULL;
		procTable[pid].startArg =  NULL;
		procTable[pid].state = 2;
		procTable[pid].priority = 0;
		procTable[pid].tag = 0;
		procTable[pid].pPID = -2;
		procTable[pid].status = 0;
		P1_SemFree(procTable[pid].childJoinSem0);
		procTable[pid].childJoinSem0 = NULL;
		P1_SemFree(procTable[pid].childJoinSem1);
		procTable[pid].childJoinSem1 = NULL;
		P1_SemFree(procTable[pid].semQuitable);
		procTable[pid].semQuitable = NULL;
		// TODO: disable interupts here?
		procTable[pid].isUsed = 0;
				 
		// decrement procces counter
		 numProcs--;
		     //USLOSS_Console("Quit success\n\n\n\n\n\n###############\n\n");	
	}
	

}

int P1_Join(int tag, int *status){
	
	   //USLOSS_Console("Join called from Parent %d\n", pid);	
	// if current does not have children with matching tag return -1
	int tagFlag = -1;
	for(int i =0; i < P1_MAXPROC; i++){
		if(procTable[i].pPID == pid && procTable[i].tag == tag){
			tagFlag = i;
			break;
		}
	}
	if(tagFlag == -1){
		  // //USLOSS_Console("JOIN COULDNT FIND A CHILD WITH TAG\n");	
		return -1;
	}
		
		
	// get semaphore
	// TODO: might have mutex problem here. can't manipulate child list multiple times at once.
	// p that semaphore
	if(tag ==0){
		 //  //USLOSS_Console("Just above childJoinSem0 for parent pid %d\n", pid);	
		P1_P(procTable[pid].childJoinSem0);
		 //  //USLOSS_Console("got past childJoinSem0 for parent pid %d\n", pid);
	} else{
		 //  //USLOSS_Console("Just above childJoinSem1 for parent pid %d\n", pid);
		P1_P(procTable[pid].childJoinSem1);
		 //  //USLOSS_Console("got past childJoinSem1 for parent pid %d\n", pid);
	}
	
	// if tags are same and the child is waiting on sem
	int joiningPID = -1;
	for(int i =0; i < P1_MAXPROC; i++){
		if(procTable[i].pPID == pid){
			if (procTable[i].tag == tag){
				if (procTable[i].state == 3 && procTable[i].stateReady == 1){
					procTable[i].stateReady = 0;
					joiningPID = i;
					break;
				} 
			}
			
		}
	}
	// set status to status of returned child
	if(joiningPID != -1){
		*status = procTable[joiningPID].status;
	P1_V(procTable[joiningPID].semQuitable);
	} else {
		   //USLOSS_Console("joining id is -1\n");
	}
	
	return joiningPID;	
	
	
}




/* ------------------------------------------------------------------------
   Name - P1_GetState
   Purpose - gets the state of the process
   Parameters - process PID
   Returns - process state
   Side Effects - none
   ------------------------------------------------------------------------ */
int P1_GetState(int PID) {
	if(PID <0){
		return -1;
	}
  return procTable[PID].state;
}

int P1_GetPID(){
	return pid;
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
	   //USLOSS_Console("In sentinel numProcs is %d\n", numProcs);
    while (numProcs > 1)
    {
        /* Check for deadlock here */
        USLOSS_WaitInt();
    }
    USLOSS_Halt(0);
    /* Never gets here. */
    return 0;
} /* End of sentinel */


/*----------------- Interupt Handlers -------------*/

void IllegalModeHandler(int interupt, void *arg){
	   //USLOSS_Console("Kernel Mode Required!\n");	
	P1_Quit(-1);
}



/* ---------------- Helper Functions ---------------- */

/*    USLOSS_PsrGet() returns the register that checks for modes and interupt handlers
		USLOSS_PSR_CURRENT_MODE aka 0001
		Since the 1st bit in the register is set to 1 if in kernal mode bitwise and the two togeather to see if in kernal mode.
  
  */
int inKernelMode() { 
	return USLOSS_PsrGet() & USLOSS_PSR_CURRENT_MODE;
}

 // finds the first available PID. Used by fork to find pid for new process
int getNewPid(){  
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

/* 
	Wraper function to call the quit function after the process' function has finished.
	Without this wraper the OS would not know when the function finished.
*/
void wrapperFunc(){
	P1_Quit(procTable[pid].startFunc((procTable[pid].startArg)));
	  // //USLOSS_Console("calling dispatcher from wrapper function\n");
	dispatcher();
}

/*
 *  PHASE 1b.
 */

//// 5. Semaphore functions. 
int semTableSearch(char* name)
{
  //  this function searches our table of semaphores. 
  //  returns the integer index of the semaphore, 
  //  -1 if the table does not contain a semaphore with the name passed in as argument .
  int i; for (i = 0; i<P1_MAXSEM ; i++)
  {
	//   //USLOSS_Console("sem table name is %s\n",semTable[i]->name);
    if ( semTable[i] != NULL && strcmp(semTable[i]->name,name) == 0) { return i;}
  }
  return -1;
}

int findSemSpace()
{
  // this function returns the index of the first open location in the semphore table. 
  int i = 0;
  while (semTable[i] != NULL) {
	  i++; 
	  }
	  return i;
}

int procsBlockedOnSem(P1_Semaphore *sem)
{
  // this function checks to see if there are processes waiting on the semaphore.
  Semaphore *castSem = (Semaphore* ) sem;
  return pq_isEmpty(castSem->q) ? 0 : 1;
}

int P1_SemCreate(char* name, unsigned int value, P1_Semaphore* sem)
{

  if (semCount == P1_MAXSEM)      { return -2; }
  if (semTableSearch(name) != -1) { return -1; }
  int inx = findSemSpace();
  Semaphore * newSem = (Semaphore*) malloc(sizeof(Semaphore));
  newSem->name = strdup(name);
  newSem->value = value;
  newSem->q = pq_create();
  *sem = (P1_Semaphore*) newSem;
	//   //USLOSS_Console("sem pointer name is %s\n", P1_GetName(sem));	
  semTable[inx] = newSem;
  semCount++;
  return 0;
}

int P1_SemFree(P1_Semaphore sem)
{
	char * name = P1_GetName(sem);
  int inx  = semTableSearch(name);
  if (inx == -1)              { return -1; }
  if (procsBlockedOnSem(sem)) { return -2; }
  semTable[inx] = NULL;
  free(name);
  free(sem);
  return 0;
}

int P1_P(P1_Semaphore sem)
{
  // proberen
  //

  Semaphore * s = (Semaphore*) sem;
  if (semTableSearch(s->name) == -1) {
	  return -1; 
	} else{
	  while(1) {
			disableInterrupt();
			if (s->value > 0) {
				s->value--;
				break;
			}
			//Move process from ready queue to s->q
			if(procTable[pid].state != 3){
				
			procTable[pid].state = 4;
			}
			pq_push(s->q, pid, 1); // pushing the pid on the semaphore queue. 
			enableInterrupt();
			dispatcher();
		}
		 //  //USLOSS_Console("Broke out of P loop\n");
		enableInterrupt();
		return 0;
	}
}

int P1_V(P1_Semaphore sem)
{
	
	Semaphore * s = (Semaphore*) sem;
	 //  //USLOSS_Console("Ving %s\n", s->name);	
  if (semTableSearch(s->name) == -1) {
	  return -1; 
	} else{
	  disableInterrupt();
	  
	  s->value++;
	  if(!pq_isEmpty(s->q)){
		  //   //USLOSS_Console("sem Q ---  ");	
		  pq_print(s->q);
		    // //USLOSS_Console("Pushing finished V\n");
			int localPID = pq_pop(s->q);
		  pq_push(readyQueue,localPID,1);//procTable[localPID].priority);
		     //USLOSS_Console("NEW QUEUE AFTER V current PID %d in state %d\n", pid, procTable[pid].state);
		  pq_print(readyQueue);
		  dispatcher();
	  }
	  enableInterrupt();
	  return 0;
	}
  
	
  
}

char *P1_GetName(P1_Semaphore sem)
{
  Semaphore * s = (Semaphore*) sem;
  char * retStr = strdup((*s).name);
  return retStr;
}

/* ----------------- Functions to implement Queues -----------*/

priority_queue * pq_create()
{
  // creates and returns a priority queue struct. 
  priority_queue * retq =  malloc(sizeof(priority_queue));
  retq->head = NULL;
  return retq;
}

/* and here are some associated functions to help us use the queue */
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
	  if(new->priority <= curr->priority)
    {
		  // insert at head
		  new->next = pq->head;
		  pq->head = new;
	  }
    else 
    {
	  // insert after head
	    while (curr->next != NULL) 
      {
        if(new->priority > curr->next->priority)
        {
  		    curr = curr->next;
	  	  }
        else 
        {
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
  // pops the highest priority element from the queue
  //  and returns the PID
  if (pq->head == NULL) { return 0; }
  int retVal = pq->head->pid;
  pq->head = pq->head->next;
  return retVal;
}

void pq_print(priority_queue * pq)
{
  // prints the contents of the priority queue 
  //  used for debugging
  p_node * curr = pq-> head;
  while(curr != NULL)
  {
	   //USLOSS_Console(" %d(%d) | ", curr->pid,procTable[curr->pid].priority);
	curr = curr->next;
  }
     //USLOSS_Console("\n");
}

void pq_remove(priority_queue * pq, int thePID)
{
  // removes an element from the queue by PID. 
  // technically breaches the definition of what a priority queue is, 
  // but we are the programmers now. 
  if (pq->head->pid == thePID)
  {
	pq->head = pq->head->next;
  }
  else 
  {
    p_node * curr = pq->head;
	  while (curr->next != NULL) 
    {
	    if(curr->next->pid == thePID)
      {
	  	  curr->next = curr->next->next;
		    return;
	    }
      else 
      {
		    curr = curr->next;
	    }
	  }
  }
}
// quick n dirty func to check if the queue is empty. 
int pq_isEmpty(priority_queue* pq) {
	return (pq->head == NULL) ? 1 : 0;
}


// stuff for interrupts. 

void modInterrupt(int x)
{
}

void enableInterrupt()  { modInterrupt(1); return; } 
void disableInterrupt() { modInterrupt(0); return; }

int  currentInterruptStat()
{
  return 0 ; 
}
