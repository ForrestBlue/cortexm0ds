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

/*
  A simple test to check the functionalities of APB timer
*/

#include "CMSDK_CM0.h"
#include <stdio.h>
#include "uart_stdout.h"

int timer_register_rw_test(CMSDK_TIMER_TypeDef *CMSDK_TIMER);
int timer_start_stop_test(CMSDK_TIMER_TypeDef *CMSDK_TIMER);
int timer_interrupt_test_1(CMSDK_TIMER_TypeDef *CMSDK_TIMER);
int timer_external_input(CMSDK_TIMER_TypeDef *CMSDK_TIMER);
int  timer0_id_check(void); // Detect Timer 0 present
int  timer1_id_check(void); // Detect Timer 1 present
int  gpio1_id_check(void);  // Detect GPIO 1 present

/* Global variables */
volatile int timer0_irq_occurred;
volatile int timer1_irq_occurred;
volatile int timer0_irq_expected;
volatile int timer1_irq_expected;
volatile int counter;

/* peripheral and component ID values */
#define APB_TIMER_PID4  0x04
#define APB_TIMER_PID5  0x00
#define APB_TIMER_PID6  0x00
#define APB_TIMER_PID7  0x00
#define APB_TIMER_PID0  0x22
#define APB_TIMER_PID1  0xB8
#define APB_TIMER_PID2  0x1B
#define APB_TIMER_PID3  0x00
#define APB_TIMER_CID0  0x0D
#define APB_TIMER_CID1  0xF0
#define APB_TIMER_CID2  0x05
#define APB_TIMER_CID3  0xB1
#define HW32_REG(ADDRESS)  (*((volatile unsigned long  *)(ADDRESS)))
#define HW8_REG(ADDRESS)   (*((volatile unsigned char  *)(ADDRESS)))

int main (void)
{
  int result=0;

  // UART init
  UartStdOutInit();

  // Test banner message and revision number
  puts("\nCortex Microcontroller System Design Kit - Timer Test - revision $Revision: 243249 $\n");

  timer0_irq_occurred = 0;
  timer1_irq_occurred = 0;
  timer0_irq_expected = 0;
  timer1_irq_expected = 0;

  puts("1) Timer 0");

  if (timer0_id_check()==0) { /* Detect if timer is available */
    /* Timer 0 present */
    result += timer_register_rw_test(CMSDK_TIMER0);
    result += timer_start_stop_test(CMSDK_TIMER0);
    result += timer_interrupt_test_1(CMSDK_TIMER0);

    if (gpio1_id_check()==0) {
      /* External input test need GPIO 1 to generate input */
      result += timer_external_input(CMSDK_TIMER0);
      } else {
      puts ("- GPIO #1 not present. Test skipped.");
      }

    } else {
    puts ("- Timer #0 not present. Tests skipped.");
    }


  puts("\n2) Timer 1");

  if (timer1_id_check()==0) { /* Detect if timer is available */
    /* Timer 1 present */
    result += timer_register_rw_test(CMSDK_TIMER1);
    result += timer_start_stop_test(CMSDK_TIMER1);
    result += timer_interrupt_test_1(CMSDK_TIMER1);
    if (gpio1_id_check()==0) {
      /* External input test need GPIO 1 to generate input */
      result += timer_external_input(CMSDK_TIMER1);
      } else {
      puts ("- GPIO #1 not present. Test skipped.");
      }
    } else {
    puts ("- Timer #1 not present. Tests skipped.");
    }

  if (result==0) {
    printf ("\n** TEST PASSED **\n");
  } else {
    printf ("\n** TEST FAILED **\n, Error code = (0x%x)", result);
  }
  UartEndSimulation();
  return 0;
}
/* --------------------------------------------------------------- */
/*  Timer register read/write tests                                */
/* --------------------------------------------------------------- */

