/*
 *-----------------------------------------------------------------------------
 * The confidential and proprietary information contained in this file may
 * only be used by a person authorised under and to the extent permitted
 * by a subsisting licensing agreement from ARM Limited.
 *
 *            (C) COPYRIGHT 2010-2015  ARM Limited or its affiliates.
 *                ALL RIGHTS RESERVED
 *
 * This entire notice must be reproduced on all copies of this file
 * and copies of this file may only be made by a person if such person is
 * permitted to do so under the terms of a subsisting license agreement
 * from ARM Limited.
 *
 *      SVN Information
 *
 *      Checked In          : $Date: 2013-03-27 23:58:01 +0000 (Wed, 27 Mar 2013) $
 *
 *      Revision            : $Revision: 242484 $
 *
 *      Release Information : Cortex-M0 DesignStart-r1p0-00rel0
 *-----------------------------------------------------------------------------
 */
/*
  Dual timer demonstration

  This test demonstrate setting the dual timer in different modes/
  Since the execution speed of the program can be affected by memory speed
  and hence result can change, the only checks in this program is the
  occurrence of interrupts.
*/

#include "CMSDK_CM0.h"
#include <stdio.h>
#include "uart_stdout.h"

#define PRESCALE_1_1   0
#define PRESCALE_1_16  1
#define PRESCALE_1_256 2
#define NO_IRQ         0
#define USE_IRQ        1

/* Software variables for interrupt detection */
volatile int dtimer_irq_expected;
volatile int dtimer1_irq_occurred;
volatile int dtimer2_irq_occurred;
volatile int dtimer_integration_test=0;

/* Demonstration functions */
void dualtimer_16_free_running_demo(CMSDK_DUALTIMER_SINGLE_TypeDef *CMSDK_DUALTIMERx);
void dualtimer_32_free_running_demo(CMSDK_DUALTIMER_SINGLE_TypeDef *CMSDK_DUALTIMERx);
void dualtimer_16_periodic_demo(CMSDK_DUALTIMER_SINGLE_TypeDef *CMSDK_DUALTIMERx);
void dualtimer_32_periodic_demo(CMSDK_DUALTIMER_SINGLE_TypeDef *CMSDK_DUALTIMERx);
void dualtimer_16_one_shot_demo(CMSDK_DUALTIMER_SINGLE_TypeDef *CMSDK_DUALTIMERx);
void dualtimer_32_one_shot_demo(CMSDK_DUALTIMER_SINGLE_TypeDef *CMSDK_DUALTIMERx);
int  dualtimer_integration_test(void);
int  dualtimer_id_check(void);

/* Functions for setting up the dual timer */
  /* 16-bit free running mode */
void dualtimer_setup_freerunning16(CMSDK_DUALTIMER_SINGLE_TypeDef *CMSDK_DUALTIMERx,
 unsigned int cycle, unsigned int prescale, unsigned int interrupt);
  /* 16-bit periodic mode */
void dualtimer_setup_periodic16(CMSDK_DUALTIMER_SINGLE_TypeDef *CMSDK_DUALTIMERx,
 unsigned int cycle, unsigned int prescale, unsigned int interrupt);
  /* 16-bit free one shot mode */
void dualtimer_setup_oneshot16(CMSDK_DUALTIMER_SINGLE_TypeDef *CMSDK_DUALTIMERx,
 unsigned int cycle, unsigned int prescale, unsigned int interrupt);
  /* 32-bit free running mode */
void dualtimer_setup_freerunning32(CMSDK_DUALTIMER_SINGLE_TypeDef *CMSDK_DUALTIMERx,
 unsigned int cycle, unsigned int prescale, unsigned int interrupt);
  /* 32-bit periodic mode */
void dualtimer_setup_periodic32(CMSDK_DUALTIMER_SINGLE_TypeDef *CMSDK_DUALTIMERx,
 unsigned int cycle, unsigned int prescale, unsigned int interrupt);
  /* 32-bit free one shot mode */
void dualtimer_setup_oneshot32(CMSDK_DUALTIMER_SINGLE_TypeDef *CMSDK_DUALTIMERx,
 unsigned int cycle, unsigned int prescale, unsigned int interrupt);

  /* Start a timer */
void dualtimer_start(CMSDK_DUALTIMER_SINGLE_TypeDef *CMSDK_DUALTIMERx);
  /* Stop a timer */
void dualtimer_stop(CMSDK_DUALTIMER_SINGLE_TypeDef *CMSDK_DUALTIMERx);
  /* Clear a timer interrupt request */
void dualtimer_irq_clear(CMSDK_DUALTIMER_SINGLE_TypeDef *CMSDK_DUALTIMERx);

  /* Text buffer for displaying value */
char text_buf[20];
void int_to_hex(unsigned int value, char text[]);

