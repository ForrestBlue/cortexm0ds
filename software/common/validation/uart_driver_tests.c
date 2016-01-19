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
  A simple test to check the functionalities of UART functions in the example device driver.
*/

#include "CMSDK_CM0.h"
#include <stdio.h>
#include "string.h"
#include "uart_stdout.h"
#include "CMSDK_driver.h"

/* variables shared between interrupt handlers and test program code */
int volatile uart_txorirq_executed = 0;  /* set to 1 if UARTOVF0_Handler (tx overflow) executed */
int volatile uart_txorirq_counter = 0;   /* number of time UARTOVF0_Handler executed */
int volatile uart_data_received = 0;     /* set to 1 if UARTRX1_Handler executed */
int volatile uart_data_sent = 0;         /* set to 1 if UARTTX0_Handler executed */
int volatile uart_rxorirq_executed = 0;  /* set to 1 if UARTOVF1_Handler (rx overflow) executed */
int volatile uart_rxorirq_counter = 0;   /* number of time UARTOVF1_Handler executed */

/* Test Functions declarations */
int Uart_Init(void);      /* Function to test UART initialization */
int Uart_Buffull(void);   /* Function to test UART buffer full */
int Uart_OR(void);        /* Function to test UART overrun */
int Uart_IRQ(void);       /* Function to test UART interrupt */
int uart0_id_check(void);  /* Detect UART 0 present */
int uart1_id_check(void);  /* Detect UART 1 present */
int gpio1_id_check(void);  /* Detect GPIO 1 present */

int main (void)
{
  int result = 0;

  // UART init
  UartStdOutInit();

  // Test banner message and revision number
  puts("\nCortex Microcontroller System Design Kit - UART Driver Test - revision $Revision: 242484 $\n");

  if ((uart0_id_check()!=0)||(uart1_id_check()!=0)||(gpio1_id_check()!=0)) {
    puts("** TEST SKIPPED ** UART 0 / UART 1 / GPIO 1 not available");
    UartEndSimulation();
    return 0;}


  result |= Uart_Init();
  result |= Uart_Buffull();
  result |= Uart_OR();
  result |= Uart_IRQ();

  if (result == 0) {
    puts("** TEST PASSED **\n");
  } else {
    printf("** TEST FAILED **, Error code: (0x%x)\n", result);
  }

  UartEndSimulation();
  return 0;
}


void UARTOVF0_Handler(void)                                   /*UART0 Overrun ISR*/
{
if(CMSDK_uart_GetOverrunStatus(CMSDK_UART0) == 1){
    uart_txorirq_executed = 1;                                /*set TX Overrun flag*/
    uart_txorirq_counter++;                                   /*increment TX Overrun counter*/
    CMSDK_uart_ClearOverrunStatus(CMSDK_UART0);             /*clear UART0 Overrun IRQ*/
  }
}

void UARTOVF1_Handler(void)                                    /*UART1 Overrun ISR*/
{
if(CMSDK_uart_GetOverrunStatus(CMSDK_UART1) == 2){
  uart_rxorirq_executed = 1;                                   /*set RX Overrun flag*/
  uart_rxorirq_counter++;                                      /*increment RX Overrun counter*/
  CMSDK_uart_ClearOverrunStatus(CMSDK_UART1);                /*clear UART1 Overrun IRQ*/
  }
}

void UARTTX0_Handler(void)                    /*UART0 TX ISR*/
{
  uart_data_sent = 1;                         /*set data sent flag*/
  CMSDK_uart_ClearTxIRQ(CMSDK_UART0);       /*clear UART0 TX IRQ*/
}

void UARTRX1_Handler(void)                    /*UART1 RX ISR*/
{
  uart_data_received = 1;                     /*set data received flag*/
  CMSDK_uart_ClearRxIRQ(CMSDK_UART1);       /*clear UART1 RX IRQ*/
}

/* Initialize UART and check return status */
int Uart_Init(void)
{
  int err_code = 0;

  puts("\nStage 1 UART Initialization\n");        //initialise UART0 and UART1 with Baud divider of 32
                                                  //and all interrupts enabled and also tx and rx enabled
  CMSDK_gpio_SetAltFunc(CMSDK_GPIO1, 0x000F);
  if(CMSDK_uart_init(CMSDK_UART0, 0x20, 1, 1, 1, 1, 1, 1) == 0)
    printf("UART0 Initialised Successfully (Baud Divider of: %d)\n", CMSDK_uart_GetBaudDivider(CMSDK_UART0));
    /* CMSDK_uart_init() returns 1 if the overflow status is non-zero */
  else
  {
    puts("UART0 Initialization Failed\n");
    err_code = 1;
  }
  if(CMSDK_uart_init(CMSDK_UART1, 0x20, 1, 1, 1, 1, 1, 1) == 0)
    printf("UART1 Initialised Successfully (Baud Divider of: %d)\n", CMSDK_uart_GetBaudDivider(CMSDK_UART1));
    /* CMSDK_uart_init() returns 1 if the overflow status is non-zero */
  else
  {
    puts("UART1 Initialization Failed\n");
    err_code |= 2;
  }

  if(!err_code) return 0;
  else return 1;
  }


