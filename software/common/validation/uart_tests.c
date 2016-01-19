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

// A simple test to check the functionalities of the APB UART


#include "CMSDK_CM0.h"
#include <stdio.h>
#include <string.h>
#include "uart_stdout.h"

#define UART_STATE_TXFULL CMSDK_UART_STATE_TXBF_Msk
#define UART_STATE_RXFULL CMSDK_UART_STATE_RXBF_Msk
#define UART_STATE_TXOVR  CMSDK_UART_STATE_TXOR_Msk
#define UART_STATE_RXOVR  CMSDK_UART_STATE_RXOR_Msk

#define UART_CTRL_TXEN         CMSDK_UART_CTRL_TXEN_Msk
#define UART_CTRL_RXEN         CMSDK_UART_CTRL_RXEN_Msk
#define UART_CTRL_TXIRQEN      CMSDK_UART_CTRL_TXIRQEN_Msk
#define UART_CTRL_RXIRQEN      CMSDK_UART_CTRL_RXIRQEN_Msk
#define UART_CTRL_TXOVRIRQEN   CMSDK_UART_CTRL_TXORIRQEN_Msk
#define UART_CTRL_RXOVRIRQEN   CMSDK_UART_CTRL_RXORIRQEN_Msk
#define UART_CTRL_HIGHSPEEDTX  CMSDK_UART_CTRL_HSTM_Msk

#define UART_INTSTATE_TX 1
#define UART_INTSTATE_RX 2
#define UART_INTSTATE_TXOVR 4
#define UART_INTSTATE_RXOVR 8

#define BAUDDIV_MASK 0x000FFFFF

#define DISPLAY 1
#define NO_DISPLAY 0

/* peripheral and component ID values */
#define APB_UART_PID4  0x04
#define APB_UART_PID5  0x00
#define APB_UART_PID6  0x00
#define APB_UART_PID7  0x00
#define APB_UART_PID0  0x21
#define APB_UART_PID1  0xB8
#define APB_UART_PID2  0x1B
#define APB_UART_PID3  0x00
#define APB_UART_CID0  0x0D
#define APB_UART_CID1  0xF0
#define APB_UART_CID2  0x05
#define APB_UART_CID3  0xB1
#define HW32_REG(ADDRESS)  (*((volatile unsigned long  *)(ADDRESS)))

/* Global variables */
volatile int uart0_irq_occurred;
volatile int uart1_irq_occurred;
volatile int uart2_irq_occurred;
volatile int uart0_irq_expected;
volatile int uart1_irq_expected;
volatile int uart2_irq_expected;

/* Function definitions */
void UartIOConfig(void);
int  uart_initial_value_check(CMSDK_UART_TypeDef *CMSDK_UART);
int  simple_uart_test(CMSDK_UART_TypeDef *CMSDK_UART, unsigned int bauddiv, int verbose);
int  simple_uart_baud_test(void);
int  simple_uart_baud_test_single(CMSDK_UART_TypeDef *CMSDK_UART,
                                 unsigned int tx_bauddiv,unsigned int rx_bauddiv,int verbose);
int  uart_enable_ctrl_test(CMSDK_UART_TypeDef *CMSDK_UART);
int  uart_tx_rx_irq_test(CMSDK_UART_TypeDef *CMSDK_UART);
int  uart_tx_rx_overflow_test(CMSDK_UART_TypeDef *CMSDK_UART);
void delay_for_character(void);
int  uart2_interrupt_test(void);
int  uart0_id_check(void);  /* Detect UART 0 present */
int  uart1_id_check(void);  /* Detect UART 1 present */
int  gpio1_id_check(void);  /* Detect GPIO 1 present */

int main (void)
{
  int result=0;

  // UART init
  UartStdOutInit();

  // Test banner message and revision number
  puts("\nCortex Microcontroller System Design Kit - UART Test - revision $Revision: 243483 $\n");

  if ((uart0_id_check()!=0)||(uart1_id_check()!=0)||(gpio1_id_check()!=0)) {
    puts("** TEST SKIPPED ** UART 0 / UART 1 / GPIO 1 not available");
    UartEndSimulation();
    return 0;}

  uart0_irq_occurred = 0;
  uart1_irq_occurred = 0;
  uart0_irq_expected = 0;
  uart1_irq_expected = 0;

  uart2_irq_occurred = 0;
  uart2_irq_expected = 0;

  UartIOConfig();

  result += uart_initial_value_check(CMSDK_UART0);
  result += uart_initial_value_check(CMSDK_UART1);

  puts("\nUART 0 for transmit, UART 1 for receive\n");

  result += simple_uart_test(CMSDK_UART0, 32, DISPLAY);
  result += simple_uart_baud_test();
  result += uart_enable_ctrl_test(CMSDK_UART0);
  result += uart_tx_rx_irq_test(CMSDK_UART0);
  result += uart_tx_rx_overflow_test(CMSDK_UART0);

  puts("\nUART 1 for transmit, UART 0 for receive\n");

  result += simple_uart_test(CMSDK_UART1, 16, DISPLAY);
  result += uart_enable_ctrl_test(CMSDK_UART1);
  result += uart_tx_rx_irq_test(CMSDK_UART1);
  result += uart_tx_rx_overflow_test(CMSDK_UART1);

  puts("\nUART 2 interrupt connectivity test\n");
  result += uart2_interrupt_test();

  if (result==0) {
    printf ("\n** TEST PASSED **\n");
  } else {
    printf ("\n** TEST FAILED ** , Error code = (0x%x)\n", result);
  }
  UartEndSimulation();
  return 0;
}

void UartIOConfig(void)
{ /* UART0 and UART1 are arranged in cross over configuration. */
  /* Enable UART TXD functions for these pins */
  CMSDK_GPIO1->ALTFUNCSET = (1<<1) | (1<<3);
  return;
}

