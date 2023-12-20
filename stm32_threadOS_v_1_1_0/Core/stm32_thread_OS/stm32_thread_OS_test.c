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

void SysTick_Handler(void) {
	HAL_IncTick();
	StaticThread.SysTickHandler();
}

///////////////////////////THREAD/////////////////////////
#define STACK_SIZE 256

int id1, id2;

uint32_t stack1[STACK_SIZE];
uint32_t stack2[STACK_SIZE];
uint32_t stack3[STACK_SIZE];

static void calculatSum(int argLen, void **args) {
	int len = (int)args[0];
	int *arr = args[1];
	int *sum = args[2];

	*sum = 0;
	for (int i = 0; i < len; i++) {
		StaticThread.delay(500);
		*sum += arr[i];
	}

	StaticThread.delete(0);
}

static void task1(int argLen, void **args) {
	StaticThread.print("%s(INIT) : %d-%p\n", __func__, argLen, args);
	int sum = 0;
	int arr[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	void *parameters[3] = {(void*)10, arr, &sum};
	StaticThread.new(calculatSum, stack3, sizeof(stack3) / sizeof(uint32_t), 3,
			parameters);

	while (1) {
		StaticThread.print("%s : %d\n", __func__, sum);
		StaticThread.delay(1000);
	}
}

static void task2(int argLen, void **args) {
	StaticThread.print("%s(INIT) : %d-%p\n", __func__, argLen, args);
	int y = 0;
	while (1) {
		y++;
		StaticThread.print("%s : %d\n", __func__, y);
		StaticThread.delay(1000);

	}
}

void run() {
	printf("Initiating....\n");
	HAL_Delay(3000);

	id1 = StaticThread.new(task1, stack1, sizeof(stack1) / sizeof(uint32_t), 0,
			NULL);
	id2 = StaticThread.new(task2, stack2, sizeof(stack2) / sizeof(uint32_t), 0,
			NULL);
	StaticThread.startScheduler();
}

