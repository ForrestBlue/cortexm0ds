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
 *      Checked In          : $Date: 2013-04-09 13:16:37 +0100 (Tue, 09 Apr 2013) $
 *
 *      Revision            : $Revision: 243344 $
 *
 *      Release Information : Cortex-M0 DesignStart-r1p0-00rel0
 *-----------------------------------------------------------------------------
 */

/*
  A simple test to check the functionalities of timer functions in the example device driver.
*/

#include "CMSDK_CM0.h"
#include <stdio.h>
#include "uart_stdout.h"
#include "CMSDK_driver.h"

int volatile timer_irq_counter;
int volatile timer_irq_executed;
uint32_t temp_test;

/* Declare test functions */
int Timer_Init(void);
int Timer_IRQ(void);
int Timer_Poll(void);
int Timer_Ext(void);

int timer0_id_check(void); // Detect Timer 0 present
int gpio1_id_check(void);  // Detect GPIO 1 present

int main (void)
{
  int result = 0;

  // UART init
  UartStdOutInit();

  // Test banner message and revision number
  puts("\nCortex Microcontroller System Design Kit - Timer Driver Test - revision $Revision: 243344 $\n");

  if (timer0_id_check()!=0) {
    puts ("** TEST SKIPPED ** Timer 0 not present.");
    UartEndSimulation();
    return 0;}

  /* Run tests */
  result |= Timer_Init();
  result |= Timer_IRQ();
  result |= Timer_Poll();

  if (gpio1_id_check()==0) {
    /* GPIO #1 is require for creating test input */
    result |= Timer_Ext(); /* Run external input test */
    }
  else {
    if(result != 0) {
       printf("** TEST FAILED **, Error code: (0x%x)\n\n", result);
       }
    puts ("** TEST SKIPPED ** GPIO 1 not present.");
    UartEndSimulation();
    return 0;
    }

  if(result == 0) {
    puts("** TEST PASSED **\n");
  } else {
    printf("** TEST FAILED **, Error code: (0x%x)\n\n", result);
  }

  UartEndSimulation();

  return result;
}



