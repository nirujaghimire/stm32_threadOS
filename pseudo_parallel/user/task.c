/*
 * task.c
 *
 *  Created on: Mar 26, 2023
 *      Author: peter
 */

#include "task.h"
#include "stdarg.h"
#include "main.h"

#define TASK_MAX_NUMBER 10

typedef enum{
	CONSOLE_ERROR = 0,
	CONSOLE_INFO,
	CONSOLE_WARNING,
}ConsoleState;

typedef enum {
	RUNNING,	// During running condition
	PAUSE,		// During pause condition (Used for semaphore)
	PRIORITY,	// Highest priority (Used for semaphore)
//	BLOCKED		// During blocked condition
//	RESTART		// During blocked condition
} TaskState;

struct Task {
	uint32_t id;			// ID of task required for any operation like delete, start.. etc
	void (*handler)(void);	// Main function of the task
	uint32_t psp;			// Current process stack pointer
	TaskState state;		// State of task
	uint32_t wait_tick;		// Task is pause until the system ticks meet this value
	uint32_t stack_size;	// Size of stack
	uint32_t stack_start;	// Stack starting address
};
typedef struct Task Task;

/*********************VARIABLES********************************/
static uint32_t TOTAL_STACK_SIZE;		// Maximum size of stack
static uint32_t MSP;					// Main Stack Pointer i.e ORIGIN(RAM)+SIZE(RAM) = end of RAM
static uint32_t MAIN_STACK_SIZE;		// Size of main stack
static uint32_t stack_tracker = 0;		// Track the stack size of all

static Task task[TASK_MAX_NUMBER]={0};	// All the tasks

static uint32_t global_ticks;			// Global ticks or counter
static int current_task_index;			// Stores index of current task
static uint8_t scheduler_started = 0;	// Scheduler started flag
static int priority_task = -1;		// track the priority task


/********************LOCAL FUNCTIONS***************************/
static void console(ConsoleState state,const char* func_name, const char* msg,...){
//	if(state!=CONSOLE_ERROR)
//		return;
	//TODO make naked and show all registers
	if(state==CONSOLE_ERROR){
		printf("TASK|%s> ERROR :",func_name);
	}else if(state==CONSOLE_INFO){
		printf("TASK|%s> INFO : ",func_name);
	}else if(state == CONSOLE_WARNING){
		printf("TASK|%s> WARNING : ",func_name);
	}else{
		printf("TASK|%s: ",func_name);
	}
	va_list args;
	va_start(args, msg);
	vprintf(msg, args);
    va_end(args);
}


uint32_t task_getCurrentPSP(){
	return task[current_task_index].psp;
}

void task_saveCurrentPSP(uint32_t psp) {
	task[current_task_index].psp = psp;
}


void task_selectNextTask(){
	if(current_task_index==TASK_MAX_NUMBER-1)
		current_task_index = 0;	//In case of last task skip ideal task

	int idx = current_task_index;
	int i=1;
	for(; i<TASK_MAX_NUMBER; i++){
		idx = (i+current_task_index)%TASK_MAX_NUMBER;
		if(priority_task>0 && priority_task!=idx)
			continue;// If priority task exist and it is not priority task

		if(task[idx].psp == 0)
			continue;	// Empty slot

		/* Non empty slot found */
		if(task[idx].state == PAUSE)
			continue;	// Task is paused

		/* Running task is found */
		if(task[idx].wait_tick > global_ticks)
			continue;	// Task is waiting or delayed

		/* Non waiting task is found */
		current_task_index = idx;
		break;
	}

	if(i>=TASK_MAX_NUMBER)
		current_task_index = 0;	//If all tasks are blocked then use idle main
}

