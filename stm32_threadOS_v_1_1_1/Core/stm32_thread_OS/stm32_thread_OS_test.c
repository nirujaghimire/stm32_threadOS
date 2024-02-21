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

void SVC_Handler(void) {
	StaticThread.SVCHandler();
}

void PendSV_Handler(void) {
	StaticThread.PendSVHandler();
}

int tick = 0;
void SysTick_Handler(void) {
	HAL_IncTick();
	tick++;
	if(tick==10){
		StaticThread.SysTickHandler();
		tick= 0;
	}
}

///////////////////////////THREAD/////////////////////////
#define STACK_SIZE 256

int id1, id2, id3;

uint32_t stack1[STACK_SIZE];
uint32_t stack2[STACK_SIZE];
uint32_t stack3[STACK_SIZE];

volatile int count = 0;
static void task1(int argLen, void **args) {
	StaticThread.print("%s(INIT) : %d-%p\n", __func__, argLen, args);
	while (1) {
		StaticThread.mutexLock();
		count++;
		StaticThread.mutexUnlock();
		StaticThread.delay(10);
	}
}

static void task2(int argLen, void **args) {
	StaticThread.print("%s(INIT) : %d-%p\n", __func__, argLen, args);
	while (1) {
		while (count % 10 != 0) {
			StaticThread.spin();
		}
		StaticThread.print("Count : %d\n", count);
		StaticThread.mutexLock();
		count++;
		StaticThread.mutexUnlock();
	}
}

static void monitoringTask(int argLen, void **args) {
	StaticThread.print("%s(INIT) : %d-%p\n", __func__, argLen, args);
	while (1) {
		StaticThread.print("UF : %d\n",
				(int) (100 * StaticThread.utilization()));
		StaticThread.delay(1000);
	}
}

void run() {
	printf("Initiating....\n");
	HAL_Delay(3000);

	id1 = StaticThread.new(task1, stack1, sizeof(stack1) / sizeof(uint32_t), 0,NULL);
	id2 = StaticThread.new(task2, stack2, sizeof(stack2) / sizeof(uint32_t), 0,NULL);
	id3 = StaticThread.new(monitoringTask, stack3,sizeof(stack3) / sizeof(uint32_t), 0,NULL);
	StaticThread.startScheduler();
}

