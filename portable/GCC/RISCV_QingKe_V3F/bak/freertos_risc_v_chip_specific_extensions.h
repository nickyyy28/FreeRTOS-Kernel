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
 * Chip-specific extensions for the WCH QingKe V3F (CH32H417) RISC-V core.
 *
 * The QingKe V3F is an RV32IMAC core with WCH-specific extensions (xw).
 * It uses a PFIC interrupt controller (not standard CLINT/PLIC) with
 * SysTick timers for the tick interrupt.  There is no memory-mapped
 * mtime/mtimecmp hardware timer.
 *
 * Include this file in the assembler's include path so it is included
 * by portcontext.h.
 */

#ifndef __FREERTOS_RISC_V_EXTENSIONS_H__
#define __FREERTOS_RISC_V_EXTENSIONS_H__

#define portasmHAS_SIFIVE_CLINT           0
#define portasmHAS_MTIME                  0
#define portasmADDITIONAL_CONTEXT_SIZE    0 /* No extra chip registers to save. */

.macro portasmSAVE_ADDITIONAL_REGISTERS
/* No additional registers to save, so this macro does nothing. */
   .endm

   .macro portasmRESTORE_ADDITIONAL_REGISTERS
/* No additional registers to restore, so this macro does nothing. */
   .endm

#endif /* __FREERTOS_RISC_V_EXTENSIONS_H__ */