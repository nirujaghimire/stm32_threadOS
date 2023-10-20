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
static uint32_t threadAction[MAX_TASK_NO];
static void (*thread[MAX_TASK_NO]);

typedef enum {
	RUNNING = 0, TASK_DELETE, TASK_RESTART, TASK_BLOCK
} ThreadAction;

static void addTask(int index, void (*task)()) {
	uint32_t *psp = (uint32_t*) &stack[index][SIZE];

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

	thread[index] = task;
	threadAction[index] = RUNNING;
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

//	void (*task)() = (void (*)())((uint32_t*)sp[index])[14];
	void (*task)() = (void (*)())(stack[index][SIZE - 2]);
	task();
	while (1)
		;
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

	while (1) {
		count++;
		count %= MAX_TASK_NO;
		if (sp[count] == 0)
			continue; //Empty

		if (threadAction[count] == TASK_DELETE) {
			sp[count] = 0;
			continue;
		} else if (threadAction[count] == TASK_RESTART){
			addTask(count, thread[count]);
		}else if (threadAction[count] == TASK_BLOCK) {
			continue;
		}
		break;
	}

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
	x = 0;
	a = 0;
	while (1) {
		x++;
		HAL_Delay(1000);
		if(x==20)
			__asm volatile("SVC 0x11");
		a++;
	}
}

static void task1() {
	y = 0;
	b = 0;
	while (1) {
		y++;
		if (y >= 10)
			__asm volatile("SVC 0x21");
		HAL_Delay(1000);
		b++;
	}
}

void SVC_Handler(void) {
	uint32_t sp;
	uint32_t pc;
	uint8_t svc;
	uint32_t lr;

	//This function SP
	__asm volatile("MRS R0, MSP");
	__asm volatile("MOV %0, R0":"=r"(sp):);

	//SP before Prologue
	sp += 16; //Reversing : sub sp, #16
	sp += 8; // Reversing : push {r7, lr}
	__asm volatile("MOV %0, LR":"=r"(lr):);

	//Program counter of SVC call
	if (lr == 0xfffffffd) { //process stack
		__asm volatile("MRS R0, PSP");
		__asm volatile("MOV %0, R0":"=r"(sp):);
	}
	pc = ((uint32_t*) sp)[6] - 2;

	//SVC number
	svc = ((uint16_t*) pc)[0];
	uint8_t thread_no = svc & (0x0F);
	uint8_t thread_action = ((svc >> 4) & (0x0F));
	threadAction[thread_no] = thread_action;
}

uint32_t tick = 0;
void SysTick_Handler(void) {
	HAL_IncTick();
	tick++;
	//	if (tick % 1000 == 0)
	SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

void PendSV_Handler(void) {
	//Reversing caller function prologue (PendSV_hanlder) : PUSH {R7, LR}
	__asm volatile("POP {R7,LR}");

	//Making this function as if __atribute__((naked))
	__asm volatile("PUSH {LR}");
	taskSwitching();
	__asm volatile("POP {LR}");
	__asm volatile("BX LR");
}

void task_switching_with_Systick_PendSV_run() {
	addTask(0, task0);
	addTask(1, task1);
	startScheduler(0);
}