/* --------------------------------------------------------------- */
/*  UART initial value tests                                       */
/* --------------------------------------------------------------- */
int uart_initial_value_check(CMSDK_UART_TypeDef *CMSDK_UART){
  int return_val=0;
  int err_code=0;
  unsigned int uart_base;
  unsigned int i;

  puts("- check initial values");
  if (CMSDK_UART->DATA     !=0) {err_code += (1<<0);}
  if (CMSDK_UART->STATE    !=0) {err_code += (1<<1);}
  if (CMSDK_UART->CTRL     !=0) {err_code += (1<<2);}
  if (CMSDK_UART->INTSTATUS!=0) {err_code += (1<<3);}
  if (CMSDK_UART->BAUDDIV  !=0) {err_code += (1<<4);}

  uart_base = CMSDK_UART0_BASE;
  if (CMSDK_UART==CMSDK_UART1) {uart_base = CMSDK_UART1_BASE;}
  if (CMSDK_UART==CMSDK_UART2) {uart_base = CMSDK_UART2_BASE;}

  if (HW32_REG(uart_base + 0xFD0) != APB_UART_PID4) {err_code += (1<<5); }
  if (HW32_REG(uart_base + 0xFD4) != APB_UART_PID5) {err_code += (1<<6); }
  if (HW32_REG(uart_base + 0xFD8) != APB_UART_PID6) {err_code += (1<<7); }
  if (HW32_REG(uart_base + 0xFDC) != APB_UART_PID7) {err_code += (1<<8); }
  if (HW32_REG(uart_base + 0xFE0) != APB_UART_PID0) {err_code += (1<<9); }
  if (HW32_REG(uart_base + 0xFE4) != APB_UART_PID1) {err_code += (1<<10); }
  if (HW32_REG(uart_base + 0xFE8) != APB_UART_PID2) {err_code += (1<<11); }
  if (HW32_REG(uart_base + 0xFEC) != APB_UART_PID3) {err_code += (1<<12); }
  if (HW32_REG(uart_base + 0xFF0) != APB_UART_CID0) {err_code += (1<<13); }
  if (HW32_REG(uart_base + 0xFF4) != APB_UART_CID1) {err_code += (1<<14); }
  if (HW32_REG(uart_base + 0xFF8) != APB_UART_CID2) {err_code += (1<<15); }
  if (HW32_REG(uart_base + 0xFFC) != APB_UART_CID3) {err_code += (1<<16); }

  /* test write to PIDs and CIDs - should be ignored */
  for (i=0; i <12; i++) {
    HW32_REG(uart_base + 0xFD0 + (i<<2)) = ~HW32_REG(uart_base + 0xFD0 + (i<<2));
    }

  /* Check read back values again, should not be changed */
  if (HW32_REG(uart_base + 0xFD0) != APB_UART_PID4) {err_code |= (1<<5); }
  if (HW32_REG(uart_base + 0xFD4) != APB_UART_PID5) {err_code |= (1<<6); }
  if (HW32_REG(uart_base + 0xFD8) != APB_UART_PID6) {err_code |= (1<<7); }
  if (HW32_REG(uart_base + 0xFDC) != APB_UART_PID7) {err_code |= (1<<8); }
  if (HW32_REG(uart_base + 0xFE0) != APB_UART_PID0) {err_code |= (1<<9); }
  if (HW32_REG(uart_base + 0xFE4) != APB_UART_PID1) {err_code |= (1<<10); }
  if (HW32_REG(uart_base + 0xFE8) != APB_UART_PID2) {err_code |= (1<<11); }
  if (HW32_REG(uart_base + 0xFEC) != APB_UART_PID3) {err_code |= (1<<12); }
  if (HW32_REG(uart_base + 0xFF0) != APB_UART_CID0) {err_code |= (1<<13); }
  if (HW32_REG(uart_base + 0xFF4) != APB_UART_CID1) {err_code |= (1<<14); }
  if (HW32_REG(uart_base + 0xFF8) != APB_UART_CID2) {err_code |= (1<<15); }
  if (HW32_REG(uart_base + 0xFFC) != APB_UART_CID3) {err_code |= (1<<16); }

  if (err_code != 0) {
    printf ("ERROR : initial value failed (0x%x)\n", err_code);
    return_val =1;
    err_code = 0;
    }

  return(return_val);
}

