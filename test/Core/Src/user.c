/*
 * user.c
 *
 *  Created on: Oct 16, 2023
 *      Author: peter
 */
#include "main.h"


/**********************EXCEPTION********************/
//void SVC_Handler(void) {
//
//}


//void SysTick_Handler(void) {
//	HAL_IncTick();
//}

//void PendSV_Handler(void) {
//
//}

/************************MAIN**********************/
extern void funtion_call_in_process_stack_run();
extern void SVC_test_run();
extern void task_switching_with_SVC_run();
extern void task_switching_with_SVC_PendSV_run();
extern void task_switching_with_Systick_PendSV_run();

void run() {
//	printf("Initiating....\n");

//	funtion_call_in_process_stack_run();
//	SVC_test_run();
//	task_switching_with_SVC_run();
//	task_switching_with_SVC_PendSV_run();
	task_switching_with_Systick_PendSV_run();

	while (1) {

	}
}
