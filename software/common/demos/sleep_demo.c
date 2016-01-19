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
 *      Checked In          : $Date: 2013-04-08 17:48:08 +0100 (Mon, 08 Apr 2013) $
 *
 *      Revision            : $Revision: 243249 $
 *
 *      Release Information : Cortex-M0 DesignStart-r1p0-00rel0
 *-----------------------------------------------------------------------------
 */
//==============================================================================
//
// Sleep Demonstration
//
//==============================================================================
//
// This test demonstrates various sleep modes and different ways to enter sleeps.
// A timer is programmed before entering sleep and interrupt is generated to
// wake up the processor.
//
// SLEEPING
//
//   - The test enters normal sleep mode using WFI and WFE.
//
//
// SLEEPDEEP
//
//   - The test checks that the SLEEPDEEP output is asserted when the core enters
//     sleep mode, by using the CM0IKMCU internal GPIO to cause a wake-up interrupt
//     when the SLEEPDEEP signal is asserted.
//
//
// WIC mode SLEEPDEEP
//
//
//
//


#include "CMSDK_CM0.h"
#include "uart_stdout.h"
#include "CMSDK_driver.h"
#include <stdio.h>

//
// Global Variables to track test progress
//

volatile uint32_t IntTaken = 0;
volatile uint32_t SleepOnExitTest = 0;

int  timer0_id_check(void); // Detect Timer 0 present


