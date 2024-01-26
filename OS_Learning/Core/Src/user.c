/*
 * user.c
 *
 *  Created on: Jan 11, 2024
 *      Author: NIRUJA
 */

#include "main.h"

#define STACK_SIZE 8

typedef struct{
	uint32_t stack[STACK_SIZE];
	uint32_t SP;
}TaskInfo;

TaskInfo info[2];
int currentTask = 0;
int x=0,y=0;


__attribute__((naked)) void taskSwitch(){
	//Context saving (Kind of prologue)
	__asm volatile("push {r7, lr}");

	__asm volatile("mrs r0, psp");
	__asm volatile("mov %0, r0":"=r"(info[currentTask].SP));


	//switching
	currentTask = (currentTask+1)%2;

	//Context Retrieving (Kind of epilogue)
	__asm volatile("mov r0, %0"::"r"(info[currentTask].SP));
	__asm volatile("msr psp, r0");

	__asm volatile("pop {r7, lr}");
	__asm volatile("bx lr");
}

void task1(){
	while(1){
		x++;
		//task switch
		taskSwitch();
		x++;
	}
}

void task2(){
	while(1){
		y++;
		//Task switch
		taskSwitch();
		y++;
	}
}




void run(){
	info[0].SP = (uint32_t)&info[0].stack[STACK_SIZE-1];
	info[1].SP = (uint32_t)&info[1].stack[STACK_SIZE-1];

	info[1].SP -=4;
	*(uint32_t*)info[1].SP = (uint32_t)task2;
	info[1].SP -=4;
	*(uint32_t*)info[1].SP = info[1].SP;


	__asm volatile("mov r0, %0"::"r"(info[currentTask].SP));
	__asm volatile("msr psp, r0");

	__asm volatile("mov r0, 2");
	__asm volatile("msr control, r0");

	//Process Stack
	task1();

}
