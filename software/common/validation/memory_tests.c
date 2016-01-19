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
  A simple test to check accesses to different memory locations
*/


#include "CMSDK_CM0.h"

#define  SRAM_BASE_ADDR  0x20000000
#define  SRAM_SIZE       0x10000

#define  ROM_BASE_ADDR   0x00000000
#define  ROM_SIZE        0x10000

#define  BOOT_BASE_ADDR  0x01000000
#define  BOOT_SIZE       0x4000
#define  BOOT_ALIAS_SIZE 0x10000

#define  APB_IO_BASE_ADDR   0x40000000
#define  APB_IO_SIZE        0x10000

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
int                 sram_test(void);
int                 sram_test_word(unsigned int test_addr);
int                 rom_test(void);
int                 boot_rom_test(void);
int                 apb_io_test(void);
int                 ahb_io_test(void);
int                 ID_Check(const unsigned char id_array[], unsigned int offset);
int                 SysCtrl_ID_Check(const unsigned char id_array[], unsigned int offset);
int                 SysCtrl_unused_addr_test(void);

/* Global variables */
volatile int hardfault_occurred;
volatile int hardfault_expected;
volatile int temp_data;
         int hardfault_verbose=0; // 0:Not displaying anything in hardfault handler

const unsigned char ahb_gpio_id[16]     = {0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
                                           0x20, 0xB8, 0x0B, 0x00, 0x0D, 0xF0, 0x05, 0xB1};
const unsigned char sysctrl_id[16]      = {0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
                                           0x26, 0xB8, 0x0B, 0x00, 0x0D, 0xF0, 0x05, 0xB1};

int main (void)
{
  int err_code = 0;

  // UART init
  UartStdOutInit();

  // Test banner message and revision number
  puts("\nCortex Microcontroller System Design Kit");
  puts(" - Simple memory test - revision $Revision: 243193 $\n");

  temp_data=0;
  hardfault_occurred = 0;
  hardfault_expected = 0;

  if (sram_test()     !=0) err_code |= 1<< 0;
  if (rom_test()      !=0) err_code |= 1<< 1;
  if (boot_rom_test() !=0) err_code |= 1<< 2;
  if (apb_io_test()   !=0) err_code |= 1<< 3;
  if (ahb_io_test()   !=0) err_code |= 1<< 4;


  /* Generate test pass/fail and return value */
  if (err_code==0) {
    printf ("\n** TEST PASSED **\n");
  } else {
    printf ("\n** TEST FAILED **, Error code = (0x%x)\n", err_code);
  }
  UartEndSimulation();
  return 0;
}