/* --------------------------------------------------------------- */
/*  UART simple operation test                                     */
/* --------------------------------------------------------------- */
int simple_uart_test(CMSDK_UART_TypeDef *CMSDK_UART, unsigned int bauddiv, int verbose)
{
  int return_val=0;
  int err_code=0;
  CMSDK_UART_TypeDef *TX_UART;
  CMSDK_UART_TypeDef *RX_UART;
  char        received_text[20];
  const char  transmit_text[20] = "Hello world\n";
  unsigned int tx_count;
  unsigned int rx_count;
  unsigned int str_size;

  puts("Simple test");
  UartPutc('-');
  UartPutc(' ');

  /* Determine which UART is the sender, and which UART is receiver */
  if (CMSDK_UART==CMSDK_UART0){
    TX_UART = CMSDK_UART0;
    RX_UART = CMSDK_UART1;
    }
  else if (CMSDK_UART==CMSDK_UART1){
    TX_UART = CMSDK_UART1;
    RX_UART = CMSDK_UART0;
    }
  else {
    puts ("ERROR: Input parameter invalid in function 'simple_uart_test'.");
    return 1;
    }

  /* Both UART are programmed with the same baud rate */
  TX_UART->BAUDDIV = bauddiv;
  if (TX_UART->BAUDDIV != bauddiv) { err_code += (1<<0);}
  RX_UART->BAUDDIV = bauddiv;
  if (RX_UART->BAUDDIV != bauddiv) { err_code += (1<<1);}

  TX_UART->CTRL =   TX_UART->CTRL | UART_CTRL_TXEN; /* Set TX enable */
  if ((TX_UART->CTRL & UART_CTRL_TXEN)==0) { err_code += (1<<2);}
  RX_UART->CTRL =   RX_UART->CTRL | UART_CTRL_RXEN; /* Set RX enable */
  if ((RX_UART->CTRL & UART_CTRL_RXEN)==0) { err_code += (1<<3);}

  tx_count = 0;
  rx_count = 0;
  str_size = strlen(transmit_text);
  do { /* test loop for both tx and rx process */
    /* tx process */
    if (((TX_UART->STATE & UART_STATE_TXFULL)==0)&&(tx_count<str_size)) {
      TX_UART->DATA = transmit_text[tx_count];
      tx_count++;
      }
    /* rx process */
    if ((RX_UART->STATE & UART_STATE_RXFULL)!=0) {
      received_text[rx_count] = RX_UART->DATA;
      if (verbose) UartPutc((char) received_text[rx_count]);
      rx_count++;
      }
  } while ( rx_count <str_size);
  received_text[rx_count]=0; /* add NULL termination */

  /* Added 3 additional null chars to overcome X-termination in test
     when reads back X's beyond null char since a load 32-bit word
     happens rather than a byte access. */
  received_text[rx_count+1]=0; /* add NULL termination */
  received_text[rx_count+2]=0; /* add NULL termination */
  received_text[rx_count+3]=0; /* add NULL termination */
  if (strcmp(transmit_text, received_text)!=0){ err_code += (1<<4);}

  TX_UART->CTRL =  0; /* Clear TX enable */
  RX_UART->CTRL =  0; /* Clear RX enable */

  if (err_code != 0) {
    printf ("ERROR : simple test failed (0x%x)\n", err_code);
    return_val =1;
    err_code = 0;
    }

  return(return_val);
}
/* --------------------------------------------------------------- */
/*  UART baud rate operation test                                  */
/* --------------------------------------------------------------- */
int simple_uart_baud_test(void)
{
  int return_val=0;
  int err_code=0;
  int i;
  short int tx_bauddiv[10] = {
  63, 64, 35, 38, 40, 46, 85, 49, 51, 37};
  short int rx_bauddiv[10] = {
  63, 64, 35, 38, 40, 46, 85, 49, 51, 37};

  puts("Data transfer test\n");

  for (i=0; i<10; i++) {
    /* Test TX and RX at same speed */
    if (simple_uart_baud_test_single(CMSDK_UART0,
      tx_bauddiv[i],  rx_bauddiv[i]   , NO_DISPLAY)!=0) {err_code |= 0x1;};
    /* Test RX slower than TX */
    if (simple_uart_baud_test_single(CMSDK_UART0,
      tx_bauddiv[i], (rx_bauddiv[i]+1), NO_DISPLAY)!=0) {err_code |= 0x2;};
    /* Test RX faster than TX */
    if (simple_uart_baud_test_single(CMSDK_UART0,
      tx_bauddiv[i], (rx_bauddiv[i]-1), NO_DISPLAY)!=0) {err_code |= 0x4;};
    if (err_code != 0) {
      printf ("ERROR : Baud rate test failed (0x%x) at loop %d\n", err_code, i);
      return_val = 1;
      err_code   = 0;
      }
    else {
      printf ("- bauddiv = %d done\n", tx_bauddiv[i]);
      }
    }
  CMSDK_UART0->CTRL = 0;
  CMSDK_UART1->CTRL = 0;
  CMSDK_UART0->BAUDDIV = 0xFFFFFFFF;
  if (CMSDK_UART0->BAUDDIV != (0xFFFFFFFF & BAUDDIV_MASK)) {err_code |= (1<<0);};
  CMSDK_UART0->BAUDDIV = 0xFF55AAC3;
  if (CMSDK_UART0->BAUDDIV != (0xFF55AAC3 & BAUDDIV_MASK)) {err_code |= (1<<1);};
  CMSDK_UART0->BAUDDIV = 0x00000000;
  if (CMSDK_UART0->BAUDDIV != (0x00000000 & BAUDDIV_MASK)) {err_code |= (1<<2);};
  CMSDK_UART1->BAUDDIV = 0xFFFFFFFF;
  if (CMSDK_UART1->BAUDDIV != (0xFFFFFFFF & BAUDDIV_MASK)) {err_code |= (1<<3);};
  CMSDK_UART1->BAUDDIV = 0xAAFF6699;
  if (CMSDK_UART1->BAUDDIV != (0xAAFF6699 & BAUDDIV_MASK)) {err_code |= (1<<4);};
  CMSDK_UART1->BAUDDIV = 0x00000000;
  if (CMSDK_UART1->BAUDDIV != (0x00000000 & BAUDDIV_MASK)) {err_code |= (1<<5);};
    if (err_code != 0) {
      printf ("ERROR : Baud rate r/w failed (0x%x)\n", err_code);
      return_val = 1;
      err_code   = 0;
      }


  return(return_val);

}
/* --------------------- */
int simple_uart_baud_test_single(CMSDK_UART_TypeDef *CMSDK_UART,
                     unsigned int tx_bauddiv,
                     unsigned int rx_bauddiv,
                     int verbose)
{
  int return_val=0;
  int err_code=0;
  CMSDK_UART_TypeDef *TX_UART;
  CMSDK_UART_TypeDef *RX_UART;
  char        received_text[20];
  const char  transmit_text[20] = "Hello world\n";
  unsigned int tx_count;
  unsigned int rx_count;
  unsigned int str_size;

  /* Determine which UART is the sender, and which UART is receiver */
  if (CMSDK_UART==CMSDK_UART0){
    TX_UART = CMSDK_UART0;
    RX_UART = CMSDK_UART1;
    }
  else if (CMSDK_UART==CMSDK_UART1){
    TX_UART = CMSDK_UART1;
    RX_UART = CMSDK_UART0;
    }
  else {
    puts ("ERROR: Input parameter invalid in function 'simple_uart_baud_test_single'.");
    return 1;
    }

  /* UART can be programmed with different baud rate */
  TX_UART->BAUDDIV = tx_bauddiv;
  if (TX_UART->BAUDDIV != tx_bauddiv) { err_code += (1<<0);}
  RX_UART->BAUDDIV = rx_bauddiv;
  if (RX_UART->BAUDDIV != rx_bauddiv) { err_code += (1<<1);}

  TX_UART->CTRL =   TX_UART->CTRL | UART_CTRL_TXEN; /* Set TX enable */
  if ((TX_UART->CTRL & UART_CTRL_TXEN)==0) { err_code += (1<<2);}
  RX_UART->CTRL =   RX_UART->CTRL | UART_CTRL_RXEN; /* Set RX enable */
  if ((RX_UART->CTRL & UART_CTRL_RXEN)==0) { err_code += (1<<3);}

  tx_count = 0;
  rx_count = 0;
  str_size = strlen(transmit_text);
  do { /* test loop for both tx and rx process */
    /* tx process */
    if (((TX_UART->STATE & UART_STATE_TXFULL)==0)&&(tx_count<str_size)) {
      TX_UART->DATA = transmit_text[tx_count];
      tx_count++;
      }
    /* rx process */
    if ((RX_UART->STATE & UART_STATE_RXFULL)!=0) {
      received_text[rx_count] = RX_UART->DATA;
      if (verbose) UartPutc((char) received_text[rx_count]);
      rx_count++;
      }
  } while ( rx_count <str_size);
  received_text[rx_count]=0; /* add NULL termination */

  /* Added 3 additional null chars to overcome X-termination in test
     when reads back X's beyond null char since a load 32-bit word
     happens rather than a byte access. */
  received_text[rx_count+1]=0; /* add NULL termination */
  received_text[rx_count+2]=0; /* add NULL termination */
  received_text[rx_count+3]=0; /* add NULL termination */
  if (strcmp(transmit_text, received_text)!=0){ err_code += (1<<4);}

  TX_UART->CTRL =  0; /* Clear TX enable */
  RX_UART->CTRL =  0; /* Clear RX enable */

  if (err_code != 0) {
    printf ("ERROR : baud test failed (0x%x)\n", err_code);
    return_val =1;
    err_code = 0;
    }

  return(return_val);
}
/* --------------------------------------------------------------- */
/*  UART enable control test                                       */
/* --------------------------------------------------------------- */
int uart_enable_ctrl_test(CMSDK_UART_TypeDef *CMSDK_UART)
{
  int return_val=0;
  int err_code=0;
  CMSDK_UART_TypeDef *TX_UART;
  CMSDK_UART_TypeDef *RX_UART;
  char ctmp;

  /* Determine which UART is the sender, and which UART is receiver */
  if (CMSDK_UART==CMSDK_UART0){
    TX_UART = CMSDK_UART0;
    RX_UART = CMSDK_UART1;
    }
  else if (CMSDK_UART==CMSDK_UART1){
    TX_UART = CMSDK_UART1;
    RX_UART = CMSDK_UART0;
    }
  else {
    puts ("ERROR: Input parameter invalid in function 'uart_enable_ctrl_test'.");
    return 1;
    }

  puts ("UART enable test");
  /* UART programmed with same baud rate */
  TX_UART->BAUDDIV = 32;
  if (TX_UART->BAUDDIV != 32) { err_code += (1<<0);}
  RX_UART->BAUDDIV = 32;
  if (RX_UART->BAUDDIV != 32) { err_code += (1<<1);}

  puts ("- both TX and RX are enabled");
  TX_UART->CTRL =   TX_UART->CTRL | UART_CTRL_TXEN; /* Set TX enable */
  if ((TX_UART->CTRL & UART_CTRL_TXEN)==0) { err_code += (1<<2);}
  RX_UART->CTRL =   RX_UART->CTRL | UART_CTRL_RXEN; /* Set RX enable */
  if ((RX_UART->CTRL & UART_CTRL_RXEN)==0) { err_code += (1<<3);}

  if (((TX_UART->STATE & UART_STATE_TXFULL)!=0) ||
      ((RX_UART->STATE & UART_STATE_RXFULL)!=0)) {
      /* Starting state incorrect */
      err_code += (1<<4);}
  TX_UART->DATA = 'A'; /* transmit a character */
  delay_for_character();
  if (((TX_UART->STATE & UART_STATE_TXFULL)!=0) ||
      ((RX_UART->STATE & UART_STATE_RXFULL)==0)) {
      /* complete state incorrect */
      err_code += (1<<5);}
  ctmp = RX_UART->DATA; /* Read received data */
  if ((RX_UART->STATE & UART_STATE_RXFULL)!=0) {
      /* receive buffer should be empty now */
      err_code += (1<<6);}
  if (  ctmp != 'A') { /* received data incorrect */
      err_code += (1<<7);}

  puts ("- TX disabled");
  TX_UART->CTRL =   TX_UART->CTRL & ~UART_CTRL_TXEN; /* Clear TX enable */
  if ((TX_UART->CTRL & UART_CTRL_TXEN)!=0) { err_code += (1<<8);}

  if (((TX_UART->STATE & UART_STATE_TXFULL)!=0) ||
      ((RX_UART->STATE & UART_STATE_RXFULL)!=0)) {
      /* Starting state incorrect */
      err_code += (1<<9);}
  TX_UART->DATA = 'B'; /* transmit a character */

  /* When TX enable is low and a data is written to transmit buffer, the
     data would be lost */
  delay_for_character();
  if ((RX_UART->STATE & UART_STATE_RXFULL)!=0)  {
      /* RX buffer should still be empty*/
      err_code += (1<<10);}
  TX_UART->CTRL =   TX_UART->CTRL | UART_CTRL_TXEN; /* Set TX enable */
  delay_for_character();

  if (((TX_UART->STATE & UART_STATE_TXFULL)!=0) ||
      ((RX_UART->STATE & UART_STATE_RXFULL)!=0)) {
      /* complete state incorrect */
      err_code += (1<<11);}

  puts ("- RX disabled");
  RX_UART->CTRL =   RX_UART->CTRL & ~UART_CTRL_RXEN; /* Clear RX enable */
  if ((RX_UART->CTRL & UART_CTRL_RXEN)!=0) { err_code += (1<<12);}

  TX_UART->DATA = 'C'; /* transmit a character */
  delay_for_character();
  if (((TX_UART->STATE & UART_STATE_TXFULL)!=0) ||
      ((RX_UART->STATE & UART_STATE_RXFULL)!=0)) {
      /* No data should be received. complete state incorrect */
      err_code += (1<<13);}
  RX_UART->CTRL =   RX_UART->CTRL | UART_CTRL_RXEN; /* Set RX enable */
  delay_for_character();
  if (((TX_UART->STATE & UART_STATE_TXFULL)!=0) ||
      ((RX_UART->STATE & UART_STATE_RXFULL)!=0)) {
      /* No data should be received. complete state incorrect */
      err_code += (1<<14);}

  TX_UART->CTRL = 0;
  RX_UART->CTRL = 0;
  while ((RX_UART->STATE & UART_STATE_RXFULL)!=0) {
    ctmp=RX_UART->DATA;
    }

  if (err_code != 0) {
    printf ("ERROR : uart enable failed (0x%x)\n", err_code);
    return_val =1;
    err_code = 0;
    }

  return(return_val);
}
/* --------------------------------------------------------------- */
/*  UART tx & rx interrupt test                                         */
/* --------------------------------------------------------------- */