int timer_register_rw_test(CMSDK_TIMER_TypeDef *CMSDK_TIMER){
  int return_val=0;
  int err_code=0;
  unsigned int timer_base;
  puts("Timer R/W test");
  puts("- initial values");

  NVIC_DisableIRQ(TIMER0_IRQn);
  NVIC_DisableIRQ(TIMER1_IRQn);

  if (CMSDK_TIMER->CTRL      != 0) { err_code += (1<<0); }
  if (CMSDK_TIMER->VALUE     != 0) { err_code += (1<<1); }
  if (CMSDK_TIMER->RELOAD    != 0) { err_code += (1<<2); }
  if (CMSDK_TIMER->INTSTATUS != 0) { err_code += (1<<3); }

  timer_base = CMSDK_TIMER0_BASE;
  if (CMSDK_TIMER==CMSDK_TIMER1) {timer_base = CMSDK_TIMER1_BASE;}

  // Perform a write access to all PIDs
  HW32_REG(timer_base + 0xFD0) = 0xFFFFFFFF;
  HW32_REG(timer_base + 0xFD4) = 0xFFFFFFFF;
  HW32_REG(timer_base + 0xFD8) = 0xFFFFFFFF;
  HW32_REG(timer_base + 0xFDC) = 0xFFFFFFFF;
  HW32_REG(timer_base + 0xFE0) = 0xFFFFFFFF;
  HW32_REG(timer_base + 0xFE4) = 0xFFFFFFFF;
  HW32_REG(timer_base + 0xFE8) = 0xFFFFFFFF;
  HW32_REG(timer_base + 0xFEC) = 0xFFFFFFFF;
  HW32_REG(timer_base + 0xFF0) = 0xFFFFFFFF;
  HW32_REG(timer_base + 0xFF4) = 0xFFFFFFFF;
  HW32_REG(timer_base + 0xFF8) = 0xFFFFFFFF;
  HW32_REG(timer_base + 0xFFC) = 0xFFFFFFFF;

  if (HW32_REG(timer_base + 0xFD0) != APB_TIMER_PID4) {err_code += (1<<4); }
  if (HW32_REG(timer_base + 0xFD4) != APB_TIMER_PID5) {err_code += (1<<5); }
  if (HW32_REG(timer_base + 0xFD8) != APB_TIMER_PID6) {err_code += (1<<6); }
  if (HW32_REG(timer_base + 0xFDC) != APB_TIMER_PID7) {err_code += (1<<7); }
  if (HW32_REG(timer_base + 0xFE0) != APB_TIMER_PID0) {err_code += (1<<8); }
  if (HW32_REG(timer_base + 0xFE4) != APB_TIMER_PID1) {err_code += (1<<9); }
  if (HW32_REG(timer_base + 0xFE8) != APB_TIMER_PID2) {err_code += (1<<10); }
  if (HW32_REG(timer_base + 0xFEC) != APB_TIMER_PID3) {err_code += (1<<11); }
  if (HW32_REG(timer_base + 0xFF0) != APB_TIMER_CID0) {err_code += (1<<12); }
  if (HW32_REG(timer_base + 0xFF4) != APB_TIMER_CID1) {err_code += (1<<13); }
  if (HW32_REG(timer_base + 0xFF8) != APB_TIMER_CID2) {err_code += (1<<14); }
  if (HW32_REG(timer_base + 0xFFC) != APB_TIMER_CID3) {err_code += (1<<15); }


  if (err_code != 0) {
    printf ("ERROR : initial value failed (0x%x)\n", err_code);
    return_val=1;
    err_code = 0;
    }
  puts("- read/write");

  CMSDK_TIMER->VALUE = 0x3355AAFF;
  if (CMSDK_TIMER->VALUE     != 0x3355AAFF) { err_code += (1<<0); }
  CMSDK_TIMER->VALUE = 0xCCAA5500;
  if (CMSDK_TIMER->VALUE     != 0xCCAA5500) { err_code += (1<<1); }
  CMSDK_TIMER->VALUE = 0x00000000;
  if (CMSDK_TIMER->VALUE     != 0x00000000) { err_code += (1<<2); }

  CMSDK_TIMER->RELOAD = 0x3355AAFF;
  if (CMSDK_TIMER->RELOAD    != 0x3355AAFF) { err_code += (1<<3); }
  CMSDK_TIMER->RELOAD = 0xCCAA5500;
  if (CMSDK_TIMER->RELOAD    != 0xCCAA5500) { err_code += (1<<4); }
  CMSDK_TIMER->RELOAD = 0x00000000;
  if (CMSDK_TIMER->RELOAD    != 0x00000000) { err_code += (1<<5); }

  CMSDK_TIMER->CTRL = 0x01; /* Set enable */
  if (CMSDK_TIMER->CTRL      != 0x01) { err_code += (1<<6); }
  CMSDK_TIMER->CTRL = 0x02; /* external select */
  if (CMSDK_TIMER->CTRL      != 0x02) { err_code += (1<<7); }
  CMSDK_TIMER->CTRL = 0x04; /* external clock select */
  if (CMSDK_TIMER->CTRL      != 0x04) { err_code += (1<<8); }
  CMSDK_TIMER->CTRL = 0x08; /* external clock select */
  if (CMSDK_TIMER->CTRL      != 0x08) { err_code += (1<<9); }
  CMSDK_TIMER->CTRL = 0x00; /* all clear */
  if (CMSDK_TIMER->CTRL      != 0x00) { err_code += (1<<10); }

  /* Trigger timer interrupt status */
  CMSDK_TIMER->RELOAD = 0x3;
  CMSDK_TIMER->VALUE  = 0x3;
  CMSDK_TIMER->CTRL   = 0x9; /* enable with internal clock as source */
  while ( CMSDK_TIMER->INTSTATUS == 0);
  CMSDK_TIMER->CTRL   = 0x0; /* disable timer */
  if (CMSDK_TIMER->INTSTATUS  != 0x01) { err_code += (1<<11); }
  CMSDK_TIMER->INTCLEAR = 0x1; /* clear timer interrupt */
  if (CMSDK_TIMER->INTSTATUS  != 0x00) { err_code += (1<<12); }

  /* Generate return value */
  if (err_code != 0) {
    printf ("ERROR : Read/write failed (0x%x)\n", err_code);
    return_val=1;
    err_code = 0;
    }

  NVIC_ClearPendingIRQ(TIMER0_IRQn);
  NVIC_ClearPendingIRQ(TIMER1_IRQn);

  return(return_val);
}