/* ----------------------------------------------------------------- */
int main(void)
{
  // UART init
  UartStdOutInit();

  // Test banner message and revision number
  puts("\nCortex Microcontroller System Design Kit - Dual Timer Demo - revision $Revision: 242484 $\n");

  if (dualtimer_id_check()!=0) {
    puts ("** TEST SKIPPED ** Dual timer not present.\n");
    UartEndSimulation();
    return 0;}

  puts ("\n--------------------------------");
  puts ("Timer 1");
  puts ("--------------------------------\n");

  dualtimer_16_free_running_demo(CMSDK_DUALTIMER1);
  dualtimer_16_periodic_demo(CMSDK_DUALTIMER1);
  dualtimer_16_one_shot_demo(CMSDK_DUALTIMER1);
  dualtimer_32_free_running_demo(CMSDK_DUALTIMER1);
  dualtimer_32_periodic_demo(CMSDK_DUALTIMER1);
  dualtimer_32_one_shot_demo(CMSDK_DUALTIMER1);

  puts ("\n--------------------------------");
  puts ("Timer 2");
  puts ("--------------------------------\n");

  dualtimer_16_free_running_demo(CMSDK_DUALTIMER2);
  dualtimer_16_periodic_demo(CMSDK_DUALTIMER2);
  dualtimer_16_one_shot_demo(CMSDK_DUALTIMER2);
  dualtimer_32_free_running_demo(CMSDK_DUALTIMER2);
  dualtimer_32_periodic_demo(CMSDK_DUALTIMER2);
  dualtimer_32_one_shot_demo(CMSDK_DUALTIMER2);

  puts ("\n--------------------------------");
  puts ("Integration test");
  puts ("--------------------------------\n");
  dualtimer_integration_test();

  puts ("** TEST PASSED ** \n");
  UartEndSimulation();
  /* Simulation stops in UartEndSimulation */

}
/* ----------------------------------------------------------------- */
/* Detect the ARM ID and part number to see if device is present     */
int dualtimer_id_check(void)
{
unsigned char dualtimer_id;
#define HW32_REG(ADDRESS)  (*((volatile unsigned long  *)(ADDRESS)))
#define HW8_REG(ADDRESS)   (*((volatile unsigned char  *)(ADDRESS)))

 dualtimer_id = HW8_REG(CMSDK_DUALTIMER_BASE  + 0xFE8) & 0x07;

 if ((HW32_REG(CMSDK_DUALTIMER_BASE + 0xFE0) != 0x23) ||
     (HW32_REG(CMSDK_DUALTIMER_BASE + 0xFE4) != 0xB8) ||
     (dualtimer_id != 0x03))
   return 1; /* part ID & ARM ID does not match dual timer */
 else
   return 0;
}
/* ----------------------------------------------------------------- */
void dualtimer_16_free_running_demo(CMSDK_DUALTIMER_SINGLE_TypeDef *CMSDK_DUALTIMERx)
{
  unsigned int read_value;
  unsigned int i;

  /* Clear software variables for interrupt tests */
  dtimer_irq_expected = 0;
  dtimer1_irq_occurred = 0;
  dtimer2_irq_occurred = 0;

  /* ------------------------------------------------------------- */
  puts ("16-bit free running mode, prescale = 1:1, no interrupt");
  /* 16-bit Free running timer, 5000 cycles, prescale = 1:1, no interrupt */
  dualtimer_setup_freerunning16(CMSDK_DUALTIMERx, 5000, PRESCALE_1_1, NO_IRQ);
  i=0;
  while (i<10){
    read_value = CMSDK_DUALTIMERx->TimerValue;
    /* Display value of timer */
     /* timing of printf can be very different between different toolchain.
        Use puts instead */
    int_to_hex(read_value, &text_buf[0]);
    puts(text_buf);
    i++;
    }
  dualtimer_stop(CMSDK_DUALTIMERx);
  /* ------------------------------------------------------------- */
  puts ("16-bit free running mode, prescale = 1:16, no interrupt");
  /* 16-bit Free running timer, 200 cycles, prescale = 1:16, no interrupt */
  dualtimer_setup_freerunning16(CMSDK_DUALTIMERx, 200, PRESCALE_1_16, NO_IRQ);
  i=0;
  while (i<10){
    read_value = CMSDK_DUALTIMERx->TimerValue;
    /* Display value of timer */
     /* timing of printf can be very different between different toolchain.
        Use puts instead */
    int_to_hex(read_value, &text_buf[0]);
    puts(text_buf);
    i++;
    }
  dualtimer_stop(CMSDK_DUALTIMERx);
  /* ------------------------------------------------------------- */
  puts ("16-bit free running mode, prescale = 1:256, no interrupt");
  /* 16-bit Free running timer, 20 cycles, prescale = 1:256, no interrupt */
  dualtimer_setup_freerunning16(CMSDK_DUALTIMERx, 20, PRESCALE_1_256, NO_IRQ);
  i=0;
  while (i<10){
    read_value = CMSDK_DUALTIMERx->TimerValue;
    /* Display value of timer */
     /* timing of printf can be very different between different toolchain.
        Use puts instead */
    int_to_hex(read_value, &text_buf[0]);
    puts(text_buf);
    i++;
    }
  dualtimer_stop(CMSDK_DUALTIMERx);
  /* ------------------------------------------------------------- */
  puts ("16-bit free running mode, prescale = 1:1, with interrupt");
  /* 16-bit Free running timer, 5000 cycles, prescale = 1:1, with interrupt */
  dtimer_irq_expected = 1;
  NVIC_EnableIRQ(DUALTIMER_IRQn);
  dualtimer_setup_freerunning16(CMSDK_DUALTIMERx, 5000, PRESCALE_1_1, USE_IRQ);
  i=0;
  while (i<1){
    /* Wait until timer interrupt take place */
    while ((dtimer1_irq_occurred + dtimer2_irq_occurred)==0);
    dtimer1_irq_occurred = 0;
    dtimer2_irq_occurred = 0;
    puts ("  IRQ");
    i++;
    }
  dualtimer_stop(CMSDK_DUALTIMERx);
  NVIC_DisableIRQ(DUALTIMER_IRQn);
  /* Clear software variables for interrupt tests */
  dtimer_irq_expected = 0;
  dtimer1_irq_occurred = 0;
  dtimer2_irq_occurred = 0;

}
/* ----------------------------------------------------------------- */
void dualtimer_32_free_running_demo(CMSDK_DUALTIMER_SINGLE_TypeDef *CMSDK_DUALTIMERx)
{
  unsigned int read_value;
  unsigned int i;

  /* Clear software variables for interrupt tests */
  dtimer_irq_expected = 0;
  dtimer1_irq_occurred = 0;
  dtimer2_irq_occurred = 0;

  /* ------------------------------------------------------------- */
  puts ("32-bit free running mode, prescale = 1:1, no interrupt");
  /* 32-bit Free running timer, 5000 cycles, prescale = 1:1, no interrupt */
  dualtimer_setup_freerunning32(CMSDK_DUALTIMERx, 5000, PRESCALE_1_1, NO_IRQ);
  i=0;
  while (i<10){
    read_value = CMSDK_DUALTIMERx->TimerValue;
    /* Display value of timer */
     /* timing of printf can be very different between different toolchain.
        Use puts instead */
    int_to_hex(read_value, &text_buf[0]);
    puts(text_buf);
    i++;
    }
  dualtimer_stop(CMSDK_DUALTIMERx);
  /* ------------------------------------------------------------- */
  puts ("32-bit free running mode, prescale = 1:16, no interrupt");
  /* 32-bit Free running timer, 200 cycles, prescale = 1:16, no interrupt */
  dualtimer_setup_freerunning32(CMSDK_DUALTIMERx, 200, PRESCALE_1_16, NO_IRQ);
  i=0;
  while (i<10){
    read_value = CMSDK_DUALTIMERx->TimerValue;
    /* Display value of timer */
     /* timing of printf can be very different between different toolchain.
        Use puts instead */
    int_to_hex(read_value, &text_buf[0]);
    puts(text_buf);
    i++;
    }
  dualtimer_stop(CMSDK_DUALTIMERx);
  /* ------------------------------------------------------------- */
  puts ("32-bit free running mode, prescale = 1:256, no interrupt");
  /* 32-bit Free running timer, 20 cycles, prescale = 1:256, no interrupt */
  dualtimer_setup_freerunning32(CMSDK_DUALTIMERx, 20, PRESCALE_1_256, NO_IRQ);
  i=0;
  while (i<10){
    read_value = CMSDK_DUALTIMERx->TimerValue;
    /* Display value of timer */
     /* timing of printf can be very different between different toolchain.
        Use puts instead */
    int_to_hex(read_value, &text_buf[0]);
    puts(text_buf);
    i++;
    }
  dualtimer_stop(CMSDK_DUALTIMERx);
  /* ------------------------------------------------------------- */
  puts ("32-bit free running mode, prescale = 1:1, with interrupt");
  /* 32-bit Free running timer, 5000 cycles, prescale = 1:1, with interrupt */
  dtimer_irq_expected = 1;
  NVIC_EnableIRQ(DUALTIMER_IRQn);
  dualtimer_setup_freerunning32(CMSDK_DUALTIMERx, 5000, PRESCALE_1_1, USE_IRQ);
  i=0;
  while (i<1){
    /* Wait until timer interrupt take place */
    while ((dtimer1_irq_occurred + dtimer2_irq_occurred)==0);
    dtimer1_irq_occurred = 0;
    dtimer2_irq_occurred = 0;
    puts ("  IRQ");
    i++;
    }
  dualtimer_stop(CMSDK_DUALTIMERx);
  NVIC_DisableIRQ(DUALTIMER_IRQn);
  /* Clear software variables for interrupt tests */
  dtimer_irq_expected = 0;
  dtimer1_irq_occurred = 0;
  dtimer2_irq_occurred = 0;

}
/* ----------------------------------------------------------------- */
void dualtimer_16_periodic_demo(CMSDK_DUALTIMER_SINGLE_TypeDef *CMSDK_DUALTIMERx)
{
  unsigned int read_value;
  unsigned int i;

  /* Clear software variables for interrupt tests */
  dtimer_irq_expected = 0;
  dtimer1_irq_occurred = 0;
  dtimer2_irq_occurred = 0;

  /* ------------------------------------------------------------- */
  puts ("16-bit periodic mode, prescale = 1:1, no interrupt");
  /* 16-bit periodic timer, 1000 cycles, prescale = 1:1, no interrupt */
  dualtimer_setup_periodic16(CMSDK_DUALTIMERx, 1000, PRESCALE_1_1, NO_IRQ);
  i=0;
  while (i<10){
    read_value = CMSDK_DUALTIMERx->TimerValue;
    /* Display value of timer */
     /* timing of printf can be very different between different toolchain.
        Use puts instead */
    int_to_hex(read_value, &text_buf[0]);
    puts(text_buf);
    i++;
    }
  dualtimer_stop(CMSDK_DUALTIMERx);
  /* ------------------------------------------------------------- */
  puts ("16-bit periodic mode, prescale = 1:16, no interrupt");
  /* 16-bit periodic timer, 200 cycles, prescale = 1:16, no interrupt */
  dualtimer_setup_periodic16(CMSDK_DUALTIMERx, 200, PRESCALE_1_16, NO_IRQ);
  i=0;
  while (i<10){
    read_value = CMSDK_DUALTIMERx->TimerValue;
    /* Display value of timer */
     /* timing of printf can be very different between different toolchain.
        Use puts instead */
    int_to_hex(read_value, &text_buf[0]);
    puts(text_buf);
    i++;
    }
  dualtimer_stop(CMSDK_DUALTIMERx);
  /* ------------------------------------------------------------- */
  puts ("16-bit periodic mode, prescale = 1:256, no interrupt");
  /* 16-bit periodic timer, 20 cycles, prescale = 1:256, no interrupt */
  dualtimer_setup_periodic16(CMSDK_DUALTIMERx, 20, PRESCALE_1_256, NO_IRQ);
  i=0;
  while (i<10){
    read_value = CMSDK_DUALTIMERx->TimerValue;
    /* Display value of timer */
     /* timing of printf can be very different between different toolchain.
        Use puts instead */
    int_to_hex(read_value, &text_buf[0]);
    puts(text_buf);
    i++;
    }
  dualtimer_stop(CMSDK_DUALTIMERx);
  /* ------------------------------------------------------------- */
  puts ("16-bit periodic mode, prescale = 1:1, with interrupt x4");
  /* 16-bit periodic timer, 3000 cycles, prescale = 1:1, with interrupt */
  dtimer_irq_expected = 1;
  NVIC_EnableIRQ(DUALTIMER_IRQn);
  dualtimer_setup_periodic16(CMSDK_DUALTIMERx, 3000, PRESCALE_1_1, USE_IRQ);
  i=0;
  while (i<4){
    /* Wait until timer interrupt take place */
    while ((dtimer1_irq_occurred + dtimer2_irq_occurred)==0);
    dtimer1_irq_occurred = 0;
    dtimer2_irq_occurred = 0;
    puts ("  IRQ");
    i++;
    }
  dualtimer_stop(CMSDK_DUALTIMERx);
  NVIC_DisableIRQ(DUALTIMER_IRQn);
  /* Clear software variables for interrupt tests */
  dtimer_irq_expected = 0;
  dtimer1_irq_occurred = 0;
  dtimer2_irq_occurred = 0;

}
/* ----------------------------------------------------------------- */
void dualtimer_32_periodic_demo(CMSDK_DUALTIMER_SINGLE_TypeDef *CMSDK_DUALTIMERx)
{
  unsigned int read_value;
  unsigned int i;

  /* Clear software variables for interrupt tests */
  dtimer_irq_expected = 0;
  dtimer1_irq_occurred = 0;
  dtimer2_irq_occurred = 0;

  /* ------------------------------------------------------------- */
  puts ("32-bit periodic mode, prescale = 1:1, no interrupt");
  /* 32-bit periodic timer, 1000 cycles, prescale = 1:1, no interrupt */
  dualtimer_setup_periodic32(CMSDK_DUALTIMERx, 1000, PRESCALE_1_1, NO_IRQ);
  i=0;
  while (i<10){
    read_value = CMSDK_DUALTIMERx->TimerValue;
    /* Display value of timer */
     /* timing of printf can be very different between different toolchain.
        Use puts instead */
    int_to_hex(read_value, &text_buf[0]);
    puts(text_buf);
    i++;
    }
  dualtimer_stop(CMSDK_DUALTIMERx);
  /* ------------------------------------------------------------- */
  puts ("32-bit periodic mode, prescale = 1:16, no interrupt");
  /* 32-bit periodic timer, 200 cycles, prescale = 1:16, no interrupt */
  dualtimer_setup_periodic32(CMSDK_DUALTIMERx, 200, PRESCALE_1_16, NO_IRQ);
  i=0;
  while (i<10){
    read_value = CMSDK_DUALTIMERx->TimerValue;
    /* Display value of timer */
     /* timing of printf can be very different between different toolchain.
        Use puts instead */
    int_to_hex(read_value, &text_buf[0]);
    puts(text_buf);
    i++;
    }
  dualtimer_stop(CMSDK_DUALTIMERx);
  /* ------------------------------------------------------------- */
  puts ("32-bit periodic mode, prescale = 1:256, no interrupt");
  /* 32-bit periodic timer, 20 cycles, prescale = 1:256, no interrupt */
  dualtimer_setup_periodic32(CMSDK_DUALTIMERx, 20, PRESCALE_1_256, NO_IRQ);
  i=0;
  while (i<10){
    read_value = CMSDK_DUALTIMERx->TimerValue;
     /* timing of printf can be very different between different toolchain.
        Use puts instead */
    int_to_hex(read_value, &text_buf[0]);
    puts(text_buf);
    i++;
    }
  dualtimer_stop(CMSDK_DUALTIMERx);
  /* ------------------------------------------------------------- */
  puts ("32-bit periodic mode, prescale = 1:1, with interrupt x4");
  /* 16-bit periodic timer, 3000 cycles, prescale = 1:1, with interrupt */
  dtimer_irq_expected = 1;
  NVIC_EnableIRQ(DUALTIMER_IRQn);
  dualtimer_setup_periodic32(CMSDK_DUALTIMERx, 3000, PRESCALE_1_1, USE_IRQ);
  i=0;
  while (i<4){
    /* Wait until timer interrupt take place */
    while ((dtimer1_irq_occurred + dtimer2_irq_occurred)==0);
    dtimer1_irq_occurred = 0;
    dtimer2_irq_occurred = 0;
    puts ("  IRQ");
    i++;
    }
  dualtimer_stop(CMSDK_DUALTIMERx);
  NVIC_DisableIRQ(DUALTIMER_IRQn);
  /* Clear software variables for interrupt tests */
  dtimer_irq_expected = 0;
  dtimer1_irq_occurred = 0;
  dtimer2_irq_occurred = 0;

}
/* ----------------------------------------------------------------- */
void dualtimer_16_one_shot_demo(CMSDK_DUALTIMER_SINGLE_TypeDef *CMSDK_DUALTIMERx)
{
  unsigned int read_value;
  unsigned int i;

  /* Clear software variables for interrupt tests */
  dtimer_irq_expected = 0;
  dtimer1_irq_occurred = 0;
  dtimer2_irq_occurred = 0;

  /* ------------------------------------------------------------- */
  puts ("16-bit one shot mode, prescale = 1:1, no interrupt");
  /* 16-bit one shot timer, 2000 cycles, prescale = 1:1, no interrupt */
  dualtimer_setup_oneshot16(CMSDK_DUALTIMERx, 2000, PRESCALE_1_1, NO_IRQ);
  i=0;
  while (i<10){
    read_value = CMSDK_DUALTIMERx->TimerValue;
    /* Display value of timer */
     /* timing of printf can be very different between different toolchain.
        Use puts instead */
    int_to_hex(read_value, &text_buf[0]);
    puts(text_buf);
    i++;
    }
  dualtimer_stop(CMSDK_DUALTIMERx);
  /* ------------------------------------------------------------- */
  puts ("16-bit one shot mode, prescale = 1:16, no interrupt");
  /* 16-bit one shot timer, 100 cycles, prescale = 1:16, no interrupt */
  dualtimer_setup_oneshot16(CMSDK_DUALTIMERx, 100, PRESCALE_1_16, NO_IRQ);
  i=0;
  while (i<10){
    read_value = CMSDK_DUALTIMERx->TimerValue;
    /* Display value of timer */
     /* timing of printf can be very different between different toolchain.
        Use puts instead */
    int_to_hex(read_value, &text_buf[0]);
    puts(text_buf);
    i++;
    }
  dualtimer_stop(CMSDK_DUALTIMERx);
  /* ------------------------------------------------------------- */
  puts ("16-bit one shot mode, prescale = 1:1, with interrupt");
  /* 16-bit one shot timer, 3000 cycles, prescale = 1:1, with interrupt */
  dtimer_irq_expected = 1;
  NVIC_EnableIRQ(DUALTIMER_IRQn);
  dualtimer_setup_oneshot16(CMSDK_DUALTIMERx, 3000, PRESCALE_1_1, USE_IRQ);
  i=0;
  while (i<1){
    /* Wait until timer interrupt take place */
    while ((dtimer1_irq_occurred + dtimer2_irq_occurred)==0);
    dtimer1_irq_occurred = 0;
    dtimer2_irq_occurred = 0;
    puts ("  IRQ");
    i++;
    }
  dualtimer_stop(CMSDK_DUALTIMERx);
  NVIC_DisableIRQ(DUALTIMER_IRQn);
  /* Clear software variables for interrupt tests */
  dtimer_irq_expected = 0;
  dtimer1_irq_occurred = 0;
  dtimer2_irq_occurred = 0;

}
/* ----------------------------------------------------------------- */
void dualtimer_32_one_shot_demo(CMSDK_DUALTIMER_SINGLE_TypeDef *CMSDK_DUALTIMERx)
{
  unsigned int read_value;
  unsigned int i;

  /* Clear software variables for interrupt tests */
  dtimer_irq_expected = 0;
  dtimer1_irq_occurred = 0;
  dtimer2_irq_occurred = 0;

  /* ------------------------------------------------------------- */
  puts ("32-bit one shot mode, prescale = 1:1, no interrupt");
  /* 32-bit one shot timer, 2000 cycles, prescale = 1:1, no interrupt */
  dualtimer_setup_oneshot32(CMSDK_DUALTIMERx, 2000, PRESCALE_1_1, NO_IRQ);
  i=0;
  while (i<10){
    read_value = CMSDK_DUALTIMERx->TimerValue;
    /* Display value of timer */
     /* timing of printf can be very different between different toolchain.
        Use puts instead */
    int_to_hex(read_value, &text_buf[0]);
    puts(text_buf);
    i++;
    }
  dualtimer_stop(CMSDK_DUALTIMERx);
  /* ------------------------------------------------------------- */
  puts ("32-bit one shot mode, prescale = 1:16, no interrupt");
  /* 32-bit one shot timer, 100 cycles, prescale = 1:16, no interrupt */
  dualtimer_setup_oneshot32(CMSDK_DUALTIMERx, 100, PRESCALE_1_16, NO_IRQ);
  i=0;
  while (i<10){
    read_value = CMSDK_DUALTIMERx->TimerValue;
    /* Display value of timer */
     /* timing of printf can be very different between different toolchain.
        Use puts instead */
    int_to_hex(read_value, &text_buf[0]);
    puts(text_buf);
    i++;
    }
  dualtimer_stop(CMSDK_DUALTIMERx);
  /* ------------------------------------------------------------- */
  puts ("32-bit one shot mode, prescale = 1:1, with interrupt");
  /* 32-bit one shot timer, 3000 cycles, prescale = 1:1, with interrupt */
  dtimer_irq_expected = 1;
  NVIC_EnableIRQ(DUALTIMER_IRQn);
  dualtimer_setup_oneshot32(CMSDK_DUALTIMERx, 3000, PRESCALE_1_1, USE_IRQ);
  i=0;
  while (i<1){
    /* Wait until timer interrupt take place */
    while ((dtimer1_irq_occurred + dtimer2_irq_occurred)==0);
    dtimer1_irq_occurred = 0;
    dtimer2_irq_occurred = 0;
    puts ("  IRQ");
    i++;
    }
  dualtimer_stop(CMSDK_DUALTIMERx);
  NVIC_DisableIRQ(DUALTIMER_IRQn);
  /* Clear software variables for interrupt tests */
  dtimer_irq_expected = 0;
  dtimer1_irq_occurred = 0;
  dtimer2_irq_occurred = 0;

}


