# Code
```c
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

int id1, id2;
uint32_t thread1Stack[STACK_SIZE];
uint32_t thread2Stack[STACK_SIZE];

static void thread1(int argLen, void **args) {
	StaticThread.print("%s(INIT) : %d-%p\n", __func__, argLen, args);
	while (1) {
		StaticThread.print("Thread : %s\n", __func__);
		StaticThread.delay(500);
	}
}

static void thread2(int argLen, void **args) {
	StaticThread.print("%s(INIT) : %d-%p\n", __func__, argLen, args);
	while (1) {
		StaticThread.print("Thread : %s\n", __func__);
		StaticThread.delay(500);
	}
}

void run() {
	printf("Initiating...\n");
	HAL_Delay(1000);

	id1 = StaticThread.new(thread1, thread1Stack, sizeof(thread1Stack) / sizeof(uint32_t), 0, NULL);
	id2 = StaticThread.new(thread2, thread2Stack, sizeof(thread2Stack) / sizeof(uint32_t), 0, NULL);
	StaticThread.startScheduler();
}

```

# Outputs
```text
Initiating...
thread1(INIT) : 0-0
Thread : thread1
thread2(INIT) : 0-0
Thread : thread2
Thread : thread1
Thread : thread2
Thread : thread1
Thread : thread2
Thread : thread1
Thread : thread2
Thread : thread1
Thread : thread2
Thread : thread1
Thread : thread2
Thread : thread1
Thread : thread2
Thread : thread1
Thread : thread2
Thread : thread1
Thread : thread2
Thread : thread1
Thread : thread2
```