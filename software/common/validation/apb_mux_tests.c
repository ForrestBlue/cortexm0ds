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
  A simple test to check the operation of APB slave multiplexer
*/


#include "CMSDK_CM0.h"
#include <stdio.h>
#include "uart_stdout.h"

#define HW32_REG(ADDRESS)  (*((volatile unsigned long  *)(ADDRESS)))
#define HW16_REG(ADDRESS)  (*((volatile unsigned short *)(ADDRESS)))
#define HW8_REG(ADDRESS)   (*((volatile unsigned char  *)(ADDRESS)))


#if defined ( __CC_ARM   )
__asm void          address_test_write(unsigned int addr, unsigned int wdata);
__asm unsigned int  address_test_read(unsigned int addr);
#else
      void          address_test_write(unsigned int addr, unsigned int wdata);
      unsigned int  address_test_read(unsigned int addr);
#endif
void                HardFault_Handler_c(unsigned int * hardfault_args, unsigned lr_value);
int                 ID_Check(const unsigned char id_array[], unsigned int offset);
int                 APB_test_slave_Check(unsigned int offset);

/* Global variables */
volatile int hardfault_occurred;
volatile int hardfault_expected;
volatile int temp_data;
         int hardfault_verbose=0; // 0:Not displaying anything in hardfault handler

/* Predefined ID values for APB peripherals */
const unsigned char ahb_gpio_id[16]     = {0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
                                           0x20, 0xB8, 0x0B, 0x00, 0x0D, 0xF0, 0x05, 0xB1};
const unsigned char apb_uart_id[16]     = {0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
                                           0x21, 0xB8, 0x0B, 0x00, 0x0D, 0xF0, 0x05, 0xB1};
const unsigned char apb_timer_id[16]    = {0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
                                           0x22, 0xB8, 0x0B, 0x00, 0x0D, 0xF0, 0x05, 0xB1};
const unsigned char apb_dualtimer_id[16]= {0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
                                           0x23, 0xB8, 0x0B, 0x00, 0x0D, 0xF0, 0x05, 0xB1};
const unsigned char apb_watchdog_id[16] = {0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
                                           0x24, 0xB8, 0x0B, 0x00, 0x0D, 0xF0, 0x05, 0xB1};
const unsigned char pl230_udma_id[16]   = {0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
                                           0x30, 0xB2, 0x0B, 0x00, 0x0D, 0xF0, 0x05, 0xB1};
const unsigned char blank_id[16]        = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

int main (void)
{

  int err_code = 0;

  // UART init
  UartStdOutInit();

  // Test banner message and revision number
  puts("\nCortex Microcontroller System Design Kit");
  puts(" - APB slave mux test - revision $Revision: 243193 $\n");

  puts(" - Detecting if default peripherals are present ...\n");
  temp_data=0;
  hardfault_occurred = 0;
  hardfault_expected = 0;
  puts ("0: timer 0");
    if ( ID_Check(&apb_timer_id[0],     CMSDK_TIMER0_BASE   ) == 1 ) err_code |= 1<<0;
  puts ("1: timer 1");
    if ( ID_Check(&apb_timer_id[0],     CMSDK_TIMER1_BASE   ) == 1 ) err_code |= 1<<1;
  puts ("2: dual timer");
    if ( ID_Check(&apb_dualtimer_id[0], CMSDK_DUALTIMER_BASE) == 1 ) err_code |= 1<<2;
  puts ("3: blank");
    if ( ID_Check(&blank_id[0],         0x40003000          ) == 1 ) err_code |= 1<<3;
  puts ("4: UART 0");
    if ( ID_Check(&apb_uart_id[0],      CMSDK_UART0_BASE    ) == 1 ) err_code |= 1<<4;
  puts ("5: UART 1");
    if ( ID_Check(&apb_uart_id[0],      CMSDK_UART1_BASE    ) == 1 ) err_code |= 1<<5;
  puts ("6: UART 2");
    if ( ID_Check(&apb_uart_id[0],      CMSDK_UART2_BASE    ) == 1 ) err_code |= 1<<6;
  puts ("7: blank");
    if ( ID_Check(&blank_id[0],         0x40007000          ) == 1 ) err_code |= 1<<7;
  puts ("8: Watchdog");
    if ( ID_Check(&apb_watchdog_id[0],  CMSDK_WATCHDOG_BASE ) == 1 ) err_code |= 1<<8;
  puts ("9: blank");
    if ( ID_Check(&blank_id[0],         0x40009000          ) == 1 ) err_code |= 1<<9;
  puts ("10: blank");
    if ( ID_Check(&blank_id[0],         0x4000A000          ) == 1 ) err_code |= 1<<10;
  puts ("11: APB test slave");
    if ( APB_test_slave_Check(          0x4000B000          ) == 1 ) err_code |= 1<<11;
  puts ("12: APB expansion port 12");
    if ( ID_Check(&blank_id[0],         0x4000C000          ) == 1 ) err_code |= 1<<12;
  puts ("13: APB expansion port 13");
    if ( ID_Check(&blank_id[0],         0x4000D000          ) == 1 ) err_code |= 1<<13;
  puts ("14: APB expansion port 14");
    if ( ID_Check(&blank_id[0],         0x4000E000          ) == 1 ) err_code |= 1<<14;
  puts ("15: APB expansion port 15 (optional DMA controller)");
    if ( ID_Check(&pl230_udma_id[0],    CMSDK_PL230_BASE    ) == 1 ) err_code |= 1<<15;


  /* Report error code */

  if (err_code==0) {
    printf ("\n** TEST PASSED **\n");
  } else {
    printf ("\n** TEST FAILED **, Error code = (0x%x)\n", err_code);
  }
  UartEndSimulation();
  return 0;
}