/* ----------------------------------------------------------------- */
/* Interrupt handler for dual timer */
void DUALTIMER_HANDLER(void)
{
if (dtimer_integration_test==0) { /* Normal operation mode */
  /* The Dual Timer interrupt handler are shared by the two timers */
  if (CMSDK_DUALTIMER1->TimerMIS != 0){
    dualtimer_irq_clear(CMSDK_DUALTIMER1);
    dtimer1_irq_occurred++;
    if (dtimer_irq_expected==0) {
      puts("ERROR: Unexpected dual timer 1 IRQ");
      UartEndSimulation(); /* Simulation stops in UartEndSimulation */
      }  /* end if (dtimer1_irq_expected==0) */
    } /* end if (CMSDK_DUALTIMER1->TimerMIS != 0) */
  if (CMSDK_DUALTIMER2->TimerMIS != 0){
    dualtimer_irq_clear(CMSDK_DUALTIMER2);
    dtimer2_irq_occurred++;
    if (dtimer_irq_expected==0) {
      puts("ERROR: Unexpected dual timer 2 IRQ");
      UartEndSimulation(); /* Simulation stops in UartEndSimulation */
      }  /* end if (dtimer2_irq_expected==0) */
    } /* end if (CMSDK_DUALTIMER2->TimerMIS != 0) */
  }
else { /* Integration test mode */
  /* Cannot tell which timer interrupt is asserted because
  ITOP is write only and both timer shared the same interrupt.
  So increment both variables. */
  CMSDK_DUALTIMER->ITOP=0;
  dtimer1_irq_occurred++;
  dtimer2_irq_occurred++;
  puts ("  [Dualtimer interrupt triggered]");
  }
}

