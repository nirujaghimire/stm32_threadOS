/*
 * tasl.h
 *
 *  Created on: Mar 26, 2023
 *      Author: peter
 */

#ifndef TASK_H_
#define TASK_H_

#include "stdint.h"

void task_init(uint32_t msp, uint32_t total_stack_size, uint32_t main_stack_size);
int task_add(uint32_t id,void (*handler)(void),uint32_t stack_size);
int task_restart(uint32_t id);
int task_pause(uint32_t id);
int task_selfPause();
int task_resume(uint32_t id);
int task_delete(uint32_t id);
int task_enableHighestPriority();
int task_disableHighestPriority();
void task_delay(uint32_t ticks);
void task_startScheduler();

__attribute__((naked)) void task_SVCHandler();
void task_SVCHandlerMain(uint32_t* SP);
void task_SysTickHandler();
__attribute__((naked)) void task_PendSVHandler();


#endif /* TASK_H_ */