/* Check the ID register value in offset 0xFC0 to 0xFFC (last 16 words, last 12 are IDs) */
int ID_Check(const unsigned char id_array[], unsigned int offset)
{
  int i; /* loop counter */
  unsigned long expected_val, actual_val;
  unsigned long compare_mask;
  int           mismatch = 0;
  int           id_is_zero = 0;
  unsigned long test_addr;

  /* Check the peripheral ID and component ID */
  for (i=0;i<16;i++) {
    test_addr = offset + 4*i + 0xFC0;
    expected_val = (int) id_array[i];
    actual_val   = HW32_REG(test_addr);

    if (actual_val == 0) id_is_zero++; // Check if all ID are zero at the end

    /* create mask to ignore version numbers */
    if      (i==10) { compare_mask = 0xF0;}  // mask out version field
    else if (i==11) { compare_mask = 0xFF;}  // mask out ECO field and patch field
    else            { compare_mask = 0x00;}  // compare whole value

    if ((expected_val & (~compare_mask)) != (actual_val & (~compare_mask))) {
      printf ("Difference found: %x, expected %x, actual %x\n", test_addr, expected_val, actual_val);
      mismatch++;
      }
    } // end_for

    if (id_is_zero == 16) {
        puts ("  All ID values are 0   : device not present\n");
        return 2; }
    else if (mismatch> 0) {
        puts ("  ID value mismatch(es) : device unknown\n");
        return 1; }
    else {
        puts ("  All ID values matched : device present\n");
        return 0;
         }

}

/* Simple test for APB test slave */
int APB_test_slave_Check(unsigned int offset)
{
  /*
  The APB test slave has got only one 32-bit register in
  the first four addresses. For each word, the wait state
  increase based on PADDR[3:2].

  The test slave is also able to generate error responses
  at offset 0x0F0 to 0xFC.
  */
  int  mismatch = 0;
  int  fault_err = 0;
  int  i;

  // Read/Write test to data_register
  HW32_REG(offset)   = 0xFFFFFFFF;
  if ((HW32_REG(offset))   !=0xFFFFFFFF) mismatch ++;
  HW32_REG(offset)   = 0x00000000;
  if ((HW32_REG(offset))   !=0x00000000) mismatch ++;
  HW32_REG(offset)   = 0x12345678;
  if ((HW32_REG(offset))   !=0x12345678) mismatch ++;
  HW16_REG(offset)   = 0x5678; /* Use consistent access size.
    This is required to allow the test to run in big endian */
  if ((HW16_REG(offset))   !=0x5678)     mismatch ++;
  HW16_REG(offset+2) = 0x1234; /* Use consistent access size.*/
  if ((HW16_REG(offset+2)) !=0x1234)     mismatch ++;
  HW8_REG(offset)   = 0x78; /* Use consistent access size between read and write.*/
  HW8_REG(offset+1) = 0x56;
  HW8_REG(offset+2) = 0x34;
  HW8_REG(offset+3) = 0x12;
  if ((HW8_REG(offset))    !=0x78)       mismatch ++;
  if ((HW8_REG(offset+1))  !=0x56)       mismatch ++;
  if ((HW8_REG(offset+2))  !=0x34)       mismatch ++;
  if ((HW8_REG(offset+3))  !=0x12)       mismatch ++;
  HW16_REG(offset+4) = 0xFFFF; // Value = 0x1234FFFF
  if ((HW16_REG(offset+8)) !=0xFFFF)     mismatch ++;
  HW16_REG(offset+6) = 0x0000; // Value = 0x0000FFFF
  if ((HW16_REG(offset+4))   !=0xFFFF)   mismatch ++;
  if ((HW16_REG(offset+6))   !=0x0000)   mismatch ++;
  HW8_REG(offset+8)   = 0xA5;  // Value = 0x0000FFA5
  if ((HW8_REG(offset+8))   !=0xA5)      mismatch ++;
  HW8_REG(offset+0xD) = 0xC3;  // Value = 0x0000C3A5
  if ((HW8_REG(offset+0xD)) !=0xC3)      mismatch ++;
  HW16_REG(offset+0xE)= 0xC53A;// Value = 0xC53AC3A5
  if ((HW16_REG(offset+0xE)) !=0xC53A)   mismatch ++;

  /* Testing of error response from APB test slave */
  for (i=0; i<4; i++) {
    temp_data=0;
    hardfault_expected = 1;
    hardfault_occurred = 0;
    address_test_write((offset + 0xF0 + (i<<2)), 0x0);
    if (hardfault_occurred==0) fault_err++;
    hardfault_occurred = 0;
    temp_data=address_test_read(offset + 0xF0 + (i<<2));
    if (hardfault_occurred==0) fault_err++;
    }

  hardfault_expected = 0;
  hardfault_occurred = 0;

  if (mismatch>0) { puts ("  Data mismatch in APB test slave\n");
                    return 1;}
  if (fault_err>0){ puts ("  Fault mismatch in APB test slave\n");
                    return 1;}
  else            puts ("  APB test slave test passed\n");

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
  if (hardfault_verbose) puts ("[Hard Fault Handler]");
  if (hardfault_expected==0) {
    puts ("ERROR : Unexpected HardFault interrupt occurred.\n");
    UartEndSimulation();
    while (1);
    }
  stacked_r0  = ((unsigned long) hardfault_args[0]);
  stacked_pc  = ((unsigned long) hardfault_args[6]);
  if (hardfault_verbose)  printf(" - Stacked R0 : 0x%x\n", stacked_r0);
  if (hardfault_verbose)  printf(" - Stacked PC : 0x%x\n", stacked_pc);
  /* Modify R0 to a valid address */
  hardfault_args[0] = (unsigned long) &temp_data;

  return;
}



