/*
 * SVC_test.c
 *
 *  Created on: Oct 18, 2023
 *      Author: peter
 */

#include "main.h"

static void printSVCNumber(uint8_t svc, uint32_t lr) {
	printf("SVC num : %d (%s)\n", svc, lr == 0xfffffff9 ? "MSP" : "PSP");
}

void SVC_test_SVC() {
	uint32_t sp;
	uint32_t pc;
	uint8_t svc;
	uint32_t lr;

	//This function SP
	__asm volatile("MRS R0, MSP");
	__asm volatile("MOV %0, R0":"=r"(sp):);

	//Caller function SP (SVC Handler)
	sp += 16; //Reversing : sub sp, #16
	sp += 8; // Reversing : push {r7, lr}
	lr = ((uint32_t*) sp)[1];

	//Caller's caller function SP (SVC handler's SP before Prologue)
	sp += 8; // Reversing : push {r7, lr}

	//Program counter of SVC call
	if (lr == 0xfffffffd) { //process stack
		__asm volatile("MRS R0, PSP");
		__asm volatile("MOV %0, R0":"=r"(sp):);
	}
	pc = ((uint32_t*) sp)[6] - 2;

	//SVC number
	svc = ((uint16_t*) pc)[0];
	printSVCNumber(svc, lr);

}

static uint32_t stack[32];

void SVC_test_run() {
	//SVC call using main stack
	__asm volatile("SVC #3");

	//change stack to process stack
	__asm volatile("MOV R0, %0"::"r"(&stack));
	__asm volatile("MSR PSP, R0");
	__asm volatile("MRS R1, CONTROL");
	__asm volatile("ORR R1, R0, #2");
	__asm volatile("MSR CONTROL, R1");

	//SVC call using process stack
	__asm volatile("SVC #9");
	while(1){

	}
}
