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

//
//  Simple boot loader
//  - display a message that the boot loader is running
//  - clear remap control (user flash accessible from address 0x0)
//  - execute program from user flash
//

#include "CMSDK_CM0.h"

void UartStdOutInit(void)
{
  CMSDK_UART2->BAUDDIV = 16;
  CMSDK_UART2->CTRL    = 0x41; // High speed test mode, TX only
  CMSDK_GPIO1->ALTFUNCSET = (1<<5);
  return;
}
// Output a character
unsigned char UartPutc(unsigned char my_ch)
{
  while ((CMSDK_UART2->STATE & 1)); // Wait if Transmit Holding register is full
  CMSDK_UART2->DATA = my_ch; // write to transmit holding register
  return (my_ch);
}
// Uart string output
void UartPuts(unsigned char * mytext)
{
  unsigned char CurrChar;
  do {
    CurrChar = *mytext;
    if (CurrChar != (char) 0x0) {
      UartPutc(CurrChar);  // Normal data
      }
    *mytext++;
  } while (CurrChar != 0);
  return;
}
#if defined ( __CC_ARM   )
/* ARM RVDS or Keil MDK */
__asm void FlashLoader_ASM(void)
{
   MOVS  R0,#0
   LDR   R1,[R0]     ; Get initial MSP value
   MOV   SP, R1
   LDR   R1,[R0, #4] ; Get initial PC value
   BX    R1
}

#else
/* ARM GCC */
void FlashLoader_ASM(void) __attribute__((naked));
void FlashLoader_ASM(void)
{
   __asm("    movs  r0,#0\n"
         "    ldr   r1,[r0]\n"     /* Get initial MSP value */
         "    mov   sp, r1\n"
         "    ldr   r1,[r0, #4]\n" /* Get initial PC value */
         "    bx    r1\n");
}

#endif

void FlashLoader(void)
{
  if (CMSDK_SYSCON->REMAP==0) {
    /* Remap is already cleared. Something has gone wrong.
    Likely that the user is trying to run bootloader as a test,
     which is not what this program is for.
    */
    UartPuts("- Error: REMAP is already clear\n");
    UartPutc(0x4); // Terminate simulation
    while (1);
    }
  CMSDK_SYSCON->REMAP = 0;  // Clear remap
  __DSB();
  __ISB();

  FlashLoader_ASM();
};

int main (void)
{
  // UART init
  UartStdOutInit();

  UartPuts("\nCMSDK Boot Loader\n");
  UartPuts("- load flash\n");
  FlashLoader();
  return 0;
}