int uart_tx_rx_irq_test(CMSDK_UART_TypeDef *CMSDK_UART)
{
  int return_val=0;
  unsigned int err_code=0;
  CMSDK_UART_TypeDef *TX_UART;
  CMSDK_UART_TypeDef *RX_UART;
  char ctmp;

  /* Determine which UART is the sender, and which UART is receiver */
  if (CMSDK_UART==CMSDK_UART0){
    TX_UART = CMSDK_UART0;
    RX_UART = CMSDK_UART1;
    }
  else if (CMSDK_UART==CMSDK_UART1){
    TX_UART = CMSDK_UART1;
    RX_UART = CMSDK_UART0;
    }
  else {
    puts ("ERROR: Input parameter invalid in function 'uart_tx_rx_irq_test'.");
    return 1;
    }

  puts ("UART TX & RX IRQ test");
  /* UART programmed with same baud rate */
  TX_UART->BAUDDIV = 33;
  if (TX_UART->BAUDDIV != 33) { err_code += (1<<0);}
  RX_UART->BAUDDIV = 33;
  if (RX_UART->BAUDDIV != 33) { err_code += (1<<1);}

  puts ("- TX irq enable");
  if (CMSDK_UART==CMSDK_UART0){
    uart0_irq_expected=1;
    uart1_irq_expected=0;
    uart0_irq_occurred=0;
    uart1_irq_occurred=0;
    NVIC_EnableIRQ(UARTTX0_IRQn);
    }
  if (CMSDK_UART==CMSDK_UART1){
    uart0_irq_expected=0;
    uart1_irq_expected=1;
    uart0_irq_occurred=0;
    uart1_irq_occurred=0;
    NVIC_EnableIRQ(UARTTX1_IRQn);
    }

  TX_UART->CTRL = UART_CTRL_TXEN | UART_CTRL_TXIRQEN;
  RX_UART->CTRL = UART_CTRL_RXEN;

  if (((TX_UART->STATE & UART_STATE_TXFULL)!=0) |
      ((RX_UART->STATE & UART_STATE_RXFULL)!=0)) {
      /* Starting state incorrect */
      err_code += (1<<2);}
  TX_UART->DATA = 'A'; /* transmit a character */
  delay_for_character();

  if (CMSDK_UART==CMSDK_UART0){
    if (uart0_irq_occurred==0){ err_code += (1<<3);}
    if (uart1_irq_occurred!=0){ err_code += (1<<4);}
    }
  if (CMSDK_UART==CMSDK_UART1){
    if (uart1_irq_occurred==0){ err_code += (1<<3);}
    if (uart0_irq_occurred!=0){ err_code += (1<<4);}
    }
  /* Interrupt status should have been cleared */
  if (TX_UART->INTSTATUS != 0) { err_code += (1<<5);}
  if (RX_UART->INTSTATUS != 0) { err_code += (1<<6);}

  /* Receive buffer should have been full */
  if ((RX_UART->STATE & UART_STATE_RXFULL) == 0) { err_code += (1<<7);}
  ctmp = RX_UART->DATA;
  if (ctmp!='A')                                 { err_code += (1<<8);}
  if ((RX_UART->STATE & UART_STATE_RXFULL) != 0) { err_code += (1<<9);}

  puts ("- TX irq disable");
  if (CMSDK_UART==CMSDK_UART0){
    uart0_irq_expected=0;
    uart1_irq_expected=0;
    uart0_irq_occurred=0;
    uart1_irq_occurred=0;
    NVIC_EnableIRQ(UARTTX0_IRQn);
    }
  if (CMSDK_UART==CMSDK_UART1){
    uart0_irq_expected=0;
    uart1_irq_expected=0;
    uart0_irq_occurred=0;
    uart1_irq_occurred=0;
    NVIC_EnableIRQ(UARTTX1_IRQn);
    }

  TX_UART->CTRL = UART_CTRL_TXEN;  /* No interrupt generation */
  RX_UART->CTRL = UART_CTRL_RXEN;  /* No interrupt generation */

  if (((TX_UART->STATE & UART_STATE_TXFULL)!=0) ||
      ((RX_UART->STATE & UART_STATE_RXFULL)!=0)) {
      /* Starting state incorrect */
      err_code += (1<<10);}
  TX_UART->DATA = 'B'; /* transmit a character */
  delay_for_character();

  if (uart0_irq_occurred!=0){ err_code += (1<<11);}
  if (uart1_irq_occurred!=0){ err_code += (1<<12);}

  /* Receive buffer should have been full */
  if ((RX_UART->STATE & UART_STATE_RXFULL) == 0) { err_code += (1<<13);}
  ctmp = RX_UART->DATA;
  if (ctmp!='B')                                 { err_code += (1<<14);}
  if ((RX_UART->STATE & UART_STATE_RXFULL) != 0) { err_code += (1<<15);}

  /* Interrupt status should have been cleared */
  if (TX_UART->INTSTATUS != 0) { err_code += (1<<16);}
  if (RX_UART->INTSTATUS != 0) { err_code += (1<<17);}

  puts ("- RX irq enable");
  if (CMSDK_UART==CMSDK_UART0){
    uart0_irq_expected=0;
    uart1_irq_expected=1;
    uart0_irq_occurred=0;
    uart1_irq_occurred=0;
    NVIC_EnableIRQ(UARTRX1_IRQn);
    }
  if (CMSDK_UART==CMSDK_UART1){
    uart0_irq_expected=1;
    uart1_irq_expected=0;
    uart0_irq_occurred=0;
    uart1_irq_occurred=0;
    NVIC_EnableIRQ(UARTRX0_IRQn);
    }

  TX_UART->CTRL = UART_CTRL_TXEN ;  /* No interrupt generation */
  RX_UART->CTRL = UART_CTRL_RXEN | UART_CTRL_RXIRQEN;

  TX_UART->DATA = 'C'; /* transmit a character */
  delay_for_character();

  if (CMSDK_UART==CMSDK_UART0){
    if (uart0_irq_occurred!=0){ err_code += (1<<18);}
    if (uart1_irq_occurred==0){ err_code += (1<<19);}
    }
  if (CMSDK_UART==CMSDK_UART1){
    if (uart1_irq_occurred!=0){ err_code += (1<<18);}
    if (uart0_irq_occurred==0){ err_code += (1<<19);}
    }
  /* Interrupt status should have been cleared */
  if (TX_UART->INTSTATUS != 0) { err_code += (1<<20);}
  if (RX_UART->INTSTATUS != 0) { err_code += (1<<21);}

  /* Receive buffer should have been full */
  if ((RX_UART->STATE & UART_STATE_RXFULL) == 0) { err_code += (1<<22);}
  ctmp = RX_UART->DATA;
  if (ctmp!='C')                                 { err_code += (1<<23);}
  if ((RX_UART->STATE & UART_STATE_RXFULL) != 0) { err_code += (1<<24);}

  puts ("- RX irq disable");
  uart0_irq_expected=0;
  uart1_irq_expected=0;
  uart0_irq_occurred=0;
  uart1_irq_occurred=0;

  TX_UART->CTRL = UART_CTRL_TXEN;  /* No interrupt generation */
  RX_UART->CTRL = UART_CTRL_RXEN;  /* No interrupt generation */

  if (((TX_UART->STATE & UART_STATE_TXFULL)!=0) ||
      ((RX_UART->STATE & UART_STATE_RXFULL)!=0)) {
      /* Starting state incorrect */
      err_code += (1<<25);}
  TX_UART->DATA = 'D'; /* transmit a character */
  delay_for_character();

  if (uart0_irq_occurred!=0){ err_code += (1<<26);}
  if (uart1_irq_occurred!=0){ err_code += (1<<27);}

  /* Receive buffer should have been full */
  if ((RX_UART->STATE & UART_STATE_RXFULL) == 0) { err_code += (1<<28);}
  ctmp = RX_UART->DATA;
  if (ctmp!='D')                                 { err_code += (1<<29);}
  if ((RX_UART->STATE & UART_STATE_RXFULL) != 0) { err_code += (1<<30);}

  /* Interrupt status should have been cleared */
  if ((TX_UART->INTSTATUS != 0)||(RX_UART->INTSTATUS != 0)) { err_code |= 0x80000000UL;}

  /* clean up */

  TX_UART->CTRL = 0;
  RX_UART->CTRL = 0;
  while ((RX_UART->STATE & UART_STATE_RXFULL)!=0) {
    ctmp=RX_UART->DATA;
    }
  NVIC_DisableIRQ(UARTRX0_IRQn);
  NVIC_DisableIRQ(UARTRX1_IRQn);
  NVIC_DisableIRQ(UARTTX0_IRQn);
  NVIC_DisableIRQ(UARTTX1_IRQn);

  if (err_code != 0) {
    printf ("ERROR : uart interrupt enable failed (0x%x)\n", err_code);
    return_val =1;
    err_code = 0;
    }

  return(return_val);
}