/* -------------------------------------------------------------------- */
/*   ROM test                                                          */
/* -------------------------------------------------------------------- */
int rom_test(void)
{
  int err_code = 0;

  puts ("Checking ROM");

  /* Test max and min addresses in ROM boundary */
  hardfault_occurred = 0;
  hardfault_expected = 0;
  temp_data = address_test_read(ROM_BASE_ADDR);
  if (hardfault_occurred!=0) err_code |= 1<<0;
  address_test_write(ROM_BASE_ADDR+ROM_SIZE-4, 0);
  temp_data = address_test_read(ROM_BASE_ADDR+ROM_SIZE-4);
  if (hardfault_occurred!=0) err_code |= 1<<1;

  /* Test addresses beyond ROM boundary */
  hardfault_occurred = 0;
  hardfault_expected = 1;
  temp_data = address_test_read(((unsigned)(ROM_BASE_ADDR-4)));
  if (hardfault_occurred==0) err_code |= 1<<2;
  hardfault_occurred = 0;
  hardfault_expected = 1;
  temp_data = address_test_read(ROM_BASE_ADDR+ROM_SIZE);
  if (hardfault_occurred==0) err_code |= 1<<3;

  if (err_code> 0) {
    puts ("  Failed\n");
    printf ("Error code : %x\n", err_code);
    return 1;
  } else {
    puts ("  Passed\n");
    return 0;
  }
}
/* -------------------------------------------------------------------- */
/*   Boot loader ROM test                                                          */
/* -------------------------------------------------------------------- */
int boot_rom_test(void)
{
  int err_code = 0;

  puts ("Checking Boot loader ROM");

  /* Detect if boot loader present */
  hardfault_occurred = 0;
  hardfault_expected = 1;
  temp_data = address_test_read(BOOT_BASE_ADDR);
  if (hardfault_occurred!=0) {
    puts ("  Boot loader not present");
    return 0;
  }

  /* Test max addresses in Boot loader ROM boundary */
  hardfault_expected = 0;
  hardfault_occurred = 0;
  address_test_write(BOOT_BASE_ADDR+BOOT_SIZE-4, 0);
  temp_data = address_test_read(BOOT_BASE_ADDR+BOOT_SIZE-4);
  if (hardfault_occurred!=0) err_code |= 1<<0;

  /* Test addresses beyond Boot loader ROM boundary */
  hardfault_occurred = 0;
  hardfault_expected = 1;
  temp_data = address_test_read(BOOT_BASE_ADDR-4);
  if (hardfault_occurred==0) err_code |= 1<<1;

  hardfault_occurred = 0;
  hardfault_expected = 0; /* 64KB range is allocated for boot ROM,
                 but only 4K is used. Access over 4KB range wraps round */
  temp_data = address_test_read(BOOT_BASE_ADDR+BOOT_SIZE);
  if (hardfault_occurred!=0) err_code |= 1<<2;

  hardfault_occurred = 0;
  hardfault_expected = 1; /* Check beyond alias range */
  temp_data = address_test_read(BOOT_BASE_ADDR+BOOT_ALIAS_SIZE);
  if (hardfault_occurred==0) err_code |= 1<<3;

  if (err_code> 0) {
    puts ("  Failed\n");
    printf ("Error code : %x\n", err_code);
    return 1;
  } else {
    puts ("  Passed\n");
    return 0;
  }
}
/* -------------------------------------------------------------------- */
/*   SRAM test                                                          */
/* -------------------------------------------------------------------- */
int sram_test(void)
{
  int err_code = 0;

  puts ("Checking SRAM");

  /* Test max and min addresses in SRAM boundary */

  // Starting location
  if (sram_test_word(SRAM_BASE_ADDR            )!=0) err_code |= 1<<0;
  // Ending location
  if (sram_test_word(SRAM_BASE_ADDR+SRAM_SIZE-4)!=0) err_code |= 1<<1;

  /* Test addresses beyond SRAM boundary */

  hardfault_occurred = 0;
  hardfault_expected = 1;
  temp_data = address_test_read(SRAM_BASE_ADDR-4);
  if (hardfault_occurred==0) err_code |= 1<<2;

  hardfault_occurred = 0;
  hardfault_expected = 1;
  temp_data = address_test_read(SRAM_BASE_ADDR+SRAM_SIZE);
  if (hardfault_occurred==0) err_code |= 1<<3;

  hardfault_occurred = 0;
  hardfault_expected = 0;

  if (err_code> 0) {
    puts ("  Failed\n");
    printf ("Error code : %x\n", err_code);
    return 1;
  } else {
    puts ("  Passed\n");
    return 0;
  }
}

int sram_test_word(unsigned int test_addr)
{
  int result=0; // This will only work if result and temp are not store in address being tested
  int temp;

  __disable_irq(); // Make sure interrupt will not affect the result of the test

  temp = HW32_REG(test_addr); // save data

  /* Use consistence memory access size in checking so that it work with
    both little endian and big endian configs */
  HW32_REG(test_addr)=0x00000000;
  if (HW32_REG(test_addr)!=0x00000000) result++;
  HW16_REG(test_addr)=0xFFFF;
  if (HW16_REG(test_addr)!=0xFFFF) result++;
  HW8_REG(test_addr+3)=0x12;
  HW8_REG(test_addr+2)=0xFF;
  if (HW8_REG(test_addr+3)!=0x12) result++;
  if (HW8_REG(test_addr+2)!=0xFF) result++;
  HW8_REG(test_addr  )=0x00;
  HW8_REG(test_addr+1)=0x00;
  if (HW8_REG(test_addr  )!=0x00) result++;
  if (HW8_REG(test_addr+1)!=0x00) result++;
  HW16_REG(test_addr+2)=0xFE00;
  if (HW16_REG(test_addr+2)!=0xFE00) result++;
  HW32_REG(test_addr)=0x00000000;
  if (HW32_REG(test_addr)!=0x00000000) result++;

  HW32_REG(test_addr) = temp; // restore data
  __enable_irq(); // re-enable IRQ
  if (result !=0) printf ("ERROR: Memory location test failed at %x\n", test_addr);
  return result;
}


