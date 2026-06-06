/*
 * FreeRTOS Kernel <DEVELOPMENT BRANCH>
 * Copyright (C) 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

/*
 * FreeRTOS Kernel RISC-V port for WCH QingKe V3F (CH32H417).
 *
 * The QingKe V3F uses vectored interrupt mode (mtvec[1:0] = 3).
 * Interrupts are dispatched directly by hardware to their respective
 * vector entries, so there is no unified trap handler.
 *
 * This file provides:
 *   - pxPortInitialiseStack()   Task stack initialization
 *   - xPortStartFirstTask()      Start the first task
 *   - Ecall_M_Mode_Handler       Handles ecall (context switch / yield)
 *   - SysTick0_Handler           Handles the RTOS tick interrupt
 *
 * Include this file's directory in the assembler include path so that
 * portcontext.h can find freertos_risc_v_chip_specific_extensions.h.
 */

#include "portcontext.h"

.global xPortStartFirstTask
.global pxPortInitialiseStack
.global Ecall_M_Mode_Handler
.global SysTick0_Handler

.extern vTaskSwitchContext
.extern xTaskIncrementTick
.extern xTaskReturnAddress
/*-----------------------------------------------------------*/

/*
 * pxPortInitialiseStack() is written in assembly to access portasmADDITIONAL_CONTEXT_SIZE.
 *
 * RISC-V calling convention:
 *   a0 = pxTopOfStack
 *   a1 = pxCode (task function)
 *   a2 = pvParameters
 *
 * Stack frame layout (grows downward from top):
 *   pxCriticalNesting        <- sp after init
 *   x31 (t6)
 *   x30 (t5)
 *   x29 (t4)
 *   x28 (t3)                 <- portCONTEXT_SIZE includes these 22 regs
 *   x27 (s11)
 *   x26 (s10)
 *   x25 (s9)
 *   x24 (s8)
 *   x23 (s7)
 *   x22 (s6)
 *   x21 (s5)
 *   x20 (s4)
 *   x19 (s3)
 *   x18 (s2)
 *   x17 (a7)
 *   x16 (a6)
 *   x15 (a5)
 *   x14 (a4)
 *   x13 (a3)
 *   x12 (a2)
 *   x11 (a1)
 *   pvParameters (x10/a0)
 *   x9 (s1)
 *   x8 (s0/fp)
 *   x7 (t2)
 *   x6 (t1)
 *   x5 (t0)
 *   xTaskReturnAddress (x1/ra)
 *   mstatus
 *   [chip specific additional regs]
 *   pxCode (mepc value)
 */
pxPortInitialiseStack:
    addi a0, a0, -portWORD_SIZE         /* Space for critical nesting count. */
    store_x x0, 0(a0)                   /* Critical nesting count starts at 0 for every task. */

    addi a0, a0, -(22 * portWORD_SIZE)  /* Space for registers x10-x31. */
    store_x a2, 0(a0)                   /* Task parameters (pvParameters) goes into x10/a0 slot. */

    addi a0, a0, -(6 * portWORD_SIZE)   /* Space for registers x5-x9 + taskReturnAddress (x1). */
    load_x t0, xTaskReturnAddress
    store_x t0, 0(a0)                   /* Return address onto the stack. */

    csrr t0, mstatus                    /* Obtain current mstatus value. */
    andi t0, t0, ~0x8                   /* Ensure interrupts are disabled when the stack is restored within an ISR. */
    addi t1, x0, 0x188                  /* Generate the value 0x1880 (MPIE=1, MPP=M_Mode). */
    slli t1, t1, 4
    or t0, t0, t1                       /* Set MPIE and MPP bits in mstatus value. */

    addi a0, a0, -portWORD_SIZE
    store_x t0, 0(a0)                   /* mstatus onto the stack. */

    addi t0, x0, portasmADDITIONAL_CONTEXT_SIZE /* Number of chip-specific additional registers. */
chip_specific_stack_frame:
    beq t0, x0, 1f                      /* No more chip-specific registers to save. */
    addi a0, a0, -portWORD_SIZE
    store_x x0, 0(a0)                   /* Give the chip-specific register an initial value of zero. */
    addi t0, t0, -1
    j chip_specific_stack_frame
1:

    addi a0, a0, -portWORD_SIZE
    store_x a1, 0(a0)                   /* mret value (pxCode parameter) onto the stack. */
    ret
/*-----------------------------------------------------------*/

/*
 * xPortStartFirstTask()
 *
 * Loads the initial context from the first task's TCB and performs
 * the equivalent of an mret to start executing the task.
 */
