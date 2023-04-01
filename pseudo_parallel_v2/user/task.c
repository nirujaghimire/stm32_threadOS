/*
 * task.c
 *
 *  Created on: Mar 26, 2023
 *      Author: peter
 */

#include "task.h"
#include "stdarg.h"
#include "main.h"
#include "stdlib.h"

#define TASK_MAX_NUMBER 3

typedef enum{
	TASK_CONSOLE_ERROR = 0,
	TASK_CONSOLE_INFO,
	TASK_CONSOLE_WARNING,
}ConsoleState;

typedef enum {
	TASK_RUNNING,	// During running condition
	TASK_PAUSE,		// During pause condition
	TASK_BLOCKED,	// During blocked condition
	TASK_RESTART,	// During restart condition
	TASK_DELETE		// During delete condition
} TaskState;


struct Task{
	uint32_t id;			// ID of task required for any operation like delete, start.. etc
	void (*handler)(void);	// Main function of the task
	uint32_t psp;			// Current process stack pointer
	TaskState state;		// State of task
	int8_t priority;		// Priority
	uint32_t wait_tick;		// Task is pause until the system ticks meet this value
	uint32_t stack_size;	// Size of stack
	uint32_t stack_start;	// Stack starting address
	uint32_t *stack;		// Stores stack in heap
	struct Task* next_task;	// Pointer to next task
};
typedef struct Task Task;



/*********************VARIABLES********************************/
static uint32_t stack_tracker = 0;		// Track the stack size of all

static Task task[TASK_MAX_NUMBER]={0};	// All the tasks
static Task *current_task;				// Current task

static volatile uint32_t global_ticks;			// Global ticks or counter
static uint8_t scheduler_started = 0;	// Scheduler started flag




/********************LOCAL FUNCTIONS***************************/
static int _isPSP(){
	int control_reg;
	__asm volatile("MRS %0, CONTROL": "=r"(control_reg));
	/**
	 * If control register 2nd bit is set then PSP
	 * Else not PSP
	 */
	return control_reg & (1<2);
}

static void _console(ConsoleState state,const char* func_name, const char* msg,...){
	int8_t priority;
	if(_isPSP()){
		priority = current_task->priority;
		current_task->priority = -1;
	}

	//TODO make naked and show all registers
	if(state==TASK_CONSOLE_ERROR){
		printf("TASK|%s> ERROR :",func_name);
	}else if(state==TASK_CONSOLE_INFO){
		printf("TASK|%s> INFO : ",func_name);
	}else if(state == TASK_CONSOLE_WARNING){
		printf("TASK|%s> WARNING : ",func_name);
	}else{
		printf("TASK|%s: ",func_name);
	}
	va_list args;
	va_start(args, msg);
	vprintf(msg, args);
    va_end(args);

    if(_isPSP() && (priority!=-1))
    	current_task->priority = 0;
}

uint32_t _task_getCurrentPSP(){
	return current_task->psp;
}

void _task_saveCurrentPSP(uint32_t psp) {
	current_task->psp = psp;
}

void _task_restartCurrentTask(){
	/* Calculate PSP */
	uint32_t *psp = (uint32_t*)current_task->stack_start;

	/* fill dummy stack frame */
	*(--psp) = 0x01000000u; 					// Dummy xPSR, just enable Thumb State bit;
	*(--psp) = (uint32_t) current_task->handler;// PC
	*(--psp) = 0xFFFFFFFDu; 					// LR with EXC_RETURN to return to Thread using PSP
	psp-=(16-3);

	/* Add task */
	current_task->state = TASK_RUNNING;
	current_task->wait_tick = 0;
	current_task->psp = (uint32_t)psp;
}

void _task_deleteCurrentTask(){
	current_task->psp = 0;
	current_task->id = 0;
	current_task->handler = 0;
	current_task->state = TASK_BLOCKED;
	free(current_task->stack);
	current_task->stack = NULL;
}

void _task_selectNextTask(){
	/* Task is set to highest priority */
	if(current_task->priority==-1)// same task should run
		return;

	int i = 0;
	for(;i<TASK_MAX_NUMBER;i++){
		current_task = current_task->next_task;
		if(current_task->id==0xFFFFFFFF)// Idle task
			continue;

		if(current_task->psp==0)		//Empty task
			continue;

		/*check ticks */
		if(current_task->wait_tick>global_ticks)
			continue;

		if(current_task->state==TASK_RESTART)
			_task_restartCurrentTask();
		else if(current_task->state==TASK_DELETE){
			_task_deleteCurrentTask();
			continue;
		}else if(current_task->state==TASK_BLOCKED)
			continue;
		else if(current_task->state == TASK_RUNNING)
			break;
	}

	if(i>=TASK_MAX_NUMBER)
		current_task = &task[0];// NO task is running then run idle task
}