int Uart_Buffull(void)             //function for testing the Buffer full functions and simple transmission
{
  int err_code = 0;
  int i, k;
  char received[12] = {0,0,0,0, 0,0,0,0, 0,0,0,0};
  char transmit[12] = "hello world";

  puts("\nStage 2 Simple Transmission - TX and RX Test\n");

  i = 0; /* transmit character counter */
  k = 0; /* receive character counter */

  while((CMSDK_uart_GetTxBufferFull(CMSDK_UART0) == 0)){    //while the TX buffer is not full send it data to transmit
    CMSDK_UART0->DATA = (uint32_t)transmit[i];
    i++;
  }

  if(CMSDK_uart_GetTxBufferFull(CMSDK_UART0)) puts("TX Buffer Full ...restarting transmission");
  else{
    err_code = (1 << 0);
    printf("** TEST FAILED **, Error Code: (0x%x)", err_code);
  }

  /*receive data from transmission and dispose of it*/

  if(CMSDK_uart_GetRxBufferFull(CMSDK_UART1) == 1) CMSDK_uart_ReceiveChar(CMSDK_UART1);
  else{
    err_code = (1 << 1);
    printf("** TEST FAILED **, Error Code: (0x%x)", err_code);
  }

  i = 0;

  while(k < 12){   //while received string is not the length of the original string

    if(CMSDK_uart_GetRxBufferFull(CMSDK_UART1) == 1){   //receive data from RX buffer when full
      received[k] = CMSDK_uart_ReceiveChar(CMSDK_UART1);
      printf("RX Buffer Full ...receiving data... %c\n", received[k]);
      k++;
    }

    /*Send data to TX buffer if the TX buffer is not
    full and the RX buffer of UART1 is also not full.
    The receive buffer status is checked because the
    printf statement in the receive polling takes
    long time so this code cannot handle maximum
    throughput */

    if((CMSDK_uart_GetTxBufferFull(CMSDK_UART0) == 0) && (CMSDK_uart_GetRxBufferFull(CMSDK_UART1) != 1)){
      if(i < 12){
        CMSDK_UART0->DATA = (uint32_t)transmit[i];
        i++;
      }
    }
  }
  printf("\nCharacters received: %s\n", received);    //when all characters received print the received string

  if(strcmp(received, transmit)){
    err_code = 4;
    puts("** TEST FAILED **, Error : Strings DO Not Match!");
  }

  if(!err_code) return 0;
  else return 2;
}

int Uart_OR(void)              /*function to test driver Overrun functions*/
{
  int i = 0, TX = 0, RX = 0, err_code = 0;
  char transmit[12] = "hello world";

  puts("\nStage 3 Polling");

  puts("\n- Stage 3a Overrun Polling\n");

  while(1)
  {
    if(i < 4) CMSDK_UART0->DATA = (uint32_t)'a';                     //if the loop iteration, the value of i, is less
                                                                      //that 4 then send the TX buffer data to cause a
    while(i > 10){                                                    //TX overrun if the loop iteration, i, is greater
      CMSDK_UART0->DATA = (uint32_t)'a';                             //than 10 then send the TX buffer to cause another
      if(CMSDK_uart_GetOverrunStatus(CMSDK_UART0) == 1) break;      //TX buffer overrun
    }

  if(CMSDK_uart_GetOverrunStatus(CMSDK_UART0) == 1){
    puts("TX Buffer Overrun Occurred");
    CMSDK_uart_ClearOverrunStatus(CMSDK_UART0);
    TX = 1;
  }else if(CMSDK_uart_GetOverrunStatus(CMSDK_UART1) == 2){
    puts("RX Buffer Overrun Occurred");                             //RX buffer overrun will occur as the data
    CMSDK_uart_ClearOverrunStatus(CMSDK_UART1);                   //is never read from the RX buffer
    RX = 1;
  }

  i++;
  if(RX & TX) break;
 }

  puts("\n- Stage 3b TX & RX IRQ Polling\n");

  //clear the TX IRQ status and then print the new status
  CMSDK_uart_ClearTxIRQ(CMSDK_UART0);
  printf("TX IRQ Status: %d\n", CMSDK_uart_GetTxIRQStatus(CMSDK_UART0));

  if(CMSDK_uart_GetTxIRQStatus(CMSDK_UART0)) err_code = (1 << 0);

  //clear the RX IRQ status and then print the new status
  CMSDK_uart_ClearRxIRQ(CMSDK_UART1);
  printf("RX IRQ Status: %d\n", CMSDK_uart_GetRxIRQStatus(CMSDK_UART1));

  if(CMSDK_uart_GetRxIRQStatus(CMSDK_UART1)) err_code = (1 << 1);

  CMSDK_uart_SendChar(CMSDK_UART0, transmit[1]);

  while(!CMSDK_uart_GetTxIRQStatus(CMSDK_UART0));
  printf("TX IRQ Status: %d\n", CMSDK_uart_GetTxIRQStatus(CMSDK_UART0));     //send data and wait until the TX IRQ status
  if(!CMSDK_uart_GetTxIRQStatus(CMSDK_UART0)) err_code = (1 << 2);           //is set and then print the new status
  else CMSDK_uart_ClearTxIRQ(CMSDK_UART0);


  while(!CMSDK_uart_GetRxIRQStatus(CMSDK_UART1));
  printf("RX IRQ Status: %d\n", CMSDK_uart_GetRxIRQStatus(CMSDK_UART1));     //send data and wait until the RX IRQ status
  if(!CMSDK_uart_GetRxIRQStatus(CMSDK_UART1)) err_code = (1 << 3);           //is set and then print the new status
  else CMSDK_uart_ClearRxIRQ(CMSDK_UART1);



  if(err_code){
    printf("** TEST FAILED **, Polling Test Error Code: (0x%x)", err_code);
  }
  else puts("Polling Test Passed");

  if(!err_code) return 0;
  else return 4;
}