xPortStartFirstTask:
    load_x  sp, pxCurrentTCB            /* Load pxCurrentTCB. */
    load_x  sp, 0( sp )                 /* Read sp from first TCB member. */

    load_x  x1, 0( sp ) /* Note: for starting the scheduler, the exception return address is used as the function return address. */

    portasmRESTORE_ADDITIONAL_REGISTERS  /* Restore any chip-specific registers. */

    load_x  x5, 1 * portWORD_SIZE( sp ) /* Initial mstatus into x5 (t0). */
    addi    x5, x5, 0x08                /* Set MIE bit so the first task starts with interrupts enabled. */
    csrw    mstatus, x5                 /* Interrupts enabled from here! */

    load_x  x7,  5  * portWORD_SIZE( sp )   /* t2 */
    load_x  x8,  6  * portWORD_SIZE( sp )   /* s0/fp */
    load_x  x9,  7  * portWORD_SIZE( sp )   /* s1 */
    load_x  x10, 8  * portWORD_SIZE( sp )   /* a0 */
    load_x  x11, 9  * portWORD_SIZE( sp )   /* a1 */
    load_x  x12, 10 * portWORD_SIZE( sp )   /* a2 */
    load_x  x13, 11 * portWORD_SIZE( sp )   /* a3 */
    load_x  x14, 12 * portWORD_SIZE( sp )   /* a4 */
    load_x  x15, 13 * portWORD_SIZE( sp )   /* a5 */
    load_x  x16, 14 * portWORD_SIZE( sp )   /* a6 */
    load_x  x17, 15 * portWORD_SIZE( sp )   /* a7 */
    load_x  x18, 16 * portWORD_SIZE( sp )   /* s2 */
    load_x  x19, 17 * portWORD_SIZE( sp )   /* s3 */
    load_x  x20, 18 * portWORD_SIZE( sp )   /* s4 */
    load_x  x21, 19 * portWORD_SIZE( sp )   /* s5 */
    load_x  x22, 20 * portWORD_SIZE( sp )   /* s6 */
    load_x  x23, 21 * portWORD_SIZE( sp )   /* s7 */
    load_x  x24, 22 * portWORD_SIZE( sp )   /* s8 */
    load_x  x25, 23 * portWORD_SIZE( sp )   /* s9 */
    load_x  x26, 24 * portWORD_SIZE( sp )   /* s10 */
    load_x  x27, 25 * portWORD_SIZE( sp )   /* s11 */
    load_x  x28, 26 * portWORD_SIZE( sp )   /* t3 */
    load_x  x29, 27 * portWORD_SIZE( sp )   /* t4 */
    load_x  x30, 28 * portWORD_SIZE( sp )   /* t5 */
    load_x  x31, 29 * portWORD_SIZE( sp )   /* t6 */

    load_x  x5, portCRITICAL_NESTING_OFFSET * portWORD_SIZE( sp )    /* Obtain xCriticalNesting for this task. */
    load_x  x6, pxCriticalNesting           /* Load the address of xCriticalNesting into x6. */
    store_x x5, 0( x6 )                     /* Restore the critical nesting value. */

    load_x  x5, 3 * portWORD_SIZE( sp )     /* Initial x5 (t0) value. */
    load_x  x6, 4 * portWORD_SIZE( sp )     /* Initial x6 (t1) value. */

    addi    sp, sp, portCONTEXT_SIZE
    ret
/*-----------------------------------------------------------*/

/*
 * Ecall_M_Mode_Handler
 *
 * Handles the ECALL instruction from M-mode, which FreeRTOS uses as
 * the yield / context switch mechanism (portYIELD() issues "ecall").
 *
 * In the QingKe V3F vectored table, this is vector entry 5.
 * Overrides the weak definition in startup_ch32h417_v3f.S.
 */
.section .text.vector_handler, "ax", @progbits
.align 1
Ecall_M_Mode_Handler:
    portcontextSAVE_EXCEPTION_CONTEXT
    /* a0 now contains mcause (should be 11 for environment call from M-mode). */
    call vTaskSwitchContext
    portcontextRESTORE_CONTEXT
/*-----------------------------------------------------------*/

/*
 * SysTick0_Handler
 *
 * Handles the SysTick0 interrupt, which serves as the FreeRTOS tick
 * interrupt on the QingKe V3F.
 *
 * In the QingKe V3F vectored table, this is vector entry 12 (0-based).
 * Overrides the weak definition in startup_ch32h417_v3f.S.
 */
.align 1
SysTick0_Handler:
    portcontextSAVE_INTERRUPT_CONTEXT
    call xTaskIncrementTick
    beqz a0, 1f                          /* Don't switch context if tick didn't unblock a task. */
    call vTaskSwitchContext
1:
    portcontextRESTORE_CONTEXT
/*-----------------------------------------------------------*/