/* ----------------------------------------------------------------- */
/* Functions for setting up the dual timer */


/* 16-bit Free running timer mode */
void dualtimer_setup_freerunning16(CMSDK_DUALTIMER_SINGLE_TypeDef *CMSDK_DUALTIMERx,
 unsigned int cycle, unsigned int prescale, unsigned int interrupt)
{
  int ctrl_val;
  CMSDK_DUALTIMERx->TimerControl = 0; /* Disable during programming */
   /* Previous timer activities might have trigger interrupt flag,
   so need to clear it */
  dualtimer_irq_clear(CMSDK_DUALTIMERx);
  CMSDK_DUALTIMERx->TimerLoad    = cycle;

  ctrl_val = (prescale  & 0x3) << CMSDK_DUALTIMER_CTRL_PRESCALE_Pos |
             (interrupt & 0x1) << CMSDK_DUALTIMER_CTRL_INTEN_Pos |
            CMSDK_DUALTIMER_CTRL_EN_Msk;

  CMSDK_DUALTIMERx->TimerControl = ctrl_val;
}

/* 32-bit Free running timer mode */
void dualtimer_setup_freerunning32(CMSDK_DUALTIMER_SINGLE_TypeDef *CMSDK_DUALTIMERx,
 unsigned int cycle, unsigned int prescale, unsigned int interrupt)
{
  int ctrl_val;
  CMSDK_DUALTIMERx->TimerControl = 0; /* Disable during programming */
   /* Previous timer activities might have trigger interrupt flag,
   so need to clear it */
  dualtimer_irq_clear(CMSDK_DUALTIMERx);
  CMSDK_DUALTIMERx->TimerLoad    = cycle;

  ctrl_val = (prescale  & 0x3) << CMSDK_DUALTIMER_CTRL_PRESCALE_Pos |
             (interrupt & 0x1) << CMSDK_DUALTIMER_CTRL_INTEN_Pos |
            CMSDK_DUALTIMER_CTRL_EN_Msk |
            CMSDK_DUALTIMER_CTRL_SIZE_Msk;

  CMSDK_DUALTIMERx->TimerControl = ctrl_val;
}

