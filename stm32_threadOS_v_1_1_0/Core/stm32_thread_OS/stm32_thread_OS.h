/*
 * threadOS.h
 *
 *  Created on: Oct 21, 2023
 *      Author: peter
 */

#ifndef STM32_THREAD_OS_THREADOS_H_
#define STM32_THREAD_OS_THREADOS_H_

#include "stdint.h"

#define MAX_THREAD 5


struct STM32ThreadControl{
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
	int (*new)(void (*threadTask)(int argLen,void**args), uint32_t *stack, uint32_t stackLen,int argLen,void**args);

	/**
	 * This starts threading
	 */
	void (*startScheduler)();

	/**
	 * This deletes this thread
	 * @param threadID  : ID of thread to be deleted
	 * 					: 0 for self delete
	 */
	void (*delete)(int threadID);

	/**
	 * This restarts this thread
	 * @param threadID  : ID of thread to be restarted
	 * 					: 0 for self restart
	 */
	void (*restart)(int threadID);

	/**
	 * This blocks this thread
	 * @param threadID  : ID of thread to be blocked
	 * 					: 0 for self block
	 */
	void (*block)(int threadID);

	/**
	 * This unblocks this thread
	 * @param threadID  : ID of thread to be unblocked
	 */
	void (*unblock)(int threadID);

	/**
	 * This is delay for thread which do not comsume clock cycle
	 * @param millis	 : duration for delay in millis
	 */
	void (*delay)(uint32_t millis);

	/**
	 * This is printf for thread which prints completely this text
	 */
	void (*print)(const char *msg, ...);

	/**
	 * This blocks all thread except current thread (this thread)
	 */
	void (*mutexLock)();

	/**
	 * This unlocks mutex lock
	 */
	void (*mutexUnlock)();

	/**
	 * This blocks this thread unless other thread give binary semaphore for this thread
	 * @param threadIDptr	: binary semaphore ID which is also thread ID of this thread
	 */
	void (*takeBinarySemaphore)(int *threadIDptr);

	/**
	 * This blocks this thread corresponding to this threadID
	 * @param threadID	: binary semaphore ID which is also thread ID of this thread to be unblocked
	 */
	void (*giveBinarySemaphore)(int threadID);

	//////////////////////HANDLER//////////////////////////
	/**
	 * This should be called in SVC_Handler
	 */
	void (*SVCHandler)();
	/**
	 * This should be called in SysTick_Handler
	 */
	void (*SysTickHandler)();

	/**
	 * This should be called in PendSV_Handler
	 */
	void (*PendSVHandler)();
};

extern struct STM32ThreadControl StaticThread;

#endif /* STM32_THREAD_OS_THREADOS_H_ */
