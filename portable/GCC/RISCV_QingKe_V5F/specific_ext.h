/*
 * FreeRTOS Kernel V10.4.6
 * Copyright (C) 2021 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
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
 * Chip specific extension header for the WCH QingKe V5F core (CH32H417).
 *
 * The V5F is a RV32IMAFC core with a hardware FPU.  Unlike the official WCH
 * FreeRTOS V5F port (which does not save any FP context at all), this port
 * eagerly saves/restores f0-f31 plus fcsr on every context switch.
 *
 * Frame layout (33 words = 132 bytes, see portASM.S):
 *
 *   +132  <- sp before portasmSAVE_ADDITIONAL_REGISTERS
 *   +128  fcsr        (word 32)
 *   +124  f31         (word 31)
 *    ...
 *   +  4  f1          (word 1)
 *   +  0  f0          (word 0) <- sp after portasmSAVE_ADDITIONAL_REGISTERS
 *
 * 33 words keeps the complete initial task frame at 30 + 33 + 1 = 64 words
 * (256 bytes), so the initial task sp stays 16-byte aligned as required by
 * the ilp32f ABI.
 *
 * NOTE: the historical V3F version of these macros allocated only 32 words
 * while storing f31 at word offset 32 - one word past the allocated frame,
 * silently clobbering the adjacent context slot.  Fixed here.
 */

#ifndef __FREERTOS_RISC_V_EXTENSIONS_H__
#define __FREERTOS_RISC_V_EXTENSIONS_H__

#define portasmHAS_SIFIVE_CLINT 0
#define portasmHAS_MTIME 0

/* The V5F has a hardware FPU; build with -march=rv32imafc... -mabi=ilp32f.
 * Set ARCH_FPU to 0 (and use the soft-float ABI) to drop the FP context. */
#define ARCH_FPU 1


#if ARCH_FPU

/* f0-f31 (32 words) + fcsr (1 word). */
#define portasmADDITIONAL_CONTEXT_SIZE 33

.macro portasmSAVE_ADDITIONAL_REGISTERS
    addi sp, sp, -(portasmADDITIONAL_CONTEXT_SIZE * portWORD_SIZE)
    fsw f0,  0 * portWORD_SIZE(sp)
    fsw f1,  1 * portWORD_SIZE(sp)
    fsw f2,  2 * portWORD_SIZE(sp)
    fsw f3,  3 * portWORD_SIZE(sp)
    fsw f4,  4 * portWORD_SIZE(sp)
    fsw f5,  5 * portWORD_SIZE(sp)
    fsw f6,  6 * portWORD_SIZE(sp)
    fsw f7,  7 * portWORD_SIZE(sp)
    fsw f8,  8 * portWORD_SIZE(sp)
    fsw f9,  9 * portWORD_SIZE(sp)
    fsw f10, 10 * portWORD_SIZE(sp)
    fsw f11, 11 * portWORD_SIZE(sp)
    fsw f12, 12 * portWORD_SIZE(sp)
    fsw f13, 13 * portWORD_SIZE(sp)
    fsw f14, 14 * portWORD_SIZE(sp)
    fsw f15, 15 * portWORD_SIZE(sp)
    fsw f16, 16 * portWORD_SIZE(sp)
    fsw f17, 17 * portWORD_SIZE(sp)
    fsw f18, 18 * portWORD_SIZE(sp)
    fsw f19, 19 * portWORD_SIZE(sp)
    fsw f20, 20 * portWORD_SIZE(sp)
    fsw f21, 21 * portWORD_SIZE(sp)
    fsw f22, 22 * portWORD_SIZE(sp)
    fsw f23, 23 * portWORD_SIZE(sp)
    fsw f24, 24 * portWORD_SIZE(sp)
    fsw f25, 25 * portWORD_SIZE(sp)
    fsw f26, 26 * portWORD_SIZE(sp)
    fsw f27, 27 * portWORD_SIZE(sp)
    fsw f28, 28 * portWORD_SIZE(sp)
    fsw f29, 29 * portWORD_SIZE(sp)
    fsw f30, 30 * portWORD_SIZE(sp)
    fsw f31, 31 * portWORD_SIZE(sp)
    csrr t0, fcsr
    store_x t0, 32 * portWORD_SIZE(sp)
.endm

.macro portasmRESTORE_ADDITIONAL_REGISTERS
    load_x t0, 32 * portWORD_SIZE(sp)
    csrw fcsr, t0
    flw f0,  0 * portWORD_SIZE(sp)
    flw f1,  1 * portWORD_SIZE(sp)
    flw f2,  2 * portWORD_SIZE(sp)
    flw f3,  3 * portWORD_SIZE(sp)
    flw f4,  4 * portWORD_SIZE(sp)
    flw f5,  5 * portWORD_SIZE(sp)
    flw f6,  6 * portWORD_SIZE(sp)
    flw f7,  7 * portWORD_SIZE(sp)
    flw f8,  8 * portWORD_SIZE(sp)
    flw f9,  9 * portWORD_SIZE(sp)
    flw f10, 10 * portWORD_SIZE(sp)
    flw f11, 11 * portWORD_SIZE(sp)
    flw f12, 12 * portWORD_SIZE(sp)
    flw f13, 13 * portWORD_SIZE(sp)
    flw f14, 14 * portWORD_SIZE(sp)
    flw f15, 15 * portWORD_SIZE(sp)
    flw f16, 16 * portWORD_SIZE(sp)
    flw f17, 17 * portWORD_SIZE(sp)
    flw f18, 18 * portWORD_SIZE(sp)
    flw f19, 19 * portWORD_SIZE(sp)
    flw f20, 20 * portWORD_SIZE(sp)
    flw f21, 21 * portWORD_SIZE(sp)
    flw f22, 22 * portWORD_SIZE(sp)
    flw f23, 23 * portWORD_SIZE(sp)
    flw f24, 24 * portWORD_SIZE(sp)
    flw f25, 25 * portWORD_SIZE(sp)
    flw f26, 26 * portWORD_SIZE(sp)
    flw f27, 27 * portWORD_SIZE(sp)
    flw f28, 28 * portWORD_SIZE(sp)
    flw f29, 29 * portWORD_SIZE(sp)
    flw f30, 30 * portWORD_SIZE(sp)
    flw f31, 31 * portWORD_SIZE(sp)
    addi sp, sp, (portasmADDITIONAL_CONTEXT_SIZE * portWORD_SIZE)
.endm

#else /* ARCH_FPU */

#define portasmADDITIONAL_CONTEXT_SIZE 0 /* Must be even number on 32-bit cores. */


.macro portasmSAVE_ADDITIONAL_REGISTERS
/* No additional registers to save, so this macro does nothing. */
.endm

.macro portasmRESTORE_ADDITIONAL_REGISTERS
/* No additional registers to restore, so this macro does nothing. */
.endm

#endif /* ARCH_FPU */

#endif /* __FREERTOS_RISC_V_EXTENSIONS_H__ */