/* 16-bit Periodic timer mode */
void dualtimer_setup_periodic16(CMSDK_DUALTIMER_SINGLE_TypeDef *CMSDK_DUALTIMERx,
 unsigned int cycle, unsigned int prescale, unsigned int interrupt)
{
  int ctrl_val;
  CMSDK_DUALTIMERx->TimerControl = 0; /* Disable during programming */
   /* Previous timer activities might have trigger interrupt flag,
   so need to clear it */
  dualtimer_irq_clear(CMSDK_DUALTIMERx);
  CMSDK_DUALTIMERx->TimerLoad    = cycle;

  ctrl_val = (prescale  & 0x3) << CMSDK_DUALTIMER_CTRL_PRESCALE_Pos |
             (interrupt & 0x1) << CMSDK_DUALTIMER_CTRL_INTEN_Pos |
            CMSDK_DUALTIMER_CTRL_EN_Msk |
            CMSDK_DUALTIMER_CTRL_MODE_Msk;

  CMSDK_DUALTIMERx->TimerControl = ctrl_val;
}

/* 32-bit Periodic timer mode */
void dualtimer_setup_periodic32(CMSDK_DUALTIMER_SINGLE_TypeDef *CMSDK_DUALTIMERx,
 unsigned int cycle, unsigned int prescale, unsigned int interrupt)
{
  int ctrl_val;
  CMSDK_DUALTIMERx->TimerControl = 0; /* Disable during programming */
   /* Previous timer activities might have trigger interrupt flag,
   so need to clear it */
  dualtimer_irq_clear(CMSDK_DUALTIMERx);
  CMSDK_DUALTIMERx->TimerLoad    = cycle;

  ctrl_val = (prescale  & 0x3) << CMSDK_DUALTIMER_CTRL_PRESCALE_Pos |
             (interrupt & 0x1) << CMSDK_DUALTIMER_CTRL_INTEN_Pos |
            CMSDK_DUALTIMER_CTRL_EN_Msk |
            CMSDK_DUALTIMER_CTRL_MODE_Msk|
            CMSDK_DUALTIMER_CTRL_SIZE_Msk;

  CMSDK_DUALTIMERx->TimerControl = ctrl_val;
}

