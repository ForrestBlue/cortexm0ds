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
 *      Checked In          : $Date: 2013-03-14 17:09:22 +0000 (Thu, 14 Mar 2013) $
 *
 *      Revision            : $Revision: 241267 $
 *
 *      Release Information : Cortex-M0 DesignStart-r1p0-00rel0
 *-----------------------------------------------------------------------------
 */

 /* Check self reset using SYSRESETREQ and LOCKUP reset */

#include "CMSDK_CM0.h"
#include <stdio.h>
#include "uart_stdout.h"

#define HW32_REG(ADDRESS)  (*((volatile unsigned long  *)(ADDRESS)))
#define BAD_ADDRESS        0xC0000001

void main_prog_part_1(void); /* Carry out SYSRESETREQ */
void main_prog_part_2(void); /* Check reset info, carry out lockup reset */
void main_prog_part_3(void); /* Check reset info */
int  sysctrl_id_check(void);

/*
  Test process:
  - if system controller is not available, skip test
  - check reset status, which should be 0 in the beginning of test, goto execute part 1
  - part 1 : Use System Reset Request function from CMSIS
  - <system reset>
  - Restarted, if system controller is not available, skip test
  - check reset status, which should be 1 (SYSRESETREQ), goto execute part 2
  - clear reset status, and check reset status is cleared.
  - enable automatic reset for lockup
  - check the reset option register read back value is correct
  - trigger hardfault by reading BAD_ADDRESS
  - enter hardfault handler, and then trigger lock up by reading BAD_ADDRESS again
  - <lockup -> system reset>
  - Restarted, if system controller is not available, skip test
  - check reset status, which should be 4 (Lockup reset), goto execute part 3
  - clear reset status, and check reset status is cleared.
  - test completed , end test

*/

/* ----------------------------------------------------------------- */
/* Start of main code */
int main (void)
{
  // UART init
  UartStdOutInit();

  // Test banner message and revision number
  puts("\nCortex Microcontroller System Design Kit - Self Reset Demo - revision $Revision: 241267 $\n");

  if (sysctrl_id_check()!=0) {
    puts ("** TEST SKIPPED ** SysCtrl not present.");
    UartEndSimulation();
    return 0;}

  puts("[System started]\n");
  /* Check CMSDK_SYSCON->RSTINFO Reset Information register */
  /*  0 = cold reset */
  /*  1 (bit 0) = reset from SYSRESETREQ */
  /*  2 (bit 1) = reset from Watchdog */
  /*  4 (bit 2) = reset from Lockup auto reset */
  if ((CMSDK_SYSCON->RSTINFO & CMSDK_SYSCON_RSTINFO_SYSRESETREQ_Msk) !=0) {
    puts ("Restarted by SYSRESETREQ reset");
    main_prog_part_2();  /* Check result from part 1 and trigger reset using lockup reset */
    UartEndSimulation(); /* Should not get here. */
    }
  else if ((CMSDK_SYSCON->RSTINFO & CMSDK_SYSCON_RSTINFO_LOCKUPRESET_Msk) !=0) {
    puts ("Restarted by Lockup reset");
    main_prog_part_3(); /* Check result */
    UartEndSimulation(); /* Should not get here. Simulation stops in UartEndSimulation */
    while(1);
    }
  else if ((CMSDK_SYSCON->RSTINFO & CMSDK_SYSCON_RSTINFO_WDOGRESETREQ_Msk) !=0) {
    puts ("Restarted by Watchdog reset");
    puts ("ERROR: Watchdog was not used in this test. Reset info register value incorrect.\n");
    UartEndSimulation(); /* Should not get here. Simulation stops in UartEndSimulation */
    }
  else {
    puts("Self Reset demo test\n");
    main_prog_part_1(); /* Trigger self reset */
    }
  UartEndSimulation(); /* Should not get here. */
  /* Simulation stops in UartEndSimulation */
}

