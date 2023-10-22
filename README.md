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
 *  Created on: Feb 21, 2023
 *      Author: peter
 */

#include "user.h"
#include "main.h"

#include "task.h"

/////////////////////////////MAIN///////////////////////////////
uint32_t count1;
uint32_t count2;
float fp = 0;

TaskSemaphore semaphore;

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
		task_printf("I am task2 %d\n",count2);
		count2++;
		fp = (float)count2/100.0f;


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
	printf("Initiating....\n");

	semaphore = task_createSemaphore(TASK_SEMAPHORE_MUTEX);

	task_init();
	task_add(1, task1, 1024);
	task_add(2, task2, 2024);

	task_startScheduler();

	printf("I am here\n");
}

void loop() {

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
