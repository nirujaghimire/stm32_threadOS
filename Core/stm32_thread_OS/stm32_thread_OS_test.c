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
uint32_t thread3Stack[STACK_SIZE];
uint32_t monitoringThreadStack[STACK_SIZE];

typedef struct {
	char name[8];
	int price;
} Fruit;

volatile Fruit fruit = { 0 };

char apple[] = "Apple";
char mango[] = "Mango";
static void change(int state) {
//	StaticThread.mutexLock();

	static int flag = 0;
	StaticThread.synchronise(&flag);

	if (state) {
		for (int i = 0; i < sizeof(apple); i++) {
			fruit.name[i] = apple[i];
			StaticThread.delay(227);
		}
		fruit.price = 10;
		StaticThread.delay(314);
	} else {
		for (int i = 0; i < sizeof(mango); i++) {
			fruit.name[i] = mango[i];
			StaticThread.delay(167);
		}
		fruit.price = 20;
		StaticThread.delay(238);
	}

	flag = 0;

//	StaticThread.mutexUnlock();
}

static void thread1(int argLen, void **args) {
	StaticThread.print("%s(INIT) : %d-%p\n", __func__, argLen, args);
	while (1) {
		change(1);
		StaticThread.print("%s: Fruit: %s : %d\n", __func__, fruit.name, fruit.price);
		StaticThread.delay(333);
	}
}

static void thread2(int argLen, void **args) {
	StaticThread.print("%s(INIT) : %d-%p\n", __func__, argLen, args);
	while (1) {
		change(0);
		StaticThread.print("%s: Fruit: %s : %d\n", __func__, fruit.name, fruit.price);
		StaticThread.delay(333);
	}
}

static void thread3(int argLen, void **args) {
	StaticThread.print("%s(INIT) : %d-%p\n", __func__, argLen, args);
	while (1) {
		change(0);
		StaticThread.print("%s: Fruit: %s : %d\n", __func__, fruit.name, fruit.price);
		StaticThread.delay(333);
	}
}

static void monitoringthread(int argLen, void **args) {
	StaticThread.print("%s(INIT) : %d-%p\n", __func__, argLen, args);
	while (1) {
		StaticThread.delay(10000);
		StaticThread.print("\nCPU UF : %d\n", (int) (100 * StaticThread.cpuUtilization()));
		for (int id = 1; id <= 4; id++)
			StaticThread.print("%d: STACK UF: %d\n", id, (int) (100 * StaticThread.stackUtilization(id)));
	}
}

void run() {
	printf("Initiating...\n");
	HAL_Delay(1000);

	id1 = StaticThread.new(thread1, thread1Stack, sizeof(thread1Stack) / sizeof(uint32_t), 0, NULL);
	id2 = StaticThread.new(thread2, thread2Stack, sizeof(thread2Stack) / sizeof(uint32_t), 0, NULL);
	id2 = StaticThread.new(thread3, thread3Stack, sizeof(thread3Stack) / sizeof(uint32_t), 0, NULL);
	id3 = StaticThread.new(monitoringthread, monitoringThreadStack, sizeof(monitoringThreadStack) / sizeof(uint32_t), 0, NULL);
	StaticThread.startScheduler();
}