/* 16-bit One shot timer mode */
void dualtimer_setup_oneshot16(CMSDK_DUALTIMER_SINGLE_TypeDef *CMSDK_DUALTIMERx,
 unsigned int cycle, unsigned int prescale, unsigned int interrupt)
{
  int ctrl_val;
  CMSDK_DUALTIMERx->TimerControl = 0; /* Disable during programming */
   /* Previous timer activities might have trigger interrupt flag,
   so need to clear it */
  dualtimer_irq_clear(CMSDK_DUALTIMERx);
  CMSDK_DUALTIMERx->TimerLoad    = cycle;

  ctrl_val = (prescale  & 0x3) << CMSDK_DUALTIMER_CTRL_PRESCALE_Pos |
             (interrupt & 0x1) << CMSDK_DUALTIMER_CTRL_INTEN_Pos |
            CMSDK_DUALTIMER_CTRL_EN_Msk |
            CMSDK_DUALTIMER_CTRL_MODE_Msk |
            CMSDK_DUALTIMER_CTRL_ONESHOOT_Msk;

  CMSDK_DUALTIMERx->TimerControl = ctrl_val;
}

/* 32-bit One shot timer mode */
void dualtimer_setup_oneshot32(CMSDK_DUALTIMER_SINGLE_TypeDef *CMSDK_DUALTIMERx,
 unsigned int cycle, unsigned int prescale, unsigned int interrupt)
{
  int ctrl_val;
  CMSDK_DUALTIMERx->TimerControl = 0; /* Disable during programming */
   /* Previous timer activities might have trigger interrupt flag,
   so need to clear it */
  dualtimer_irq_clear(CMSDK_DUALTIMERx);
  CMSDK_DUALTIMERx->TimerLoad    = cycle;

  ctrl_val = (prescale  & 0x3) << CMSDK_DUALTIMER_CTRL_PRESCALE_Pos |
             (interrupt & 0x1) << CMSDK_DUALTIMER_CTRL_INTEN_Pos |
            CMSDK_DUALTIMER_CTRL_EN_Msk |
            CMSDK_DUALTIMER_CTRL_MODE_Msk |
            CMSDK_DUALTIMER_CTRL_ONESHOOT_Msk|
            CMSDK_DUALTIMER_CTRL_SIZE_Msk;

  CMSDK_DUALTIMERx->TimerControl = ctrl_val;
}

