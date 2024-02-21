//#define Comparision_task

#ifdef Comparision_task

#include "stm32_thread_OS.h"
#include "main.h"
#include "stdio.h"
#include "stdarg.h"
#include "math.h"

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

void SVC_Handler(void) {
	StaticThread.SVCHandler();
}

void PendSV_Handler(void) {
	StaticThread.PendSVHandler();
}

void SysTick_Handler(void) {
	HAL_IncTick();
	StaticThread.SysTickHandler();
}

////////////////////////////CALC/////////////////////////////////
int isPrime(int number){
    if(number==2)
        return 1;
    // if(startingNumber<2)
    //     startingNumber = 2;
    if(number<2 || (number%2==0))
        return 0;
    int sqrRoot = (int)sqrt(number);
    for(int n=3;n<=sqrRoot;n++){
        if(number%n==0)
            return 0;
    }
    return 1;
}

int getNPrime(int n){
    int countPrime = 0;
    int number = 0;
    while(1){
        if(isPrime(number)){
            countPrime++;
            if(countPrime==n)
                return number;
        }
        number++;
    }
}

int isPallindrome(int n){
    int temp = n;
    int rev = 0;
    while(temp!=0){
        int r = temp%10;
        rev = 10*rev+r;
        temp/=10;
    }
    if(n==rev)
        return 1;
    else
        return 0;
}

int getNPallindrome(int n){
   int countPall = 0;
   int num = 0;
   while(1){
       if(isPallindrome(num)){
    	   countPall++;
    	   if(countPall == n)
    		   return num;

       }
        num++;
   }
}
//////////////////////////THREAD/////////////////////////
#define STACK_SIZE 256

int id1, idPrime, idPallindrome;

uint32_t stack1[STACK_SIZE];
uint32_t stackPrime[STACK_SIZE];
uint32_t stackPallindrome[STACK_SIZE];


typedef enum{
	START,
	CALULATING,
	COMPLETED
}STATUS;

void taskPrime(int argLen, void **args){
	STATUS *statusPtr = args[0];
	*statusPtr = CALULATING;
	int *nPtr = args[1];
	int *primePtr = args[2];
	*primePtr = getNPrime(*nPtr);
	*statusPtr = COMPLETED;
	StaticThread.delete(0);
}

void taskPallindrome(int argLen, void **args){
	STATUS *statusPtr = args[0];
	statusPtr[0] = CALULATING;
	int *nPtr = args[1];
	int *pallinPtr = args[2];
	pallinPtr[0] = getNPallindrome(nPtr[0]);
	statusPtr[0] = COMPLETED;
	StaticThread.delete(0);
}


void task1(int argLen, void **args){
	int n=1;
	int primeStatus = START;
	int primeCalculated = -1;
	void *primeArgs[3]={&primeStatus,&n,&primeCalculated};

	int pallinStatus = START;
	int pallinCalculated = -1;
	void *pallinArgs[3]={&pallinStatus,&n,&pallinCalculated};
	while(1){
		if(primeStatus==START){
			StaticThread.new(taskPrime, stackPrime, sizeof(stackPrime)/sizeof(uint32_t), 3, primeArgs);
			StaticThread.delay(10);
		}

		if(pallinStatus==START){
			StaticThread.new(taskPallindrome, stackPallindrome, sizeof(stackPallindrome)/sizeof(uint32_t), 3, pallinArgs);
			StaticThread.delay(10);
		}


		printf("\n");
		if(primeStatus==COMPLETED && pallinStatus==COMPLETED){
			StaticThread.print("Position : %d\n",n);
			StaticThread.print("Prime    : %d\n",primeCalculated);
			StaticThread.print("Pallin   : %d\n",pallinCalculated);
			n+=100;
			// Start next calculation
			primeStatus = START;
			pallinStatus = START;
		}else if(primeStatus==COMPLETED){
			StaticThread.print("Position : %d\n",n);
			StaticThread.print("Prime    : %d\n",primeCalculated);
			StaticThread.print("Pallin   : ........\n");
		}else if(pallinStatus==COMPLETED){
			StaticThread.print("Position : %d\n",n);
			StaticThread.print("Prime    : ........\n");
			StaticThread.print("Pallin   : %d\n",pallinCalculated);
		}else{
			StaticThread.print("Position : %d\n",n);
			StaticThread.print("Prime    : ........\n");
			StaticThread.print("Pallin   : ........\n");
		}
		StaticThread.delay(1000);
	}
}

void run() {
	printf("Initializing...\n");
	HAL_Delay(3000);
	StaticThread.new(task1, stack1, sizeof(stack1)/sizeof(uint32_t),0,NULL);
	StaticThread.startScheduler();
}

#endif
