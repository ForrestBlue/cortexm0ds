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
  Interrupt demonstration

  This test demonstrate interrupt generation using various peripherals.
  - using simple timer
  - using gpio with various configurations
  - using uart

*/

#include "CMSDK_CM0.h"

#include "uart_stdout.h"
#include "CMSDK_driver.h"
#include <stdio.h>

//
// Global Variables to track test progress
//

volatile uint32_t tx_count = 0;                 /*transmit counter */
volatile uint32_t rx_count = 0;                 /*receive counter */
const     char str_tx[12] = "hello world";      /*transmission string*/
const int uart_str_length = 11;
volatile char str_rx[12] ;             /*string that is received*/

volatile int irq_triggered;          /* Detected interrupt operation from GPIO IRQ handler */
volatile uint32_t timer_stopped = 0; /* timer irq executed and stopped */

//
// Demonstrations
//

int  TimerExample(void);    // Timer interrupt
int  GPIOIntExample(void);  // GPIO interrupt
void UartExample(void);     // UART interrupt
int  gpio0_id_check(void);  // Detect GPIO 0 present
int  timer0_id_check(void); // Detect Timer 0 present
int  uart0_id_check(void);  // Detect UART 0 present
int  uart1_id_check(void);  // Detect UART 1 present
int  Check_IRQNUM(void);    // Check number of interrupts

// ----------------------------------------------------------
// Main program
// ----------------------------------------------------------

int main (void)
{
  // UART init
  UartStdOutInit();  // Initialize UART2 for printf (retargeting)

  // Test banner message and revision number
  puts("\nCortex Microcontroller System Design Kit - Interrupt Demo - revision $Revision: 243249 $\n");

  if (timer0_id_check()!=0)  puts ("Timer 0 not present. TimerExample skipped.");
  else                       TimerExample();    // Timer 0 interrupt example

  if (gpio0_id_check()!=0)   puts ("GPIO 0 not present. GPIOIntExample skipped.");
  else                       GPIOIntExample();  // GPIO PORT0 interrupt example

  if ((uart0_id_check()!=0)||(uart1_id_check()!=0))
     puts ("UART 0 or UART 1 not present. UartExample skipped.");
  else                       UartExample();     // Uart interrupt example

  if (Check_IRQNUM() == 0)
     puts ("** TEST PASSED ** \n");
  else
     puts ("** TEST FAILED ** \n");
  
  UartEndSimulation();    // send test end character to finish test
  /* Simulation stops in UartEndSimulation */

  return 0;
}

// ----------------------------------------------------------
// Check number of Interrupts
// ----------------------------------------------------------
int Check_IRQNUM()
{
  //
  // Number of implemented Interrupts
  //
  // Determine number of implemented interrupts by pending all interrupts
  // then reading back the pend register contents. Only configured interrupts will
  // show as pending. This checks if the number of implemented interrupts are
  // consistent with the values on NUMIRQ.
  //
  // NOTE : Higher interrupts in the context of this check refers to interrupts with
  //        a bigger exception number

  int numirq;
  int ispr_rd;
  int j;
  int EXPECTED_IRQNUM = 32;
  int fail = 1;

  // Ensure interrupts are disabled
  NVIC->ICER[0] = 0xFFFFFFFFUL;

  // Pend all interrupts
  NVIC->ISPR[0] = 0xFFFFFFFFUL;

  // Count pending interrupts
  numirq = 0;
  ispr_rd = NVIC->ISPR[0];
  for (j=0; j<32; j++)
    {
      if ((ispr_rd & 0x00000001) == 1) numirq++;
      ispr_rd >>= 1;
    }

  if (numirq != EXPECTED_IRQNUM)
    {
      printf ("\n\nIRQNUM: %u, expected %d\tFAIL\n\n", numirq, EXPECTED_IRQNUM);
      fail = 1;
    }
  else
    {
      printf ("\n\nIRQNUM: %u\t\tPASS\n\n", numirq);
      fail = 0;
    }

  // Clear all pending interrupts
  NVIC->ICPR[0] = 0xFFFFFFFFUL;
  return fail;
}