/* --------------------------------------------------------------- */
/*  UART tx & rx overflow test                                     */
/* --------------------------------------------------------------- */

int uart_tx_rx_overflow_test(CMSDK_UART_TypeDef *CMSDK_UART)
{
  int return_val=0;
  int err_code=0;
  CMSDK_UART_TypeDef *TX_UART;
  CMSDK_UART_TypeDef *RX_UART;
  char ctmp;
  int i;

  /* Determine which UART is the sender, and which UART is receiver */
  if (CMSDK_UART==CMSDK_UART0){
    TX_UART = CMSDK_UART0;
    RX_UART = CMSDK_UART1;
    }
  else if (CMSDK_UART==CMSDK_UART1){
    TX_UART = CMSDK_UART1;
    RX_UART = CMSDK_UART0;
    }
  else {
    puts ("ERROR: Input parameter invalid in function 'uart_tx_rx_overflow_test'.");
    return 1;
    }

  puts ("UART TX & RX overflow test");
  /* UART programmed with same baud rate */
  TX_UART->BAUDDIV = 34;
  if (TX_UART->BAUDDIV != 34) { err_code += (1<<0);}
  RX_UART->BAUDDIV = 34;
  if (RX_UART->BAUDDIV != 34) { err_code += (1<<1);}

  puts ("- TX without overflow");
  uart0_irq_expected=0;
  uart1_irq_expected=0;
  uart0_irq_occurred=0;
  uart1_irq_occurred=0;

  TX_UART->CTRL = UART_CTRL_TXEN ;  /* No interrupt generation */
  RX_UART->CTRL = UART_CTRL_RXEN ;  /* No interrupt generation */

  TX_UART->DATA = 'A';
  TX_UART->DATA = 'B';
  if ((TX_UART->STATE & UART_STATE_TXOVR)!=0)  { err_code += (1<<2);}
  for (i=0; i<2;i++) {
    while ((RX_UART->STATE & UART_STATE_RXFULL)==0); /* wait for data */
    ctmp=  RX_UART->DATA;
    if (i==0) {
      if (ctmp!='A') { err_code += (1<<3);}
      }
    if (i==1) {
      if (ctmp!='B') { err_code += (1<<4);}
      }
    }
  if ((RX_UART->STATE != 0)||(TX_UART->STATE != 0))  { err_code += (1<<5);}

  puts ("- TX with overflow");
  TX_UART->DATA = 'A';
  TX_UART->DATA = 'B';
  TX_UART->DATA = 'C';
  if ((TX_UART->STATE & UART_STATE_TXOVR)==0)  { err_code += (1<<6);}
  for (i=0; i<2;i++) {
    while ((RX_UART->STATE & UART_STATE_RXFULL)==0); /* wait for data */
    ctmp=  RX_UART->DATA;
    if (i==0) {
      if (ctmp!='A') { err_code += (1<<7);}
      }
      /* if i=1, data unpredictable */
    }
  /* Overrun state should stay high */
  if ((TX_UART->STATE & UART_STATE_TXOVR)==0)  { err_code += (1<<8);}
  /* Overrun interrupt status should be low because TX overrun interrupt is not set */
  if ((TX_UART->INTSTATUS & UART_INTSTATE_TXOVR)!=0)  { err_code += (1<<9);}

  TX_UART->CTRL = UART_CTRL_TXEN | UART_CTRL_TXOVRIRQEN;  /* Enable overflow interrupt generation */
  /* Overrun interrupt status should be high now */
  if ((TX_UART->INTSTATUS & UART_INTSTATE_TXOVR)==0)  { err_code += (1<<10);}

  /* enable the overflow interrupt in NVIC to trigger the overflow interrupt */
  if (CMSDK_UART==CMSDK_UART0){
    uart0_irq_expected=1;
    uart1_irq_expected=0;
    uart0_irq_occurred=0;
    uart1_irq_occurred=0;
    NVIC_EnableIRQ(UARTOVF0_IRQn);
    }
  if (CMSDK_UART==CMSDK_UART1){
    uart0_irq_expected=0;
    uart1_irq_expected=1;
    uart0_irq_occurred=0;
    uart1_irq_occurred=0;
    NVIC_EnableIRQ(UARTOVF1_IRQn);
    }

  __DSB();
  __ISB();
  /* The interrupt should be taken */

  /* Overrun state should be cleared by interrupt handler */
  if ((TX_UART->STATE & UART_STATE_TXOVR)!=0)  { err_code += (1<<11);}
  /* interrupt handler should be executed once */
  if ((CMSDK_UART==CMSDK_UART0)&&(uart0_irq_occurred==0)) { err_code += (1<<12);}
  if ((CMSDK_UART==CMSDK_UART1)&&(uart1_irq_occurred==0)) { err_code += (1<<12);}

  TX_UART->CTRL = UART_CTRL_TXEN ;  /* No interrupt generation */
  RX_UART->CTRL = UART_CTRL_RXEN ;  /* No interrupt generation */

  NVIC_DisableIRQ(UARTOVF0_IRQn);
  NVIC_DisableIRQ(UARTOVF1_IRQn);
  uart0_irq_expected = 0;
  uart1_irq_expected = 0;

  puts ("- RX overflow");
  TX_UART->DATA = 'A';
  TX_UART->DATA = 'B';
  /* TX overflow should not occur */
  if ((TX_UART->STATE & UART_STATE_TXOVR)!=0)  { err_code += (1<<13);}
  /* wait until RX buffer full */
  while ((RX_UART->STATE & UART_STATE_RXFULL)==0);
  /* Should not overflow yet */
  if ((RX_UART->STATE & UART_STATE_RXOVR)!=0)  { err_code += (1<<14);}
  /* wait until RX overflow */
  while ((RX_UART->STATE & UART_STATE_RXOVR)==0);
  /* RX overflow interrupt should be low because RX overflow interrupt enable is not set */
  if ((RX_UART->INTSTATUS & UART_INTSTATE_RXOVR)!=0)  { err_code += (1<<15);}

  RX_UART->CTRL = UART_CTRL_RXEN | UART_CTRL_RXOVRIRQEN;  /* Enable overflow interrupt generation */
  /* Overrun interrupt status should be high now */
  if ((RX_UART->INTSTATUS & UART_INTSTATE_RXOVR)==0)  { err_code += (1<<16);}

  /* enable the overflow interrupt in NVIC to trigger the overflow interrupt */
  if (CMSDK_UART==CMSDK_UART0){
    uart0_irq_expected=0;
    uart1_irq_expected=1;
    uart0_irq_occurred=0;
    uart1_irq_occurred=0;
    NVIC_EnableIRQ(UARTOVF1_IRQn);
    }
  if (CMSDK_UART==CMSDK_UART1){
    uart0_irq_expected=1;
    uart1_irq_expected=0;
    uart0_irq_occurred=0;
    uart1_irq_occurred=0;
    NVIC_EnableIRQ(UARTOVF0_IRQn);
    }

  __DSB();
  __ISB();
  /* The interrupt should be taken */

  /* Overrun state should be cleared by interrupt handler */
  if ((RX_UART->STATE & UART_STATE_RXOVR)!=0)  { err_code += (1<<17);}
  /* interrupt handler should be executed once */
  if ((CMSDK_UART==CMSDK_UART0)&&(uart1_irq_occurred==0)) { err_code += (1<<18);}
  if ((CMSDK_UART==CMSDK_UART1)&&(uart0_irq_occurred==0)) { err_code += (1<<18);}


  /* clean up */
  uart0_irq_expected=0;
  uart1_irq_expected=0;
  uart0_irq_occurred=0;
  uart1_irq_occurred=0;

  TX_UART->CTRL = 0;
  RX_UART->CTRL = 0;
  while ((RX_UART->STATE & UART_STATE_RXFULL)!=0) {
    ctmp=RX_UART->DATA;
    }

  NVIC_DisableIRQ(UARTOVF0_IRQn);
  NVIC_DisableIRQ(UARTOVF1_IRQn);

  if (err_code != 0) {
    printf ("ERROR : uart overflow test failed (0x%x)\n", err_code);
    return_val =1;
    err_code = 0;
    }

  return(return_val);
}