/* ----------------------------------------------------------------- */
/* Use ID value to detect if the system controller is present */
int sysctrl_id_check(void)
{ /* CMSDK SysCtrl part ID range from 826 to 829 */
#define HW32_REG(ADDRESS)  (*((volatile unsigned long  *)(ADDRESS)))
if ((HW32_REG(CMSDK_SYSCTRL_BASE + 0xFE0) < 0x26) ||
    (HW32_REG(CMSDK_SYSCTRL_BASE + 0xFE0) > 0x29) ||
    (HW32_REG(CMSDK_SYSCTRL_BASE + 0xFE4) != 0xB8))
  return 1; /* part ID does not match */
else
  return 0;
}

/* ----------------------------------------------------------------- */
/* First part of the test program */
void main_prog_part_1(void)
{
  puts("Using CMSIS function to generate self reset...\n");

  NVIC_SystemReset();
  while(1);

}

/* ----------------------------------------------------------------- */
/* Second part of the main test program - execute after SYSRESETREQ reset */
void main_prog_part_2(void)
{
  int result=0;
  unsigned int read_data;

  puts("- Self reset completed");
  read_data = CMSDK_SYSCON->RSTINFO;
  printf ("  SYSCON->RSTINFO = %x\n", read_data);
  if (read_data != CMSDK_SYSCON_RSTINFO_SYSRESETREQ_Msk) result++;
  puts("- Clear reset info");
  CMSDK_SYSCON->RSTINFO = CMSDK_SYSCON_RSTINFO_SYSRESETREQ_Msk;
  read_data = CMSDK_SYSCON->RSTINFO;
  printf ("  SYSCON->RSTINFO = %x\n", read_data);
  if (read_data != 0) result++;
  if (result != 0) {
    puts ("ERROR: reset info register value incorrect.\n");
  } else {
    puts ("SYSRESETREQ test done\nNow test reset caused by lockup\n");
  }

  /* Test Lockup reset */
  CMSDK_SYSCON->RESETOP = CMSDK_SYSCON_LOCKUPRST_RESETOP_Msk;
  read_data = CMSDK_SYSCON->RESETOP;
  if (read_data != CMSDK_SYSCON_LOCKUPRST_RESETOP_Msk) {
    puts ("ERROR: reset option register value incorrect.\n");
    UartEndSimulation();
    }
  read_data = CMSDK_SYSCON->RSTINFO;
  if (read_data != 0) {
    puts ("ERROR: reset info register value incorrect.\n");
    UartEndSimulation();
    }

  /* Trigger hard fault */
  read_data=HW32_REG(BAD_ADDRESS);

  puts ("ERROR: Hard fault not triggered");

  UartEndSimulation();
    /* Simulation stops in UartEndSimulation */

  return;
}

/* ----------------------------------------------------------------- */
/* Get to here after lockup auto-reset */
void main_prog_part_3(void)
{
  int result=0;
  unsigned int read_data;

  puts("- Self reset completed");
  /* Check reset info register value */
  read_data = CMSDK_SYSCON->RSTINFO;
  printf ("  SYSCON->RSTINFO = %x\n", read_data);
  if (read_data != CMSDK_SYSCON_RSTINFO_LOCKUPRESET_Msk) result++;
  puts("- Clear reset info");
  CMSDK_SYSCON->RSTINFO = CMSDK_SYSCON_RSTINFO_LOCKUPRESET_Msk;
  read_data = CMSDK_SYSCON->RSTINFO;
  printf ("  SYSCON->RSTINFO = %x\n", read_data);
  if (read_data != 0) result++;
  if (result != 0) {
    puts ("ERROR: reset info register value incorrect.\n");
  } else {
    puts ("Lockup reset test completed successfully\n");
    puts ("** TEST PASSED ** \n");
  }
  UartEndSimulation();
    /* Simulation stops in UartEndSimulation */

  return;
}

/* ----------------------------------------------------------------- */
void HardFault_Handler(void)
{
  unsigned int dummy; /* dummy variable for read that trigger hardfault */

  puts("[Hard fault handler]");
  puts("- trigger fault again to enter lockup");

  /* Trigger lockup */
  dummy=HW32_REG(BAD_ADDRESS);
  puts ("ERROR: Lockup not triggered");

  UartEndSimulation();
    /* Simulation stops in UartEndSimulation */
}

