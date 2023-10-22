/*
 * function_call_in_process_stack.c
 *
 *  Created on: Oct 18, 2023
 *      Author: peter
 */

/**
 * r7 		: stores stack pointer
 * r14/lr 	: Link register
 */

#include "main.h"

static void voidEmptyFunc() {
	/**
	 * Prologue
	 * push {r7}
	 * add	r7, sp, #0
	 */

	/**
	 * Epilogue
	 * mov	sp, r7
	 * pop	{r7}
	 * bx 	lr
	 */
}

static void voidFunc() {
	/**
	 * Prologue
	 * push {r7}
	 * sub	sp, #12
	 * add	r7, sp, #0
	 */

	uint8_t x = 0xAA;
	/**
	 * mov	r3, 0xAA
	 * strb	r3, [r7, #7]
	 */
	uint16_t y = 0xBBBB;
	/**
	 * mov	r3, 0xBBBB
	 * strh	r3, [r7, #4]
	 */
	uint32_t z = 0xCCCCCCCC;
	/**
	 * mov	r3, 0xCCCCCCCC
	 * str	r3, [r7, #0]
	 */
	z = x + y + z;
	/**
	 * ldrb	r2, [r7, #7]
	 * ldrh	r3, [r7, #4]
	 * add	r3, r2
	 * mov	r2, r3
	 * ldr	r3, [r7, #0]
	 * add	r3, r2
	 * str	r3, [r7, #0]
	 */

	/**
	 * Epilogue
	 * add	r7, #12
	 * mov	sp, r7
	 * pop	{r7}
	 * bx 	lr
	 */
}

static void voidFuncWithCall() {
	/**
	 * Prologue
	 * push {r7, lr}
	 * add	r7, sp, #0
	 */

	voidEmptyFunc();
	/**
	 * bl	voidEmptyFunc
	 */

	/**
	 * Epilogue
	 * pop	{r7, pc}
	 */
}

static void voidEmptyFuncWithArg(uint8_t x) {
	/**
	 * Prologue
	 * push {r7}
	 * sub	sp, #12
	 * add	r7, sp, #0
	 * mov	r3, r0
	 * strb	r3, [sp, #7]
	 */

	/**
	 * Epilogue
	 * add	r7, 12
	 * mov	sp, r7
	 * pop	{r7}
	 * bx 	lr
	 */
}

__attribute__ ((naked)) static void nakedVoidEmptyFunc() {
	/**
	 * nop
	 */
}

__attribute__ ((naked)) static void nakedVoidFunc() {
	uint8_t x = 0xAA;
	/**
	 * nop
	 */
}

__attribute__ ((naked)) static void nakedVoidFuncWithCall() {
	nakedVoidEmptyFunc();
	/**
	 * bl	nakedVoidEmptyFunc
	 */
}

__attribute__ ((naked)) static void nakedVoidEmptyFuncWithArg(uint8_t x) {
	/**
	 * nop
	 */
}

void funtion_call_in_process_stack_run() {
//	printf("%s : RUNNING\n", __func__);

//	voidEmptyFunc();
//	voidFunc();
//	voidFuncWithCall();
//	voidEmptyFuncWithArg(0xA);

//	nakedVoidEmptyFunc();
//	nakedVoidFunc();
//	nakedVoidFuncWithCall();
//	nakedVoidEmptyFuncWithArg(0xA);

//	while (1) {
//
//	}
}
