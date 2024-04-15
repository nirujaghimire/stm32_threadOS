/*
 * stm32_thread_OS.c
 *
 *  Created on: Oct 21, 2023
 *      Author: Niruja
 */

#include "stm32_thread_OS.h"
#include "main.h"
#include "stdio.h"
#include "stdarg.h"

#define STACK_OFFSET 32

typedef enum {
	STM32_THREAD_ACTION_BLOCK = 0,
	STM32_THREAD_ACTION_RUNNING,
	STM32_THREAD_ACTION_DELETE,
	STM32_THREAD_ACTION_RESTART
} STM32ThreadAction;

typedef struct {
	uint8_t ID;
	uint32_t *stack;
	uint32_t stackLen;
	uint32_t SP;
	STM32ThreadAction action;
	void (*threadFunc)(int argLen,void**args);
	uint32_t waitTill;
	int argLen;
	void **args;
	uint32_t timeTillNow;
} STM32Thread;

static volatile STM32Thread thread[MAX_THREAD];
static volatile int currentThread = -1;
static volatile int countThread;
static volatile uint8_t mutexLock = 0;
static volatile uint8_t userMutexLock = 0;
static volatile uint32_t taskTime = 0;


static uint32_t idleThreadStack[64+STACK_OFFSET];
uint32_t stm32_thread_idle_count;
static void idleThread() {
	stm32_thread_idle_count = 0;
	while (1) {
		stm32_thread_idle_count++;
	}
}

static void addThread(int freeIndex, void (*threadTask)(), uint32_t *stack,
		uint32_t stackLen,int argLen,void**args) {
	int i = freeIndex;
	//Empty thread found
	thread[i].ID = freeIndex;
	thread[i].stack = stack;
	thread[i].stackLen = stackLen;
	thread[i].threadFunc = threadTask;
	thread[i].action = STM32_THREAD_ACTION_RUNNING;
	thread[i].argLen = argLen;
	thread[i].args = args;
	thread[i].timeTillNow = 0;


	uint32_t *psp = (uint32_t*) &stack[stackLen-STACK_OFFSET];

	// fill dummy stack frame
	*(--psp) = 0x01000000u; // Dummy xPSR, just enable Thumb State bit;
	*(--psp) = (uint32_t) threadTask; // PC
	*(--psp) = 0xFFFFFFFDu; // LR with EXC_RETURN to return to Thread using PSP
	*(--psp) = 0x12121212u; // Dummy R12
	*(--psp) = 0x03030303u; // Dummy R3
	*(--psp) = 0x02020202u; // Dummy R2
	*(--psp) = (uint32_t)args; // Dummy R1
	*(--psp) = (uint32_t)argLen; // Dummy R0

	*(--psp) = 0x11111111u; // Dummy R11
	*(--psp) = 0x10101010u; // Dummy R10
	*(--psp) = 0x09090909u; // Dummy R9
	*(--psp) = 0x08080808u; // Dummy R8
	*(--psp) = 0x07070707u; // Dummy R7
	*(--psp) = 0x06060606u; // Dummy R6
	*(--psp) = 0x05050505u; // Dummy R5
	*(--psp) = 0x04040404u; // Dummy R4
	thread[i].SP = (uint32_t) psp;
}

/**
 * This adds the thread
 * @param threadTask	: Infinite function for thread  (!!! should be infinite running)
 * @param stack			: Pointer to stack array
 * @param stackLen		: Length of stack array
 * @param argLen		: Length of arguments
 * @param args			: Pointer to arguments
 * @return				: ID of created thread (This ID is required for restart or delete of thread)
 * 						: -1 if MAX_THREAD limit exceeds
 */
static int new(void (*threadTask)(int argLen,void**args), uint32_t *stack, uint32_t stackLen,int argLen,void**args) {
	mutexLock = 1;
	int freeIndex = 1;
	for (; freeIndex < MAX_THREAD; freeIndex++)
		if (thread[freeIndex].ID == 0)
			break; //empty thread found
	if (freeIndex >= MAX_THREAD)
		return -1;
	addThread(freeIndex, threadTask, stack, stackLen,argLen,args);
	mutexLock = 0;
	return freeIndex;
}