__attribute__((naked)) void _task_switchTask() {
	__asm volatile(
			/*save LR back to main, must do this firstly*/
			"PUSH {LR}\n\t"

			/* Save the context of current task */

			/* get current PSP */
			"MRS R0, PSP\n\t"
			/* save R4 to R11 to PSP Frame Stack */
			"STMDB R0!, {R4-R11}\n\t"		// R0 is updated after decrement
			/* save current value of PSP */
			"BL _task_saveCurrentPSP\n\t"	// R0 is first argument

			/* Do scheduling */

			/* select next task */
			"BL _task_selectNextTask\n\t"

			/* Retrieve the context of next task */
			/* get its past PSP value */
			"BL _task_getCurrentPSP\n\t"		// return PSP is in R0
			/* retrieve R4-R11 from PSP Fram Stack */
			"LDMIA R0!, {R4-R11}\n\t"		// R0 is updated after increment
			/* update PSP */
			"MSR PSP, R0\n\t"

			/* exit */
			"POP {LR}\n\t"
			"BX LR\n\t"
	);
}

void _task_privilegeReschedule(){
	/* Enable pendSV exception */
	*(uint32_t*) 0xE000ED04 |= (0x1 << 28);
}

void _task_unprivilegeReschedule(){
	/* call Supervisor exception to get Privileged access */
	__asm volatile("SVC #255");
}

void _task_idle_main(void) {
	while(1) {
		__asm volatile("NOP");
	}
}

/*********************HANDLER**********************************/
void task_SysTickHandler(){
	if(!scheduler_started)
		return;
	global_ticks++;

	if(global_ticks%1!=0)
		return;
	_task_privilegeReschedule();
}

__attribute__((naked)) void task_PendSVHandler(void){
	/*save LR back to main, must do this firstly*/
	__asm volatile("PUSH {LR}");

	uint32_t interrupt_enabled = !__get_PRIMASK();
	__disable_irq();

	_task_switchTask();

	if(interrupt_enabled)
		__enable_irq();

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
void task_init() {
	uint32_t interrupt_enabled = !__get_PRIMASK();
	__disable_irq();


	for(int i=0;i<TASK_MAX_NUMBER;i++){
		task[i].psp = 0;
		task[i].stack = NULL;// Stack is not allocated
		task[i].next_task = &task[(i+1)%TASK_MAX_NUMBER];
		task[i].priority = 0;
	}
	current_task = &task[0];

	stack_tracker = 0;
	global_ticks = 0;
	scheduler_started = 0;


	if(interrupt_enabled)
		__enable_irq();

	/* Add idle task which run when there is no any task to run*/
	task_add(0xFFFFFFFF, _task_idle_main, 100);
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
	uint32_t interrupt_enabled = !__get_PRIMASK();
		__disable_irq();


	int8_t priority;
	if(_isPSP()){
		priority = current_task->priority;
		current_task->priority = -1;
	}

	/*Search for empty slot index*/
	int idx = 0;
	for(;idx<TASK_MAX_NUMBER;idx++)
		if(task[idx].psp==0)
			break;

	for(int i = 0;i<TASK_MAX_NUMBER;i++){
		/* ID already exist */
		if(task[i].psp!=0 && task[i].id == id){
			_console(TASK_CONSOLE_ERROR,__func__, "Handler %p addition failed. ID %d is already associated with handler %p.\n",handler,id,task[i].handler);
			return 0;
		}
	}

	for(int i = 0;i<TASK_MAX_NUMBER;i++){
		/* Handler already exist */
		if(task[i].psp!=0 && task[i].handler == handler){
			_console(TASK_CONSOLE_ERROR,__func__, "Handler %p addition failed. Handler already in slots, associated with ID %d.\n",handler,task[i].id);
			return 0;
		}
	}

	if(idx>=TASK_MAX_NUMBER){
		/* All slots are full */
		_console(TASK_CONSOLE_ERROR,__func__, "Handler %p addition failed. All %d slots are full.\n",handler,TASK_MAX_NUMBER);
		return 0;
	}

	/* Memory Allocation */
	task[idx].stack = (uint32_t*) malloc(stack_size/4+1);
	if(task[idx].stack == NULL){
		/* Out of stack */
		_console(TASK_CONSOLE_ERROR,__func__, "Handler %p addition failed. Not Enough heap to make stack.\n",handler);
		return 0;
	}

	/* Calculate PSP */
	stack_tracker += stack_size;
	uint32_t *psp = &task[idx].stack[stack_size/4];
	task[idx].stack_start = (uint32_t)psp;

	/* fill dummy stack frame */
	*(--psp) = 0x01000000u; 		// Dummy xPSR, just enable Thumb State bit;
	*(--psp) = (uint32_t) (handler);// PC
	*(--psp) = 0xFFFFFFFDu; 		// LR with EXC_RETURN to return to Thread using PSP
	psp-=(16-3);

	/* Add task */
	task[idx].id = id;
	task[idx].handler = handler;
	task[idx].state = TASK_RUNNING;
	task[idx].stack_size = stack_size;
	task[idx].wait_tick = 0;
	task[idx].psp = (uint32_t)psp;

	if(idx!=0)	//Skip the idle task
		_console(TASK_CONSOLE_INFO,__func__, "Handler %p is added successfully with id %d.\n",handler,id);

    if(_isPSP() && (priority!=-1)){
    	current_task->priority = 0;
    }

	if(interrupt_enabled)
		__enable_irq();

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
		_console(TASK_CONSOLE_ERROR,__func__, "ID %d is not associated with any handler.\n",id);
		return 0;
	}

	_console(TASK_CONSOLE_INFO,__func__, "Handler %p associated with id %d is restarted.\n",task[idx].handler,id);
	task[idx].state = TASK_RESTART;
	if(current_task->id==id)
		while(1);//self restart
	return 1;
}