/* -------------------------------------------------------------------- */
/*   APB test                                                          */
/* -------------------------------------------------------------------- */
int apb_io_test(void)
{
  int err_code = 0;

  puts ("Checking APB space");

  /* Test max and min addresses in IO boundary */
  hardfault_occurred = 0;
  hardfault_expected = 0;
  temp_data = address_test_read(APB_IO_BASE_ADDR);
  if (hardfault_occurred!=0) err_code |= 1<<0;
  temp_data = address_test_read(APB_IO_BASE_ADDR+APB_IO_SIZE-4);
  if (hardfault_occurred!=0) err_code |= 1<<1;

  /* Test addresses beyond IO boundary */
  hardfault_occurred = 0;
  hardfault_expected = 1;
  temp_data = address_test_read(APB_IO_BASE_ADDR-4);
  if (hardfault_occurred==0) err_code |= 1<<2;

  /* Address above APB IO space is AHB GPIO, not test here */

  if (err_code> 0) {
    puts ("  Failed\n");
    printf ("Error code : %x\n", err_code);
    return 1;
  } else {
    puts ("  Passed\n");
    return 0;
  }
}
/* -------------------------------------------------------------------- */
/*   AHB test                                                          */
/* -------------------------------------------------------------------- */
int ahb_io_test(void)
{
  int           err_code = 0;
  unsigned int  test_addr;
  int           i; /* loop counter */

  puts ("Checking AHB I/O space");

  /* Check ID for AHB peripheral */
  puts (" - GPIO #0 ID values");
  if (ID_Check(&ahb_gpio_id[0], CMSDK_GPIO0_BASE   ) == 1 ) err_code |= 1<<0;
  puts (" - GPIO #1 ID values");
  if (ID_Check(&ahb_gpio_id[0], CMSDK_GPIO1_BASE   ) == 1 ) err_code |= 1<<1;
  puts (" - System Controller ID values");
  if (SysCtrl_ID_Check(&sysctrl_id[0],  CMSDK_SYSCTRL_BASE ) == 1 ) err_code |= 1<<2;
  puts (" - System Controller unused addresses");
  if (SysCtrl_unused_addr_test() == 1) err_code |= 1<<3;

  /* Check limits of memory maps */
  puts (" - Address range check");
  hardfault_occurred = 0;
  hardfault_expected = 0;
  temp_data = address_test_read(CMSDK_GPIO0_BASE);  /* Should have no hard fault */
  if (hardfault_occurred!=0) err_code |= 1<<4;
  hardfault_occurred = 0;
  hardfault_expected = 0;
  temp_data = address_test_read(CMSDK_GPIO1_BASE);  /* Should have no hard fault */
  if (hardfault_occurred!=0) err_code |= 1<<5;
  hardfault_occurred = 0;
  hardfault_expected = 0;
  temp_data = address_test_read(CMSDK_SYSCTRL_BASE);  /* Should have no hard fault */
  if (hardfault_occurred!=0) err_code |= 1<<6;

  /* Unused AHB I/O space - should have hard fault */
  for (i=0;i<13;i++) { // From 0x40010000 to 0x4001FFFF are AHB I/O space
    hardfault_occurred = 0; // 0x40012000 to 0x4001EFFF are unused (13 slots)
    hardfault_expected = 1;
    test_addr = CMSDK_AHB_BASE+0x2000UL+(i*0x1000UL);
    temp_data = address_test_read(test_addr); // Starting address of each slot
    if (hardfault_occurred==0) {
      err_code |= 1<<7;
      printf("ERROR:Expected bus fault at %x did not take place\n", test_addr);
    }
    hardfault_occurred = 0;
    hardfault_expected = 1;
    test_addr = test_addr + 0xFFC;
    temp_data = address_test_read(test_addr); // Ending address of each slot
    if (hardfault_occurred==0) {
      err_code |= 1<<8;
      printf("ERROR:Expected bus fault at %x did not take place\n", test_addr);
    }
  } // end_for

  /* Test end of AHB I/O address range */
  hardfault_occurred = 0;
  hardfault_expected = 1;
  temp_data = address_test_read(0x40020000); // address beyond AHB I/O space
  if (hardfault_occurred==0) {
    err_code |= 1<<9;
    puts("ERROR:Expected bus fault at 0x40020000 did not take place");
  }

  if (err_code> 0) {
    puts ("  Failed\n");
    printf ("Error code : %x\n", err_code);
    return 1;
  } else {
    puts ("  Passed\n");
    return 0;
  }
}

