/*
 * user.c
 *
 *  Created on: Feb 21, 2023
 *      Author: peter
 */

/*
 * TODO
 * Solve issues:-
 * ->Mutex semaphore is automatically changing to binary semaphore
 * ->Stack Tracing
 * ->Exception Handling
 */

#include "user.h"
#include "main.h"

#include "task.h"

extern UART_HandleTypeDef huart1;

/////////////////////////////MAIN///////////////////////////////
uint32_t count1;
uint32_t count2;
float fp = 0;

TaskSemaphore semaphore;
void printChar(char ch){
	HAL_UART_Transmit(&huart1, (uint8_t*)&ch, 1, HAL_MAX_DELAY);
}

void printString(char*str,uint32_t len){
	HAL_UART_Transmit(&huart1, (uint8_t*)str, len, HAL_MAX_DELAY);
}

void task1() {
	task_printf("Task1 starting\n");
	count1 = 0;
	while (1) {
		task_printf("I am task1\n");
		count1++;
//		if(count1==4)
//			task_takeSemaphore(&semaphore);
//			task_delete(1);

		task_delay(500);
	}
}

void task2() {
	task_printf("Task2 starting\n");
	count2 = 0;
	while (1) {
		count2++;
		fp = (float)count2/100.0f;
		task_printf("I am task2 %f\n",fp);

		semaphore.type = TASK_SEMAPHORE_MUTEX;
		if(count2==4)
			task_takeSemaphore(&semaphore);
//			task_restart(2);
//			task_deleteSelf();
//			task_enableHighestPriority();
//			task_pause(1);
		if(count2==8)
			task_giveSemaphore(&semaphore);
//			task_resume(1);
//			task_disableHighestPriority();

		task_delay(1000);
	}
}

void init() {
//	printf("Initiating....\n");

	semaphore = task_createSemaphore(TASK_SEMAPHORE_MUTEX);

	task_init();
	task_add(1, task1, 2*1024);
	task_add(2, task2, 2*1024);

	task_startScheduler();

//	printf("I am here\n");
}

void loop() {

}