/* --------------------------------------------------------------- */
/*  Timer start/stop tests                                         */
/* --------------------------------------------------------------- */
/*
  Timer value set to 0x1000, and enabled.
  A polling loop is then use to check that timer had decremented to 0x900
  within a certain period of time. A software variable called counter to
  used to detect timeout, which make sure that the test report fail
  if the timer does not decrement.

  The timer is then stopped, and the value is sampled twice and checked
  that the timer is really stopped.

  The timer value is then set to 0x10, and then enabled, with interrupt feature
  enabled (Note : NVIC is not enabled for this test so interrupt doesn't get
  triggered).
  A polling loop is then use to wait until timer reach zero and reloaded
  (by checking interrupt status register).  The current value of the timer
  is then checked to make sure it is in the expected value range.

*/
int timer_start_stop_test(CMSDK_TIMER_TypeDef *CMSDK_TIMER){
  int return_val=0;
  int err_code=0;
  unsigned long tvalue;
  puts("Timer start/stop test");
  puts("- timer enable");

  NVIC_DisableIRQ(TIMER0_IRQn);
  NVIC_DisableIRQ(TIMER1_IRQn);

  CMSDK_TIMER->RELOAD = 0x1000;
  CMSDK_TIMER->VALUE  = 0x1000;
  CMSDK_TIMER->CTRL = 0x01; /* Set enable */
  counter = 0x100; /* Time out counter */
  while ((CMSDK_TIMER->VALUE > 0x900) && (counter>= 0)){
    counter --;
    }
  CMSDK_TIMER->CTRL = 0x00; /* stop timer for now */
  if (CMSDK_TIMER->VALUE > 0x900) {
     puts("ERROR : Timer not decrementing.");
     err_code += (1<<0);
     }

  tvalue = CMSDK_TIMER->VALUE;
  puts("- timer disable");
  __NOP();
  __NOP();
  if (CMSDK_TIMER->VALUE != tvalue) {
     puts("ERROR : Timer not stopping.");
     err_code += (1<<1);
     }

  /* Check reload operation */
  puts("- timer reload");
  CMSDK_TIMER->VALUE  = 0x10;
  CMSDK_TIMER->CTRL = 0x09; /* Set enable, and interrupt generation */
  while ( CMSDK_TIMER->INTSTATUS == 0);
  CMSDK_TIMER->CTRL = 0x00; /* Stop timer */
  if (CMSDK_TIMER->VALUE > CMSDK_TIMER->RELOAD) {
     puts("ERROR : Timer reload fail 1.");
     err_code += (1<<2);
     }
  if ( (CMSDK_TIMER->RELOAD - CMSDK_TIMER->VALUE) > 0x100 ) {
     puts("ERROR : Timer reload fail 2.");
     err_code += (1<<3);
     }

  // Need to lower the interrupt in the timer before clearing in the NVIC
  CMSDK_TIMER->INTCLEAR = 1;
  // Clear the NVIC interrupts related to both timers to save working out which
  // timer is currently being tested
  NVIC_ClearPendingIRQ(TIMER0_IRQn);
  NVIC_ClearPendingIRQ(TIMER1_IRQn);

  /* Generate return value */
  if (err_code != 0) {
    printf ("ERROR : start/stop failed (0x%x)\n", err_code);
    return_val=1;
    err_code = 0;
    }

  return(return_val);
}
/* --------------------------------------------------------------- */
/*  Timer interrupt test 1                                         */
/* --------------------------------------------------------------- */
/*
  Interrupt enable:
   Timer is enabled, with reload value set to 0x7F (128 cycles),
   and timer interrupt is enabled.
   check that timer interrupt has take place as least twice
   when counter (software variable) is increased from 0 to 0x300.
   If counter is > 0x300 but less than two timer interrupt is received
   (timerx_irq_occurred < 2), then flag it as time out error.

  Interrupt disable:
   Timer is enabled, with reload value set to 0x1F (32 cycles),
   and timer interrupt is disabled.
   The counter (software variable) is increased from 0 to 0x100.
   Check that timer interrupt did not take place.
   (timer0_irq_occurred and timer1_irq_occurred are 0).

*/
int timer_interrupt_test_1(CMSDK_TIMER_TypeDef *CMSDK_TIMER){
  int return_val=0;
  int err_code=0;

  puts ("Timer interrupt test");
  puts ("- Test interrupt generation enabled.");
  CMSDK_TIMER->VALUE = 0; /* Disable timer */

  NVIC_DisableIRQ(TIMER0_IRQn);
  NVIC_DisableIRQ(TIMER1_IRQn);
  NVIC_ClearPendingIRQ(TIMER0_IRQn);
  NVIC_ClearPendingIRQ(TIMER1_IRQn);

  /* Check which timer is being tested and setup software flags */
  if (CMSDK_TIMER==CMSDK_TIMER0) {
    timer0_irq_expected = 1;
    timer1_irq_expected = 0;
    timer0_irq_occurred = 0;
    timer1_irq_occurred = 0;

    NVIC_EnableIRQ(TIMER0_IRQn);  /* Enable Timer Interrupt */

    }

  if (CMSDK_TIMER==CMSDK_TIMER1) {
    timer0_irq_expected = 0;
    timer1_irq_expected = 1;
    timer0_irq_occurred = 0;
    timer1_irq_occurred = 0;

    NVIC_EnableIRQ(TIMER1_IRQn);  /* Enable Timer Interrupt */

    }


  CMSDK_TIMER->RELOAD = 0x01FF;
  CMSDK_TIMER->VALUE  = 0x01FF;
  CMSDK_TIMER->CTRL   = 0x0009;  /* Timer enabled */
  counter = 0;
  if (CMSDK_TIMER==CMSDK_TIMER0) {
    while (( timer0_irq_occurred < 2) && (counter < 0x300)){
      counter ++;
      };
    CMSDK_TIMER->CTRL   = 0x0000;  /* Stop Timer */
    }

  if (CMSDK_TIMER==CMSDK_TIMER1) {
    while (( timer1_irq_occurred < 2) && (counter < 0x300)){
      counter ++;
      };
    CMSDK_TIMER->CTRL   = 0x0000;  /* Stop Timer */
    }
  /* Check timeout has not occurred */
  if (counter >= 0x300) {
     puts("ERROR : Timer interrupt enable fail.");
     err_code += (1<<0);
    }
  counter = 0;

  puts ("- Test interrupt generation disabled.");
  timer0_irq_expected = 0;
  timer1_irq_expected = 0;
  timer0_irq_occurred = 0;
  timer1_irq_occurred = 0;

  CMSDK_TIMER->RELOAD = 0x001F;
  CMSDK_TIMER->VALUE  = 0x001F;
  CMSDK_TIMER->CTRL   = 0x0001;  /* Timer enabled, with interrupt generation disabled */
  counter = 0;
  while ((counter<0x100) && (timer0_irq_occurred==0) && (timer1_irq_occurred==0)) {
    counter++;
  }
  CMSDK_TIMER->CTRL   = 0x0000;  /* Stop Timer */
  if ((timer0_irq_occurred!=0) || (timer1_irq_occurred!=0)) {
     puts("ERROR : Timer interrupt disable fail.");
     err_code += (1<<1);
    }
  counter = 0;

  NVIC_DisableIRQ(TIMER0_IRQn);
  NVIC_DisableIRQ(TIMER1_IRQn);

  /* Generate return value */
  if (err_code != 0) {
    printf ("ERROR : Interrupt test failed (0x%x)\n", err_code);
    return_val=1;
    err_code = 0;
    }

  return(return_val);
}

