/*
 * user.c
 *
 *  Created on: Oct 28, 2023
 *      Author: NIRUJA
 */

#include "main.h"
#include "stdio.h"

extern UART_HandleTypeDef huart1;

int _write(int file, char *data, int len) {
	HAL_UART_Transmit(&huart1, (uint8_t*) data, len, HAL_MAX_DELAY);
	return len;
}

#define STACK_SIZE 256

void task0();
void task1();

uint32_t stack[2][STACK_SIZE];

uint32_t sp[2];

int current_task = 0;

/////////////////////////////////////

void HardFault_Handler(void) {
	printf("Hard fault\n");
	while (1) {

	}
}

void SVC_Handler(void) {
//	//Making this function as if __attribute__((naked))
//	__asm volatile("PUSH {LR}");
//
//	/* Save the context of current task */
//
//	// get current PSP
//	__asm volatile("MRS R0, PSP");
//	// save R4 to R11 to PSP Frame Stack
//	__asm volatile("STMDB R0!, {R4-R11}");
//	// save current value of PSP
//	__asm volatile("MOV %0, R0":"=r"(sp[current_task]));
//
//	/* Scheduling */
//	current_task += 1;
//	current_task = current_task%2;
//
//	/* Retrieve the context of next task */
//
//	// get its past PSP value
//	__asm volatile("MOV R0, %0"::"r"(sp[current_task]));
//	// retrieve R4-R11 from PSP Fram Stack
//	__asm volatile("LDMIA R0!, {R4-R11}");
//	// update PSP
//	__asm volatile("MSR PSP, R0");
//
//	__asm volatile("POP {LR}");
//	__asm volatile("NOP");
//	__asm volatile("BX LR");

}

__attribute__((naked)) void PendSV_Handler(void) {
	//Making this function as if __attribute__((naked))
	__asm volatile("PUSH {LR}");

	/* Save the context of current task */

	// get current PSP
	__asm volatile("MRS R0, PSP");
	// save R4 to R11 to PSP Frame Stack
	__asm volatile("STMDB R0!, {R4-R11}");
	// save current value of PSP
	__asm volatile("MOV %0, R0":"=r"(sp[current_task]));

	/* Scheduling */
	current_task += 1;
	current_task = current_task % 2;

	/* Retrieve the context of next task */

	// get its past PSP value
	__asm volatile("MOV R0, %0"::"r"(sp[current_task]));
	// retrieve R4-R11 from PSP Fram Stack
	__asm volatile("LDMIA R0!, {R4-R11}");
	// update PSP
	__asm volatile("MSR PSP, R0");

	__asm volatile("POP {LR}");
	__asm volatile("NOP");
	__asm volatile("BX LR");


}

void SysTick_Handler(void) {

	HAL_IncTick();
	SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
}

////////////////////////////////////

int x = 0;
int y = 0;

void addTask0() {
	uint32_t *psp = (uint32_t*) &stack[0][STACK_SIZE];

	// fill dummy stack frame
	*(--psp) = 0x01000000u; // Dummy xPSR, just enable Thumb State bit;
	*(--psp) = (uint32_t) task0; // PC
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
	sp[0] = (uint32_t) psp;
}

void addtask1() {
	uint32_t *psp = (uint32_t*) &stack[1][STACK_SIZE];

	// fill dummy stack frame
	*(--psp) = 0x01000000u; // Dummy xPSR, just enable Thumb State bit;
	*(--psp) = (uint32_t) task1; // PC
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
	sp[1] = (uint32_t) psp;
}

void startScheduling() {
	uint32_t psp = sp[0];

	__asm volatile("MOV R0, %0"::"r"(psp));
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

	current_task = 0;
//	void (*task)() = (void (*)())((uint32_t*)psp)[14];
	task0();
}

void task0() {

	while (1) {
		x++;
//		__asm volatile("SVC #0");
		HAL_Delay(1000);
		printf("x = %d\n",x);
	}

}

void task1() {

	while (1) {
		y++;
//		__asm volatile("SVC #0");
		HAL_Delay(1000);
		printf("y = %d\n",y);
	}

}

////////////////////////////////////

void run() {
//	printf("Initiating....\n");

	addTask0();
	addtask1();

	startScheduling();
}