int main (void)
{
  // UART init
  UartStdOutInit();

  // Test banner message and revision number
  puts("\nCortex Microcontroller System Design Kit - Sleep Demo - revision $Revision: 243249 $\n");

  if (timer0_id_check()!=0)  {
    puts ("** TEST SKIPPED ** Timer 0 not present.");
    UartEndSimulation();
    return 0;}


  // --------------------------------------------
  // WFI Normal SLEEP
  // --------------------------------------------
  puts("\n- WFI SLEEP:");

  // Ensure we use normal SLEEP - SLEEPDEEP should be clear
  // SCR[2] = SLEEPDEEP
  SCB->SCR &= ~(1UL << 2);

  // Ensure timer interrupt is not pending before the test
  NVIC_ClearPendingIRQ(TIMER0_IRQn);

  // Set timer 0 to decrement from 0xF0 with internal clock,
  // interrupt enabled
  CMSDK_timer_Init_IntClock(CMSDK_TIMER0, 0xF0, 1);

  // Enable Interrupts
  NVIC_EnableIRQ(TIMER0_IRQn);

            // Wait For Interrupt
  __WFI();  // Enter sleep mode
            // Wakeup when timer interrupt is triggered

  // Stop the timer and disable interrupt generation
  CMSDK_timer_StopTimer(CMSDK_TIMER0);
  CMSDK_timer_DisableIRQ(CMSDK_TIMER0);

  // Check result
  if (IntTaken) puts("  TIMER IRQ.\n");

  // Disable Interrupt and Interrupt Source
  NVIC_DisableIRQ(TIMER0_IRQn);
  IntTaken = 0; // Reset software variable


  // --------------------------------------------
  // WFE Normal SLEEP
  // --------------------------------------------
  puts("\n- WFE SLEEP:");

  // Ensure we use normal SLEEP - SLEEPDEEP should be clear
  // SCR[2] = SLEEPDEEP
  SCB->SCR &= ~(1UL << 2);

  // Ensure timer interrupt is not pending before the test
  NVIC_ClearPendingIRQ(TIMER0_IRQn);

  IntTaken = 0; // Reset software variable

  // Set timer 0 to decrement from 0xF0 with internal clock,
  // interrupt enabled
  CMSDK_timer_Init_IntClock(CMSDK_TIMER0, 0xF0, 1);

  // Enable Interrupts
  NVIC_EnableIRQ(TIMER0_IRQn);

  while  (IntTaken==0) { /* Loop for sleep :
     (first WFE could be woken up immediately due to event from the past)
     */
            // Wait For Event
    __WFE();// Enter sleep mode
            // Wakeup when timer interrupt is triggered
  }

  // Stop the timer and disable interrupt generation
  CMSDK_timer_StopTimer(CMSDK_TIMER0);
  CMSDK_timer_DisableIRQ(CMSDK_TIMER0);

  // Show message (no need to test variable because of the loop)
  puts("  TIMER IRQ.\n");
  IntTaken = 0; // Reset software variable

  // --------------------------------------------
  // SLEEP-ON-EXIT
  // --------------------------------------------
  puts("\n- SLEEP-ON-EXIT:");

  // Ensure we use normal SLEEP - SLEEPDEEP should be clear
  // SCR[2] = SLEEPDEEP
  SCB->SCR &= ~(1UL << 2);

  // Enable sleep on exit feature
  SCB->SCR |= (1UL << 1);

  // Ensure timer interrupt is not pending before the test
  NVIC_ClearPendingIRQ(TIMER0_IRQn);

  IntTaken = 0; // Reset software variable
  SleepOnExitTest = 1;

  // Set timer 0 to decrement from 0x2000 with internal clock,
  // interrupt enabled
  // Duration is longer than other test due to message display
  // inside interrupt handler
  CMSDK_timer_Init_IntClock(CMSDK_TIMER0, 0x2000, 1);

  // Enable Interrupts
  NVIC_EnableIRQ(TIMER0_IRQn);

  while  (IntTaken==0) {
    /* Wait for first interrupt */
    /* Then the processor will enter sleep after Timer IRQ handler */
    /* Sleep on exit is disabled by timer IRQ handler in second occurrence */
    }

  // Stop the timer and disable interrupt generation
  CMSDK_timer_StopTimer(CMSDK_TIMER0);
  CMSDK_timer_DisableIRQ(CMSDK_TIMER0);

  // Show message (no need to test variable because of the loop)
  IntTaken = 0; // Reset software variable
  SleepOnExitTest = 0;
  // Disable sleep on exit feature
  SCB->SCR &= ~(1UL << 1);



  // --------------------------------------------
  // WFI Deep SLEEP
  // --------------------------------------------
  puts("\n- WFI DEEP SLEEP:");

  // Ensure we use deep SLEEP - SLEEPDEEP should be set
  // SCR[2] = SLEEPDEEP
  SCB->SCR |= (1UL << 2);

  // Ensure timer interrupt is not pending before the test
  NVIC_ClearPendingIRQ(TIMER0_IRQn);

  // Set timer 0 to decrement from 0xF0 with internal clock,
  // interrupt enabled
  CMSDK_timer_Init_IntClock(CMSDK_TIMER0, 0xF0, 1);

  // Enable Interrupts
  NVIC_EnableIRQ(TIMER0_IRQn);

            // Wait For Interrupt
  __WFI();  // Enter sleep mode
            // Wakeup when timer interrupt is triggered

  // Stop the timer and disable interrupt generation
  CMSDK_timer_StopTimer(CMSDK_TIMER0);
  CMSDK_timer_DisableIRQ(CMSDK_TIMER0);

  // Check result
  if (IntTaken) puts("  TIMER IRQ.\n");

  // Disable Interrupt and Interrupt Source
  NVIC_DisableIRQ(TIMER0_IRQn);
  IntTaken = 0; // Reset software variable


  // --------------------------------------------
  // WFE Deep SLEEP
  // --------------------------------------------
  puts("\n- WFE DEEP SLEEP:");

  // Ensure we use deep SLEEP - SLEEPDEEP should be set
  // SCR[2] = SLEEPDEEP
  SCB->SCR |= (1UL << 2);

  // Ensure timer interrupt is not pending before the test
  NVIC_ClearPendingIRQ(TIMER0_IRQn);

  IntTaken = 0; // Reset software variable

  // Set timer 0 to decrement from 0xF0 with internal clock,
  // interrupt enabled
  CMSDK_timer_Init_IntClock(CMSDK_TIMER0, 0xF0, 1);

  // Enable Interrupts
  NVIC_EnableIRQ(TIMER0_IRQn);

  while  (IntTaken==0) { /* Loop for sleep :
     (first WFE could be woken up immediately due to event from the past)
     */
            // Wait For Event
    __WFE();// Enter sleep mode
            // Wakeup when timer interrupt is triggered
  }

  // Stop the timer and disable interrupt generation
  CMSDK_timer_StopTimer(CMSDK_TIMER0);
  CMSDK_timer_DisableIRQ(CMSDK_TIMER0);

  // Show message (no need to test variable because of the loop)
  puts("  TIMER IRQ.\n");
  IntTaken = 0; // Reset software variable

  // --------------------------------------------
  // SLEEP-ON-EXIT
  // --------------------------------------------
  puts("\n- SLEEP-ON-EXIT deep sleep:");

  // Ensure we use deep SLEEP - SLEEPDEEP should be set
  // SCR[2] = SLEEPDEEP
  SCB->SCR |= (1UL << 2);

  // Enable sleep on exit feature
  SCB->SCR |= (1UL << 1);

  // Ensure timer interrupt is not pending before the test
  NVIC_ClearPendingIRQ(TIMER0_IRQn);

  IntTaken = 0; // Reset software variable
  SleepOnExitTest = 1;

  // Set timer 0 to decrement from 0x2000 with internal clock,
  // interrupt enabled
  // Duration is longer than other test due to message display
  // inside interrupt handler
  CMSDK_timer_Init_IntClock(CMSDK_TIMER0, 0x2000, 1);

  // Enable Interrupts
  NVIC_EnableIRQ(TIMER0_IRQn);

  while  (IntTaken==0) {
    /* Wait for first interrupt */
    /* Then the processor will enter sleep after Timer IRQ handler */
    /* Sleep on exit is disabled by timer IRQ handler in second occurrence */
    }

  // Stop the timer and disable interrupt generation
  CMSDK_timer_StopTimer(CMSDK_TIMER0);
  CMSDK_timer_DisableIRQ(CMSDK_TIMER0);

  // Show message (no need to test variable because of the loop)
  IntTaken = 0; // Reset software variable
  SleepOnExitTest = 0;
  // Disable sleep on exit feature
  SCB->SCR &= ~(1UL << 1);

  // --------------------------------------------
  // WFI Deep SLEEP with PMU enabled
  // --------------------------------------------

  puts("\n- Detecting PMU control");
  CMSDK_SYSCON->PMUCTRL = CMSDK_SYSCON_PMUCTRL_EN_Msk; // set bit
  if ((CMSDK_SYSCON->PMUCTRL & CMSDK_SYSCON_PMUCTRL_EN_Msk)==0) {
    puts ("PMU not available\n");
    }
  else {

    puts("\n- WFI DEEP SLEEP with WIC");

    // Ensure we use deep SLEEP - SLEEPDEEP should be set
    // SCR[2] = SLEEPDEEP
    SCB->SCR |= (1UL << 2);

    // Ensure timer interrupt is not pending before the test
    NVIC_ClearPendingIRQ(TIMER0_IRQn);

    // Set timer 0 to decrement from 0xF0 with internal clock,
    // interrupt enabled
    CMSDK_timer_Init_IntClock(CMSDK_TIMER0, 0xF0, 1);

    // Enable Interrupts
    NVIC_EnableIRQ(TIMER0_IRQn);

              // Wait For Interrupt
    __WFI();  // Enter sleep mode
              // Wakeup when timer interrupt is triggered

    // Stop the timer and disable interrupt generation
    CMSDK_timer_StopTimer(CMSDK_TIMER0);
    CMSDK_timer_DisableIRQ(CMSDK_TIMER0);

    // Check result
    if (IntTaken) puts("  TIMER IRQ.\n");

    // Disable Interrupt and Interrupt Source
    NVIC_DisableIRQ(TIMER0_IRQn);
    IntTaken = 0; // Reset software variable


  // --------------------------------------------
  // WFE Deep SLEEP with PMU enabled
  // --------------------------------------------
    puts("\n- WFE DEEP SLEEP with WIC");

    // Ensure we use deep SLEEP - SLEEPDEEP should be set
    // SCR[2] = SLEEPDEEP
    SCB->SCR |= (1UL << 2);

    // Ensure timer interrupt is not pending before the test
    NVIC_ClearPendingIRQ(TIMER0_IRQn);

    IntTaken = 0; // Reset software variable

    // Set timer 0 to decrement from 0xF0 with internal clock,
    // interrupt enabled
    CMSDK_timer_Init_IntClock(CMSDK_TIMER0, 0xF0, 1);

    // Enable Interrupts
    NVIC_EnableIRQ(TIMER0_IRQn);

    while  (IntTaken==0) { /* Loop for sleep :
       (first WFE could be woken up immediately due to event from the past)
       */
              // Wait For Event
      __WFE();// Enter sleep mode
              // Wakeup when timer interrupt is triggered
    }

    // Stop the timer and disable interrupt generation
    CMSDK_timer_StopTimer(CMSDK_TIMER0);
    CMSDK_timer_DisableIRQ(CMSDK_TIMER0);

    // Check result
    // Show message (no need to test variable because of the loop)
    puts("  TIMER IRQ.\n");
    IntTaken = 0; // Reset software variable

    // --------------------------------------------
    // SLEEP-ON-EXIT
    // --------------------------------------------
    puts("\n- SLEEP-ON-EXIT with WIC:");

    // Enable sleep on exit feature
    SCB->SCR |= (1UL << 1);

    // Ensure timer interrupt is not pending before the test
    NVIC_ClearPendingIRQ(TIMER0_IRQn);

    IntTaken = 0; // Reset software variable
    SleepOnExitTest = 1;

    // Set timer 0 to decrement from 0x2000 with internal clock,
    // interrupt enabled
    // Duration is longer than other test due to message display
    // inside interrupt handler
    CMSDK_timer_Init_IntClock(CMSDK_TIMER0, 0x2000, 1);

    // Enable Interrupts
    NVIC_EnableIRQ(TIMER0_IRQn);

    while  (IntTaken==0) {
      /* Wait for first interrupt */
      /* Then the processor will enter sleep after Timer IRQ handler */
      /* Sleep on exit is disabled by timer IRQ handler in second occurrence */
      }

    // Stop the timer and disable interrupt generation
    CMSDK_timer_StopTimer(CMSDK_TIMER0);
    CMSDK_timer_DisableIRQ(CMSDK_TIMER0);

    // Show message (no need to test variable because of the loop)
    IntTaken = 0; // Reset software variable
    SleepOnExitTest = 0;
    // Disable sleep on exit feature
    SCB->SCR &= ~(1UL << 1);

    }

    // --------------------------------------------
    // Disable PMU
    // --------------------------------------------
    CMSDK_SYSCON->PMUCTRL = 0; // Clear bit
    if ((CMSDK_SYSCON->PMUCTRL & CMSDK_SYSCON_PMUCTRL_EN_Msk)!=0) {
      puts ("ERROR:PMU control cannot be cleared\n");
    }

    puts("\n- WFI DEEP SLEEP with WIC switched off");

    // Ensure we use deep SLEEP - SLEEPDEEP should be set
    // SCR[2] = SLEEPDEEP
    SCB->SCR |= (1UL << 2);

    // Ensure timer interrupt is not pending before the test
    NVIC_ClearPendingIRQ(TIMER0_IRQn);

    // Set timer 0 to decrement from 0xF0 with internal clock,
    // interrupt enabled
    CMSDK_timer_Init_IntClock(CMSDK_TIMER0, 0xF0, 1);

    // Enable Interrupts
    NVIC_EnableIRQ(TIMER0_IRQn);

              // Wait For Interrupt
    __WFI();  // Enter sleep mode
              // Wakeup when timer interrupt is triggered

    // Stop the timer and disable interrupt generation
    CMSDK_timer_StopTimer(CMSDK_TIMER0);
    CMSDK_timer_DisableIRQ(CMSDK_TIMER0);

    // Check result
    if (IntTaken) puts("  TIMER IRQ.\n");

  // Disable Interrupt and Interrupt Source
  NVIC_DisableIRQ(TIMER0_IRQn);
  IntTaken = 0;

  printf("** TEST PASSED ** \n");

  UartEndSimulation();
  /* Simulation stops in UartEndSimulation */

  return 0;
}

