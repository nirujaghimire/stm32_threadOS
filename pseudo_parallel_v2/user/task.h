/*
 * tasl.h
 *
 *  Created on: Mar 26, 2023
 *      Author: peter
 */

#ifndef TASK_H_
#define TASK_H_

#include "stdint.h"

typedef enum{
	TASK_SEMAPHORE_BINARY,		// Binary semaphore
	TASK_SEMAPHORE_MUTEX		// Mutex Semaphore
}TaskSemaphoreType;

typedef struct{
	int8_t prevPriority;
	volatile uint8_t take;
	TaskSemaphoreType type;
	struct Task *task;
}TaskSemaphore;

void task_init();
int task_add(uint32_t id,void (*handler)(void),uint32_t stack_size);
int task_restart(uint32_t id);
int task_restartSelf();
int task_pause(uint32_t id);
int task_resume(uint32_t id);
int task_resumeSelf();
int task_delete(uint32_t id);
int task_deleteSelf();
int task_enableHighestPriority();
int task_disableHighestPriority();
void task_delayInMillis(uint32_t millis);
void task_delayInMicros(uint32_t micros);
void task_delay(uint32_t ticks);
void task_startScheduler();
void task_printf(char* msg,...);
void task_return(uint8_t status);

TaskSemaphore task_createSemaphore(TaskSemaphoreType type);
void task_takeSemaphore(TaskSemaphore* semaphore);
void task_giveSemaphore(TaskSemaphore* semaphore);

void task_SysTickHandler();
void task_PendSVHandler();


#endif /* TASK_H_ */