__attribute__((naked)) void task_switchTask() {
	/*save LR back to main, must do this firstly*/
	__asm volatile("PUSH {LR}");
//	printf("****\n");

	/* Save the context of current task */

	/* get current PSP */
	__asm volatile("MRS R0, PSP");

	/* save R4 to R11 to PSP Frame Stack */
	__asm volatile("STMDB R0!, {R4-R11}");		// R0 is updated after decrement
	/* save current value of PSP */
	__asm volatile("BL task_saveCurrentPSP");	// R0 is first argument

	/* Do scheduling */

	/* select next task */
	__asm volatile("BL task_selectNextTask");

	/* Retrieve the context of next task */

	/* get its past PSP value */
	__asm volatile("BL task_getCurrentPSP");	// return PSP is in R0
	/* retrieve R4-R11 from PSP Fram Stack */
	__asm volatile("LDMIA R0!, {R4-R11}");		// R0 is updated after increment
	/* update PSP */
	__asm volatile("MSR PSP, R0");

	/* exit */
	__asm volatile("POP {LR}");
	__asm volatile("BX LR");
}

void task_reschedule(uint32_t priv) {
  if(priv) //Already has privileged access
	  /* Enable pendSV exception */
	  *(uint32_t*) 0xE000ED04 |= (0x1 << 28);
  else
	  /* call Supervisor exception to get Privileged access */
	  __asm volatile("SVC #255");
}

void task_idle_main(void) {
	while(1) {
		__asm volatile("NOP");
	}
}

/*********************HANDLER**********************************/
void task_SVCHandler(){
	/**
	 * LR contains:
	 * 0xFFFFFFF9 (0b111....1001) for Thread mode (MSP)-> 3rd bit 0
	 * 0xFFFFFFFD (0b111....1101) for Thread mode (PSP)-> 3rd bit 1
	 */

	/* Check LR to know which stack is used */
	__asm volatile("TST LR, 4"); /* LR bitwise and with 4 */

	/* 2 next instructions are conditional */
	__asm volatile("ITE EQ"); /* If-then-else */

	/* save MSP to R0 if 3rd bit is equal 0 */
	__asm volatile("MRSEQ R0, MSP");

	/* save PSP to R0 if 3rd bit is not equal 0 */
	__asm volatile("MRSNE R0, PSP");

	/* pass R0 as argument*/
	__asm volatile("B task_SVCHandlerMain");
}

void task_SVCHandlerMain(uint32_t* SP){
	/**
	 * Previous instruction address (PC of previous thread)
	 * is stored in stack with offset 6 from Stack Pointer (SP)
	 */

	/* get the address of the instruction saved in PC */
	uint8_t *pInstruction = (uint8_t*)(SP[6]);

	/* go back 2 bytes (16-bit opcode) */
	pInstruction -= 2;

	/* get the opcode, in little endian */
	uint8_t svc_num = *pInstruction;

	if(svc_num == 255)
		/* Enable pendSV exception */
		*(uint32_t*) 0xE000ED04 |= (0x1 << 28);
}

void task_SysTickHandler(){
	if(!scheduler_started)
		return;
	global_ticks++;
	if(global_ticks<100)
		return;

	if(global_ticks%10!=0)
		return;
	task_reschedule(1);// 1 for privileged
}

void task_PendSVHandler(void){


	/*save LR back to main, must do this firstly*/
	__asm volatile("PUSH {LR}");

	task_switchTask();

	/* exit */
	__asm volatile("POP {LR}");
	__asm volatile("BX LR");
}

/**********************PUBLIC**********************************/
/**
 * This sets the total stack size and main stack pointer
 * @param msp 				: Main stack pointer
 * @param total_stack_size	: Total stack size
 * @param main_stack_size	: Size of main stack
 */
void task_init(uint32_t msp, uint32_t total_stack_size, uint32_t main_stack_size) {
	MSP = msp;
	TOTAL_STACK_SIZE = total_stack_size;
	MAIN_STACK_SIZE = main_stack_size;
	for(int i=0;i<TASK_MAX_NUMBER;i++)
		task[i].psp = 0;
	stack_tracker += main_stack_size;
	current_task_index = 0;
	global_ticks = 0;
	scheduler_started = 0;

	/* Add idle task which run when there is no any task to run*/
	task_add(0xFFFFFFFF, task_idle_main, 100);
}