//
// Timer 0 detection
//
/* Macros for word access to address */
#define HW32_REG(ADDRESS)  (*((volatile unsigned long  *)(ADDRESS)))

int timer0_id_check(void)
{
if ((HW32_REG(CMSDK_TIMER0_BASE + 0xFE0) != 0x22) ||
    (HW32_REG(CMSDK_TIMER0_BASE + 0xFE4) != 0xB8))
  return 1; /* part ID does not match */
else
  return 0;
}


//
// Timer interrupt handler
//

void TIMER0_Handler(void)
{
  CMSDK_timer_ClearIRQ(CMSDK_TIMER0);
  // Signal to main code that the INT was taken
  IntTaken++;
  if (SleepOnExitTest==1){
    puts("  TIMER IRQ.");
    if (IntTaken==1){
      puts("  Enter sleep...");
      }

    if (IntTaken==2){
      // Disable sleep on exit feature
      SCB->SCR &= ~(1UL << 1);
      // Stop the timer and disable interrupt generation
      CMSDK_timer_StopTimer(CMSDK_TIMER0);
      CMSDK_timer_DisableIRQ(CMSDK_TIMER0);
      }
   } else {
    // Stop the timer and disable interrupt generation
    CMSDK_timer_StopTimer(CMSDK_TIMER0);
    CMSDK_timer_DisableIRQ(CMSDK_TIMER0);
  }

}