/**
 * This starts threading
 */
static void startScheduler() {
	// Adding idle thread
	addThread(0, idleThread, idleThreadStack,
			sizeof(idleThreadStack) / sizeof(uint32_t),0,NULL);
	NVIC_SetPriority(PendSV_IRQn, 0xFF);

	currentThread = 0;
	for(int i=1;i<MAX_THREAD;i++){
		if(thread[i].ID!=0){
			currentThread=i;
			break;// Non empty thread found
		}
	}

	uint32_t SP = thread[currentThread].SP;

	__asm volatile("MOV R0, %0"::"r"(SP));
	__asm volatile("MSR PSP, R0");

	//Stack to PSP
	__asm volatile(
			"MRS R0, CONTROL\n\t"
			"ORR R0, R0, #2\n\t"
			"MSR CONTROL, r0"
	);

	//Unprivileged Mode
	__asm volatile(
			"MRS R0, CONTROL\n\t"
			"ORR R0, R0, #1\n\t"
			"MSR CONTROL, r0"
	);

	void (*task)(int argLen,void**args) = (void (*)(int argLen,void**args))((uint32_t*)SP)[14];
	task(thread[currentThread].argLen,thread[currentThread].args);
}

static void threadSwitching() {
	//Round Robin Heuristics
	if (mutexLock || userMutexLock)
		return;

	taskTime = HAL_GetTick()-taskTime;
	thread[currentThread].timeTillNow+=taskTime;

	for (countThread = 0; countThread <= MAX_THREAD; countThread++) {
		currentThread = (currentThread + 1) % MAX_THREAD;
		if (thread[currentThread].ID == 0)
			continue; //Empty or idle thread

		if (HAL_GetTick() <= thread[currentThread].waitTill)
			continue; //Delay has been executing

		if (thread[currentThread].action == STM32_THREAD_ACTION_DELETE) {
			thread[currentThread].ID = 0;
			continue;
		} else if (thread[currentThread].action
				== STM32_THREAD_ACTION_RESTART) {
			addThread(currentThread, thread[currentThread].threadFunc,
					thread[currentThread].stack,
					thread[currentThread].stackLen,thread[currentThread].argLen,thread[currentThread].args);
		} else if (thread[currentThread].action == STM32_THREAD_ACTION_BLOCK) {
			continue;
		}

		countThread = 0;
		break;
	}

	if (countThread > 0) {
		//No thread remaining so assign idle
		currentThread = 0;
	}
	taskTime = HAL_GetTick();
}

/**
 * This should be called in SysTick_Handler
 */