/* Initialisation test - set timer reload to 0xF00 with interrupt enabled */
/* The test also check read back values of the control register and reload register */
int Timer_Init(void)
{
  int err_code = 0;
  uint32_t CTRL = 0;

  puts("\nStage 1 - Timer Initialisation\n");

  /* Stage 1 Test sequence
  After CMSDK_timer_Init_IntClock is executed,
  - read back timer #0 control register and save it to a variable
  - stop timer (to stop interrupt for the moment to improve speed of the checking)
  - check timer control register is set with correct value
  - check timer reload register is set with correct value
  - write back the previous control register value to resume timer operations
  */

  CMSDK_timer_Init_IntClock(CMSDK_TIMER0, 0xF00, 1);    /*test timer interrupt using internal clock*/
  CTRL = CMSDK_TIMER0->CTRL;
  CMSDK_timer_DisableIRQ(CMSDK_TIMER0);

  if((CTRL & CMSDK_TIMER_CTRL_EN_Msk) != 0) {
    puts("Timer Enabled");
  }
  else{
    err_code |= (1 << 0);
    printf("** TEST FAILED **, Error code: (0x%x)\n", err_code);
  }
  if((CTRL & CMSDK_TIMER_CTRL_SELEXTEN_Msk) == 0){
    puts("Ext Enable Not Selected");
  }
  else{
    err_code |= (1 << 1);
    printf("** TEST FAILED **, Error code: (0x%x)\n", err_code);
  }
  if((CTRL & CMSDK_TIMER_CTRL_SELEXTCLK_Msk) == 0){
    puts("Ext Clock Not Selected");
  }
  else{
    err_code |= (1 << 2);
    printf("** TEST FAILED **, Error code: (0x%x)\n", err_code);
  }
  if((CTRL & CMSDK_TIMER_CTRL_IRQEN_Msk) != 0) {
    puts("Timer IRQ Enabled");
  }
  else{
    err_code |= (1 << 3);
    printf("** TEST FAILED **, Error code: (0x%x)\n", err_code);
  }
  if((CMSDK_TIMER0->RELOAD == 0xF00) != 0){
    puts("Reload Value of 0xF00");
  }
  else{
    err_code |= (1 << 4);
    printf("** TEST FAILED **, Error code: (0x%x)\n", err_code);
  }
  if((CTRL & 0x9)){
    CMSDK_TIMER0->CTRL = CTRL;
    puts("\nTimer Initialisation Successful\n");
  }
  else{
    printf("\n** TEST FAILED ** Timer Initialisation Failed Program Will Now Quit, Error code: (0x%x)\n", err_code);
    return err_code;
  }

  if(!err_code) return 0;
  else return 1;
}
/* Interrupt test */
int Timer_IRQ(void)
{
  int counter = 0;               /* Loop counter */
  int err_code = 0;              /* Variable to record errors */
  uint32_t CTRL = 0;             /* Variable to hold read back CTRL value */
  uint32_t RELOAD = 0;           /* Variable to hold updated RELOAD value */

  timer_irq_counter = 0;        /* Set timer interrupt variables to 0 */
  timer_irq_executed = 0;

  puts("Stage 2 - interrupt\n");

  // For robustness, disable the timer, clear the IRQ in case it's set, then
  // clear the current value to zero, and then enable again; this should ensure
  // that operation of this test part starts from a stable point
  //
  // Also set the reload value to 0x1f00 to allow the test more time to execute
  // the test loop, and increment the loop counter sufficiently quickly in
  // relation to the timer to allow all aspects of this test part to be
  // executed
  CMSDK_timer_StopTimer (CMSDK_TIMER0);
  CMSDK_timer_ClearIRQ  (CMSDK_TIMER0);
  CMSDK_timer_SetValue  (CMSDK_TIMER0, 0x0);
  CMSDK_timer_SetReload (CMSDK_TIMER0, 0x1f00);
  // Now start the timer before the test sequence
  CMSDK_timer_StartTimer(CMSDK_TIMER0);

  /* Stage 2 Test sequence
  - timer enabled by CMSDK_timer_Init_IntClock, with interrupt enabled
  - first interrupt triggered
  - after first IRQ, timer interrupt is disabled
  - after loop executed 5 times, reload value is reprogrammed to double the value
  - after a while, second interrupt taken place
  - after second IRQ, timer interrupt is disabled
  - the loop is executed to 10th time (or multiple of 10), timer interrupt is re-enabled
    (multiple of 10 is used to prevent the test from getting stuck.
     Due to timing dependency nature of this test, the test sequence could be different
     from expected when there are wait states in the memory system.)
  - after a while, third interrupt taken place
  - the timer is stopped inside the timer interrupt handler
  - the loop is exited
  */

  NVIC_ClearPendingIRQ(TIMER0_IRQn);
  NVIC_EnableIRQ(TIMER0_IRQn);
  while (timer_irq_counter < 3)       /* Wait until IRQ take place 3 times*/
  {
    if (timer_irq_executed!=0)        /* This part is executed each time a timer interrupt is detected */
    {
      puts ("Timer IRQ\n");
      printf("Timer Status: %u\n", timer_irq_executed);
      CMSDK_timer_DisableIRQ(CMSDK_TIMER0);
      puts("Timer IRQ Disabled");
      timer_irq_executed=0;           /* clear variable so that this part is not executed until next timer interrupt */
    }

    printf("Timer Status: %u\n", CMSDK_timer_StatusIRQ(CMSDK_TIMER0));

    counter++; /* loop counter increment */

    if (counter == 5) /* after loop executed 5 times, reload value is reprogrammed to double the value */
    {
      printf("Timer Value: %u\n", CMSDK_timer_GetValue(CMSDK_TIMER0));
      printf("Timer Reload Value: %u\n",CMSDK_timer_GetReload(CMSDK_TIMER0));
      CMSDK_timer_SetReload(CMSDK_TIMER0, (RELOAD = 2*CMSDK_timer_GetReload(CMSDK_TIMER0)));
      /*test other timer functions including set reload and set value*/
      printf("Timer Reload Value now set to: %u\n",CMSDK_timer_GetReload(CMSDK_TIMER0));

      if(RELOAD  == CMSDK_timer_GetReload(CMSDK_TIMER0)) {
        puts("Timer Reload Value Is Correct");   /*check for reload value */
      }
      else{
        err_code = (1 << 0);
        printf("** TEST FAILED **, Error code: (0x%x)\n", err_code);
        break;
      }

      CMSDK_timer_StopTimer(CMSDK_TIMER0);
      CMSDK_timer_SetValue(CMSDK_TIMER0, 0x12C);    /*set to 300 to test instant underflow*/

      if(0x12C  == CMSDK_timer_GetValue(CMSDK_TIMER0)) {
        puts("Timer Value Set to 300");    /*check for reload value */
      }
      else{
        err_code = (1 << 1);
        printf("** TEST FAILED **, Error code: (0x%x)\n", err_code);
        break;
      }
      CMSDK_timer_StartTimer(CMSDK_TIMER0);
      CMSDK_timer_EnableIRQ(CMSDK_TIMER0);
    }
    if ((counter % 10) == 0) /* the loop is executed to 10th time (or multiple of 10), timer interrupt is re-enabled */
    {
      CMSDK_timer_StopTimer(CMSDK_TIMER0);
      CMSDK_timer_EnableIRQ(CMSDK_TIMER0);
      CTRL = CMSDK_TIMER0->CTRL;  /* Check timer interrupt is enabled */
      if((CTRL & CMSDK_TIMER_CTRL_IRQEN_Msk) != 0) {
        puts("Timer IRQ enabled");
      }
      else{
        err_code |= (1 << 2);
        printf("** TEST FAILED **, Error code: (0x%x)\n", err_code);
        break;
      }
      CMSDK_timer_StartTimer(CMSDK_TIMER0);
    }
  }

  CMSDK_timer_StopTimer(CMSDK_TIMER0);
  /*read back of control register to make sure value is expected */

  if((CMSDK_TIMER0->CTRL & CMSDK_TIMER_CTRL_EN_Msk) == 0) {
    puts("\nTimer Stopped");
  }
  else{
    err_code = (1 << 3);
    printf("** TEST FAILED **, Error code: (0x%x)\n", err_code);
  }

  if(!err_code) {
     puts("** Timer IRQ Test Passed **\n");
  }
  else printf("** TEST FAILED **, Error code: (0x%x)\n", err_code);

  if(!err_code) return 0;
  else return 2;
}
/* Timer polling test */
int Timer_Poll(void)
{
  int err_code = 0;           /* Variable to record errors */
  int Reload = 0;             /* Read back value of reload */

  /* Stage 3 Test sequence
  - timer interrupt is disabled at NVIC
  - timer is started
  - each time the timer interrupt status register is set (timer overflow),
    the counter variable (timer_irq_counter) increments and the interrupt status is cleared.
  - the loop is executed three times
  */
  printf ("\nStage 3 - Polling\n\n");

  NVIC_DisableIRQ(TIMER0_IRQn);                 /*disable IRQ ready to test polling*/
  // Also enable the Timer IRQ since the test relies on reading the IRQ status
  // as non-zero to progress - should be enabled by the previous part, but the
  // behaviour of that part could be different if there are wait-states in the
  // memory system, and hence it could leave the Timer IRQ disabled
  CMSDK_timer_EnableIRQ(CMSDK_TIMER0);
  CMSDK_timer_SetReload(CMSDK_TIMER0,0xF00);    /*reload timer and start it*/

  Reload = CMSDK_timer_GetReload(CMSDK_TIMER0);

  if(Reload  == 0xF00)  {
    puts("Timer Reload Initialised as 0xF00");   /*check for reload value */
  }
  else{
    err_code = (1 << 0);
    printf("** TEST FAILED **, Error code: (0x%x)\n", err_code);
  }

  CMSDK_timer_StartTimer(CMSDK_TIMER0);

  timer_irq_counter = 0;             /* irq counter is re-used for this test although polling is used */

  do {
    printf("Timer Status: %u\n", CMSDK_timer_StatusIRQ(CMSDK_TIMER0));

    while (CMSDK_timer_StatusIRQ(CMSDK_TIMER0) == 0);  /*wait until timer underflow*/
    printf("Timer Status: %u\n", CMSDK_timer_StatusIRQ(CMSDK_TIMER0));

    CMSDK_timer_ClearIRQ(CMSDK_TIMER0);        /*Test Timer status function 3 times by polling for interrupt*/
    timer_irq_counter ++;                      /*status then clearing the IRQ and repeating until 3
                                                 interrupt requests have occurred*/

  } while (timer_irq_counter<3);

  CMSDK_timer_StopTimer(CMSDK_TIMER0);         /*read back of control register to make sure value is expected */

  if((CMSDK_TIMER0->CTRL & CMSDK_TIMER_CTRL_EN_Msk) == 0) {
    puts("\nTimer Stopped");
  }
  else{
    err_code = (1 << 1);
    printf("** TEST FAILED **, Error code: (0x%x)\n", err_code);
  }

  if(!err_code) puts("** Timer Polling Test Passed\n");
  else printf("** TEST FAILED **, Error code: (0x%x)\n", err_code);

  if(!err_code) return 0;
  else return 4;
}

