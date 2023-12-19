/*
 * user.c
 *
 *  Created on: Oct 21, 2023
 *      Author: peter
 */

#include "stm32_thread_OS.h"
#include "main.h"
#include "stdio.h"
#include "stdarg.h"

extern UART_HandleTypeDef huart1;

int _write(int file, char *data, int len) {
	StaticThread.mutexLock();
	HAL_UART_Transmit(&huart1, (uint8_t*) data, len, HAL_MAX_DELAY);
	StaticThread.mutexUnlock();
	return len;
}


///////////////////////////HANDLER///////////////////////
void HardFault_Handler(void) {
	printf("Hard Fault\n");
	while (1) {

	}
}

void SVC_Handler(void){
	StaticThread.SVCHandler();
}

void PendSV_Handler(void) {
	StaticThread.PendSVHandler();
}

void SysTick_Handler(void) {
	HAL_IncTick();
	StaticThread.SysTickHandler();
}

///////////////////////////THREAD/////////////////////////

//extern uint32_t stm32_thread_idle_count;
//uint32_t stack1[256];
//uint32_t stack2[128];
//int semaphore;
//int id1;
//int id2;
//static void task2() {
//	int y = 0;
//	printf("Task 2 initiating \n");
//	while (1) {
//		printf("Task 2 : %d :: %ld\n", y++, stm32_thread_idle_count);
//		if(y == 10){
////			StaticThread.giveBinarySemaphore(semaphore);
//			StaticThread.unblock(id1);
////			StaticThread.restart(id1);
//		}
//		StaticThread.delay(1000);
//	}
//}
//static void task1() {
//	int x = 0;
//	printf("Task 1 initiating \n");
//	while (1) {
//		printf("Task 1 : %d :: %ld\n", x++, stm32_thread_idle_count);
//		if(x == 5){
////			StaticThread.takeBinarySemaphore(&semaphore);
//			id2 = StaticThread.new(task2,stack2,sizeof(stack2) / 4);
//			StaticThread.block(id1);
//		}
//		StaticThread.delay(1000);
//	}
//}
//int id3;
//uint32_t stack3[256];
//static void task3(){
//	StaticThread.mutexLock();
//	for(int i=0;i<10;i++){
//		printf("%d\n",i);
//	}
//	StaticThread.mutexUnlock();
//	StaticThread.delete(id3);
//}







void printOdd(int* arr, int len){
	for(int i = 0; i<len; i++){
//		if(arr[i]%2!=0)
			printf("%d\n",arr[i]);
	}
}

int *x_rr;
int x_len;
int x_id;
void printX(){
//	printOdd(x_rr, x_len);
	for(int i = 0; i<x_len; i++){
		StaticThread.print("x = %d\n",x_rr[i]);
		HAL_Delay(10);
	}
	StaticThread.delete(x_id);
}

int *y_rr;
int y_len;
int y_id;
void printY(){
//	printOdd(y_rr, y_len);
	for(int i = 0; i<y_len; i++){
		StaticThread.print("y = %d\n",y_rr[i]);
		HAL_Delay(10);
	}
	StaticThread.delete(y_id);
}


int idMain;
uint32_t stackMain[1024];
void taskMain(){
	StaticThread.print("Initiating....\n");

	int x[] = {1,2,3,4,5,6,7,8,9};
	int y[] = {10,11,12,13,14,15,16,17,18};

	uint32_t x_stack[256],y_stack[256];


	x_rr = x;
	x_len = sizeof(x)/sizeof(int);
	x_id = StaticThread.new(printX,x_stack,sizeof(x_stack) / 4);


	y_rr = y;
	y_len = sizeof(y)/sizeof(int);
	y_id = StaticThread.new(printY,y_stack,sizeof(y_stack) / 4);

	while(1){



	}
}

void run() {
//	id1 = StaticThread.new(task1,stack1,sizeof(stack1) / 4);
//	id2 = StaticThread.new(task2,stack2,sizeof(stack2) / 4);
//	id3 = StaticThread.new(task3,stack3,sizeof(stack3) / 4);;

	idMain = StaticThread.new(taskMain,stackMain,sizeof(stackMain) / 4);
	StaticThread.startScheduler();

}