/* --------------------------------------------------------------- */
/*  Timer extin test 1                                             */
/* --------------------------------------------------------------- */
/* External input :
   TIMER 0 - GPIO #1 [bit 8]
   TIMER 1 - GPIO #1 [bit 9]

   Checking of external input as enable:
   Timer is programmed so that it use external input as enable.
   The external input is set to 0 (disabled), and the timer value is sample twice
   to make sure it is not decrementing.

   The program then set the timer current value to 99, and then use the
   SysTick timer to generate a pulse of 100 cycles on the timer
   external input.  This is done by
    - setting up the SysTick to be triggered at every 100 cycles,
    - the first timer SysTick interrupt is triggered, set ext input to 1
    - the second timer SysTick interrupt is triggered, set ext input to 0, and disable SysTick
   After the SysTick exception is entered twice, it then check if the timer has underflowed.
   (check current value range between 0xFF to 0xA0).

   Since the timer could be running at a divided APB clock speed which is half of the AHB,
   the value check might fail in such configuration. If the current counter value is between
   30 and 60, generate another pulse of 100 clocks cycle to the external input and do the
   check again.

   It then check the timer interrupt status, and the clearing of the interrupt status.

   ------------------------------------
   Checking of external input as clock:
   The test then program the timer to use external input as "clock" (rising edge detection).
   Timer starting value is set to 0xC0.
   It first keep the external input as 0, and check that the timer current value hasn't been
   changed.

   It then change the external input to 1, and check if the timer has decrement. (value = 0xBF)

   It then wait for a short period (counter variable decrement from 10 to 0),
   then check that the timer has not been decremented further.(value = 0xBF)

   It then set external input to 0, again, wait for short period of time,
   and check that the timer has not been decremented further.(value = 0xBF)

   It then generate 15 pulses on the external input, and check the timer has decrement
   15 times. (value = 0xB0) It also check that at this stage the timer interrupt status
   should still be 0.

   It then set the timer value to 2, with interrupt generation enabled, use external input
   as clock.  The external input is then switched from 0 to 1, (value changed from 2 to 1)
   and the timer interrupt shouldn't be triggered. (it should only trigger at 1 to 0 value
   change).

   It then generate another pulse at the external input, and wait for a short period.
   This time the timer interrupt should be triggered.

   it then clear up the interrupt settings and report test status.

*/
int timer_external_input(CMSDK_TIMER_TypeDef *CMSDK_TIMER){
  int return_val=0;
  int err_code=0;
  unsigned long tvalue;
  uint32_t timer_value;

  puts ("Timer external input test");
  /* Set external inputs low */
  CMSDK_GPIO1->OUTENABLESET    = (1<<8) | (1<<9);
  CMSDK_GPIO1->UB_MASKED[3] = 0x0000;

  CMSDK_TIMER->RELOAD = 0x00FF;
  CMSDK_TIMER->VALUE  = 0x00FF;
  CMSDK_TIMER->CTRL   = 0x000B;  /* Timer enabled, use external input, interrupt enabled */

  tvalue = CMSDK_TIMER->VALUE;
  puts("- timer use ext. input");
  __NOP();  /* Some delay is needed due to double flip-flop synchronization logic */
  __NOP();
  if (CMSDK_TIMER->VALUE != tvalue) {
     puts("ERROR : Timer not stopping when external input is low.");
     err_code += (1<<0);
     }

  puts("- use SysTick to generate ext. input pulse");
  CMSDK_TIMER->VALUE  = 0xFE; /* Use 0xFE so that it count to 0 */
  counter = 0;
  NVIC_DisableIRQ(TIMER0_IRQn);
  NVIC_DisableIRQ(TIMER1_IRQn);
  NVIC_ClearPendingIRQ(TIMER0_IRQn);
  NVIC_ClearPendingIRQ(TIMER1_IRQn);

  /* Create a pulse of approximately four times the length of the expected
   * duration that the system timer will be counting down.
   * APB is possibly running at half speed, but this should be allowed for by
   * the fact that the SysTick reload value is set to a larger value than the
   * system timer start value.  This therefore should allow the system timer to
   * count round multiple times, and should have the expected outcome
   * regardless of whether APB is running at full speed or half speed
   */
  SysTick->LOAD = (0x100<<2)-1;
  SysTick->VAL  = 0;
  SysTick->CTRL = 0x7; /* Enable, use internal clock, interrupt enable */

  while (counter < 2); /* SysTick handler trigger twice */

  /* Timer should be reloaded, expected value between 0xA0 to 0xFF (reload value) */
  timer_value = CMSDK_TIMER->VALUE;
  if ((timer_value > 0xFF) || (timer_value < 0xA0)) {
    printf("ERROR : Timer not running when external input is high (0x%x).\n", timer_value);
    err_code += (1<<1);
  }

  /* Timer interrupt should be pending */
  if (((NVIC_GetPendingIRQ(TIMER0_IRQn)==0)&&(CMSDK_TIMER==CMSDK_TIMER0))||
      ((NVIC_GetPendingIRQ(TIMER1_IRQn)==0)&&(CMSDK_TIMER==CMSDK_TIMER1)))
    {
     puts("ERROR : Timer not trigger #1.");
     err_code += (1<<2);
    }

  if (CMSDK_TIMER->INTSTATUS==0) {
    puts("ERROR : Timer not trigger #2.");
    err_code += (1<<3);
    }
  else {
    CMSDK_TIMER->INTCLEAR=1; /* Clear interrupt status */
    if (CMSDK_TIMER->INTSTATUS!=0) {
      puts("ERROR : Timer not clear.");
      err_code += (1<<4);
      }
    }

  NVIC_ClearPendingIRQ(TIMER0_IRQn);
  NVIC_ClearPendingIRQ(TIMER1_IRQn);


  puts("- use EXTIN as clock");
  CMSDK_TIMER->VALUE = 0xC0;
  CMSDK_TIMER->CTRL   = 0x0007;  /* Timer enabled, use external input as clock, interrupt disabled */
  CMSDK_GPIO1->UB_MASKED[3] = 0x0000;
  counter = 10;                        /* delay */
  while (counter > 0) {
    counter --;
    }
  if (CMSDK_TIMER->VALUE != 0xC0) {
     puts("ERROR : Timer change unexpectedly 1.");
     err_code += (1<<5);
    }
  CMSDK_GPIO1->UB_MASKED[3] = 0x0300; /* Timer should decrement */
  __ISB();
  __ISB();

  if (CMSDK_TIMER->VALUE != 0xBF) {
     puts("ERROR : Timer decrement failed.");
     err_code += (1<<6);
    }
  counter = 10;                        /* delay */
  while (counter > 0) {
    counter --;
    }
  if (CMSDK_TIMER->VALUE != 0xBF) {
     puts("ERROR : Timer change unexpectedly 2.");
     err_code += (1<<7);
    }
  CMSDK_GPIO1->UB_MASKED[3] = 0x0000; /* Timer should NOT decrement */
  counter = 10;                        /* delay */
  while (counter > 0) {
    counter --;
    }
  if (CMSDK_TIMER->VALUE != 0xBF) {
     puts("ERROR : Timer change unexpectedly 3.");
     err_code += (1<<8);
    }
  counter = 15;
  while (counter > 0) {
    CMSDK_GPIO1->UB_MASKED[3] = 0x0300; /* Timer should decrement */
    __ISB();
    CMSDK_GPIO1->UB_MASKED[3] = 0x0000; /* Timer should not decrement */
    counter --;
    }
  if (CMSDK_TIMER->VALUE != 0xB0) {
     puts("ERROR : Timer counting failed");
     err_code += (1<<9);
    }

  /* Check interrupt should not be trigger */
  if ((NVIC_GetPendingIRQ(TIMER0_IRQn)!=0)||(NVIC_GetPendingIRQ(TIMER1_IRQn)!=0)||
      (CMSDK_TIMER->INTSTATUS!=0))
    {
     puts("ERROR : Timer IRQ set unexpectedly.");
     err_code += (1<<10);
    }

  puts("- IRQ at 1->0 transition");
  CMSDK_TIMER->VALUE  = 0x02;
  CMSDK_TIMER->CTRL   = 0x000F;  /* Timer enabled, use external input as clock, interrupt enabled */
  CMSDK_GPIO1->UB_MASKED[3] = 0x0000;

  timer0_irq_expected = 0;
  timer1_irq_expected = 0;
  timer0_irq_occurred = 0;
  timer1_irq_occurred = 0;

  if (CMSDK_TIMER==CMSDK_TIMER0) {
    NVIC_EnableIRQ(TIMER0_IRQn);
    }

  if (CMSDK_TIMER==CMSDK_TIMER1) {
    NVIC_EnableIRQ(TIMER1_IRQn);
    }

  /* Test no IRQ generation for 2 to 1 transition */
  CMSDK_GPIO1->UB_MASKED[3] = 0x0300; /* Timer should decrement */
    __ISB();
  CMSDK_GPIO1->UB_MASKED[3] = 0x0000; /* Timer should not decrement */

  counter = 10;                        /* delay */
  while (counter > 0) {
    counter --;
    }

  if (CMSDK_TIMER==CMSDK_TIMER0) {
    timer0_irq_expected=1;
    }

  if (CMSDK_TIMER==CMSDK_TIMER1) {
    timer1_irq_expected=1;
    }

  /* Test IRQ generation for 1 to 0 transition */
  CMSDK_GPIO1->UB_MASKED[3] = 0x0300; /* Timer should decrement */
    __ISB();
  CMSDK_GPIO1->UB_MASKED[3] = 0x0000; /* Timer should not decrement */

  counter = 10;                        /* delay */
  while (counter > 0) {
    counter --;
    }
  if (((CMSDK_TIMER==CMSDK_TIMER0)&&(timer0_irq_occurred==0))||
      ((CMSDK_TIMER==CMSDK_TIMER1)&&(timer1_irq_occurred==0))) {
    puts("ERROR : Timer not trigger.");
    err_code += (1<<11);
    }

  timer0_irq_expected = 0;
  timer1_irq_expected = 0;
  timer0_irq_occurred = 0;
  timer1_irq_occurred = 0;

  /* Test IRQ generation for 0 to RELOAD transition */
  CMSDK_GPIO1->UB_MASKED[3] = 0x0300; /* Timer should decrement */
    __ISB();
  CMSDK_GPIO1->UB_MASKED[3] = 0x0000; /* Timer should not decrement */

  counter = 10;                        /* delay */
  while (counter > 0) {
    counter --;
    }
  if ((timer0_irq_occurred!=0)||(timer1_irq_occurred!=0)) {
    puts("ERROR : Timer triggered unexpectedly.");
    err_code += (1<<12);
    }

  NVIC_DisableIRQ(TIMER0_IRQn);
  NVIC_DisableIRQ(TIMER1_IRQn);
  NVIC_ClearPendingIRQ(TIMER0_IRQn);
  NVIC_ClearPendingIRQ(TIMER1_IRQn);

  /* Clearing up */
  SysTick->CTRL = 0x0;
  CMSDK_TIMER->CTRL   = 0x00;/* Disable, use internal clock, interrupt enable */
  CMSDK_TIMER->RELOAD = 0x0000;
  CMSDK_TIMER->VALUE  = 0x0000;
  CMSDK_GPIO1->OUTENABLECLR    = 0xFFFF;
  CMSDK_GPIO1->UB_MASKED[3] = 0x0000;

  timer0_irq_expected = 0;
  timer1_irq_expected = 0;
  timer0_irq_occurred = 0;
  timer1_irq_occurred = 0;

  /* Generate return value */
  if (err_code != 0) {
    printf ("ERROR : Interrupt test failed (0x%x)\n", err_code);
    return_val=1;
    err_code = 0;
    }

  return(return_val);
}

