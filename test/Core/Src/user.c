/*
 * user.c
 *
 *  Created on: Oct 16, 2023
 *      Author: peter
 */
#include "main.h"


/**********************EXCEPTION********************/

extern void SVC_test_SVC();
void SVC_Handler(void) {
	SVC_test_SVC();
}


void SysTick_Handler(void) {
	HAL_IncTick();
}

void PendSV_Handler(void) {

}


void startScheduler() {

}

/************************MAIN**********************/
extern void funtion_call_in_process_stack_run();
extern void SVC_test_run();

void run() {
//	printf("Initiating....\n");

//	funtion_call_in_process_stack_run();
	SVC_test_run();

	while (1) {

	}
}