/**
 * This add the task in slots
 * @param id			: Give identifier for task to identify it later
 * @param handler		: Main function
 * @param stack_size	: Size provided for it
 * @return				: 1 => Successfully task added
 *						: 0 => Failed to add task
 */
int task_add(uint32_t id,void (*handler)(void),uint32_t stack_size){
	/*Search for empty slot index*/
	int idx = 0;
	for(;idx<TASK_MAX_NUMBER;idx++)
		if(task[idx].psp==0)
			break;

	for(int i = 0;i<TASK_MAX_NUMBER;i++){
		if(task[i].psp!=0 && task[i].id == id){
			console(CONSOLE_ERROR,__func__, "Handler %p addition failed. ID %d is already associated with handler %p.\n",handler,id,task[i].handler);
			return 0;
		}
	}

	for(int i = 0;i<TASK_MAX_NUMBER;i++){
		if(task[i].psp!=0 && task[i].handler == handler){
			console(CONSOLE_ERROR,__func__, "Handler %p addition failed. Handler already in slots, associated with ID %d.\n",handler,task[i].id);
			return 0;
		}
	}

	if(idx>=TASK_MAX_NUMBER){
		/* All slots are full */
		console(CONSOLE_ERROR,__func__, "Handler %p addition failed. All %d slots are full.\n",handler,TASK_MAX_NUMBER);
		return 0;
	}

	uint32_t remaining = TOTAL_STACK_SIZE-stack_tracker;
	if(remaining<stack_size){
		/* Out of stack */
		uint32_t overflow = stack_size-remaining;
		console(CONSOLE_ERROR,__func__, "Handler %p addition failed. Not Enough Stack. %ld bytes more stack is required.\n",handler,overflow);
		return 0;
	}

	/* Calculate PSP */
	stack_tracker += stack_size;
	uint32_t *psp = (uint32_t*)(MSP-stack_tracker);
	task[idx].stack_start = (uint32_t)psp;

	/* fill dummy stack frame */
	*(--psp) = 0x01000000u; 		// Dummy xPSR, just enable Thumb State bit;
	*(--psp) = (uint32_t) handler; 	// PC
	*(--psp) = 0xFFFFFFFDu; 		// LR with EXC_RETURN to return to Thread using PSP
	*(--psp) = 0x12121212u; 		// Dummy R12
	*(--psp) = 0x03030303u; 		// Dummy R3
	*(--psp) = 0x02020202u; 		// Dummy R2
	*(--psp) = 0x01010101u; 		// Dummy R1
	*(--psp) = 0x00000000u; 		// Dummy R0
	*(--psp) = 0x11111111u; 		// Dummy R11
	*(--psp) = 0x10101010u; 		// Dummy R10
	*(--psp) = 0x09090909u; 		// Dummy R9
	*(--psp) = 0x08080808u; 		// Dummy R8
	*(--psp) = 0x07070707u; 		// Dummy R7
	*(--psp) = 0x06060606u; 		// Dummy R6
	*(--psp) = 0x05050505u; 		// Dummy R5
	*(--psp) = 0x04040404u; 		// Dummy R4

	/* Add task */
	task[idx].id = id;
	task[idx].handler = handler;
	task[idx].state = RUNNING;
	task[idx].stack_size = stack_size;
	task[idx].wait_tick = 0;
	task[idx].psp = (uint32_t)psp;

	if(idx!=0)	//Skip the idle task
		console(CONSOLE_INFO,__func__, "Handler %p is added successfully with id %d.\n",handler,id);
	return 1;
}

/**
 * This Restart the task
 * @param id: ID associated with task
 * @return	: 1 => Successfully restarted
 *			: 0 => Failed to restart
 */