// ----------------------------------------------------------
// Timer demo
// ----------------------------------------------------------
int TimerExample(void)
{
  puts("\n\n\n");
  puts("+*************************+");
  puts("*                         *");
  puts("*  Timer0 Interrupt demo  *");
  puts("*                         *");
  puts("+*************************+\n\n");

  NVIC_ClearPendingIRQ(TIMER0_IRQn);
  NVIC_EnableIRQ(TIMER0_IRQn);

  // initialise Timer0 with internal clock, with interrupt generation
  CMSDK_timer_Init_IntClock(CMSDK_TIMER0, 0x100, 1);

  while (timer_stopped==0) {
    __WFE(); // enter sleep
    }
  puts("   Timer test done");  // Banner

  return 0;
}
// ----------------------------------------------------------
// UART demo
// ----------------------------------------------------------

/* Timer IRQ Driven UART Transmission of "hello world"

    - Program UART 0 to operate as transmit only, with transmit IRQ enabled
    - Program UART 1 to operate as receive only, with receive IRQ enabled
    - The first character of the "hello world" message is transmit
    - The rest of the message transmission is handled by UART transmit IRQ, until
      all characters are transmitted.
    - The receive process is also handled by UART receive IRQ.
    - A while loop is used to wait until both transmit and receive has completed the test
    - When finished then print string received
   */


// ----------------------------------------------------------
// UART interrupt test
// ----------------------------------------------------------

void UartExample(void)
{
  uint32_t transmission_complete = 0;    /*transmission complete bool*/

  puts("+*************************+");
  puts("*                         *");
  puts("*   UART Interrupt demo   *");
  puts("*                         *");
  puts("+*************************+\n\n");

  CMSDK_gpio_SetAltFunc(CMSDK_GPIO1, 0x000F);  //enable alt functions for UART transmission

  // Ensure Interrupt is not pending

  NVIC_ClearPendingIRQ(UARTTX0_IRQn);
  NVIC_ClearPendingIRQ(UARTRX1_IRQn);

  // Enable Interrupts

  NVIC_EnableIRQ(UARTTX0_IRQn);
  NVIC_EnableIRQ(UARTRX1_IRQn);

  /* Initialize UART in cross over configuration
   uint32_t CMSDK_uart_init(CMSDK_UART_TypeDef *CMSDK_UART,
                             uint32_t            divider,
                             uint32_t            tx_en,
                             uint32_t            rx_en,
                             uint32_t            tx_irq_en,
                             uint32_t            rx_irq_en,
                             uint32_t            tx_ovrirq_en,
                             uint32_t            rx_ovrirq_en)
  */
  /* enable UARTs with selected baud rate
       UART #0 - transmit
       UART #1 - receive
  */
  CMSDK_uart_init(CMSDK_UART0, 0x200, 1, 0, 1, 0, 0, 0);
  CMSDK_uart_init(CMSDK_UART1, 0x200, 0, 1, 0, 1, 0, 0);

  rx_count = 0;
  tx_count = 0;

  printf ("Transmit message : %s\n", str_tx);

  /* Start first character transfer */
  tx_count++;
  CMSDK_uart_SendChar(CMSDK_UART0, str_tx[0]); // send the character
  /* The rest of the transfers are handled by interrupts */

  while(transmission_complete==0)    // loop until transmission completed
  {

    if ((tx_count==uart_str_length) && (rx_count==uart_str_length)) transmission_complete = 1;
  }

  printf ("Received message : %s\n", str_rx);


  NVIC_DisableIRQ(UARTTX0_IRQn);   //disable both UART0 TX and UART1 RX IRQs
  NVIC_DisableIRQ(UARTRX1_IRQn);

  return;
}

