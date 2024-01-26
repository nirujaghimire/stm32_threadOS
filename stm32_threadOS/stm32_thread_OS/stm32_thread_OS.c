/*
 * threadOS.c
 *
 *  Created on: Oct 21, 2023
 *      Author: peter
 */

#include "stm32_thread_OS.h"
#include "main.h"
#include "stdio.h"
#include "stdarg.h"

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
	uint32_t sp;
	STM32ThreadAction action;
	void (*threadFunc)();
	uint32_t waitTill;
} STM32Thread;

static volatile STM32Thread thread[MAX_THREAD];
static volatile int currentThread = -1;
static volatile int countThread;
static volatile uint8_t mutexLock = 0;

static uint32_t idleThreadStack[64];
uint32_t stm32_thread_idle_count;
static void idleThread() {
	stm32_thread_idle_count = 0;
	while (1) {
		stm32_thread_idle_count++;
	}
}

static void addThread(int freeIndex, void (*threadTask)(), uint32_t *stack,
		uint32_t stackLen) {
	int i = freeIndex;
	//Empty thread found
	thread[i].ID = freeIndex;
	thread[i].stack = stack;
	thread[i].stackLen = stackLen;
	thread[i].threadFunc = threadTask;
	thread[i].action = STM32_THREAD_ACTION_RUNNING;

	uint32_t *psp = (uint32_t*) &stack[stackLen];

	// fill dummy stack frame
	*(--psp) = 0x01000000u; // Dummy xPSR, just enable Thumb State bit;
	*(--psp) = (uint32_t) threadTask; // PC
	*(--psp) = 0xFFFFFFFDu; // LR with EXC_RETURN to return to Thread using PSP
	*(--psp) = 0x12121212u; // Dummy R12
	*(--psp) = 0x03030303u; // Dummy R3
	*(--psp) = 0x02020202u; // Dummy R2
	*(--psp) = 0x01010101u; // Dummy R1
	*(--psp) = 0x00000000u; // Dummy R0

	*(--psp) = 0x11111111u; // Dummy R11
	*(--psp) = 0x10101010u; // Dummy R10
	*(--psp) = 0x09090909u; // Dummy R9
	*(--psp) = 0x08080808u; // Dummy R8
	*(--psp) = 0x07070707u; // Dummy R7
	*(--psp) = 0x06060606u; // Dummy R6
	*(--psp) = 0x05050505u; // Dummy R5
	*(--psp) = 0x04040404u; // Dummy R4
	thread[i].sp = (uint32_t) psp;
}

/**
 * This adds the thread
 * @param threadTask	: Infinite function for thread  (!!! should be infinite running)
 * @param stack			: Pointer to stack array
 * @param stackLen		: Length of stack array
 * @return				: ID of created thread (This ID is required for restart or delete of thread)
 * 						: -1 if MAX_THREAD limit exceeds
 */
static int new(void (*threadTask)(), uint32_t *stack, uint32_t stackLen) {
	mutexLock = 1;
	int freeIndex = 1;
	for (; freeIndex < MAX_THREAD; freeIndex++)
		if (thread[freeIndex].ID == 0)
			break; //empty thread found
	if (freeIndex >= MAX_THREAD)
		return -1;
	addThread(freeIndex, threadTask, stack, stackLen);
	mutexLock = 0;
	return freeIndex;
}

/**
 * This starts threading
 */
static void startScheduler() {
	//Adding idle thread
	addThread(0, idleThread, idleThreadStack,
			sizeof(idleThreadStack) / sizeof(uint32_t));

	NVIC_SetPriority(PendSV_IRQn, 0xFF);

	uint32_t sp = thread[0].sp;

	__asm volatile("MOV R0, %0"::"r"(sp));
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

	currentThread = 0;
	void (*task)() = (void (*)())((uint32_t*)sp)[14];
	task();
}

static void threadSwitching() {
	if (mutexLock)
		return;
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
					thread[currentThread].stackLen);
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
	__asm volatile("MOV %0, R0":"=r"(thread[currentThread].sp));

	/* Scheduling */
	threadSwitching();

	/* Retrieve the context of next task */

	// get its past PSP value
	__asm volatile("MOV R0, %0"::"r"(thread[currentThread].sp));
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
 * @param threadID : ID of thread to be deleted
 */
static void threadDelete(int threadID) {
	mutexLock = 1;
	if (threadID > 0 && threadID<MAX_THREAD)
		thread[threadID].action = STM32_THREAD_ACTION_DELETE;
	mutexLock = 0;
	if (threadID == 0 && threadID != currentThread)
		return;
	reschedule();
}

/**
 * This restarts this thread
 * @param threadID : ID of thread to be restarted
 */
static void threadRestart(int threadID) {
	mutexLock = 1;
	if (threadID > 0 && threadID<MAX_THREAD)
		thread[threadID].action = STM32_THREAD_ACTION_RESTART;
	mutexLock = 0;
	if (threadID == 0 && threadID != currentThread)
		return;
	reschedule();
}

/**
 * This blocks this thread
 * @param threadID : ID of thread to be blocked
 */
static void threadBlock(int threadID) {
	mutexLock = 1;
	if (threadID > 0 && threadID<MAX_THREAD)
		thread[threadID].action = STM32_THREAD_ACTION_BLOCK;
	mutexLock = 0;
	if (threadID == 0 && threadID != currentThread)
		return;
	reschedule();
}

/**
 * This unblocks this thread
 * @param threadID : ID of thread to be unblocked
 */
static void threadUnblock(int threadID) {
	mutexLock = 1;
	if (threadID > 0 && threadID<MAX_THREAD)
		thread[threadID].action = STM32_THREAD_ACTION_RUNNING;
	mutexLock = 0;
}

/**
 * This is delay for thread which do not comsume clock cycle
 * @param millis	: duration for delay in millis
 */
static void threadDelay(uint32_t millis) {
	mutexLock = 1;
	if (currentThread > 0 && currentThread<MAX_THREAD)
		thread[currentThread].waitTill = HAL_GetTick() + millis;
	mutexLock = 0;
	reschedule();
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
	mutexLock = 1;
}

/**
 * This unlocks mutex lock
 */
static void threadMutexUnlock(){
	mutexLock = 0;
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


struct STM32ThreadControl StaticThread = {
		.new = new,
		.startScheduler = startScheduler,
		.delete = threadDelete,
		.restart = threadRestart,
		.block  = threadBlock,
		.unblock = threadUnblock,
		.delay = threadDelay,
		.print = threadPrint,
		.mutexLock = threadMutexLock,
		.mutexUnlock = threadMutexUnlock,
		.takeBinarySemaphore = threadTakeBinarySemaphore,
		.giveBinarySemaphore = threadGiveBinarySemaphore,

		.SVCHandler = threadSVCHandler,
		.SysTickHandler = threadSysTickHandler,
		.PendSVHandler = threadPendSVHandler
};

