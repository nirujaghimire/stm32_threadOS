/*
 * user.c
 *
 *  Created on: Oct 21, 2023
 *      Author: peter
 */

#include "stm32_thread_OS.h"
#include "main.h"
#include "stdio.h"
#include "stdarg.h"

extern UART_HandleTypeDef huart1;

int _write(int file, char *data, int len) {
	StaticThread.mutexLock();
	HAL_UART_Transmit(&huart1, (uint8_t*) data, len, HAL_MAX_DELAY);
	StaticThread.mutexUnlock();
	return len;
}


///////////////////////////HANDLER///////////////////////
void HardFault_Handler(void) {
	printf("Hard Fault\n");
	while (1) {

	}
}

void SVC_Handler(void){
	StaticThread.SVCHandler();
}

void PendSV_Handler(void) {
	StaticThread.PendSVHandler();
}

void SysTick_Handler(void) {
	HAL_IncTick();
	StaticThread.SysTickHandler();
}

///////////////////////////THREAD/////////////////////////
#define STACK_SIZE 256

int id1,id2;
int x,y;
int semaphore;

uint32_t stack1[STACK_SIZE];
uint32_t stack2[STACK_SIZE];



static void task1() {
	StaticThread.print("Task1 initiating...\n");
	x = 0;
	while (1) {
		x++;
		StaticThread.print("x = %d\n", x);
		StaticThread.delay(1000);
		if(x==5){
			StaticThread.print("Task1 taking semaphore\n");
			StaticThread.takeBinarySemaphore(&semaphore);
		}else if (x == 10){
			StaticThread.print("Task1 restarting by itself\n");
			StaticThread.restart(id1);
		}
	}
}

static void task2() {
	StaticThread.print("Task2 initiating...\n");
	y = 0;
	while (1) {
		y++;
		StaticThread.print("y = %d\n", y);
		StaticThread.delay(1000);
		if(y==10){
			StaticThread.print("Task2 give semaphore\n");
			StaticThread.giveBinarySemaphore(semaphore);
		}else if(y==15){
			StaticThread.print("Task1 is being deleted by task2 \n");
			StaticThread.delete(id1);
		}else if(y==20){
			StaticThread.print("Task1 is being added by task2 \n");
			id1 = StaticThread.new(task1, stack1, sizeof(stack1) / sizeof(uint32_t));
		}

	}
}

void run() {
	printf("Initiating....\n");
	HAL_Delay(3000);

	id1 = StaticThread.new(task1, stack1, sizeof(stack1) / sizeof(uint32_t));
	id2 = StaticThread.new(task2, stack2, sizeof(stack2) / sizeof(uint32_t));
	StaticThread.startScheduler();
}