// ----------------------------------------------------------
// GPIO interrupt test
// ----------------------------------------------------------
/*
      GPIO interrupt example

    - Enable all pins as output
    - Set DataOut to 0xA ready for a test of all IRQs
    - Set pin0 as a High Level, pin1 as a Low Level, pin2 as a Rising Edge and pin3 as a Falling Edge,
      then clear all pending IRQs
    - enable the CMSDK GPIO interrupt for pins 0, 1, 2, 3.
    - set Dataout to 0xB to test the high level interrupt on pin 0
    - if irq_triggered != 0 (set in ISR) then print message saying IRQ occurred and set irq_triggered = 0
    - else amend err_code
    - set Dataout to 0x9 to test the low level interrupt on pin 1
    - if irq_triggered != 0 (set in ISR) then print message saying IRQ occurred and set irq_triggered = 0
    - else amend err_code
    - set Dataout to 0xD to test the rising edge interrupt on pin 2
    - if irq_triggered != 0 (set in ISR) then print message saying IRQ occurred and set irq_triggered = 0
    - else amend err_code
    - set Dataout to 0x5 to test the falling edge interrupt on pin 3
    - if irq_triggered != 0 (set in ISR) then print message saying IRQ occurred and set irq_triggered = 0
    - else amend err_code
    - if test on all pins pass the test as a whole passes and return 0 to main
    - else return an error and print error message
*/
int GPIOIntExample(void)
{

  int i = 0;
  int irq_counter = 0;
  int err_code = 0;

  puts("\n\n\n");
  puts("+*************************+");
  puts("*                         *");
  puts("*  GPIO PORT0: Interrupt  *");
  puts("*         Example         *");
  puts("*                         *");
  puts("+*************************+\n\n");

  CMSDK_gpio_SetOutEnable(CMSDK_GPIO0, 0xFFFF); //set output enable to output on all ports of GPIO 0
  // By setting the port to output the pins are controllable by software

  CMSDK_GPIO0->DATAOUT = 0xA;   // set current I/O port value

  CMSDK_gpio_SetIntHighLevel(CMSDK_GPIO0, 0);   //set pin 0 to high level interrupts
  CMSDK_gpio_SetIntLowLevel(CMSDK_GPIO0, 1);    //set pin 1 to low level interrupts
  CMSDK_gpio_SetIntRisingEdge(CMSDK_GPIO0, 2);  //set pin 2 to rising edge interrupts
  CMSDK_gpio_SetIntFallingEdge(CMSDK_GPIO0, 3); //set pin 3 to falling edge interrupts

  NVIC_ClearPendingIRQ(PORT0_0_IRQn);                   //clear all global NVIC PORT0 pending interrupts
  NVIC_ClearPendingIRQ(PORT0_1_IRQn);
  NVIC_ClearPendingIRQ(PORT0_2_IRQn);
  NVIC_ClearPendingIRQ(PORT0_3_IRQn);

  NVIC_EnableIRQ(PORT0_0_IRQn);                         //enable NVIC interrupts on PORT0
  NVIC_EnableIRQ(PORT0_1_IRQn);
  NVIC_EnableIRQ(PORT0_2_IRQn);
  NVIC_EnableIRQ(PORT0_3_IRQn);

  if ((NVIC->ISER[0]>>PORT0_0_IRQn)!=0x0F) {            // Cortex-M0 DesignStart only has 16 IRQ
    printf("Not all of IRQ[%d to %d] are available.\nUse combined GPIO interrupt for test\n\n",
           PORT0_0_IRQn,PORT0_3_IRQn);
    NVIC_EnableIRQ(PORT0_ALL_IRQn);                     //enable combined NVIC interrupts on PORT0
    }


  for(i = 0; i < 4; i++) CMSDK_gpio_SetIntEnable(CMSDK_GPIO0, i);    //enable interrupts on pins 0 -3

  CMSDK_GPIO0->DATAOUT = 0xB; // emulating high level input on pin 0.

  puts("    ...Test GPIO0[0]...\n");

  if(irq_triggered){   //if irq flag set then print message else amend error code
    puts("      High Level IRQ:\n     Detected On Pin 0\n\n");
    irq_triggered = 0;
    irq_counter++;
  }
  else err_code |= (1 << irq_counter);

  CMSDK_GPIO0->DATAOUT = 0x9; // emulating low level input on pin 1.

  puts("    ...Test GPIO0[1]...\n");

  if(irq_triggered){  //if irq flag set then print message else amend error code
    puts("       Low Level IRQ\n     Detected On Pin 0\n\n");
    irq_triggered = 0;
    irq_counter++;
  }
  else err_code |= (1 << 1);

  CMSDK_GPIO0->DATAOUT = 0xD; // emulating rising edge input on pin 2.

  puts("    ...Test GPIO0[2]...\n");

  if(irq_triggered){  //if irq flag set then print message else amend error code
    puts("      Rising Edge IRQ\n     Detected On Pin 0\n\n");
    irq_triggered = 0;
    irq_counter++;
  }
  else err_code |= (1 << 2);

  CMSDK_GPIO0->DATAOUT = 0x5; // emulating falling edge input on pin 3.

  puts("    ...Test GPIO0[3]...\n");

  if(irq_triggered){  //if irq flag set then print message else amend error code
    puts("     Falling Edge IRQ:\n     Detected On Pin 0\n\n");
    irq_triggered = 0;
    irq_counter++;
  }
   else err_code |= (1 << 3);

  /* check to see whether intstatus, for the specified pin, is 1, which corresponds to a rising edge interrupt */

  if(irq_counter == 4){
    printf("    All %d IRQs Detected\n\n", i);
  }


  // print pass or fail message depending on the status of the test

  if(err_code == 0){
    puts("\n");
    puts(" +***********************+");
    puts(" *                       *");
    puts(" *   GPIO 0 IRQ Tests    *");
    puts(" *  Passed Successfully  *");
    puts(" *                       *");
    puts(" +***********************+\n");
  }
  else{

    /*if the port did not have 1 of each IRQs as expected then display error*/

    printf("\n** TEST FAILED ** IRQ Tests Error Code: (0x%x\n", err_code);
  }

  NVIC_DisableIRQ(PORT0_0_IRQn);    //disable GPIO0 IRQ
  NVIC_DisableIRQ(PORT0_1_IRQn);
  NVIC_DisableIRQ(PORT0_2_IRQn);
  NVIC_DisableIRQ(PORT0_3_IRQn);
  NVIC_DisableIRQ(PORT0_ALL_IRQn);
  return err_code;
}

