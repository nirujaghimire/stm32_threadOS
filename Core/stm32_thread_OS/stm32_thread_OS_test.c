/*
 * stm32_thread_OS_test.c
 *
 *  Created on: Oct 21, 2023
 *      Author: Niruja
 */

#include "stm32_thread_OS.h"
#include "main.h"
#include "stdio.h"
#include "string.h"

extern UART_HandleTypeDef huart1;

int _write(int file, char *data, int len) {
	HAL_UART_Transmit(&huart1, (uint8_t*) data, len, HAL_MAX_DELAY);
	return len;
}

///////////////////////////HANDLER///////////////////////
void HardFault_Handler(void) {
	printf("Hard Fault\n");
	StaticThread.printStack(0);

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

int id1, id2, id3;

uint32_t thread1Stack[STACK_SIZE];
uint32_t thread2Stack[STACK_SIZE];
uint32_t monitoringThreadStack[STACK_SIZE];

volatile int count = 0;
struct {
	char name[8];
	int price;
} fruit;

static void thread1(int argLen, void **args) {
	StaticThread.print("%s(INIT) : %d-%p\n", __func__, argLen, args);
	while (1) {
		int *x = NULL;
		*x = 0;

//		StaticThread.mutexLock();
//		strcpy(fruit.name, "Apple\0");
//		StaticThread.delay(1000);
//		fruit.price = 10;
//		StaticThread.print("%s : %d\n", fruit.name, fruit.price);
//		StaticThread.mutexUnlock();
		StaticThread.delay(10);
	}
}

static void thread2(int argLen, void **args) {
	StaticThread.print("%s(INIT) : %d-%p\n", __func__, argLen, args);
	while (1) {
//		StaticThread.mutexLock();
//		strcpy(fruit.name, "Mango\0");
//		StaticThread.delay(1000);
//		fruit.price = 20;
//		StaticThread.print("%s : %d\n", fruit.name, fruit.price);
//		StaticThread.mutexUnlock();
		StaticThread.delay(10);
	}
}

static void monitoringthread(int argLen, void **args) {
	StaticThread.print("%s(INIT) : %d-%p\n", __func__, argLen, args);
	while (1) {
		StaticThread.print("UF : %d\n", (int) (100 * StaticThread.utilization()));
//		StaticThread.print("%s : %d\n", fruit.name, fruit.price);
		StaticThread.delay(1000);
	}
}

void run() {
	printf("Initiating...\n");
	HAL_Delay(1000);

	id1 = StaticThread.new(thread1, thread1Stack, sizeof(thread1Stack) / sizeof(uint32_t), 0, NULL);
	id2 = StaticThread.new(thread2, thread2Stack, sizeof(thread2Stack) / sizeof(uint32_t), 0, NULL);
	id3 = StaticThread.new(monitoringthread, monitoringThreadStack, sizeof(monitoringThreadStack) / sizeof(uint32_t), 0, NULL);
	StaticThread.startScheduler();
}

