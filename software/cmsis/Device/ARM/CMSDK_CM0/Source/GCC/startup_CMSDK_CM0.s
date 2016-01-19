/**************************************************************************//**
 * @file     startup_CMSDK_CM0.s
 * @brief    CMSIS Cortex-M0 Core Device Startup File for
 *           Device CMSDK_CM0
 * @version  V3.01
 * @date     06. March 2012
 *
 * @note      Should use with GCC for ARM Embedded Processors
 * Copyright (C) 2012 ARM Limited. All rights reserved.
 *
 * @par
 * ARM Limited (ARM) is supplying this software for use with Cortex-M
 * processor based microcontrollers.  This file can be freely distributed
 * within development tools that are supporting such ARM based processors.
 *
 * @par
 * THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 * OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 * ARM SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 ******************************************************************************/
/*****************************************************************************/
/* startup_CMSDK_CM3.s: Startup file for CMSDK device series               */
/*****************************************************************************/
/* Version: GNU Tools for ARM Embedded Processors                          */
/*****************************************************************************/


    .syntax unified
    .arch armv6-m

    .section .stack
    .align 3

/*
// <h> Stack Configuration
//   <o> Stack Size (in Bytes) <0x0-0xFFFFFFFF:8>
// </h>
*/

    .section .stack
    .align 3
#ifdef __STACK_SIZE
    .equ    Stack_Size, __STACK_SIZE
#else
    .equ    Stack_Size, 0x200
#endif
    .globl    __StackTop
    .globl    __StackLimit
__StackLimit:
    .space    Stack_Size
    .size __StackLimit, . - __StackLimit
__StackTop:
    .size __StackTop, . - __StackTop


/*
// <h> Heap Configuration
//   <o>  Heap Size (in Bytes) <0x0-0xFFFFFFFF:8>
// </h>
*/

    .section .heap
    .align 3
#ifdef __HEAP_SIZE
    .equ    Heap_Size, __HEAP_SIZE
#else
    .equ    Heap_Size, 0
#endif
    .globl    __HeapBase
    .globl    __HeapLimit
__HeapBase:
    .if    Heap_Size
    .space    Heap_Size
    .endif
    .size __HeapBase, . - __HeapBase
__HeapLimit:
    .size __HeapLimit, . - __HeapLimit


/* Vector Table */

    .section .isr_vector
    .align 2
    .globl __isr_vector
__isr_vector:
    .long   __StackTop                  /* Top of Stack                  */
    .long   Reset_Handler               /* Reset Handler                 */
    .long   NMI_Handler                 /* NMI Handler                   */
    .long   HardFault_Handler           /* Hard Fault Handler            */
    .long   0                           /* Reserved                      */
    .long   0                           /* Reserved                      */
    .long   0                           /* Reserved                      */
    .long   0                           /* Reserved                      */
    .long   0                           /* Reserved                      */
    .long   0                           /* Reserved                      */
    .long   0                           /* Reserved                      */
    .long   SVC_Handler                 /* SVCall Handler                */
    .long   0                           /* Debug Monitor Handler         */
    .long   0                           /* Reserved                      */
    .long   PendSV_Handler              /* PendSV Handler                */
    .long   SysTick_Handler             /* SysTick Handler               */

    /* External Interrupts */
    .long   UARTRX0_Handler             /* 16+ 0: UART 0 RX Handler          */
    .long   UARTTX0_Handler             /* 16+ 1: UART 0 TX Handler          */
    .long   UARTRX1_Handler             /* 16+ 2: UART 1 RX Handler          */
    .long   UARTTX1_Handler             /* 16+ 3: UART 1 TX Handler          */
    .long   UARTRX2_Handler             /* 16+ 4: UART 2 RX Handler          */
    .long   UARTTX2_Handler             /* 16+ 5: UART 2 TX Handler          */
    .long   PORT0_COMB_Handler          /* 16+ 6: GPIO Port 0 Combined Handler */
    .long   PORT1_COMB_Handler          /* 16+ 7: GPIO Port 1 Combined Handler */
    .long   TIMER0_Handler              /* 16+ 8: TIMER 0 handler            */
    .long   TIMER1_Handler              /* 16+ 9: TIMER 1 handler            */
    .long   DUALTIMER_HANDLER           /* 16+10: Dual timer 2 handler       */
    .long   0                           /* 16+11: Reserved                   */
    .long   UARTOVF0_Handler            /* 16+12: UART 0 Overflow Handler    */
    .long   UARTOVF1_Handler            /* 16+13: UART 1 Overflow Handler    */
    .long   UARTOVF2_Handler            /* 16+14: UART 2 Overflow Handler    */
    .long   DMA_Handler                 /* 16+15: DMA done + error Handler   */
    .long   PORT0_0_Handler             /* 16+16: GPIO Port 0 pin 0 Handler  */
    .long   PORT0_1_Handler             /* 16+17: GPIO Port 0 pin 1 Handler  */
    .long   PORT0_2_Handler             /* 16+18: GPIO Port 0 pin 2 Handler  */
    .long   PORT0_3_Handler             /* 16+19: GPIO Port 0 pin 3 Handler  */
    .long   PORT0_4_Handler             /* 16+20: GPIO Port 0 pin 4 Handler  */
    .long   PORT0_5_Handler             /* 16+21: GPIO Port 0 pin 5 Handler  */
    .long   PORT0_6_Handler             /* 16+22: GPIO Port 0 pin 6 Handler  */
    .long   PORT0_7_Handler             /* 16+23: GPIO Port 0 pin 7 Handler  */
    .long   PORT0_8_Handler             /* 16+24: GPIO Port 0 pin 8 Handler  */
    .long   PORT0_9_Handler             /* 16+25: GPIO Port 0 pin 9 Handler  */
    .long   PORT0_10_Handler            /* 16+26: GPIO Port 0 pin 10 Handler */
    .long   PORT0_11_Handler            /* 16+27: GPIO Port 0 pin 11 Handler */
    .long   PORT0_12_Handler            /* 16+28: GPIO Port 0 pin 12 Handler */
    .long   PORT0_13_Handler            /* 16+29: GPIO Port 0 pin 13 Handler */
    .long   PORT0_14_Handler            /* 16+30: GPIO Port 0 pin 14 Handler */
    .long   PORT0_15_Handler            /* 16+31: GPIO Port 0 pin 15 Handler */

    .size    __isr_vector, . - __isr_vector