/* Demonstrate integration test feature in the dual timer */
int dualtimer_integration_test(void)
{
  int result = 0;
   /* Instead of triggering timer interrupt using normal timer operation, you can also
      test the watchdog connection using integration test register.
      ITCR - bit 0 enable integration test mode
      ITOP - bit 0 enable timer 1 irq when integration test mode is enabled
             bit 1 enable timer 2 irq when integration test mode is enabled
   */

  puts ("Integration test demo");
  dtimer_integration_test = 1;

  dtimer_irq_expected = 0;
  dtimer1_irq_occurred = 0;
  dtimer2_irq_occurred = 0;
  CMSDK_DUALTIMER->ITOP=0; // Ensure interrupts are not asserted before start
  CMSDK_DUALTIMER->ITCR=1; // Enable integration test mode
  if (CMSDK_DUALTIMER->ITCR == 0) {
     puts ("  - Integration Test Control Register read error");
     result++;
     }
  else {
     dtimer_integration_test = 1;
     dtimer_irq_expected = 1;
     NVIC_EnableIRQ(DUALTIMER_IRQn);
     CMSDK_DUALTIMER->ITOP=1; // Set timer 1 interrupt
     CMSDK_DUALTIMER->ITOP=0; // Deassert timer 1 interrupt

     if (dtimer1_irq_occurred == 0) {
       puts ("  - Integration Test operation 1 failed");
       result++;
       }
     dtimer1_irq_occurred = 0; /* clear variables */
     dtimer2_irq_occurred = 0;

     CMSDK_DUALTIMER->ITOP=2; // Set timer 2 interrupt
     CMSDK_DUALTIMER->ITOP=0; // Deassert timer 2 interrupt

     if (dtimer2_irq_occurred == 0) {
       puts ("  - Integration Test operation 2 failed");
       result++;
       }
     CMSDK_DUALTIMER->ITCR = 0;  // Disable integration test mode
     if (CMSDK_DUALTIMER->ITCR != 0) {
      puts ("  - Integration Control clear error");
      result++;
      }
    }
  dtimer_integration_test = 0;
  NVIC_DisableIRQ(DUALTIMER_IRQn);

  if (result != 0) {
    puts ("ERROR: Integration test failed\n");
    return (1);
    } else {
    puts ("Integration test completed successfully\n");
    return (0);
    }

}



