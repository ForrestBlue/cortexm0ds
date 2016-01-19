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
 *      Checked In          : $Date: 2013-04-08 14:40:10 +0100 (Mon, 08 Apr 2013) $
 *
 *      Revision            : $Revision: 243193 $
 *
 *      Release Information : Cortex-M0 DesignStart-r1p0-00rel0
 *-----------------------------------------------------------------------------
 */

/*
  A simple test to check the operation of default slave and the handling of
  error response in the AHB interconnection.
*/
#include "CMSDK_CM0.h"
#include <stdio.h>
#include "uart_stdout.h"

#if defined ( __CC_ARM   )
__asm void          address_test_write(unsigned int addr, unsigned int wdata);
__asm unsigned int  address_test_read(unsigned int addr);
#else
      void          address_test_write(unsigned int addr, unsigned int wdata);
      unsigned int  address_test_read(unsigned int addr);
#endif
void                HardFault_Handler_c(unsigned int * hardfault_args, unsigned lr_value);

/* Global variables */
volatile int hardfault_occurred;
volatile int hardfault_expected;
volatile int temp_data;

int main (void)
{
  int err_code = 0;
  int rdata; /* dummy variable for read data in bus fault testing */

  // UART init
  UartStdOutInit();

  // Test banner message and revision number
  puts("\nCortex Microcontroller System Design Kit - Default Slave Test - revision $Revision: 243193 $\n");


  puts("Default slave tests - hard faults expected\n");
  temp_data=0;
  hardfault_occurred = 0;
  hardfault_expected = 1;
  address_test_write(0x18000000, 0x3456789A);
  if (hardfault_occurred==0) {err_code |= (1<<0);}
  hardfault_occurred = 0;

  rdata = address_test_read(0x18000000);
  if (hardfault_occurred==0) {err_code |= (1<<1);}
  hardfault_occurred = 0;

  address_test_write(0x58000000, 0x3456789A);
  if (hardfault_occurred==0) {err_code |= (1<<2);}
  hardfault_occurred = 0;

  rdata = address_test_read(0x58000000);
  if (hardfault_occurred==0) {err_code |= (1<<3);}
  hardfault_occurred = 0;

  puts("\nAPB test slave - hard faults expected\n");
  rdata = address_test_read(0x4000B0F0);
  if (hardfault_occurred==0) {err_code |= (1<<4);}
  hardfault_occurred = 0;

  rdata = address_test_read(0x4000B0F4);
  if (hardfault_occurred==0) {err_code |= (1<<5);}
  hardfault_occurred = 0;

  rdata = address_test_read(0x4000B0F8);
  if (hardfault_occurred==0) {err_code |= (1<<6);}
  hardfault_occurred = 0;

  rdata = address_test_read(0x4000B0FC);
  if (hardfault_occurred==0) {err_code |= (1<<7);}
  hardfault_occurred = 0;

  /* clean up */
  hardfault_expected = 0;

  /* Generate test pass/fail and return value */
  if (err_code==0) {
    printf ("\n** TEST PASSED **\n");
  } else {
    printf ("\n** TEST FAILED **, Error code = (0x%x)\n", err_code);
  }
  UartEndSimulation();
  return 0;
}

#if defined ( __CC_ARM   )
/* Test function for write - for ARM / Keil */
__asm void address_test_write(unsigned int addr, unsigned int wdata)
{
  STR    R1,[R0]
  DSB    ; Ensure bus fault occurred before leaving this subroutine
  BX     LR
}

#else
/* Test function for write - for gcc */
void address_test_write(unsigned int addr, unsigned int wdata) __attribute__((naked));
void address_test_write(unsigned int addr, unsigned int wdata)
{
  __asm("  str   r1,[r0]\n"
        "  dsb          \n"
        "  bx    lr     \n"
  );
}
#endif

/* Test function for read */
#if defined ( __CC_ARM   )
/* Test function for read - for ARM / Keil */
__asm unsigned int address_test_read(unsigned int addr)
{
  LDR    R1,[R0]
  DSB    ; Ensure bus fault occurred before leaving this subroutine
  MOVS   R0, R1
  BX     LR
}
#else
/* Test function for read - for gcc */
unsigned int  address_test_read(unsigned int addr) __attribute__((naked));
unsigned int  address_test_read(unsigned int addr)
{
  __asm("  ldr   r1,[r0]\n"
        "  dsb          \n"
        "  movs  r0, r1 \n"
        "  bx    lr     \n"
  );
}
#endif


#if defined ( __CC_ARM   )
/* ARM or Keil toolchain */
__asm void HardFault_Handler(void)
{
  MOVS   r0, #4
  MOV    r1, LR
  TST    r0, r1
  BEQ    stacking_used_MSP
  MRS    R0, PSP ; // first parameter - stacking was using PSP
  B      get_LR_and_branch
stacking_used_MSP
  MRS    R0, MSP ; // first parameter - stacking was using MSP
get_LR_and_branch
  MOV    R1, LR  ; // second parameter is LR current value
  LDR    R2,=__cpp(HardFault_Handler_c)
  BX     R2
  ALIGN
}
#else
/* gcc toolchain */
void HardFault_Handler(void) __attribute__((naked));
void HardFault_Handler(void)
{
  __asm("  movs   r0,#4\n"
        "  mov    r1,lr\n"
        "  tst    r0,r1\n"
        "  beq    stacking_used_MSP\n"
        "  mrs    r0,psp\n" /*  first parameter - stacking was using PSP */
        "  ldr    r1,=HardFault_Handler_c  \n"
        "  bx     r1\n"
        "stacking_used_MSP:\n"
        "  mrs    r0,msp\n" /*  first parameter - stacking was using PSP */
        "  ldr    r1,=HardFault_Handler_c  \n"
        "  bx     r1\n"
        ".pool\n" );
}

#endif
/* C part of the fault handler - common between ARM / Keil /gcc */
void HardFault_Handler_c(unsigned int * hardfault_args, unsigned lr_value)
{
  unsigned int stacked_pc;
  unsigned int stacked_r0;
  hardfault_occurred++;
  puts ("[Hard Fault Handler]");
  if (hardfault_expected==0) {
    puts ("ERROR : Unexpected HardFault interrupt occurred.\n");
    UartEndSimulation();
    while (1);
    }
  stacked_r0  = ((unsigned long) hardfault_args[0]);
  stacked_pc  = ((unsigned long) hardfault_args[6]);
  printf(" - Stacked R0 : 0x%x\n", stacked_r0);
  printf(" - Stacked PC : 0x%x\n", stacked_pc);
  /* Modify R0 to a valid address */
  hardfault_args[0] = (unsigned long) &temp_data;

  return;
}