int Timer_Ext(void){

  int test_disabled = 0;  /* variable for stage 4b:to count tests carried out when timer is disabled by external input*/
  int test_enabled = 0;   /* variable for stage 4b:to count tests carried out when timer is enabled by external input*/
  int test_error   = 0;  /* variable for stage 4b:to check the test error result */
  int enabled = 0;        /* variable for stage 4b:to handle test_disabled and test_enabled variable */
  int counter = 0;        /* loop counter */

  puts ("\nStage 4 - Ext Stimuli\n");

  /* Stage 4a Test sequence
  - timer is configured to running with external clock
  - timer is started
  - each time the timer interrupt status register is set (timer overflow),
    the counter variable (timer_irq_counter) increments and the interrupt status is cleared.
  - the loop is executed to toggle the external clock input until timer interrupt triggers three times
  */

  puts ("\n -Stage 4a - Ext Clock\n");

  timer_irq_counter = 0;

  NVIC_ClearPendingIRQ(TIMER0_IRQn);                /*Clear pending IRQs before test started*/
  NVIC_EnableIRQ(TIMER0_IRQn);                      /*enable IRQs*/
  CMSDK_GPIO1->DATAOUT     = 0x000;                 /*Set external clock input to 0 at start of test*/
  CMSDK_gpio_SetOutEnable(CMSDK_GPIO1, 0x100);      /*set pin 8 as output so that it can be used as an external clock*/
  CMSDK_timer_Init_ExtClock(CMSDK_TIMER0, 0x100, 1);/*setup timer to use external clock*/


  while(timer_irq_counter < 3)
  {
    CMSDK_GPIO1->DATAOUT = 0x100;    /*creates square wave on pin 8 to be used as the external clock*/
    __ISB();
    CMSDK_GPIO1->DATAOUT = 0x000;
    if (timer_irq_executed != 0)
    {
      puts ("Timer IRQ");
      timer_irq_executed = 0;
    }
  }

  puts ("\n -Stage 4b - Ext Enable\n");

  /* Stage 4b Test sequence
  - Timer is configured to running with external enable
  - Timer is enabled using ext input
  - The external enable input is toggled for every iteration of the loop
    to disable/enable the timer for the next iterations
  - if the timer is enabled and the value of the timer is not the same as the previous value,
    enable timer test is classed as passed.
  - if the timer is disabled and the value of the timer is the same as the previous value
    then the disable timer test is classed as passed.
  - this process is repeated within one timer overflow and if neither any of the enable timer or disable timer
    tests ever fail then the test as a whole (Stage 4b) passes.
  */

  timer_irq_executed = 0;
  timer_irq_counter = 0;
  counter = 0;            /*counter used to count the external enable input toggle times*/

  enabled = 1;            /* used to indicate whether the timer is enabled or disabled (if enabled is even,
                          timer disabled, if enabled is odd, timer enabled) */

  CMSDK_timer_Init_ExtEnable(CMSDK_TIMER0, 0xF00, 1);    /*setup timer so that it uses an external enable*/
  CMSDK_GPIO1->DATAOUT = 0x100;                          /*set pin 8 (external enable) high so as to enable the timer*/

  puts ("Timer 0 Enabled\n");

  while(timer_irq_counter < 1) /* this test only execute within one timer overflow so that when timer is enabled
                               the sampled timer values are always different*/
  {
      CMSDK_GPIO1->DATAOUT ^= 0x100;
      /* XOR output port with 0x100 so as to invert the external enable signal*/
      enabled++;
      temp_test = CMSDK_timer_GetValue(CMSDK_TIMER0);
      /*record the timer value so that it can be tested whether or not the timer has actually been disabled or not*/

      if((enabled % 2) == 0) //if timer is disabled i.e. enabled is even, increment the test_disabled counter
      {
        test_disabled = 1;
      }
      else //
      {
        test_enabled = 1;
      }

    if((test_disabled == 1) && (test_enabled == 0))
     /*Disable pass condition - only pass if value doesn't change*/
    {
      test_disabled = 0;
      if(temp_test == CMSDK_timer_GetValue(CMSDK_TIMER0))
      /*if the value of the sampled value is the same as the current timer value,
      then it can be inferred that the timer is disabled*/
      {
        puts("Disable Test Passed");
      }
      else
      {
        puts("** TEST FAILED ** Disable Test");
        test_error++;
      }
    }
   else if((test_enabled ==1) && (test_disabled == 0))
    /*Enable pass condition - only pass if value is different*/
    {
      test_enabled = 0;
      if (temp_test != CMSDK_timer_GetValue(CMSDK_TIMER0))
        /*if the value of the sampled value is different from the current then
          it can be inferred that the timer is enabled*/
      {
        puts("Enable Test Passed");
      }
      else
      {
        puts("** TEST FAILED ** Enable Test");
        test_error++;
      }
    }

   counter++; //Enable Toggle counter

  }

  printf("\nTimer enable/disable toggled %d times\n", counter);
  if(test_error == 0)
  {
    puts("\nExt Stimuli Tests Completed Successfully\n");
    return 0;
  }
  else
  {
    printf("\n** TEST FAILED **, Error code: (0x%x)", test_error);
    return 8;
  }
}

void TIMER0_Handler(void)    /*Timer ISR*/
{
  timer_irq_counter++;
  timer_irq_executed=1;
  if (timer_irq_counter>=3) {
    CMSDK_timer_StopTimer(CMSDK_TIMER0);
  }
  CMSDK_timer_ClearIRQ(CMSDK_TIMER0);
  return;
}

// ----------------------------------------------------------
// Peripheral detection
// ----------------------------------------------------------
/* Detect the ARM ID and the part number to see if device is present */
#define HW32_REG(ADDRESS)  (*((volatile unsigned long  *)(ADDRESS)))
#define HW8_REG(ADDRESS)   (*((volatile unsigned char  *)(ADDRESS)))
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