/* --------------------------------------------------------------- */
/* Peripheral detection                                            */
/* --------------------------------------------------------------- */
/* Detect the part number to see if device is present              */
int timer0_id_check(void)
{
unsigned char timer_id;
  timer_id = HW8_REG(CMSDK_TIMER0_BASE + 0xFE8) & 0x07;

  if ((HW32_REG(CMSDK_TIMER0_BASE + 0xFE0) != 0x22) ||
      (HW32_REG(CMSDK_TIMER0_BASE + 0xFE4) != 0xB8) ||
      (timer_id != 0x03))
    return 1; /* part ID & ARM ID does not match */
  else
    return 0;
}

int timer1_id_check(void)
{
unsigned char timer_id;
  timer_id = HW8_REG(CMSDK_TIMER1_BASE + 0xFE8) & 0x07;

  if ((HW32_REG(CMSDK_TIMER1_BASE + 0xFE0) != 0x22) ||
      (HW32_REG(CMSDK_TIMER1_BASE + 0xFE4) != 0xB8) ||
      (timer_id != 0x03))
    return 1; /* part ID & ARM ID does not match */
  else
    return 0;
}

int gpio1_id_check(void)
{
unsigned char gpio_id;
  gpio_id = HW8_REG(CMSDK_GPIO1_BASE + 0xFE8) & 0x07;

  if ((HW32_REG(CMSDK_GPIO1_BASE + 0xFE0) != 0x20) ||
      (HW32_REG(CMSDK_GPIO1_BASE + 0xFE4) != 0xB8) ||
      (gpio_id != 0x03))
    return 1; /* part ID & ARM ID does not match */
  else
    return 0;
}

