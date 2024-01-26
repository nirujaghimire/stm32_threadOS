/*
 * user.c
 *
 *  Created on: Dec 20, 2023
 *      Author: NIRUJA
 */

#include "main.h"
#include "stdio.h"

extern UART_HandleTypeDef huart1;

int _write(int file, char *data, int len){
	HAL_UART_Transmit(&huart1, (uint8_t*)data, len, HAL_MAX_DELAY);
	return len;
}

#define STACK_SIZE 256

void task0();
void task1();

uint32_t stack[2][STACK_SIZE];

uint32_t sp[2];

int current_task = 0;

/////////////////////////////////////

void HardFault_Handler(void){
	printf("Hard fault\n");
	while(1){

	}
}

__attribute__((naked)) void PendSV_Handler(void){
	//Maling this function as if __attribute__((naked))
	__asm volatile("PUSH {LR}");

	/*Save the context of current task*/

	//get current PSP
	__asm volatile("MRS R0, PSP");
	//save R4 to R11 to PSP Frame Stack
	__asm volatile("STMDB R0!, {R4-R11}");
	//save current value of PSP
	__asm volatile("MOV %0, R0":"=r"(sp[current_task]));

	/* Scheduling */
	current_task += 1;
	current_task = current_task % 2;

	/* Retrieve the context of next task */

	//get its past PSP value
	__asm volatile("MOV R0, %0"::"r"(sp[current_task]));
	//retrieve R4-R11 from PSP Frame Stack
	__asm volatile("LDMIA R0!, {R4-R11}");
	//update PSP
	__asm volatile("MSR PSP, R0");

	__asm volatile("POP {LR}");
	__asm volatile("NOP");
	__asm volatile("BX LR");

}

void SysTick_Handler(void){

	HAL_IncTick();
	SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk
}