// ----------------------------------------------------------
// Peripheral detection
// ----------------------------------------------------------
/* Detect the part number to see if device is present                */

int gpio0_id_check(void)
{
#define HW32_REG(ADDRESS)  (*((volatile unsigned long  *)(ADDRESS)))
if ((HW32_REG(CMSDK_GPIO0_BASE + 0xFE0) != 0x20) ||
    (HW32_REG(CMSDK_GPIO0_BASE + 0xFE4) != 0xB8))
  return 1; /* part ID does not match */
else
  return 0;
}

int timer0_id_check(void)
{
if ((HW32_REG(CMSDK_TIMER0_BASE + 0xFE0) != 0x22) ||
    (HW32_REG(CMSDK_TIMER0_BASE + 0xFE4) != 0xB8))
  return 1; /* part ID does not match */
else
  return 0;
}

int uart0_id_check(void)
{
if ((HW32_REG(CMSDK_UART0_BASE + 0xFE0) != 0x21) ||
    (HW32_REG(CMSDK_UART0_BASE + 0xFE4) != 0xB8))
  return 1; /* part ID does not match */
else
  return 0;
}
int uart1_id_check(void)
{
if ((HW32_REG(CMSDK_UART1_BASE + 0xFE0) != 0x21) ||
    (HW32_REG(CMSDK_UART1_BASE + 0xFE4) != 0xB8))
  return 1; /* part ID does not match */
else
  return 0;
}
// ----------------------------------------------------------
// Handlers
// ----------------------------------------------------------
// ---------------------------------
// UART 0 Interrupt service routines
// ---------------------------------
//

