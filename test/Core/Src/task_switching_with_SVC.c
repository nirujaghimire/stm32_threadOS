/*
 * user.c
 *
 *  Created on: Oct 18, 2023
 *      Author: peter
 */
#include "main.h"

#define MAX_TASK_NO 2
#define SIZE 64

static uint32_t stack[MAX_TASK_NO][SIZE];
static uint32_t sp[MAX_TASK_NO];

static void addTask(int index, void (*task)()) {
	uint32_t *psp = (uint32_t*) &stack[index][SIZE - 1];

	// fill dummy stack frame
	*(--psp) = 0x01000000u; // Dummy xPSR, just enable Thumb State bit;
	*(--psp) = (uint32_t) task; // PC
	*(--psp) = 0xFFFFFFFDu; // LR with EXC_RETURN to return to Thread using PSP
	*(--psp) = 0x12121212u; // Dummy R12
	*(--psp) = 0x03030303u; // Dummy R3
	*(--psp) = 0x02020202u; // Dummy R2
	*(--psp) = 0x01010101u; // Dummy R1
	*(--psp) = 0x00000000u; // Dummy R0

	*(--psp) = 0x11111111u; // Dummy R11
	*(--psp) = 0x10101010u; // Dummy R10
	*(--psp) = 0x09090909u; // Dummy R9
	*(--psp) = 0x08080808u; // Dummy R8
	*(--psp) = 0x07070707u; // Dummy R7
	*(--psp) = 0x06060606u; // Dummy R6
	*(--psp) = 0x05050505u; // Dummy R5
	*(--psp) = 0x04040404u; // Dummy R4
	sp[index] = (uint32_t) psp;
}

static void startScheduler(int index) {
	__asm volatile("MOV R0, %0"::"r"(sp[index]));
	__asm volatile("MSR PSP, R0");

	//Stack to PSP
	__asm volatile(
			"MRS R0, CONTROL\n\t"
			"ORR R0, R0, #2\n\t"
			"MSR CONTROL, r0"
	);

	//Unprivileged Mode
	__asm volatile(
			"MRS R0, CONTROL\n\t"
			"ORR R0, R0, #1\n\t"
			"MSR CONTROL, r0"
	);

	void (*task)() = (void (*))((uint32_t*)sp[0])[14];
	task();
	while(1);
}

static int count = 0;
__attribute__((naked)) static void taskSwitching() {
	__asm volatile("PUSH {LR}");

	/* Save the context of current task */

	// get current PSP
	__asm volatile("MRS R0, PSP");
	// save R4 to R11 to PSP Frame Stack
	__asm volatile("STMDB R0!, {R4-R11}");
	// save current value of PSP
	__asm volatile("MOV %0, R0":"=r"(sp[count]));

	count += 1;
	count %= MAX_TASK_NO;

	/* Retrieve the context of next task */

	// get its past PSP value
	__asm volatile("MOV R0, %0"::"r"(sp[count]));
	// retrieve R4-R11 from PSP Fram Stack
	__asm volatile("LDMIA R0!, {R4-R11}");
	// update PSP
	__asm volatile("MSR PSP, R0");

	__asm volatile("POP {LR}");
	__asm volatile("BX LR");
}

/////////////////////////////MAIN/////////////////////////////////
static int x = 0;
static int a = 0;
static int y = 0;
static int b = 0;

static void task0() {
	while (1) {
		x++;
		HAL_Delay(100);
		__asm volatile("SVC #0");
		a++;
	}
}

static void task1() {
	while (1) {
		y++;
		HAL_Delay(100);
		__asm volatile("SVC #1");
		b++;
	}
}


//void SVC_Handler(void) {
//	taskSwitching();
//}

void task_switching_with_SVC_run() {
	addTask(0, task0);
	addTask(1, task1);
	startScheduler(0);
}