/**
 * This Restart the this task
 * @return	: 1 => Successfully restarted
 *			: 0 => Failed to restart
 */
int task_restartSelf(){
	return task_restart(current_task->id);
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
		_console(TASK_CONSOLE_ERROR,__func__, "ID %d is not associated with any handler.\n",id);
		return 0;
	}else if(task[idx].state == TASK_PAUSE){
		_console(TASK_CONSOLE_WARNING,__func__, "Handler %p associated with id %d is already paused.\n",task[idx].handler,id);
		return 1;
	}

	_console(TASK_CONSOLE_INFO,__func__, "Handler %p associated with id %d is paused.\n",task[idx].handler,id);
	task[idx].state = TASK_PAUSE;
	if(current_task->id == id)
		while(current_task->state==TASK_PAUSE);	//Self pause
	return 1;
}

/**
 * This pause the this task
 * @return	: 1 => Successfully paused
 *			: 0 => Failed to pause
 */
int task_pauseSelf(){
	return task_pause(current_task->id);
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
		_console(TASK_CONSOLE_ERROR,__func__, "ID %d is not associated with any handler.\n",id);
		return 0;
	}else if(task[idx].state == TASK_RUNNING){
		_console(TASK_CONSOLE_WARNING,__func__, "Handler %p associated with id %d is already running.\n",task[idx].handler,id);
		return 1;
	}

	_console(TASK_CONSOLE_INFO,__func__, "Handler %p associated with id %d is resumed.\n",task[idx].handler,id);
	task[idx].state = TASK_RUNNING;
	return 1;
}

/**
 * This delete the task
 * @param id: ID associated with task
 * @return	: 1 => Successfully deleted
 *			: 0 => Failed to delete
 */
int task_delete(uint32_t id){
	int idx = 0;
	for(;idx<TASK_MAX_NUMBER;idx++)
		if(task[idx].psp!=0 && task[idx].id == id)
			break;

	if(idx>=TASK_MAX_NUMBER){
		_console(TASK_CONSOLE_ERROR,__func__, "ID %d is not associated with any handler.\n",id);
		return 0;
	}

	_console(TASK_CONSOLE_INFO,__func__, "Handler %p associated with id %d is deleted.\n",task[idx].handler,id);

	task[idx].state = TASK_DELETE;
	if(current_task->id==id)
		while(1);
	return 1;
}

/**
 * This delete the task
 * @return	: 1 => Successfully delete
 *			: 0 => Failed to delete
 */
int task_deleteSelf(){
	return task_delete(current_task->id);
}

/**
 * This priorities the task. These block all other tasks.
 * @return	: 1 => Successfully enabled
 *			: 0 => Failed to enable
 */
int task_enableHighestPriority(){
	if(current_task->priority == -1){
		_console(TASK_CONSOLE_WARNING,__func__, "Handler %p associated with id %d is already enabled as highest priority task.\n",current_task->handler,current_task->id);
		return 1;
	}

	current_task->priority = -1;
	_console(TASK_CONSOLE_INFO,__func__, "Handler %p associated with id %d is enabled as highest priority task.\n",current_task->handler,current_task->id);
	return 1;
}