/* -------------------------------------------------------------------- */
/*   ID value check                                                     */
/* -------------------------------------------------------------------- */

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
    return 2;
  } else if (mismatch> 0) {
    puts ("  ID value mismatch(es) : device unknown\n");
    return 1;
  } else {
    puts ("  All ID values matched : device present\n");
    return 0;
  }
}
/* Check the ID register value in offset 0xFC0 to 0xFFC (last 16 words, last 12 are IDs) */
int SysCtrl_ID_Check(const unsigned char id_array[], unsigned int offset)
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

    if (i==8) {
      /* The part number of the example system controller in CMSDK can be
      26 to 29 */
      if ((actual_val<0x26)||(actual_val>0x29)) {
        printf ("Difference found: %x, expected 0x26 to 0x29, actual %x\n", test_addr, actual_val);
        mismatch++;
      }
    }
    else {
      if ((expected_val & (~compare_mask)) != (actual_val & (~compare_mask))) {
        printf ("Difference found: %x, expected %x, actual %x\n", test_addr, expected_val, actual_val);
        mismatch++;
      }
    } // end_if_i_eq_8
  } // end_for

  if (id_is_zero == 16) {
    puts ("  All ID values are 0   : device not present\n");
    return 2;
  } else if (mismatch> 0) {
    puts ("  ID value mismatch(es) : device unknown\n");
    return 1;
  } else {
    puts ("  All ID values matched : device present\n");
    return 0;
  }
}
/* --------------------------------------------------------------- */
/*  SysCtrl Unused Addresses access tests                          */
/* --------------------------------------------------------------- */

int unused_addresses_test_single(unsigned int address)
{
  if (HW32_REG(address) != 0) return (1);
  HW32_REG(address) = 0xFFFFFFFF;
  if (HW32_REG(address) != 0) return (1);
  else return (0);
}

/* Test unused address in SysCtrl */
int SysCtrl_unused_addr_test(void)
{
  int return_val=0;
  unsigned int err_code=0;

  if (unused_addresses_test_single(CMSDK_SYSCTRL_BASE + 0x00C)) err_code |= 1 << 0;
  if (unused_addresses_test_single(CMSDK_SYSCTRL_BASE + 0x014)) err_code |= 1 << 1;
  if (unused_addresses_test_single(CMSDK_SYSCTRL_BASE + 0x018)) err_code |= 1 << 2;
  if (unused_addresses_test_single(CMSDK_SYSCTRL_BASE + 0x01C)) err_code |= 1 << 3;
  if (unused_addresses_test_single(CMSDK_SYSCTRL_BASE + 0x020)) err_code |= 1 << 4;
  if (unused_addresses_test_single(CMSDK_SYSCTRL_BASE + 0xC00)) err_code |= 1 << 5;
  if (unused_addresses_test_single(CMSDK_SYSCTRL_BASE + 0xD00)) err_code |= 1 << 6;
  if (unused_addresses_test_single(CMSDK_SYSCTRL_BASE + 0xE00)) err_code |= 1 << 7;
  if (unused_addresses_test_single(CMSDK_SYSCTRL_BASE + 0xF00)) err_code |= 1 << 8;
  if (unused_addresses_test_single(CMSDK_SYSCTRL_BASE + 0xFC0)) err_code |= 1 << 9;
  if (unused_addresses_test_single(CMSDK_SYSCTRL_BASE + 0xFC4)) err_code |= 1 << 10;
  if (unused_addresses_test_single(CMSDK_SYSCTRL_BASE + 0xFC8)) err_code |= 1 << 11;
  if (unused_addresses_test_single(CMSDK_SYSCTRL_BASE + 0xFCC)) err_code |= 1 << 12;


  /* Generate return value */
  if (err_code != 0) {
    printf ("Error : Unused addresses failed (0x%x)\n", err_code);
    return_val =1;
  }
  else puts("   Unused addresses Test Passed\n");

  return(return_val);
}
/* -------------------------------------------------------------------- */
/*   Helper functions for testing bus fault                             */
/* -------------------------------------------------------------------- */

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