/* --------------------------------------------------------------- */
/*  Timer exception handlers                                       */
/* --------------------------------------------------------------- */

void TIMER0_Handler(void)
{
  timer0_irq_occurred ++;
  CMSDK_TIMER0->INTCLEAR = 1;
  if (timer0_irq_expected ==0) {
    puts ("ERROR : Unexpected Timer 0 interrupt occurred.\n");
    UartEndSimulation();
    while (1);
    }
  return;
}

void TIMER1_Handler(void)
{
  timer1_irq_occurred ++;
  CMSDK_TIMER1->INTCLEAR = 1;
  if (timer1_irq_expected ==0) {
    puts ("ERROR : Unexpected Timer 1 interrupt occurred.\n");
    UartEndSimulation();
    while (1);
    }
  return;
}
/* --------------------------------------------------------------- */
/*  SysTick exception handlers                                       */
/* --------------------------------------------------------------- */
void SysTick_Handler(void)
{
  switch (counter) {
    case 0 :
      /* Assert the EXTIN to the system timer */
      CMSDK_GPIO1->UB_MASKED[3] = 0x0300;
      counter ++;
      break;
    case 1 :
      CMSDK_GPIO1->UB_MASKED[3] = 0x0000;
      SysTick->CTRL = 0;
      SCB->ICSR = SCB->ICSR & 0xFDFFFFFF; /* Clear SysTick pending bit */
      counter ++;
      break;
    default:
      SysTick->CTRL = 0;
      SCB->ICSR = SCB->ICSR & 0xFDFFFFFF; /* Clear SysTick pending bit */
      break;
    }
  return;
}