int Uart_IRQ(void){

/*function to test the TX & RX overrun IRQ functions and the TX and RX IRQ driver
  functions using a simple interrupt orientated send and receive*/

  int i = 0, j = 0; /* i=transmit character counter, j = receive character counter */
  int err_code = 0;
  char received[12] = {0,0,0,0, 0,0,0,0, 0,0,0,0};
  char transmit[12] = "hello world";

  puts("\nStage 4 IRQ\n");
  puts("- Stage 4a Overrun IRQ\n");

  NVIC_EnableIRQ(UARTOVF0_IRQn);         //enable both UART0 and UART1 overflow IRQs
  NVIC_EnableIRQ(UARTOVF1_IRQn);

  while(uart_txorirq_counter <= 3)       //repeat until 3 TX OR IRQs have occurred
  {
    if(uart_txorirq_executed){
      puts("UART TX Overrun IRQ");       //if an TX OR IRQ is performed then this variable is set,
      uart_txorirq_executed = 0;         //uart_txorirq_executed, and this statement will be printed
    }
    CMSDK_UART0->DATA = (uint32_t)'a';  //always send data to the TX buffer to cause TX OR and do not
    if(uart_rxorirq_executed){           //receive data to cause RX OR
      puts("UART RX Overrun IRQ");
      uart_rxorirq_executed = 0;
    }
  }

  if(uart_rxorirq_counter < 3){
    err_code = (1 << 0);
    printf("** TEST FAILED ** UART RX Overrun Error, Error Code: (0x%x)", err_code);
  }
  else puts("UART RX Overrun Passed");

  j = 0;
  uart_data_received = 1;  //set uart_data_received to one so that the first character is sent

  puts("\n- Stage 4b TX/RX IRQ\n");

  /*- Send a character from the transmit variable
    - When its received by UART1 transfer it from RX buffer to the received variable
    - set flag to say it's been received
    - when received flag has been set send the next character from transmit variable
    - repeat until all characters have been received*/

  NVIC_EnableIRQ(UARTTX0_IRQn);   //enable both UART0 TX and UART1 RX IRQs
  NVIC_EnableIRQ(UARTRX1_IRQn);

  while(j < 11)   /*while j, the received character counter, is less than 11, the number of characters to be sent*/
  { /* uart_data_received and uart_data_sent are updated by TX and RX handlers */
    if(uart_data_received){
      puts("UART TX IRQ ....data sent");                    //if the data has been received (which is set in the
      CMSDK_uart_SendChar(CMSDK_UART0, transmit[i]);      //RX IRQ) then send the character corresponding to
      i++;                                                  //the character counter, i, increment character counter
      uart_data_received = 0;
    }
    if(uart_data_sent){                                     //if the data has been set (which is set in the
      printf("UART RX IRQ ....data received.... ");         //TX IRQ) then receive the character corresponding to
      received[j] = CMSDK_uart_ReceiveChar(CMSDK_UART1);  //the character counter, j, increment character counter
      printf("%c\n", received[j]);
      j++;
      uart_data_sent = 0;
    }
  }

  printf("\nString received: %s\n\n", received);

  if(strcmp(received, transmit)){
    err_code = 1;
    puts("** TEST FAILED ** Strings Do Not Match!");
  }

  NVIC_DisableIRQ(UARTOVF0_IRQn);
  NVIC_DisableIRQ(UARTOVF1_IRQn);       //disable all the enabled IRQs
  NVIC_DisableIRQ(UARTTX0_IRQn);
  NVIC_DisableIRQ(UARTRX1_IRQn);

  if(!err_code) return 0;
  else return 8;
}

// ----------------------------------------------------------
// Peripheral detection
// ----------------------------------------------------------
/* Detect the part number to see if device is present                */
#define HW32_REG(ADDRESS)  (*((volatile unsigned long  *)(ADDRESS)))

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

int gpio1_id_check(void)
{
if ((HW32_REG(CMSDK_GPIO1_BASE + 0xFE0) != 0x20) ||
    (HW32_REG(CMSDK_GPIO1_BASE + 0xFE4) != 0xB8))
  return 1; /* part ID does not match */
else
  return 0;
}
