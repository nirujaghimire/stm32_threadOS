# stm32 threadOS
→This is library which allow us to run multiple threads in pseudo parallel in stm32  
Features :
- Multiple thread can be add anytime with stack size
- Each thread can be:
    - Restart in any time. Self restart is also possible
    - Pause in any time
    - Resumed any time
    - Deleted in any time. Self delete is also possible
- Thread delay which doesn’t consumes the clock
- Thread Semaphore 
- Thread Mutex lock and unlock
```rb
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
```
#Outputs
```rb
Initiating....
TASK|task_add> INFO : Handler 0x8002601 is added successfully with id 1.
TASK|task_add> INFO : Handler 0x8002639 is added successfully with id 2.
Task1 starting
I am task1
Task2 starting
I am task2 0
I am task1
I am task1
I am task2 1
I am task1
I am task1
I am task2 2
I am task1
I am task1
I am task2 3
TASK|task_takeSemaphore> INFO : Handler 0x8002639 associated with id 2 took semaphore.
I am task2 4
I am task2 5
I am task2 6
I am task2 7
TASK|task_giveSemaphore> INFO : Handler 0x8002639 associated with id 2 gave semaphore of handler 0x8002639.
I am task1
I am task1
I am task2 8
I am task1
I am task1
I am task2 9
I am task1
I am task1
I am task2 10
I am task1
```