/* --------------------------------------------------------------- */
/*  UART 2 interrupt connectivity test                             */
/* --------------------------------------------------------------- */
int uart2_interrupt_test(void){
  int return_val=0;
  int err_code=0;
  int i;
  char ctmp; /* dummy variable for overflow test (data can be corrupted so it is not checked) */


  puts ("UART 2 interrupt connectivity test");

  puts ("- UART 2 TX IRQ");
  CMSDK_UART2->CTRL = UART_CTRL_TXEN | UART_CTRL_TXIRQEN | UART_CTRL_HIGHSPEEDTX;
  NVIC_EnableIRQ(UARTTX2_IRQn);
  uart2_irq_expected = 1;
  UartPutc('.');
  for (i=0; i<3;i++){ __ISB(); } /* small delay */
  CMSDK_UART2->CTRL = UART_CTRL_TXEN | UART_CTRL_HIGHSPEEDTX;
  if (uart2_irq_occurred==0)                         { err_code += (1<<0);}
  uart2_irq_occurred = 0;
  NVIC_DisableIRQ(UARTTX2_IRQn);

  puts ("\n- UART 2 TX overflow IRQ");
  NVIC_EnableIRQ(UARTOVF2_IRQn);
  CMSDK_UART2->CTRL = UART_CTRL_TXEN | UART_CTRL_TXOVRIRQEN | UART_CTRL_HIGHSPEEDTX;
  CMSDK_UART2->DATA = '.';
  CMSDK_UART2->DATA = '.';
  CMSDK_UART2->DATA = '.';
  for (i=0; i<3;i++){ __ISB(); } /* small delay */
  if (uart2_irq_occurred==0)                         { err_code += (1<<1);}
  uart2_irq_occurred = 0;
  NVIC_DisableIRQ(UARTOVF2_IRQn);

  puts ("\n- UART 2 RX IRQ");
  /* UART 2 RXD is shared with GPIO1[4] */
  CMSDK_GPIO1->OUTENABLESET = (1<<4);
  CMSDK_GPIO1->DATAOUT = CMSDK_GPIO1->DATAOUT | (1<<4);
  NVIC_EnableIRQ(UARTRX2_IRQn);
  CMSDK_UART2->CTRL = UART_CTRL_TXEN | UART_CTRL_RXEN | UART_CTRL_RXIRQEN | UART_CTRL_HIGHSPEEDTX;
  CMSDK_GPIO1->DATAOUT = CMSDK_GPIO1->DATAOUT & ~(1<<4);
  for (i=0; i<2;i++){ __ISB(); } /* small delay to create start bit */
  CMSDK_GPIO1->DATAOUT = CMSDK_GPIO1->DATAOUT | (1<<4);
  delay_for_character();
  if (uart2_irq_occurred==0)                         { err_code += (1<<2);}
  if ((CMSDK_UART2->STATE & UART_STATE_RXFULL)==0)  { err_code += (1<<3);}
  ctmp=CMSDK_UART2->DATA;
  /*printf ("Receive data = 0x%x\n", ctmp);*/
  uart2_irq_occurred = 0;
  NVIC_DisableIRQ(UARTRX2_IRQn);

  puts ("\n- UART 2 RX overflow IRQ");
  NVIC_EnableIRQ(UARTOVF2_IRQn);
  CMSDK_UART2->CTRL = UART_CTRL_TXEN | UART_CTRL_RXEN | UART_CTRL_RXOVRIRQEN | UART_CTRL_HIGHSPEEDTX;
  /* First character */
  CMSDK_GPIO1->DATAOUT = CMSDK_GPIO1->DATAOUT & ~(1<<4);
  for (i=0; i<2;i++){ __ISB(); } /* small delay to create start bit */
  CMSDK_GPIO1->DATAOUT = CMSDK_GPIO1->DATAOUT | (1<<4);
  delay_for_character();
  /* Second character */
  CMSDK_GPIO1->DATAOUT = CMSDK_GPIO1->DATAOUT & ~(1<<4);
  for (i=0; i<2;i++){ __ISB(); } /* small delay to create start bit */
  CMSDK_GPIO1->DATAOUT = CMSDK_GPIO1->DATAOUT | (1<<4);
  delay_for_character();
  if (uart2_irq_occurred==0)                         { err_code += (1<<4);}

  /* Remove receive data in buffer */
  while ((CMSDK_UART2->STATE & UART_STATE_RXFULL)!=0) {
    ctmp=CMSDK_UART2->DATA;
    }
  /* clear up */
  uart2_irq_occurred = 0;
  NVIC_DisableIRQ(UARTOVF2_IRQn);
  CMSDK_UART2->CTRL = UART_CTRL_TXEN | UART_CTRL_HIGHSPEEDTX;

  CMSDK_GPIO1->OUTENABLECLR = (1<<4);
  CMSDK_GPIO1->DATAOUT = CMSDK_GPIO1->DATAOUT & ~(1<<4);
  uart2_irq_expected = 0;



  if (err_code != 0) {
    printf ("ERROR : uart overflow test failed (0x%x)\n", err_code);
    return_val =1;
    err_code = 0;
    }

  return(return_val);
}
/* --------------------------------------------------------------- */
/*  delay function to provide delay for one character              */
/* --------------------------------------------------------------- */