int task_restart(uint32_t id){
	int idx = 0;
	for(;idx<TASK_MAX_NUMBER;idx++)
		if(task[idx].psp!=0 && task[idx].id == id)
			break;

	if(idx>=TASK_MAX_NUMBER){
		console(CONSOLE_ERROR,__func__, "ID %d is not associated with any handler.\n",id);
		return 0;
	}


	if(idx==current_task_index){
		console(CONSOLE_ERROR,__func__, "Handler %p associated with id %d is itself "
				"and self restart is not allowed.\n",task[idx].handler,id);
		return 0;
	}

	/* Calculate PSP */
	uint32_t *psp = (uint32_t*)task[idx].stack_start;

	/* fill dummy stack frame */
	*(--psp) = 0x01000000u; 					// Dummy xPSR, just enable Thumb State bit;
	*(--psp) = (uint32_t) task[idx].handler;	// PC
	*(--psp) = 0xFFFFFFFDu; 					// LR with EXC_RETURN to return to Thread using PSP
	*(--psp) = 0x12121212u; 					// Dummy R12
	*(--psp) = 0x03030303u; 					// Dummy R3
	*(--psp) = 0x02020202u; 					// Dummy R2
	*(--psp) = 0x01010101u; 					// Dummy R1
	*(--psp) = 0x00000000u; 					// Dummy R0
	*(--psp) = 0x11111111u; 					// Dummy R11
	*(--psp) = 0x10101010u; 					// Dummy R10
	*(--psp) = 0x09090909u; 					// Dummy R9
	*(--psp) = 0x08080808u; 					// Dummy R8
	*(--psp) = 0x07070707u; 					// Dummy R7
	*(--psp) = 0x06060606u; 					// Dummy R6
	*(--psp) = 0x05050505u; 					// Dummy R5
	*(--psp) = 0x04040404u; 					// Dummy R4

	/* Add task */
	task[idx].state = RUNNING;
	task[idx].wait_tick = 0;
	task[idx].psp = (uint32_t)psp;

	console(CONSOLE_INFO,__func__, "Handler %p associated with id %d is restarted.\n",task[idx].handler,id);

	return 0;
}

/**
 * This pause the task
 * @param id: ID associated with task
 * @return	: 1 => Successfully paused
 *			: 0 => Failed to pause
 */
int task_pause(uint32_t id){
	int idx = 0;
	for(;idx<TASK_MAX_NUMBER;idx++)
		if(task[idx].psp!=0 && task[idx].id == id)
			break;

	if(idx>=TASK_MAX_NUMBER){
		console(CONSOLE_ERROR,__func__, "ID %d is not associated with any handler.\n",id);
		return 0;
	}else if(task[idx].state == PAUSE){
		console(CONSOLE_WARNING,__func__, "Handler %p associated with id %d is already paused.\n",task[idx].handler,id);
		return 0;
	}


	task[idx].state = PAUSE;
	console(CONSOLE_INFO,__func__, "Handler %p associated with id %d is paused.\n",task[idx].handler,id);

	if(idx==current_task_index)
		task_reschedule(0);	//Switch task

	return 1;
}

/**
 * This pause the task itself
 * @return	: 1 => Successfully paused
 *			: 0 => Failed to pause
 */
int task_selfPause(){
	return task_pause(task[current_task_index].id);
}

/**
 * This resume the task
 * @param id: ID associated with task
 * @return	: 1 => Successfully resumed
 *			: 0 => Failed to resume
 */
int task_resume(uint32_t id){
	int idx = 0;
	for(;idx<TASK_MAX_NUMBER;idx++)
		if(task[idx].psp!=0 && task[idx].id == id)
			break;

	if(idx>=TASK_MAX_NUMBER){
		console(CONSOLE_ERROR,__func__, "ID %d is not associated with any handler.\n",id);
		return 0;
	}else if(task[idx].state == RUNNING){
		console(CONSOLE_WARNING,__func__, "Handler %p associated with id %d is already running.\n",task[idx].handler,id);
		return 0;
	}

	task[idx].state = RUNNING;
	console(CONSOLE_INFO,__func__, "Handler %p associated with id %d is resumed.\n",task[idx].handler,id);
	return 1;
}