/**
 * This disable prioritized task.
 * @return	: 1 => Successfully disabled
 *			: 0 => Failed to disabled
 */
int task_disableHighestPriority(){
	if(current_task->priority == 0){
		_console(TASK_CONSOLE_WARNING,__func__, "Handler %p associated with id %d is not highest priority task.\n",current_task->handler,current_task->id);
		return 1;
	}

	current_task->priority = 0;
	_console(TASK_CONSOLE_INFO,__func__, "Handler %p associated with id %d is disabled as highest priority task.\n",current_task->handler,current_task->id);
	return 1;
}


/**
 * This delay the task
 * @param ticks	: time in ticks
 */
void task_delay(uint32_t ticks){
	current_task->wait_tick = global_ticks+ticks;
	while(current_task->wait_tick>global_ticks);
}

/**
 * This starts the scheduler from main
 */
void task_startScheduler(){
	uint32_t interrupt_enabled = !__get_PRIMASK();
	__disable_irq();

	/* prepare PSP of the first task */
	__asm volatile("BL _task_getCurrentPSP");	// return PSP in R0
	__asm volatile("MSR PSP, R0");		// set PSP

	/* change to use PSP */
	__asm volatile("MRS R0, CONTROL");
	__asm volatile("ORR R0, R0, #2");	// set bit[1] SPSEL
	__asm volatile("MSR CONTROL, R0");

	if(interrupt_enabled)
		__enable_irq();

	/* Move to Unprivileged level */
	__asm volatile("MRS R0, CONTROL");
	__asm volatile("ORR R0, R0, #1");	// Set bit[0] nPRIV
	__asm volatile("MSR CONTROL, R0");



	/* get the handler of the first task by tracing back from PSP which is at R4 slot */
	void (*handler)() = (void (*))((uint32_t*)current_task->psp)[8+6];
	scheduler_started = 1;
	/* execute the handler */
	handler();
}

/**
 * This is printf for task
 */
void task_printf(char* msg,...){
	int8_t priority;
	if(_isPSP()){
		priority = current_task->priority;
		current_task->priority = -1;
	}

	va_list args;
	va_start(args, msg);
	vprintf(msg, args);
    va_end(args);

    if(_isPSP() && (priority!=-1))
    	current_task->priority = 0;
}

/**
 * It returns the function if status true(1) or do nothing
 * @param status	: Status
 */
__attribute__((naked)) void task_return(uint8_t status){
	if(status){
		__asm volatile("POP {LR}");
		__asm volatile("POP {LR}");
	}
	__asm volatile("BX LR");
}


/////////////////////////////SEMAOHORE////////////////////////////////////
/**
 * Create semaphore
 * @param type		: Semaphore type BINARY or MUTEX
 * @return			: TaskSemaphore
 */
TaskSemaphore task_createSemaphore(TaskSemaphoreType type){
	TaskSemaphore taskSemaphore = {.type = type};
	return taskSemaphore;
}

/**
 * Take semaphore
 * If semaphore is binary this task will wait for other task to give semaphore
 * If semaphore is mutex then only this task will run until this task give semaphore
 *
 * @param semaphore		: Task semaphore
 */
void task_takeSemaphore(TaskSemaphore* semaphore){
	_console(TASK_CONSOLE_INFO,__func__, "Handler %p associated with id %d took semaphore.\n",current_task->handler,current_task->id);
	semaphore->take = 1;
	semaphore->task = current_task;
	if(semaphore->type==TASK_SEMAPHORE_BINARY){
		while(semaphore->take);
	}else if(semaphore->type==TASK_SEMAPHORE_MUTEX){
		semaphore->prevPriority = current_task->priority;
		current_task->priority = -1;
	}
}

/**
 * Give semaphore
 * @param semaphore		: Task semaphore
 */
void task_giveSemaphore(TaskSemaphore* semaphore){
	if(semaphore->type==TASK_SEMAPHORE_MUTEX){
		if(semaphore->task==current_task){
			semaphore->take = 0;
			semaphore->task->priority=semaphore->prevPriority;
		}else{
			_console(TASK_CONSOLE_ERROR,__func__, "Handler %p associated with id %d can't give mutex semaphore of handler %p.\n",current_task->handler,current_task->id,semaphore->task->handler);
			return;
		}
	}else{
		semaphore->take = 0;
	}
	_console(TASK_CONSOLE_INFO,__func__, "Handler %p associated with id %d gave semaphore of handler %p.\n",current_task->handler,current_task->id,semaphore->task->handler);

}