void threadSysTickHandler() {
	if (currentThread >= 0)
		SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

/**
 * This should be called in PendSV_Handler
 */
__attribute__((naked)) void threadPendSVHandler() {
	__asm volatile("POP {R7,LR}");

	//Making this function as if __attribute__((naked))
	__asm volatile("PUSH {LR}");

	/* Save the context of current task */

	// get current PSP
	__asm volatile("MRS R0, PSP");
	// save R4 to R11 to PSP Frame Stack
	__asm volatile("STMDB R0!, {R4-R11}");
	// save current value of PSP
	__asm volatile("MOV %0, R0":"=r"(thread[currentThread].SP));

	/* Scheduling */
	threadSwitching();

	/* Retrieve the context of next task */

	// get its past PSP value
	__asm volatile("MOV R0, %0"::"r"(thread[currentThread].SP));
	// retrieve R4-R11 from PSP Fram Stack
	__asm volatile("LDMIA R0!, {R4-R11}");
	// update PSP
	__asm volatile("MSR PSP, R0");

	__asm volatile("POP {LR}");
	__asm volatile("NOP");
	__asm volatile("BX LR");
}

static void reschedule(){
	__asm volatile("SVC #0");
}

/**
 * This should be called in SVC_Handler
 */
void threadSVCHandler(){
	if (currentThread >= 0)
		SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

//////////////////////////UTILS/////////////////////////
/**
 * This deletes this thread
 * @param threadID 	: ID of thread to be deleted
 * 					: 0 for self delete
 */
static void threadDelete(int threadID) {
	mutexLock = 1;

	if(threadID==0)
		threadID = currentThread;

	if (threadID > 0 && threadID<MAX_THREAD)
		thread[threadID].action = STM32_THREAD_ACTION_DELETE;
	mutexLock = 0;
	if (threadID == 0 && threadID != currentThread)
		return;
	reschedule();
}


/**
 * This restarts this thread
 * @param threadID 	: ID of thread to be restarted
 * 					: 0 for self restart
 */
static void threadRestart(int threadID) {
	mutexLock = 1;
	if(threadID==0)
		threadID = currentThread;
	if (threadID > 0 && threadID<MAX_THREAD)
		thread[threadID].action = STM32_THREAD_ACTION_RESTART;
	mutexLock = 0;
	if (threadID == 0 && threadID != currentThread)
		return;
	reschedule();
}

/**
 * This blocks this thread
 * @param threadID  : ID of thread to be blocked
 * 					: 0 for self block
 */
static void threadBlock(int threadID) {
	mutexLock = 1;
	if(threadID==0)
		threadID = currentThread;
	if (threadID > 0 && threadID<MAX_THREAD)
		thread[threadID].action = STM32_THREAD_ACTION_BLOCK;
	mutexLock = 0;
	if (threadID == 0 && threadID != currentThread)
		return;
	reschedule();
}

/**
 * This unblocks this thread
 * @param threadID  : ID of thread to be unblocked
 */
static void threadUnblock(int threadID) {
	mutexLock = 1;
	if (threadID > 0 && threadID<MAX_THREAD)
		thread[threadID].action = STM32_THREAD_ACTION_RUNNING;
	mutexLock = 0;
}

/**
 * This is delay for thread which do not comsume clock cycle
 * @param millis	 : duration for delay in millis
 */
static void threadDelay(uint32_t millis) {
	mutexLock = 1;
	if (currentThread > 0 && currentThread<MAX_THREAD)
		thread[currentThread].waitTill = HAL_GetTick() + millis;
	mutexLock = 0;
	reschedule();
	while(thread[currentThread].waitTill>HAL_GetTick()){

	}
}

/**
 * This is printf for thread which prints completely this text
 */
static void threadPrint(const char *msg, ...) {
	mutexLock = 1;
	va_list args;
	va_start(args, msg);
	vprintf(msg, args);
	va_end(args);
	mutexLock = 0;
}

/**
 * This blocks all thread except current thread (this thread)
 */
static void threadMutexLock(){
	userMutexLock = 1;
}

/**
 * This unlocks mutex lock
 */
static void threadMutexUnlock(){
	userMutexLock = 0;
}


/**
 * This blocks this thread unless other thread give binary semaphore for this thread
 * @param threadIDptr	: binary semaphore ID which is also thread ID of this thread
 */
static void threadTakeBinarySemaphore(int *threadIDptr){
	mutexLock = 1;
	if(threadIDptr!=NULL && (currentThread>0 && currentThread<MAX_THREAD)){
		*threadIDptr = currentThread;
		thread[currentThread].action = STM32_THREAD_ACTION_BLOCK;
	}
	mutexLock = 0;
	reschedule();
}

/**
 * This blocks this thread corresponding to this threadID
 * @param threadID	: binary semaphore ID which is also thread ID of this thread to be unblocked
 */
static void threadGiveBinarySemaphore(int threadID){
	mutexLock = 1;
	if(threadID>0 && threadID<MAX_THREAD)
		thread[threadID].action = STM32_THREAD_ACTION_RUNNING;
	mutexLock = 0;
}

/**
 * It gives the CPU utilization factor (0~1)
 */
static float cpuUtilization(){
	mutexLock = 1;
	uint32_t totalTime = thread[0].timeTillNow;
	for(int i=1;i<MAX_THREAD;i++){
		if(thread[i].ID!=0){
			totalTime+=thread[i].timeTillNow;
		}
	}
	float uf = 1- (float)thread[0].timeTillNow/(float)totalTime;
	mutexLock = 0;
	return uf;
}


/**
 * It gives the stack utilization factor (0~1) all the value of stack should be 0 initially
 * @param threadID 	: ID of thread
 * 					: 0 for self stack utilization
 * return			: stack utilization factor (0~1)
 * 					: -1 if threadID doesn't exist
 */
static float stackUtilization(int threadID){
	mutexLock = 1;
	if(threadID==0)
		threadID = currentThread;
	if(threadID>=MAX_THREAD){
		mutexLock = 0;
		return -1.0f;
	}
	STM32Thread t = thread[threadID];
	if(t.ID==0){
		mutexLock = 0;
		return -1.0f;
	}
	int peakLen=0;
	for(int i=0;i<t.stackLen;i++){
		if(t.stack[i]==0)
			continue;
		peakLen = (int)t.stackLen-i;
		break;
	}
	float uf = (float)peakLen/(float)t.stackLen;
	mutexLock = 0;
	return uf;
}

/**
 * It is should be called during waiting in while loop
 */
static void spin(){
	reschedule();
}

/**
 * This will make sure the function containing this can be called by only one task
 * This doesn't block all other tasks like mutex lock
 * @param flag : pointer to static or global variable (Should be made 0 initially and at the end of function !!!)
 * e.g.
 * 	void func(){
 * 		static int flag = 0;
 * 		StaticThread.synchronise(&flag);
 * 		// Do stuff
 * 		flag = 0;
 * 	}
 */
static void synchronise(int *flag){
	while((*flag)!=0) StaticThread.spin();
	*flag = 1;
}


/**
 * It is generally called from hardfault or from handler for stack tracing
 * @param threadID 		: threadID
 * 				   		: 0 for hardfault causing thread
 * 				   		: -1 for print all thread stack of nonempty task
 * @param isFromHandler : 1 for calling from handler
 * 						: 0 for calling from thread
 */
static void printStack(int threadID,int isFromHandler){
	if(threadID>=MAX_THREAD || threadID<-1){
		printf("Thread ID doesn't exist\n");
		return;
	}
	if(threadID!=0 && threadID!=-1){
        if( thread[threadID].ID==0) {
            printf("Empty thread\n");
            return;
        }
	}
	int ID = threadID;
	if(threadID==0 || threadID==-1)
		ID = currentThread;

	uint32_t PSP = thread[ID].SP;
	if(isFromHandler){
		__asm volatile("MRS R0, PSP");
		__asm volatile("MOV %0,R0":"=r"(PSP));
	}

	STM32Thread t = thread[ID];
	printf("ID : %d\tPSP : 0x%x\n",ID,(int)PSP);
	int ptr = (int)((PSP-((uint32_t)t.stack))/sizeof(uint32_t));
	for(int i=ptr; i<t.stackLen; i++)
		printf("%4d: 0x%x: 0x%x\n",i,(int)(&t.stack[i]),(int)t.stack[i]);


	if(threadID== -1){
		for(int n=1;n<MAX_THREAD;n++){
			if(n==currentThread)
				continue;//Above thread already printed
			t = thread[n];
			if(t.ID==0)
				continue;//empty thread
			PSP = t.SP;
			printf("ID : %d\tPSP : 0x%x\n",t.ID,(int)PSP);
			ptr = (int)((PSP-((uint32_t)t.stack))/sizeof(uint32_t));
			for(int i=ptr; i<t.stackLen; i++)
				printf("%4d: 0x%x: 0x%x\n",i,(int)(&t.stack[i]),(int)t.stack[i]);
		}
	}
}

struct STM32ThreadControl StaticThread = {
		.new = new,
		.startScheduler = startScheduler,
		.delete = threadDelete,
		.restart = threadRestart,
		.block  = threadBlock,
		.unblock = threadUnblock,
		.delay = threadDelay,
		.print = threadPrint,
		.print = threadPrint,
		.mutexLock = threadMutexLock,
		.mutexUnlock = threadMutexUnlock,
		.takeBinarySemaphore = threadTakeBinarySemaphore,
		.giveBinarySemaphore = threadGiveBinarySemaphore,
		.cpuUtilization = cpuUtilization,
		.stackUtilization = stackUtilization,
		.spin = spin,
		.synchronise = synchronise,
		.printStack = printStack,

		.SVCHandler = threadSVCHandler,
		.SysTickHandler = threadSysTickHandler,
		.PendSVHandler = threadPendSVHandler
};

