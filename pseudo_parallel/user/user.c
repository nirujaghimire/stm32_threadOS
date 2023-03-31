/*
 * user.c
 *
 *  Created on: Feb 21, 2023
 *      Author: peter
 */

#include "user.h"
#include "main.h"

#include "task.h"

#define RAM_START       (0x20000000u)
#define RAM_SIZE        (20 * 1024) // 20 KB
#define MAIN_STACK    	(RAM_START + RAM_SIZE)

/////////////////////////////MAIN///////////////////////////////
void task1() {
	printf("Task1 starting\n");
	while (1) {
		printf("I am task1\n");
		task_delay(200);
	}
}
void task2() {
	printf("Task2 starting\n");
	while (1) {
		printf("I am task2\n");
		task_delay(500);
	}
}

int x = 0;
void task3() {
	printf("Task3 starting\n");
	while (1) {
		printf("I am task3\n\n");
		task_delay(1000);
		x++;
		if(x==2)
			task_enableHighestPriority();
		else if(x==6)
			task_disableHighestPriority();
	}
}


void init() {
	printf("Initiating....\n");

	task_init(MAIN_STACK, 5000,1024);
	task_add(1, task1, 1024);
	task_add(2, task2, 1024);
	task_add(3, task3, 1024);

	task_startScheduler();
}

void loop() {

}