void UARTTX0_Handler(void)
{
  CMSDK_uart_ClearTxIRQ(CMSDK_UART0); // clear TX IRQ
  // If the message output is not finished, output next character
  if (tx_count < uart_str_length) {
    CMSDK_uart_SendChar(CMSDK_UART0,str_tx[tx_count]);
    tx_count++;
    }

}

// ---------------------------------
// UART 1 Interrupt service routines
// ---------------------------------

void UARTRX1_Handler(void)
{
  CMSDK_uart_ClearRxIRQ(CMSDK_UART1); //clear RX IRQ
  str_rx[rx_count]=CMSDK_uart_ReceiveChar(CMSDK_UART1); // Read data
  rx_count++;
}

// ---------------------------------
// Timer 0 Interrupt service routines
// ---------------------------------

void TIMER0_Handler(void)
{
  timer_stopped = 1;                      // set timer stopped bool, so that
                                          // system does not wait for another interrupt
  CMSDK_timer_StopTimer(CMSDK_TIMER0);    // stop timer
  CMSDK_timer_ClearIRQ(CMSDK_TIMER0);     // clear timer 0 IRQ
  puts("   [Timer 0 IRQ]");
}

// ---------------------------------
// GPIO Port 0 Interrupt service routines
// ---------------------------------
//
void PORT0_0_Handler(void)
{
  irq_triggered = 1;                            /* high level */
  CMSDK_GPIO0->DATAOUT = 0xA;                  /* Deassert Port 0 pin 0 to 0 */
  CMSDK_gpio_IntClear(CMSDK_GPIO0, 0);        //clear GPIO interrupt on pin N
}

void PORT0_1_Handler(void)
{
  irq_triggered = 1;                            /*low level*/
  CMSDK_GPIO0->DATAOUT = 0xB;                  /* Deassert Port 0 pin 1 to 1 */
  CMSDK_gpio_IntClear(CMSDK_GPIO0, 1);        //clear GPIO interrupt on pin N
}

void PORT0_2_Handler(void)
{
  irq_triggered = 1;                            /*rising edge*/
  CMSDK_gpio_IntClear(CMSDK_GPIO0, 2);        //clear GPIO interrupt on pin N
}

void PORT0_3_Handler(void)
{
  irq_triggered = 1;                            /*falling edge*/
  CMSDK_gpio_IntClear(CMSDK_GPIO0, 3);        //clear GPIO interrupt on pin N
}

void PORT0_COMB_Handler(void)   /* Combined handler */
{
  irq_triggered = 1;
  if (CMSDK_GPIO0->INTSTATUS & 0x01){ /* high level */
    CMSDK_GPIO0->DATAOUT = 0xA;                  /* Deassert Port 0 pin 0 to 0 */
    CMSDK_gpio_IntClear(CMSDK_GPIO0, 0);        //clear GPIO interrupt on pin N
    }
  if (CMSDK_GPIO0->INTSTATUS & 0x02){ /* low level*/
    CMSDK_GPIO0->DATAOUT = 0xB;                  /* Deassert Port 0 pin 1 to 1 */
    CMSDK_gpio_IntClear(CMSDK_GPIO0, 1);        //clear GPIO interrupt on pin N
    }
  if (CMSDK_GPIO0->INTSTATUS & 0x04){ /* rising edge*/
    CMSDK_gpio_IntClear(CMSDK_GPIO0, 2);         //clear GPIO interrupt on pin N
    }
  if (CMSDK_GPIO0->INTSTATUS & 0x08){ /* falling edge*/
    CMSDK_gpio_IntClear(CMSDK_GPIO0, 3);         //clear GPIO interrupt on pin N
    }
  return;
}