void delay_for_character(void)
{
  int i;
  for (i=0; i<120;i++){
    __ISB();
    }
  return;
}
/* --------------------------------------------------------------- */
/*  Peripheral ID detection to check if device is present          */
/* --------------------------------------------------------------- */
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
/* --------------------------------------------------------------- */
/*  UART interrupt handlers                                        */
/* --------------------------------------------------------------- */
void UARTRX0_Handler(void)
{
  int err_code = 0;
  if (uart0_irq_expected==0)                           {err_code += (1<<0);}
  if ((CMSDK_UART0->INTSTATUS & UART_INTSTATE_RX)==0) {err_code += (1<<1);}
  CMSDK_UART0->INTCLEAR = UART_INTSTATE_RX; /* Clear interrupt status */
  if ((CMSDK_UART0->INTSTATUS & UART_INTSTATE_RX)!=0) {err_code += (1<<2);}
  uart0_irq_occurred++;
  if (err_code != 0) {
    printf ("ERROR : UART 0 RX handler failed (0x%x)\n", err_code);
    UartEndSimulation();
    while(1);
    }
  return;
}

void UARTTX0_Handler(void)
{
  int err_code = 0;
  if (uart0_irq_expected==0)                           {err_code += (1<<0);}
  if ((CMSDK_UART0->INTSTATUS & UART_INTSTATE_TX)==0) {err_code += (1<<1);}
  CMSDK_UART0->INTCLEAR = UART_INTSTATE_TX; /* Clear interrupt status */
  if ((CMSDK_UART0->INTSTATUS & UART_INTSTATE_TX)!=0) {err_code += (1<<2);}
  uart0_irq_occurred++;
  if (err_code != 0) {
    printf ("ERROR : UART 0 TX handler failed (0x%x)\n", err_code);
    UartEndSimulation();
    while(1);
    }
  return;
}
void UARTRX1_Handler(void)
{
  int err_code = 0;
  if (uart1_irq_expected==0)                           {err_code += (1<<0);}
  if ((CMSDK_UART1->INTSTATUS & UART_INTSTATE_RX)==0) {err_code += (1<<1);}
  CMSDK_UART1->INTCLEAR = UART_INTSTATE_RX; /* Clear interrupt status */
  if ((CMSDK_UART1->INTSTATUS & UART_INTSTATE_RX)!=0) {err_code += (1<<2);}
  uart1_irq_occurred++;
  if (err_code != 0) {
    printf ("ERROR : UART 1 RX handler failed (0x%x)\n", err_code);
    UartEndSimulation();
    while(1);
    }
  return;
}