/* Reset Handler */
    .text
    .thumb
    .thumb_func
    .align 2
    .globl    Reset_Handler
    .type    Reset_Handler, %function
Reset_Handler:
/*     Loop to copy data from read only memory to RAM. The ranges
 *      of copy from/to are specified by following symbols evaluated in
 *      linker script.
 *      __etext: End of code section, i.e., begin of data sections to copy from.
 *      __data_start__/__data_end__: RAM address range that data should be
 *      copied to. Both must be aligned to 4 bytes boundary.  */

    ldr    r1, =__etext
    ldr    r2, =__data_start__
    ldr    r3, =__data_end__

    subs    r3, r2
    ble    .LC1
.LC0:
    subs    r3, #4
    ldr    r0, [r1, r3]
    str    r0, [r2, r3]
    bgt    .LC0
.LC1:

#ifdef __STARTUP_CLEAR_BSS
/*     This part of work usually is done in C library startup code. Otherwise,
 *     define this macro to enable it in this startup.
 *
 *     Loop to zero out BSS section, which uses following symbols
 *     in linker script:
 *      __bss_start__: start of BSS section. Must align to 4
 *      __bss_end__: end of BSS section. Must align to 4
 */
    ldr r1, =__bss_start__
    ldr r2, =__bss_end__

    subs    r2, r1
    ble .LC3

    movs    r0, 0
.LC2:
    str r0, [r1, r2]
    subs    r2, 4
    bge .LC2
.LC3:
#endif /* __STARTUP_CLEAR_BSS */

#ifndef __NO_SYSTEM_INIT
    /* bl    SystemInit */
    ldr     r0,=SystemInit
    blx     r0
#endif

    bl    _start

    .pool
    .size Reset_Handler, . - Reset_Handler

/*    Macro to define default handlers. Default handler
 *    will be weak symbol and just dead loops. They can be
 *    overwritten by other handlers */
    .macro    def_default_handler    handler_name
    .align 1
    .thumb_func
    .weak    \handler_name
    .type    \handler_name, %function
\handler_name :
    b    .
    .size    \handler_name, . - \handler_name
    .endm

/* System Exception Handlers */

    def_default_handler    NMI_Handler
    def_default_handler    HardFault_Handler
    def_default_handler    MemManage_Handler
    def_default_handler    BusFault_Handler
    def_default_handler    UsageFault_Handler
    def_default_handler    SVC_Handler
    def_default_handler    DebugMon_Handler
    def_default_handler    PendSV_Handler
    def_default_handler    SysTick_Handler

/* IRQ Handlers */

    def_default_handler    UARTRX0_Handler
    def_default_handler    UARTTX0_Handler
    def_default_handler    UARTRX1_Handler
    def_default_handler    UARTTX1_Handler
    def_default_handler    UARTRX2_Handler
    def_default_handler    UARTTX2_Handler
    def_default_handler    PORT0_COMB_Handler
    def_default_handler    PORT1_COMB_Handler
    def_default_handler    TIMER0_Handler
    def_default_handler    TIMER1_Handler
    def_default_handler    DUALTIMER_HANDLER
    def_default_handler    UARTOVF0_Handler
    def_default_handler    UARTOVF1_Handler
    def_default_handler    UARTOVF2_Handler
    def_default_handler    DMA_Handler
    def_default_handler    PORT0_0_Handler
    def_default_handler    PORT0_1_Handler
    def_default_handler    PORT0_2_Handler
    def_default_handler    PORT0_3_Handler
    def_default_handler    PORT0_4_Handler
    def_default_handler    PORT0_5_Handler
    def_default_handler    PORT0_6_Handler
    def_default_handler    PORT0_7_Handler
    def_default_handler    PORT0_8_Handler
    def_default_handler    PORT0_9_Handler
    def_default_handler    PORT0_10_Handler
    def_default_handler    PORT0_11_Handler
    def_default_handler    PORT0_12_Handler
    def_default_handler    PORT0_13_Handler
    def_default_handler    PORT0_14_Handler
    def_default_handler    PORT0_15_Handler


    /*
    def_default_handler    Default_Handler
    .weak    DEF_IRQHandler
    .set    DEF_IRQHandler, Default_Handler
    */
    .end