/**
 * This delete the task
 * @param id: ID associated with task
 * @return	: 1 => Successfully resumed
 *			: 0 => Failed to resume
 */
int task_delete(uint32_t id){
	int idx = 0;
	for(;idx<TASK_MAX_NUMBER;idx++)
		if(task[idx].psp!=0 && task[idx].id == id)
			break;

	if(idx>=TASK_MAX_NUMBER){
		console(CONSOLE_ERROR,__func__, "ID %d is not associated with any handler.\n",id);
		return 0;
	}

	if(idx==current_task_index){
		console(CONSOLE_ERROR,__func__, "Handler %p associated with id %d is itself "
				"and self delete is not allowed.\n",task[idx].handler,id);
		return 0;
	}

	task[idx].id = 0;
	task[idx].handler = 0;
	task[idx].psp = 0;
	console(CONSOLE_INFO,__func__, "Handler %p associated with id %d is deleted.\n",task[idx].handler,id);

	return 1;
}

/**
 * This priorities the task. These block all other tasks.
 * @return	: 1 => Successfully resumed
 *			: 0 => Failed to resume
 */
int task_enableHighestPriority(){
	if(task[current_task_index].state == PRIORITY){
		console(CONSOLE_WARNING,__func__, "Handler %p associated with id %d is already enabled as highest priority task.\n",task[current_task_index].handler,task[current_task_index].id);
		return 0;
	}

	task[current_task_index].state = PRIORITY;
	priority_task = current_task_index;
	console(CONSOLE_INFO,__func__, "Handler %p associated with id %d is enabled as highest priority task.\n",task[current_task_index].handler,task[current_task_index].id);
	return 1;
}

/**
 * This disable prioritized task.
 * @return	: 1 => Successfully resumed
 *			: 0 => Failed to resume
 */
int task_disableHighestPriority(){
	if(task[current_task_index].state == RUNNING){
		console(CONSOLE_WARNING,__func__, "Handler %p associated with id %d is not highest priority task.\n",task[current_task_index].handler,task[current_task_index].id);
		return 0;
	}

	task[current_task_index].state = RUNNING;
	priority_task = -1;
	console(CONSOLE_INFO,__func__, "Handler %p associated with id %d is disabled as highest priority task.\n",task[current_task_index].handler,task[current_task_index].id);
	return 1;
}


/**
 * This delay the task
 * @param ticks	: Ticks upto which task should be delayed
 */
void task_delay(uint32_t ticks){
	if(current_task_index==0)
		return;
	task[current_task_index].wait_tick = global_ticks+ticks;
	task_reschedule(0);
}

/**
 * This starts the scheduler from main
 */
void task_startScheduler(){
	/* Start with the first task */
	current_task_index = 0;

	/* prepare PSP of the first task */
	__asm volatile("BL task_getCurrentPSP");	// return PSP in R0
	__asm volatile("MSR PSP, R0");		// set PSP

	/* change to use PSP */
	__asm volatile("MRS R0, CONTROL");
	__asm volatile("ORR R0, R0, #2");	// set bit[1] SPSEL
	__asm volatile("MSR CONTROL, R0");

	/* Move to Unprivileged level */
	__asm volatile("MRS R0, CONTROL");
	__asm volatile("ORR R0, R0, #1");	// Set bit[0] nPRIV
	__asm volatile("MSR CONTROL, R0");

	/* get the handler of the first task by tracing back from PSP which is at R4 slot */
	void (*handler)() = (void (*))((uint32_t*)task[current_task_index].psp)[8+6];
	scheduler_started = 1;
	/* execute the handler */
	handler();
//	task[current_task_index].handler();

}