void UARTTX1_Handler(void)
{
  int err_code = 0;
  if (uart1_irq_expected==0)                           {err_code += (1<<0);}
  if ((CMSDK_UART1->INTSTATUS & UART_INTSTATE_TX)==0) {err_code += (1<<1);}
  CMSDK_UART1->INTCLEAR = UART_INTSTATE_TX; /* Clear interrupt status */
  if ((CMSDK_UART1->INTSTATUS & UART_INTSTATE_TX)!=0) {err_code += (1<<2);}
  uart1_irq_occurred++;
  if (err_code != 0) {
    printf ("ERROR : UART 1 TX handler failed (0x%x)\n", err_code);
    UartEndSimulation();
    while(1);
    }
  return;
}
void UARTOVF0_Handler(void)
{
  int err_code = 0;
  if (uart0_irq_expected==0)                               {err_code += (1<<0);}
  if (((CMSDK_UART0->INTSTATUS & UART_INTSTATE_TXOVR)==0) &
      ((CMSDK_UART0->INTSTATUS & UART_INTSTATE_RXOVR)==0)) {err_code += (1<<1);}
  if ((CMSDK_UART0->INTSTATUS & UART_INTSTATE_TXOVR)!=0){
    CMSDK_UART0->STATE = UART_STATE_TXOVR; /* Clear TX overrun status */
    }
  if ((CMSDK_UART0->INTSTATUS & UART_INTSTATE_RXOVR)!=0){
    CMSDK_UART0->STATE = UART_STATE_RXOVR; /* Clear RX overrun status */
    }
  if ((CMSDK_UART0->INTSTATUS & UART_INTSTATE_TXOVR)!=0)  {err_code += (1<<2);}
  if ((CMSDK_UART0->INTSTATUS & UART_INTSTATE_RXOVR)!=0)  {err_code += (1<<3);}
  uart0_irq_occurred++;
  if (err_code != 0) {
    printf ("ERROR : UART 0 overrun handler failed (0x%x)\n", err_code);
    UartEndSimulation();
    while(1);
    }  return;
}

void UARTOVF1_Handler(void)
{
  int err_code = 0;
  if (uart1_irq_expected==0)                               {err_code += (1<<0);}
  if (((CMSDK_UART1->INTSTATUS & UART_INTSTATE_TXOVR)==0) &
      ((CMSDK_UART1->INTSTATUS & UART_INTSTATE_RXOVR)==0)) {err_code += (1<<1);}
  if ((CMSDK_UART1->INTSTATUS & UART_INTSTATE_TXOVR)!=0){
    CMSDK_UART1->STATE = UART_STATE_TXOVR; /* Clear TX overrun status */
    }
  if ((CMSDK_UART1->INTSTATUS & UART_INTSTATE_RXOVR)!=0){
    CMSDK_UART1->STATE = UART_STATE_RXOVR; /* Clear RX overrun status */
    }
  if ((CMSDK_UART1->INTSTATUS & UART_INTSTATE_TXOVR)!=0)  {err_code += (1<<2);}
  if ((CMSDK_UART1->INTSTATUS & UART_INTSTATE_RXOVR)!=0)  {err_code += (1<<3);}
  uart1_irq_occurred++;
  if (err_code != 0) {
    printf ("ERROR : UART 1 overrun handler failed (0x%x)\n", err_code);
    UartEndSimulation();
    while(1);
    }
  return;
}
void UARTRX2_Handler(void)
{
  int err_code = 0;
  if (uart2_irq_expected==0)                           {err_code += (1<<0);}
  if ((CMSDK_UART2->INTSTATUS & UART_INTSTATE_RX)==0) {err_code += (1<<1);}
  CMSDK_UART2->INTCLEAR = UART_INTSTATE_RX; /* Clear interrupt status */
  if ((CMSDK_UART2->INTSTATUS & UART_INTSTATE_RX)!=0) {err_code += (1<<2);}
  uart2_irq_occurred++;
  if (err_code != 0) {
    printf ("ERROR : UART 2 RX handler failed (0x%x)\n", err_code);
    UartEndSimulation();
    while(1);
    }
  return;
}

void UARTTX2_Handler(void)
{
  int err_code = 0;
  if (uart2_irq_expected==0)                           {err_code += (1<<0);}
  if ((CMSDK_UART2->INTSTATUS & UART_INTSTATE_TX)==0) {err_code += (1<<1);}
  CMSDK_UART2->INTCLEAR = UART_INTSTATE_TX; /* Clear interrupt status */
  if ((CMSDK_UART2->INTSTATUS & UART_INTSTATE_TX)!=0) {err_code += (1<<2);}
  uart2_irq_occurred++;
  if (err_code != 0) {
    printf ("ERROR : UART 2 TX handler failed (0x%x)\n", err_code);
    UartEndSimulation();
    while(1);
    }
  return;
}

void UARTOVF2_Handler(void)
{
  int err_code = 0;
  if (uart2_irq_expected==0)                               {err_code += (1<<0);}
  if (((CMSDK_UART2->INTSTATUS & UART_INTSTATE_TXOVR)==0) &
      ((CMSDK_UART2->INTSTATUS & UART_INTSTATE_RXOVR)==0)) {err_code += (1<<1);}
  if ((CMSDK_UART2->INTSTATUS & UART_INTSTATE_TXOVR)!=0){
    CMSDK_UART2->STATE = UART_STATE_TXOVR; /* Clear TX overrun status */
    }
  if ((CMSDK_UART2->INTSTATUS & UART_INTSTATE_RXOVR)!=0){
    CMSDK_UART2->STATE = UART_STATE_RXOVR; /* Clear RX overrun status */
    }
  if ((CMSDK_UART2->INTSTATUS & UART_INTSTATE_TXOVR)!=0)  {err_code += (1<<2);}
  if ((CMSDK_UART2->INTSTATUS & UART_INTSTATE_RXOVR)!=0)  {err_code += (1<<3);}
  uart2_irq_occurred++;
  if (err_code != 0) {
    printf ("ERROR : UART 2 overrun handler failed (0x%x)\n", err_code);
    UartEndSimulation();
    while(1);
    }
  return;
}