/* Start the timer */
void dualtimer_start(CMSDK_DUALTIMER_SINGLE_TypeDef *CMSDK_DUALTIMERx)
{
  CMSDK_DUALTIMERx->TimerControl |= CMSDK_DUALTIMER_CTRL_EN_Msk;
}
/* Stop the timer */
void dualtimer_stop(CMSDK_DUALTIMER_SINGLE_TypeDef *CMSDK_DUALTIMERx)
{
  CMSDK_DUALTIMERx->TimerControl &= ~CMSDK_DUALTIMER_CTRL_EN_Msk;
}
/* Clear the interrupt request */
void dualtimer_irq_clear(CMSDK_DUALTIMER_SINGLE_TypeDef *CMSDK_DUALTIMERx)
{
  CMSDK_DUALTIMERx->TimerIntClr = 0;
}
/* Convert hex value into text in text buffer */
void int_to_hex(unsigned int value, char text[])
{
  int          i=0;
  int          length=0;
  unsigned int mask = 0xF0000000UL;
  int          nibble;

  /* extra space in front for indent */
  for (length=0;length<2; length++){
    text[length]  =' ';     /* space */
    }
    length = 2;
  /* Remove leading zero */
  while (((mask & value)==0) & (i!=7)) {
    i ++;
    mask = mask >> 4;
    }
  while (i!=8) {
    nibble  = (mask & value) >> ((7-i)*4); /* shift to LSB */
    if (nibble>9) text[length] = (char) (nibble+87); // Convert to ASCII a to f
    else          text[length] = (char) (nibble+48); // Convert to ASCII 0 to 9
    i++;
    length++;
    mask = mask>> 4;
    }
  text[length]=(char) 0; /* null termination */
  return;
}

