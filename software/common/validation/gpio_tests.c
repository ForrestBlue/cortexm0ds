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
  A test to check the functionality of the AHB GPIO.
*/

#include "CMSDK_CM0.h"
#define GPIO1_ALTERNATE_FUNC_MASK     0x002A    /* Implemented bits */
#include <stdio.h>
#include "uart_stdout.h"

/* Alternate function control register mask */
  /* Implemented bits */
#define GPIO0_ALTERNATE_FUNC_MASK     0x0000
  /* Default value */
#define GPIO0_ALTERNATE_FUNC_DEFAULT  0x0000

  /* Default value */
#define GPIO1_ALTERNATE_FUNC_DEFAULT  0x0000

/* Macros for word, half word and byte */
#define HW32_REG(ADDRESS)  (*((volatile unsigned long  *)(ADDRESS)))
#define HW16_REG(ADDRESS)  (*((volatile unsigned short *)(ADDRESS)))
#define HW8_REG(ADDRESS)   (*((volatile unsigned char  *)(ADDRESS)))

/* A trick to add a small delay in program */
/* small_delay() is the same as __ISB()    */
#define small_delay  __ISB

/* peripheral and component ID values */
#define AHB_GPIO_PID4  0x04
#define AHB_GPIO_PID5  0x00
#define AHB_GPIO_PID6  0x00
#define AHB_GPIO_PID7  0x00
#define AHB_GPIO_PID0  0x20
#define AHB_GPIO_PID1  0xB8
#define AHB_GPIO_PID2  0x1B
#define AHB_GPIO_PID3  0x00
#define AHB_GPIO_CID0  0x0D
#define AHB_GPIO_CID1  0xF0
#define AHB_GPIO_CID2  0x05
#define AHB_GPIO_CID3  0xB1


/* Declare test functions */
int  gpio_register_rw_test(CMSDK_GPIO_TypeDef *CMSDK_GPIO);
int  gpio_interrupt_test(CMSDK_GPIO_TypeDef *CMSDK_GPIO);
int  gpio_masked_access_test(CMSDK_GPIO_TypeDef *CMSDK_GPIO);
int  gpio_unused_addresses_test(CMSDK_GPIO_TypeDef *CMSDK_GPIO);
void medium_delay(void);
int  gpio0_id_check(void);
int  gpio1_id_check(void);

/* Global variables */
volatile int gpio0_irq_occurred;
volatile int gpio1_irq_occurred;
volatile int gpio0_irq_expected;
volatile int gpio1_irq_expected;
volatile int gpio0_irq_level_irq; // indicate it is a level IRQ test
                                  // Need to toggle interrupt source
volatile int gpio1_irq_level_irq; // indicate it is a level IRQ test
                                  // Need to toggle interrupt source
volatile int multi_irq_test;      // indicate continuous level trigger IRQ test

volatile int counter;
volatile char gpio0_irq_flags[16];

int main (void)
{
  int result=0;

  // UART init
  UartStdOutInit();

  // Test banner message and revision number
  puts("Cortex Microcontroller System Design Kit - GPIO Test - revision $Revision: 243249 $\n");

  gpio0_irq_occurred = 0;
  gpio1_irq_occurred = 0;
  gpio0_irq_expected = 0;
  gpio1_irq_expected = 0;

  puts("1) GPIO 0");

  if (gpio0_id_check()==0) {
    /* GPIO 0 present */
    result += gpio_register_rw_test(CMSDK_GPIO0);
    result += gpio_interrupt_test(CMSDK_GPIO0);
    result += gpio_masked_access_test(CMSDK_GPIO0);
    result += gpio_unused_addresses_test(CMSDK_GPIO0);
    } else {
    puts ("- GPIO #0 not present. Tests skipped.");
    }

  puts("\n2) GPIO 1");

  if (gpio1_id_check()==0) {
    /* GPIO 1 present */
    result += gpio_register_rw_test(CMSDK_GPIO1);
    result += gpio_interrupt_test(CMSDK_GPIO1);
    result += gpio_masked_access_test(CMSDK_GPIO1);
    result += gpio_unused_addresses_test(CMSDK_GPIO1);
    } else {
    puts ("- GPIO #1 not present. Tests skipped.");
    }

  if (result==0) {
    printf ("\n** TEST PASSED **\n");
  } else {
    printf ("\n** TEST FAILED **, Error code = (0x%x)\n", result);
  }
  UartEndSimulation();
  return 0;
}

int gpio0_id_check(void)
{
unsigned char gpio_id;

  gpio_id = HW8_REG(CMSDK_GPIO0_BASE  + 0xFE8) & 0x07;

  if ((HW32_REG(CMSDK_GPIO0_BASE + 0xFE0) != 0x20) ||
      (HW32_REG(CMSDK_GPIO0_BASE + 0xFE4) != 0xB8) ||
      (gpio_id != 0x03))
    return 1; /* part ID & ARM ID does not match */
  else
    return 0;
}
int gpio1_id_check(void)
{
unsigned char gpio_id;

  gpio_id = HW8_REG(CMSDK_GPIO1_BASE  + 0xFE8) & 0x07;

  if ((HW32_REG(CMSDK_GPIO1_BASE + 0xFE0) != 0x20) ||
      (HW32_REG(CMSDK_GPIO1_BASE + 0xFE4) != 0xB8) ||
      (gpio_id != 0x03))
    return 1; /* part ID & ARM ID does not match */
  else
    return 0;
}

void medium_delay(void)
{
  int i;
  for (i=0;i<3;i++){
    small_delay();
    }
  return;
}
/* --------------------------------------------------------------- */
/*  GPIO masked access tests                                       */
/* --------------------------------------------------------------- */
int gpio_masked_access_test(CMSDK_GPIO_TypeDef *CMSDK_GPIO){
  int return_val=0;
  unsigned int err_code=0;
  int i;

  unsigned int gpio_datamask;   /* Data compare mask */
  unsigned int gpio_byte_base;  /* Base address of GPIO masked access*/
  unsigned int current_value;   /*  */
  unsigned int read_value;   /*  */
  unsigned int expected_value;   /*  */
  unsigned int write_value;
  int test_counter = 0;
  int ul_err = 0;


  puts("GPIO mask access test");
  puts("- lower byte");

  if (CMSDK_GPIO==CMSDK_GPIO0) {
    gpio_datamask        = 0xFFFFFFFF;
    gpio_byte_base       = CMSDK_GPIO0_BASE + 0x400;
    }
  else if (CMSDK_GPIO==CMSDK_GPIO1) {
    gpio_datamask        = ~((1<<5)|(1<<4)|(1<<2)|(1<<0)); /* Bits that cannot be tested */
    gpio_byte_base       = CMSDK_GPIO1_BASE + 0x400;
    }
  else {
    puts ("ERROR: Input parameter invalid in function 'gpio_masked_access_test'.");
    return 1;
    }

  CMSDK_GPIO->INTENCLR = 0xFFFF;
  CMSDK_GPIO->OUTENABLESET = gpio_datamask;
  CMSDK_GPIO->DATAOUT = 0;
  small_delay(); /* DATA read back take time to get updated due to synchronisation registers*/
  current_value = CMSDK_GPIO->DATA;

  for (i=0;i<256;i++){
    /* Use masked access to read current value */
    read_value       = CMSDK_GPIO->LB_MASKED[i];
    expected_value   = current_value & i;
    if ((read_value & gpio_datamask) != (expected_value  & gpio_datamask)) { err_code |= (1<<0);}

    /* Use half word read access to carry out mask access */
    read_value       = HW16_REG(gpio_byte_base + 4 * i);
    expected_value   = current_value & i;
    if ((read_value & gpio_datamask) != (expected_value  & gpio_datamask)) { err_code |= (1<<1);}

    read_value       = HW16_REG(gpio_byte_base + 4 * i + 2);
    expected_value   = 0;
    if ((read_value                ) != (expected_value                 )) { err_code |= (1<<2);}

    /* Use byte read access to carry out mask access */
    read_value       = HW8_REG(gpio_byte_base + 4 * i);
    expected_value   = current_value & i;
    if ((read_value & gpio_datamask) != (expected_value  & gpio_datamask)) { err_code |= (1<<3);}

    read_value       = HW8_REG(gpio_byte_base + 4 * i + 1);
    expected_value   = 0;
    if ((read_value                ) != (expected_value                 )) { err_code |= (1<<4);}

    read_value       = HW8_REG(gpio_byte_base + 4 * i + 2);
    expected_value   = 0;
    if ((read_value                ) != (expected_value                 )) { err_code |= (1<<5);}

    read_value       = HW8_REG(gpio_byte_base + 4 * i + 3);
    expected_value   = 0;
    if ((read_value                ) != (expected_value                 )) { err_code |= (1<<6);}

    /* Use masked access to toggle current value, check it with word access */
    write_value               = ~CMSDK_GPIO->LB_MASKED[i];
    CMSDK_GPIO->LB_MASKED[i] = write_value;
    current_value             = (current_value & (~i)) | (write_value & i);
    small_delay(); /* DATA read back take time to get updated due to synchronisation registers*/

    /* Use masked access to read current value */
    read_value       = CMSDK_GPIO->LB_MASKED[i];
    expected_value   = current_value & i;
    if ((read_value & gpio_datamask) != (expected_value  & gpio_datamask)) { err_code |= (1<<7);}

    /* Use normal access to read current value */
    read_value       = CMSDK_GPIO->DATA;
    expected_value   = current_value;
    if ((read_value & gpio_datamask) != (expected_value  & gpio_datamask)) { err_code |= (1<<8);}

    /* Use half word read access to carry out mask access */
    read_value       = HW16_REG(gpio_byte_base + 4 * i);
    expected_value   = current_value & i;
    if ((read_value & gpio_datamask) != (expected_value  & gpio_datamask)) { err_code |= (1<<9);}

    read_value       = HW16_REG(gpio_byte_base + 4 * i + 2);
    expected_value   = 0;
    if ((read_value                ) != (expected_value                 )) { err_code |= (1<<10);}

    /* Use byte read access to carry out mask access */
    read_value       = HW8_REG(gpio_byte_base + 4 * i);
    expected_value   = current_value & i;
    if ((read_value & gpio_datamask) != (expected_value  & gpio_datamask)) { err_code |= (1<<11);}

    read_value       = HW8_REG(gpio_byte_base + 4 * i + 1);
    expected_value   = 0;
    if ((read_value                ) != (expected_value                 )) { err_code |= (1<<12);}

    read_value       = HW8_REG(gpio_byte_base + 4 * i + 2);
    expected_value   = 0;
    if ((read_value                ) != (expected_value                 )) { err_code |= (1<<13);}

    read_value       = HW8_REG(gpio_byte_base + 4 * i + 3);
    expected_value   = 0;
    if ((read_value                ) != (expected_value                 )) { err_code |= (1<<14);}

    /* Use half word masked access to toggle current value, check it with word access */
    write_value               = ~CMSDK_GPIO->LB_MASKED[i];
    HW16_REG(gpio_byte_base + 4 * i) = write_value;
    current_value             = (current_value & (~i)) | (write_value & i);
    small_delay(); /* DATA read back take time to get updated due to synchronisation registers*/

    /* Use masked access to read current value */
    read_value       = CMSDK_GPIO->LB_MASKED[i];
    expected_value   = current_value & i;
    if ((read_value & gpio_datamask) != (expected_value  & gpio_datamask)) { err_code |= (1<<15);}

    /* Use normal access to read current value */
    read_value       = CMSDK_GPIO->DATA;
    expected_value   = current_value;
    if ((read_value & gpio_datamask) != (expected_value  & gpio_datamask)) { err_code |= (1<<16);}

    /* Use half word access to write to upper 16-bit, should have no change in result */
    write_value               = ~CMSDK_GPIO->LB_MASKED[i];
    HW16_REG(gpio_byte_base + 4 * i + 2) = write_value;
    small_delay(); /* DATA read back take time to get updated due to synchronisation registers*/
    read_value       = CMSDK_GPIO->DATA;
    expected_value   = current_value;
    if ((read_value & gpio_datamask) != (expected_value  & gpio_datamask)) { err_code |= (1<<17);}

    /* Use half word read access to carry out mask access */
    read_value       = HW16_REG(gpio_byte_base + 4 * i);
    expected_value   = current_value & i;
    if ((read_value & gpio_datamask) != (expected_value  & gpio_datamask)) { err_code |= (1<<18);}

    read_value       = HW16_REG(gpio_byte_base + 4 * i + 2);
    expected_value   = 0;
    if ((read_value                ) != (expected_value                 )) { err_code |= (1<<19);}

    /* Use byte read access to carry out mask access */
    read_value       = HW8_REG(gpio_byte_base + 4 * i);
    expected_value   = current_value & i;
    if ((read_value & gpio_datamask) != (expected_value  & gpio_datamask)) { err_code |= (1<<20);}

    read_value       = HW8_REG(gpio_byte_base + 4 * i + 1);
    expected_value   = 0;
    if ((read_value                ) != (expected_value                 )) { err_code |= (1<<21);}

    read_value       = HW8_REG(gpio_byte_base + 4 * i + 2);
    expected_value   = 0;
    if ((read_value                ) != (expected_value                 )) { err_code |= (1<<22);}

    read_value       = HW8_REG(gpio_byte_base + 4 * i + 3);
    expected_value   = 0;
    if ((read_value                ) != (expected_value                 )) { err_code |= (1<<23);}

    /* Use byte masked access to toggle current value, check it with word access */
    write_value               = ~CMSDK_GPIO->LB_MASKED[i];
    HW8_REG(gpio_byte_base + 4 * i) = write_value;
    current_value             = (current_value & (~i)) | (write_value & i);
    small_delay(); /* DATA read back take time to get updated due to synchronisation registers*/

    /* Use normal access to read current value */
    read_value       = CMSDK_GPIO->DATA;
    expected_value   = current_value;
    if ((read_value & gpio_datamask) != (expected_value  & gpio_datamask)) { err_code |= (1<<24);}

    /* Write to second byte, should have no effect */
    write_value               = ~CMSDK_GPIO->LB_MASKED[i];
    HW8_REG(gpio_byte_base + 4 * i + 1) = write_value;
    small_delay(); /* DATA read back take time to get updated due to synchronisation registers*/

    /* Use normal access to read current value */
    read_value       = CMSDK_GPIO->DATA;
    expected_value   = current_value;
    if ((read_value & gpio_datamask) != (expected_value  & gpio_datamask)) { err_code |= (1<<25);}

    /* Use masked access to read current value */
    read_value       = CMSDK_GPIO->LB_MASKED[i];
    expected_value   = current_value & i;
    if ((read_value & gpio_datamask) != (expected_value  & gpio_datamask)) { err_code |= (1<<26);}

    /* Use half word read access to carry out mask access */
    read_value       = HW16_REG(gpio_byte_base + 4 * i);
    expected_value   = current_value & i;
    if ((read_value & gpio_datamask) != (expected_value  & gpio_datamask)) { err_code |= (1<<27);}

    read_value       = HW16_REG(gpio_byte_base + 4 * i + 2);
    expected_value   = 0;
    if ((read_value                ) != (expected_value                 )) { err_code |= (1<<28);}

    /* Use byte read access to carry out mask access */
    read_value       = HW8_REG(gpio_byte_base + 4 * i);
    expected_value   = current_value & i;
    if ((read_value & gpio_datamask) != (expected_value  & gpio_datamask)) { err_code |= (1<<29);}

    read_value       = HW8_REG(gpio_byte_base + 4 * i + 1);
    expected_value   = 0;
    if ((read_value                ) != (expected_value                 )) { err_code |= (1<<30);}

    read_value       = HW8_REG(gpio_byte_base + 4 * i + 2);
    expected_value   = 0;
    if ((read_value                ) != (expected_value                 )) { err_code |= 0x80000000UL;}

    if (err_code != 0) {
      printf ("ERROR : LB masked access failed (0x%x) at mask = %d\n", err_code, i);
      return_val =1;
      err_code = 0;
      }else test_counter++;
    }

    if(test_counter == 256){
      puts("Lower Byte Mask Access Tests Passed\n");
    }
    else{
      puts("Lower Byte Mask Access Tests Failed\n");
      ul_err = 1;
    }

  test_counter = 0;

  puts("- upper byte");
  gpio_byte_base       = gpio_byte_base + 0x400;

  for (i=0;i<256;i++){
    /* Use masked access to read current value */
    read_value       = CMSDK_GPIO->UB_MASKED[i];
    expected_value   = current_value & (i<<8);
    if ((read_value & gpio_datamask) != (expected_value  & gpio_datamask)) { err_code |= (1<<0);}

    /* Use half word read access to carry out mask access */
    read_value       = HW16_REG(gpio_byte_base + 4 * i);
    expected_value   = current_value & (i<<8);
    if ((read_value & gpio_datamask) != (expected_value  & gpio_datamask)) { err_code |= (1<<1);}

    read_value       = HW16_REG(gpio_byte_base + 4 * i + 2);
    expected_value   = 0;
    if ((read_value                ) != (expected_value                 )) { err_code |= (1<<2);}

    /* Use byte read access to carry out mask access */
    read_value       = HW8_REG(gpio_byte_base + 4 * i);
    expected_value   = 0;
    if ((read_value                ) != (expected_value                 )) { err_code |= (1<<4);}

    read_value       = HW8_REG(gpio_byte_base + 4 * i + 1);
    expected_value   = (current_value>>8) & i;
    if ((read_value & gpio_datamask) != (expected_value  & gpio_datamask)) { err_code |= (1<<3);}

    read_value       = HW8_REG(gpio_byte_base + 4 * i + 2);
    expected_value   = 0;
    if ((read_value                ) != (expected_value                 )) { err_code |= (1<<5);}

    read_value       = HW8_REG(gpio_byte_base + 4 * i + 3);
    expected_value   = 0;
    if ((read_value                ) != (expected_value                 )) { err_code |= (1<<6);}

    /* Use masked access to toggle current value, check it with word access */
    write_value               = ~CMSDK_GPIO->UB_MASKED[i];
    CMSDK_GPIO->UB_MASKED[i] = write_value;
    current_value             = (current_value & (~(i<<8))) | (write_value & (i<<8));
    small_delay(); /* DATA read back take time to get updated due to synchronisation registers*/

    /* Use masked access to read current value */
    read_value       = CMSDK_GPIO->UB_MASKED[i];
    expected_value   = current_value & (i<<8);
    if ((read_value & gpio_datamask) != (expected_value  & gpio_datamask)) { err_code |= (1<<7);}

    /* Use normal access to read current value */
    read_value       = CMSDK_GPIO->DATA;
    expected_value   = current_value;
    if ((read_value & gpio_datamask) != (expected_value  & gpio_datamask)) { err_code |= (1<<8);}

    /* Use half word read access to carry out mask access */
    read_value       = HW16_REG(gpio_byte_base + 4 * i);
    expected_value   = current_value & (i<<8);
    if ((read_value & gpio_datamask) != (expected_value  & gpio_datamask)) { err_code |= (1<<9);}

    read_value       = HW16_REG(gpio_byte_base + 4 * i + 2);
    expected_value   = 0;
    if ((read_value                ) != (expected_value                 )) { err_code |= (1<<10);}

    /* Use byte read access to carry out mask access */
    read_value       = HW8_REG(gpio_byte_base + 4 * i);
    expected_value   = 0;
    if ((read_value                ) != (expected_value                 )) { err_code |= (1<<11);}

    read_value       = HW8_REG(gpio_byte_base + 4 * i + 1);
    expected_value   = (current_value>>8) & i;
    if ((read_value & gpio_datamask) != (expected_value  & gpio_datamask)) { err_code |= (1<<12);}

    read_value       = HW8_REG(gpio_byte_base + 4 * i + 2);
    expected_value   = 0;
    if ((read_value                ) != (expected_value                 )) { err_code |= (1<<13);}

    read_value       = HW8_REG(gpio_byte_base + 4 * i + 3);
    expected_value   = 0;
    if ((read_value                ) != (expected_value                 )) { err_code |= (1<<14);}

    /* Use half word masked access to toggle current value, check it with word access */
    write_value               = ~CMSDK_GPIO->UB_MASKED[i];
    HW16_REG(gpio_byte_base + 4 * i) = write_value;
    current_value             = (current_value & (~(i<<8))) | (write_value & (i<<8));
    small_delay(); /* DATA read back take time to get updated due to synchronisation registers*/

    /* Use masked access to read current value */
    read_value       = CMSDK_GPIO->UB_MASKED[i];
    expected_value   = current_value & (i<<8);
    if ((read_value & gpio_datamask) != (expected_value  & gpio_datamask)) { err_code |= (1<<15);}

    /* Use normal access to read current value */
    read_value       = CMSDK_GPIO->DATA;
    expected_value   = current_value;
    if ((read_value & gpio_datamask) != (expected_value  & gpio_datamask)) { err_code |= (1<<16);}

    /* Use half word access to write to upper 16-bit, should have no change in result */
    write_value               = ~HW16_REG(gpio_byte_base + 4 * i + 2);
    HW16_REG(gpio_byte_base + 4 * i + 2) = write_value;
    small_delay(); /* DATA read back take time to get updated due to synchronisation registers*/
    read_value       = CMSDK_GPIO->DATA;
    expected_value   = current_value;
    if ((read_value & gpio_datamask) != (expected_value  & gpio_datamask)) { err_code |= (1<<17);}

    /* Use half word read access to carry out mask access */
    read_value       = HW16_REG(gpio_byte_base + 4 * i);
    expected_value   = current_value & (i<<8);
    if ((read_value & gpio_datamask) != (expected_value  & gpio_datamask)) { err_code |= (1<<18);}

    read_value       = HW16_REG(gpio_byte_base + 4 * i + 2);
    expected_value   = 0;
    if ((read_value                ) != (expected_value                 )) { err_code |= (1<<19);}

    /* Use byte read access to carry out mask access */
    read_value       = HW8_REG(gpio_byte_base + 4 * i);
    expected_value   = 0;
    if ((read_value                ) != (expected_value                 )) { err_code |= (1<<20);}

    read_value       = HW8_REG(gpio_byte_base + 4 * i + 1);
    expected_value   = (current_value>>8) & i;
    if ((read_value & gpio_datamask) != (expected_value  & gpio_datamask)) { err_code |= (1<<21);}

    read_value       = HW8_REG(gpio_byte_base + 4 * i + 2);
    expected_value   = 0;
    if ((read_value                ) != (expected_value                 )) { err_code |= (1<<22);}

    read_value       = HW8_REG(gpio_byte_base + 4 * i + 3);
    expected_value   = 0;
    if ((read_value                ) != (expected_value                 )) { err_code |= (1<<23);}

    /* Use byte masked access to toggle current value, check it with word access */
    write_value               = ~CMSDK_GPIO->UB_MASKED[i];
    HW8_REG(gpio_byte_base + 4 * i + 1) = (write_value>>8) & 0xFF;
    current_value             = (current_value & (~(i<<8))) | (write_value & (i<<8));
    small_delay(); /* DATA read back take time to get updated due to synchronisation registers*/

    /* Use normal access to read current value */
    read_value       = CMSDK_GPIO->DATA;
    expected_value   = current_value;
    if ((read_value & gpio_datamask) != (expected_value  & gpio_datamask)) { err_code |= (1<<24);}

    /* Write to lower byte, should have no effect */
    write_value               = ~CMSDK_GPIO->UB_MASKED[i];
    HW8_REG(gpio_byte_base + 4 * i ) = (write_value) & 0xFF;
    small_delay(); /* DATA read back take time to get updated due to synchronisation registers*/

    /* Use normal access to read current value */
    read_value       = CMSDK_GPIO->DATA;
    expected_value   = current_value;
    if ((read_value & gpio_datamask) != (expected_value  & gpio_datamask)) { err_code |= (1<<25);}

    /* Use masked access to read current value */
    read_value       = CMSDK_GPIO->UB_MASKED[i];
    expected_value   = current_value & (i<<8);
    if ((read_value & gpio_datamask) != (expected_value  & gpio_datamask)) { err_code |= (1<<26);}

    /* Use half word read access to carry out mask access */
    read_value       = HW16_REG(gpio_byte_base + 4 * i);
    expected_value   = current_value & (i<<8);
    if ((read_value & gpio_datamask) != (expected_value  & gpio_datamask)) { err_code |= (1<<27);}

    read_value       = HW16_REG(gpio_byte_base + 4 * i + 2);
    expected_value   = 0;
    if ((read_value                ) != (expected_value                 )) { err_code |= (1<<28);}

    /* Use byte read access to carry out mask access */
    read_value       = HW8_REG(gpio_byte_base + 4 * i);
    expected_value   = 0;
    if ((read_value                ) != (expected_value                 )) { err_code |= (1<<29);}

    read_value       = HW8_REG(gpio_byte_base + 4 * i + 1);
    expected_value   = (current_value>>8) & i;
    if ((read_value & gpio_datamask) != (expected_value  & gpio_datamask)) { err_code |= (1<<30);}

    read_value       = HW8_REG(gpio_byte_base + 4 * i + 2);
    expected_value   = 0;
    if ((read_value                ) != (expected_value                 )) { err_code |= (0x80000000UL);}



    if (err_code != 0) {
      printf ("ERROR : UB masked access failed (0x%x) at mask = %d\n", err_code, i);
      return_val =1;
      err_code = 0;
      UartEndSimulation();
      }
      else test_counter++;
    }

    if(test_counter == 256){
      puts("Upper Byte Mask Access Tests Passed\n");
    }
    else{
      puts("Upper Byte Mask Access Tests Failed\n");
      ul_err |= 2;
      }

    if(ul_err) printf("Mask Access Tests Failed, Error Code: (0x%x)\n", ul_err);
    else puts("Mask Access Tests Passed\n");

  /* Clean up */
  CMSDK_GPIO->OUTENABLECLR = 0xFFFFFFFF;
  CMSDK_GPIO->DATAOUT = 0;

  return(return_val);
}



/* --------------------------------------------------------------- */
/*  GPIO register read/write tests                                */
/* --------------------------------------------------------------- */

int gpio_register_rw_test(CMSDK_GPIO_TypeDef *CMSDK_GPIO){
  int return_val=0;
  unsigned int err_code=0;
  int i;
  int test_individual_irq = 1;  /* Test individual IRQ in GPIO #0 by default */

  unsigned int gpio_datamask;   /* Data compare mask */
  unsigned int gpio_outen_mask; /* Output enable mask */
  unsigned int gpio_altfunc_initval;
  unsigned int gpio_altfunc_mask;
  unsigned int gpio_altfunc_usedmask;
  unsigned int gpio_base;
  unsigned int saved_altfunc;


  if (CMSDK_GPIO==CMSDK_GPIO0) {
    saved_altfunc = CMSDK_GPIO0->ALTFUNCSET; /* Keep a copy of current ALTFUNC */
    /* GPIO #0 is not shared in example system */
    gpio_datamask          = 0xFFFFFFFF;
    gpio_outen_mask        = 0xFFFFFFFF;
    gpio_altfunc_mask      = GPIO0_ALTERNATE_FUNC_MASK; /* Mask bits that are not implemented */
    gpio_altfunc_initval   = GPIO0_ALTERNATE_FUNC_DEFAULT;
    gpio_altfunc_usedmask  = 0xFFFFFFFF;
    gpio_base              = CMSDK_GPIO0_BASE;
    }
  else if (CMSDK_GPIO==CMSDK_GPIO1) {
    saved_altfunc = CMSDK_GPIO1->ALTFUNCSET; /* Keep a copy of current ALTFUNC */
    /* GPIO #1 is shared with peripherals in example system */
    /* bit 5 is used by stdout                             */
    gpio_datamask         = ~((1<<5)|(1<<2)|(1<<0)); /* Ignore return values for these bits */
    gpio_outen_mask       = ~((1<<2)|(1<<0)); /* Do not set output enable for these bits */
    gpio_altfunc_mask     = GPIO1_ALTERNATE_FUNC_MASK; /* Mask bits that are not implemented */
    gpio_altfunc_initval  = GPIO1_ALTERNATE_FUNC_DEFAULT;
    gpio_altfunc_usedmask = ~(1<<5); /* This bit is use by stdout */
    gpio_base             = CMSDK_GPIO1_BASE;
    }
  else {
    puts ("ERROR: Input parameter invalid in function 'gpio_register_rw_test'.");
    return 1;
    }

  puts("GPIO R/W test");
  puts("- initial values");

  NVIC_DisableIRQ(PORT0_ALL_IRQn);
  NVIC_DisableIRQ(PORT1_ALL_IRQn);
  for (i=0;i<16;i++){
    NVIC_DisableIRQ(((IRQn_Type)(((int)PORT0_0_IRQn) + i)));
    }

  /* Test initial values. */
  /* Current data value could be affected by external signal */
  if (CMSDK_GPIO==CMSDK_GPIO0) {
    /* By default GPIO #0 signals are pulled up */
    if (CMSDK_GPIO->DATA      != 0x0000FFFF) {err_code |= (1<<0); }
    }

  if (CMSDK_GPIO->DATAOUT   != 0x00000000) {err_code |= (1<<1); }
  if (CMSDK_GPIO->OUTENABLESET != 0x00000000) {err_code |= (1<<2); }

  /* Alternate function initial value can be affected by
     1) Verilog parameter
     2) Retargeting initialization code */
    /* GPIO #1 is shared with peripherals in example system */
    /* bit 5 is used by stdout                             */
  if ((CMSDK_GPIO->ALTFUNCSET & gpio_altfunc_usedmask) != (gpio_altfunc_initval & gpio_altfunc_usedmask))
    {err_code |= (1<<3); }

  if (CMSDK_GPIO->INTENSET     != 0x00000000) {err_code |= (1<<4); }
  if (CMSDK_GPIO->INTTYPESET   != 0x00000000) {err_code |= (1<<5); }
  if (CMSDK_GPIO->INTPOLSET    != 0x00000000) {err_code |= (1<<6); }
  if (CMSDK_GPIO->INTSTATUS    != 0x00000000) {err_code |= (1<<7); }
  puts ("- check ID values (WORD accesses)");
  if (HW32_REG(gpio_base + 0xFD0) != AHB_GPIO_PID4) {err_code |= (1<<8); }
  if (HW32_REG(gpio_base + 0xFD4) != AHB_GPIO_PID5) {err_code |= (1<<9); }
  if (HW32_REG(gpio_base + 0xFD8) != AHB_GPIO_PID6) {err_code |= (1<<10); }
  if (HW32_REG(gpio_base + 0xFDC) != AHB_GPIO_PID7) {err_code |= (1<<11); }
  if (HW32_REG(gpio_base + 0xFE0) != AHB_GPIO_PID0) {err_code |= (1<<12); }
  if (HW32_REG(gpio_base + 0xFE4) != AHB_GPIO_PID1) {err_code |= (1<<13); }
  if (HW32_REG(gpio_base + 0xFE8) != AHB_GPIO_PID2) {err_code |= (1<<14); }
  if (HW32_REG(gpio_base + 0xFEC) != AHB_GPIO_PID3) {err_code |= (1<<15); }
  if (HW32_REG(gpio_base + 0xFF0) != AHB_GPIO_CID0) {err_code |= (1<<16); }
  if (HW32_REG(gpio_base + 0xFF4) != AHB_GPIO_CID1) {err_code |= (1<<17); }
  if (HW32_REG(gpio_base + 0xFF8) != AHB_GPIO_CID2) {err_code |= (1<<18); }
  if (HW32_REG(gpio_base + 0xFFC) != AHB_GPIO_CID3) {err_code |= (1<<19); }

  /* test write to PIDs and CIDs - should be ignored */
  for (i=0; i <12; i++) {
    HW8_REG(gpio_base + 0xFD0 + (i<<2)) = ~HW32_REG(gpio_base + 0xFD0 + (i<<2));
    }

  /* Check read back values again, should not be changed */
  if (HW16_REG(gpio_base + 0xFD0) != AHB_GPIO_PID4) {err_code |= (1<<8); }
  if (HW16_REG(gpio_base + 0xFD4) != AHB_GPIO_PID5) {err_code |= (1<<9); }
  if (HW16_REG(gpio_base + 0xFD8) != AHB_GPIO_PID6) {err_code |= (1<<10); }
  if (HW16_REG(gpio_base + 0xFDC) != AHB_GPIO_PID7) {err_code |= (1<<11); }
  if (HW16_REG(gpio_base + 0xFE0) != AHB_GPIO_PID0) {err_code |= (1<<12); }
  if (HW16_REG(gpio_base + 0xFE4) != AHB_GPIO_PID1) {err_code |= (1<<13); }
  if (HW16_REG(gpio_base + 0xFE8) != AHB_GPIO_PID2) {err_code |= (1<<14); }
  if (HW16_REG(gpio_base + 0xFEC) != AHB_GPIO_PID3) {err_code |= (1<<15); }
  if (HW16_REG(gpio_base + 0xFF0) != AHB_GPIO_CID0) {err_code |= (1<<16); }
  if (HW16_REG(gpio_base + 0xFF4) != AHB_GPIO_CID1) {err_code |= (1<<17); }
  if (HW16_REG(gpio_base + 0xFF8) != AHB_GPIO_CID2) {err_code |= (1<<18); }
  if (HW16_REG(gpio_base + 0xFFC) != AHB_GPIO_CID3) {err_code |= (1<<19); }

  if (err_code != 0) {
    printf ("ERROR : initial value failed (0x%x)\n", err_code);
    return_val=1;
    err_code = 0;
    }


  puts ("- write ID values (HALF WORD accesses, lower) - should have no effect");
  HW16_REG(gpio_base + 0xFD0) = 0xFFFF;
  HW16_REG(gpio_base + 0xFD4) = 0xFFFF;
  HW16_REG(gpio_base + 0xFD8) = 0xFFFF;
  HW16_REG(gpio_base + 0xFDC) = 0xFFFF;
  HW16_REG(gpio_base + 0xFE0) = 0xFFFF;
  HW16_REG(gpio_base + 0xFE4) = 0xFFFF;
  HW16_REG(gpio_base + 0xFE8) = 0xFFFF;
  HW16_REG(gpio_base + 0xFEC) = 0xFFFF;
  HW16_REG(gpio_base + 0xFF0) = 0xFFFF;
  HW16_REG(gpio_base + 0xFF4) = 0xFFFF;
  HW16_REG(gpio_base + 0xFF8) = 0xFFFF;
  HW16_REG(gpio_base + 0xFFC) = 0xFFFF;


  puts ("- check ID values (HALF WORD accesses, lower)");
  if (HW16_REG(gpio_base + 0xFD0) != AHB_GPIO_PID4) {err_code |= (1<<0); }
  if (HW16_REG(gpio_base + 0xFD4) != AHB_GPIO_PID5) {err_code |= (1<<1); }
  if (HW16_REG(gpio_base + 0xFD8) != AHB_GPIO_PID6) {err_code |= (1<<2); }
  if (HW16_REG(gpio_base + 0xFDC) != AHB_GPIO_PID7) {err_code |= (1<<3); }
  if (HW16_REG(gpio_base + 0xFE0) != AHB_GPIO_PID0) {err_code |= (1<<4); }
  if (HW16_REG(gpio_base + 0xFE4) != AHB_GPIO_PID1) {err_code |= (1<<5); }
  if (HW16_REG(gpio_base + 0xFE8) != AHB_GPIO_PID2) {err_code |= (1<<6); }
  if (HW16_REG(gpio_base + 0xFEC) != AHB_GPIO_PID3) {err_code |= (1<<7); }
  if (HW16_REG(gpio_base + 0xFF0) != AHB_GPIO_CID0) {err_code |= (1<<8); }
  if (HW16_REG(gpio_base + 0xFF4) != AHB_GPIO_CID1) {err_code |= (1<<9); }
  if (HW16_REG(gpio_base + 0xFF8) != AHB_GPIO_CID2) {err_code |= (1<<10); }
  if (HW16_REG(gpio_base + 0xFFC) != AHB_GPIO_CID3) {err_code |= (1<<11); }


  puts ("- write ID values (HALF WORD accesses, upper) - should have no effect");
  HW16_REG(gpio_base + 0xFD2) = 0xFFFF;
  HW16_REG(gpio_base + 0xFD6) = 0xFFFF;
  HW16_REG(gpio_base + 0xFDA) = 0xFFFF;
  HW16_REG(gpio_base + 0xFDE) = 0xFFFF;
  HW16_REG(gpio_base + 0xFE2) = 0xFFFF;
  HW16_REG(gpio_base + 0xFE6) = 0xFFFF;
  HW16_REG(gpio_base + 0xFEA) = 0xFFFF;
  HW16_REG(gpio_base + 0xFEE) = 0xFFFF;
  HW16_REG(gpio_base + 0xFF2) = 0xFFFF;
  HW16_REG(gpio_base + 0xFF6) = 0xFFFF;
  HW16_REG(gpio_base + 0xFFA) = 0xFFFF;
  HW16_REG(gpio_base + 0xFFE) = 0xFFFF;

  puts ("- check ID values (HALF WORD accesses, upper)");
  if (HW16_REG(gpio_base + 0xFD2) != 0x00) {err_code |= (1<<12); }
  if (HW16_REG(gpio_base + 0xFD6) != 0x00) {err_code |= (1<<13); }
  if (HW16_REG(gpio_base + 0xFDA) != 0x00) {err_code |= (1<<14); }
  if (HW16_REG(gpio_base + 0xFDE) != 0x00) {err_code |= (1<<15); }
  if (HW16_REG(gpio_base + 0xFE2) != 0x00) {err_code |= (1<<16); }
  if (HW16_REG(gpio_base + 0xFE6) != 0x00) {err_code |= (1<<17); }
  if (HW16_REG(gpio_base + 0xFEA) != 0x00) {err_code |= (1<<18); }
  if (HW16_REG(gpio_base + 0xFEE) != 0x00) {err_code |= (1<<19); }
  if (HW16_REG(gpio_base + 0xFF2) != 0x00) {err_code |= (1<<20); }
  if (HW16_REG(gpio_base + 0xFF6) != 0x00) {err_code |= (1<<21); }
  if (HW16_REG(gpio_base + 0xFFA) != 0x00) {err_code |= (1<<22); }
  if (HW16_REG(gpio_base + 0xFFE) != 0x00) {err_code |= (1<<23); }

  if (err_code != 0) {
    printf ("ERROR : initial value failed (0x%x)\n", err_code);
    return_val=1;
    err_code = 0;
    }

  puts ("- write ID values (BYTE accesses, byte 0) - should have no effect");
  HW8_REG(gpio_base + 0xFD0) = 0xFF;
  HW8_REG(gpio_base + 0xFD4) = 0xFF;
  HW8_REG(gpio_base + 0xFD8) = 0xFF;
  HW8_REG(gpio_base + 0xFDC) = 0xFF;
  HW8_REG(gpio_base + 0xFE0) = 0xFF;
  HW8_REG(gpio_base + 0xFE4) = 0xFF;
  HW8_REG(gpio_base + 0xFE8) = 0xFF;
  HW8_REG(gpio_base + 0xFEC) = 0xFF;
  HW8_REG(gpio_base + 0xFF0) = 0xFF;
  HW8_REG(gpio_base + 0xFF4) = 0xFF;
  HW8_REG(gpio_base + 0xFF8) = 0xFF;
  HW8_REG(gpio_base + 0xFFC) = 0xFF;

  puts ("- check ID values (BYTE accesses, byte 0)");
  if (HW8_REG(gpio_base + 0xFD0) != AHB_GPIO_PID4) {err_code |= (1<<0); }
  if (HW8_REG(gpio_base + 0xFD4) != AHB_GPIO_PID5) {err_code |= (1<<1); }
  if (HW8_REG(gpio_base + 0xFD8) != AHB_GPIO_PID6) {err_code |= (1<<2); }
  if (HW8_REG(gpio_base + 0xFDC) != AHB_GPIO_PID7) {err_code |= (1<<3); }
  if (HW8_REG(gpio_base + 0xFE0) != AHB_GPIO_PID0) {err_code |= (1<<4); }
  if (HW8_REG(gpio_base + 0xFE4) != AHB_GPIO_PID1) {err_code |= (1<<5); }
  if (HW8_REG(gpio_base + 0xFE8) != AHB_GPIO_PID2) {err_code |= (1<<6); }
  if (HW8_REG(gpio_base + 0xFEC) != AHB_GPIO_PID3) {err_code |= (1<<7); }
  if (HW8_REG(gpio_base + 0xFF0) != AHB_GPIO_CID0) {err_code |= (1<<8); }
  if (HW8_REG(gpio_base + 0xFF4) != AHB_GPIO_CID1) {err_code |= (1<<9); }
  if (HW8_REG(gpio_base + 0xFF8) != AHB_GPIO_CID2) {err_code |= (1<<10); }
  if (HW8_REG(gpio_base + 0xFFC) != AHB_GPIO_CID3) {err_code |= (1<<11); }

  puts ("- write ID values (BYTE accesses, byte 1) - should have no effect");
  HW8_REG(gpio_base + 0xFD1) = 0xFF;
  HW8_REG(gpio_base + 0xFD5) = 0xFF;
  HW8_REG(gpio_base + 0xFD9) = 0xFF;
  HW8_REG(gpio_base + 0xFDD) = 0xFF;
  HW8_REG(gpio_base + 0xFE1) = 0xFF;
  HW8_REG(gpio_base + 0xFE5) = 0xFF;
  HW8_REG(gpio_base + 0xFE9) = 0xFF;
  HW8_REG(gpio_base + 0xFED) = 0xFF;
  HW8_REG(gpio_base + 0xFF1) = 0xFF;
  HW8_REG(gpio_base + 0xFF5) = 0xFF;
  HW8_REG(gpio_base + 0xFF9) = 0xFF;
  HW8_REG(gpio_base + 0xFFD) = 0xFF;

  puts ("- check ID values (BYTE accesses, byte 1)");
  if (HW8_REG(gpio_base + 0xFD1) != 0x00) {err_code |= (1<<12); }
  if (HW8_REG(gpio_base + 0xFD5) != 0x00) {err_code |= (1<<13); }
  if (HW8_REG(gpio_base + 0xFD9) != 0x00) {err_code |= (1<<14); }
  if (HW8_REG(gpio_base + 0xFDD) != 0x00) {err_code |= (1<<15); }
  if (HW8_REG(gpio_base + 0xFE1) != 0x00) {err_code |= (1<<16); }
  if (HW8_REG(gpio_base + 0xFE5) != 0x00) {err_code |= (1<<17); }
  if (HW8_REG(gpio_base + 0xFE9) != 0x00) {err_code |= (1<<18); }
  if (HW8_REG(gpio_base + 0xFED) != 0x00) {err_code |= (1<<19); }
  if (HW8_REG(gpio_base + 0xFF1) != 0x00) {err_code |= (1<<20); }
  if (HW8_REG(gpio_base + 0xFF5) != 0x00) {err_code |= (1<<21); }
  if (HW8_REG(gpio_base + 0xFF9) != 0x00) {err_code |= (1<<22); }
  if (HW8_REG(gpio_base + 0xFFD) != 0x00) {err_code |= (1<<23); }

  if (err_code != 0) {
    printf ("ERROR : initial value failed on byte 0 and 1 accesses (0x%x)\n", err_code);
    return_val=1;
    err_code = 0;
    }

  puts ("- write ID values (BYTE accesses, byte 2) - should have no effect");
  HW8_REG(gpio_base + 0xFD2) = 0xFF;
  HW8_REG(gpio_base + 0xFD6) = 0xFF;
  HW8_REG(gpio_base + 0xFDA) = 0xFF;
  HW8_REG(gpio_base + 0xFDE) = 0xFF;
  HW8_REG(gpio_base + 0xFE2) = 0xFF;
  HW8_REG(gpio_base + 0xFE6) = 0xFF;
  HW8_REG(gpio_base + 0xFEA) = 0xFF;
  HW8_REG(gpio_base + 0xFEE) = 0xFF;
  HW8_REG(gpio_base + 0xFF2) = 0xFF;
  HW8_REG(gpio_base + 0xFF6) = 0xFF;
  HW8_REG(gpio_base + 0xFFA) = 0xFF;
  HW8_REG(gpio_base + 0xFFE) = 0xFF;

  puts ("- check ID values (BYTE accesses, byte 2)");
  if (HW8_REG(gpio_base + 0xFD2) != 0x00) {err_code |= (1<<0); }
  if (HW8_REG(gpio_base + 0xFD6) != 0x00) {err_code |= (1<<1); }
  if (HW8_REG(gpio_base + 0xFDA) != 0x00) {err_code |= (1<<2); }
  if (HW8_REG(gpio_base + 0xFDE) != 0x00) {err_code |= (1<<3); }
  if (HW8_REG(gpio_base + 0xFE2) != 0x00) {err_code |= (1<<4); }
  if (HW8_REG(gpio_base + 0xFE6) != 0x00) {err_code |= (1<<5); }
  if (HW8_REG(gpio_base + 0xFEA) != 0x00) {err_code |= (1<<6); }
  if (HW8_REG(gpio_base + 0xFEE) != 0x00) {err_code |= (1<<7); }
  if (HW8_REG(gpio_base + 0xFF2) != 0x00) {err_code |= (1<<8); }
  if (HW8_REG(gpio_base + 0xFF6) != 0x00) {err_code |= (1<<9); }
  if (HW8_REG(gpio_base + 0xFFA) != 0x00) {err_code |= (1<<10); }
  if (HW8_REG(gpio_base + 0xFFE) != 0x00) {err_code |= (1<<11); }

  puts ("- write ID values (BYTE accesses, byte 3) - should have no effect");
  /* test write to PIDs and CIDs - should be ignored */
  HW8_REG(gpio_base + 0xFD3) = 0xFF;
  HW8_REG(gpio_base + 0xFD7) = 0xFF;
  HW8_REG(gpio_base + 0xFDB) = 0xFF;
  HW8_REG(gpio_base + 0xFDF) = 0xFF;
  HW8_REG(gpio_base + 0xFE3) = 0xFF;
  HW8_REG(gpio_base + 0xFE7) = 0xFF;
  HW8_REG(gpio_base + 0xFEB) = 0xFF;
  HW8_REG(gpio_base + 0xFEF) = 0xFF;
  HW8_REG(gpio_base + 0xFF3) = 0xFF;
  HW8_REG(gpio_base + 0xFF7) = 0xFF;
  HW8_REG(gpio_base + 0xFFB) = 0xFF;
  HW8_REG(gpio_base + 0xFFF) = 0xFF;

  puts ("- check ID values (BYTE accesses, byte 3)");
  if (HW8_REG(gpio_base + 0xFD3) != 0x00) {err_code |= (1<<12); }
  if (HW8_REG(gpio_base + 0xFD7) != 0x00) {err_code |= (1<<13); }
  if (HW8_REG(gpio_base + 0xFDB) != 0x00) {err_code |= (1<<14); }
  if (HW8_REG(gpio_base + 0xFDF) != 0x00) {err_code |= (1<<15); }
  if (HW8_REG(gpio_base + 0xFE3) != 0x00) {err_code |= (1<<16); }
  if (HW8_REG(gpio_base + 0xFE7) != 0x00) {err_code |= (1<<17); }
  if (HW8_REG(gpio_base + 0xFEB) != 0x00) {err_code |= (1<<18); }
  if (HW8_REG(gpio_base + 0xFEF) != 0x00) {err_code |= (1<<19); }
  if (HW8_REG(gpio_base + 0xFF3) != 0x00) {err_code |= (1<<20); }
  if (HW8_REG(gpio_base + 0xFF7) != 0x00) {err_code |= (1<<21); }
  if (HW8_REG(gpio_base + 0xFFB) != 0x00) {err_code |= (1<<22); }
  if (HW8_REG(gpio_base + 0xFFF) != 0x00) {err_code |= (1<<23); }

  if (err_code != 0) {
    printf ("ERROR : initial value failed on byte 2 and 3 accesses (0x%x)\n", err_code);
    return_val=1;
    err_code = 0;
    }

  puts("- read/write");

  /* Test output enable read/write */
  /* word size */
  CMSDK_GPIO->OUTENABLESET = 0xFFFFFFFF & gpio_outen_mask;
  if (CMSDK_GPIO->OUTENABLESET     != (0x0000FFFF & gpio_outen_mask)) { err_code |= (1<<0); }
  CMSDK_GPIO->OUTENABLECLR = 0xCCAA5500 & gpio_outen_mask;
  if (CMSDK_GPIO->OUTENABLESET     != (0x0000AAFF & gpio_outen_mask)) { err_code |= (1<<1); }
  CMSDK_GPIO->OUTENABLECLR = 0xFFFFFFFF & gpio_outen_mask;
  if (CMSDK_GPIO->OUTENABLESET     != (0x00000000 & gpio_outen_mask)) { err_code |= (1<<2); }

  /* half word size and byte size */
  HW16_REG(gpio_base + 0x14) = 0xFFFF & gpio_outen_mask;
  HW16_REG(gpio_base + 0x10) = 0x3366 & gpio_outen_mask;
  if (CMSDK_GPIO->OUTENABLESET     != (0x00003366 & gpio_outen_mask)) { err_code |= (1<<3); }

  HW16_REG(gpio_base + 0x12) = 0x9988 & gpio_outen_mask; /* This transfer is ignored */
  if (CMSDK_GPIO->OUTENABLESET     != (0x00003366 & gpio_outen_mask)) { err_code |= (1<<4); }
    HW16_REG(gpio_base + 0x14) = 0xFFFF & gpio_outen_mask;
  HW8_REG( gpio_base + 0x10) =   0x44 & gpio_outen_mask;
  HW8_REG( gpio_base + 0x11) =   0x33;
  if (HW16_REG(gpio_base + 0x10)  != (0x00003344 & gpio_outen_mask)) { err_code |= (1<<5); }
  HW16_REG(gpio_base + 0x10) = 0x77EE & gpio_outen_mask;
  if (HW8_REG( gpio_base + 0x10)  != (0x000000EE & gpio_outen_mask)) { err_code |= (1<<6); }
  if (HW8_REG( gpio_base + 0x11)  != (0x00000077 & (gpio_outen_mask>>8))) { err_code |= (1<<7); }

  CMSDK_GPIO->OUTENABLESET = 0x0000FFFF & gpio_outen_mask; /* Set all output enabled */
  if (CMSDK_GPIO->OUTENABLESET     != (0x0000FFFF & gpio_outen_mask)) { err_code |= (1<<8); }

  /* Now we can test read back data */
  small_delay(); /* required because the DATA readback is delay by double-flop-flop synchronisation */
  if ((CMSDK_GPIO->DATA & gpio_datamask) != (0x0000 & gpio_datamask)) {err_code |= (1<<9); }
  if ((HW16_REG( gpio_base + 0x0) & gpio_datamask) != (0x0000 & gpio_datamask)) {err_code |= (1<<10); }

  /* Test byte size write to data register, which in effect change dataout register */
  HW8_REG( gpio_base + 0x0) = 0xFF;
  if (CMSDK_GPIO->DATAOUT   != 0x00FF)     {err_code |= (1<<11); }
  small_delay(); /* required because the DATA readback is delay by double-flop-flop synchronisation */
  if ((CMSDK_GPIO->DATA & gpio_datamask) != (0x00FF & gpio_datamask)) {err_code |= (1<<12); }
  HW8_REG( gpio_base + 0x1) = 0xFF;
  if (CMSDK_GPIO->DATAOUT   != 0xFFFF)     {err_code |= (1<<13); }
  small_delay(); /* required because the DATA readback is delay by double-flop-flop synchronisation */
  if ((CMSDK_GPIO->DATA & gpio_datamask) != (0xFFFF & gpio_datamask)) {err_code |= (1<<14); }
  HW8_REG( gpio_base + 0x2) = 0xFF;
  if (CMSDK_GPIO->DATAOUT   != 0xFFFF)     {err_code |= (1<<13); }
  small_delay(); /* required because the DATA readback is delay by double-flop-flop synchronisation */
  if ((CMSDK_GPIO->DATA & gpio_datamask) != (0xFFFF & gpio_datamask)) {err_code |= (1<<15); }
  HW8_REG( gpio_base + 0x3) = 0xFF;
  if (CMSDK_GPIO->DATAOUT   != 0xFFFF)     {err_code |= (1<<14); }
  small_delay(); /* required because the DATA readback is delay by double-flop-flop synchronisation */
  if ((CMSDK_GPIO->DATA & gpio_datamask) != (0xFFFF & gpio_datamask)) {err_code |= (1<<16); }

  CMSDK_GPIO->DATAOUT = 0x1234;
  if (CMSDK_GPIO->DATAOUT   != 0x1234)      {err_code |= (1<<17); }
  if (HW8_REG( gpio_base + 0x4) != 0x34)     {err_code |= (1<<18); }
  if (HW8_REG( gpio_base + 0x5) != 0x12)     {err_code |= (1<<19); }
  if (HW16_REG(gpio_base + 0x4) != 0x1234)   {err_code |= (1<<20); }
  if ((CMSDK_GPIO->DATA & gpio_datamask) != (0x1234 & gpio_datamask)) {err_code |= (1<<21); }
  if ((HW8_REG( gpio_base + 0x0) & gpio_datamask) != (0x34 & gpio_datamask)) {err_code |= (1<<22); }
  if ((HW8_REG( gpio_base + 0x1) & (gpio_datamask>>8)) != (0x12 & (gpio_datamask>>8))) {err_code |= (1<<23); }
  if ( HW8_REG( gpio_base + 0x2) != 0x00)    {err_code |= (1<<24); }
  if ( HW8_REG( gpio_base + 0x3) != 0x00)    {err_code |= (1<<25); }
  if (HW16_REG( gpio_base + 0x6) != 0x0000)  {err_code |= (1<<26); }

  HW16_REG( gpio_base + 0x0) = 0xFFFF;
  if (CMSDK_GPIO->DATAOUT   != 0xFFFF)     {err_code |= (1<<27); }
  small_delay(); /* required because the DATA readback is delay by double-flop-flop synchronisation */
  if ((CMSDK_GPIO->DATA & gpio_datamask) != (0xFFFF & gpio_datamask)) {err_code |= (1<<28); }

  HW16_REG( gpio_base + 0x4) = 0x0000;
  if (CMSDK_GPIO->DATAOUT   != 0x0000)     {err_code |= (1<<29); }
  small_delay(); /* required because the DATA readback is delay by double-flop-flop synchronisation */
  if ((CMSDK_GPIO->DATA & gpio_datamask) != (0x0000 & gpio_datamask)) {err_code |= (1<<30); }

  CMSDK_GPIO->OUTENABLECLR = 0xFFFFFFFF; /* Disable output */

  /* Generate return value */
  if (err_code != 0) {
    printf ("ERROR : Read/write #1 failed(0x%x)\n", err_code);
    return_val=1;
    err_code = 0;
    }

  small_delay();

  /* Test ALTFUNC register */
  CMSDK_GPIO->ALTFUNCCLR = 0xFFFF;
  if ((CMSDK_GPIO->ALTFUNCSET & gpio_altfunc_mask) != (0x0000 & gpio_altfunc_mask)) {err_code |= (1<<0); }
  if ((HW16_REG(gpio_base + 0x18) & gpio_altfunc_mask) != (0x0000 & gpio_altfunc_mask)) {err_code |= (1<<1); }
  if ((HW16_REG(gpio_base + 0x1A)                    ) != (0x0000                    )) {err_code |= (1<<2); }

  CMSDK_GPIO->ALTFUNCSET = 0xFFFF;
  if ((CMSDK_GPIO->ALTFUNCSET & gpio_altfunc_mask) != (0xFFFF & gpio_altfunc_mask)) {err_code |= (1<<3); }
  if ((HW16_REG(gpio_base + 0x18) & gpio_altfunc_mask) != (0xFFFF & gpio_altfunc_mask)) {err_code |= (1<<4); }
  if ((HW16_REG(gpio_base + 0x1A)                    ) != (0x0000                    )) {err_code |= (1<<5); }
  if ((HW8_REG( gpio_base + 0x18) & gpio_altfunc_mask) != (0xFF   & gpio_altfunc_mask)) {err_code |= (1<<6); }
  if ((HW8_REG( gpio_base + 0x19) & (gpio_altfunc_mask>>8)) != (0xFF & (gpio_altfunc_mask>>8))) {err_code |= (1<<7); }

  HW16_REG(gpio_base + 0x1C) = 0x996E;
  if ((CMSDK_GPIO->ALTFUNCSET & gpio_altfunc_mask) != (0x6691 & gpio_altfunc_mask)) {err_code |= (1<<8); }
  HW16_REG(gpio_base + 0x1A) = 0xFFFF; /* ignored */
  if ((HW16_REG(gpio_base + 0x18) & gpio_altfunc_mask) != (0x6691 & gpio_altfunc_mask)) {err_code |= (1<<9); }
  if ((HW16_REG(gpio_base + 0x1A)                    ) != (0x0000                    )) {err_code |= (1<<10); }
  if ((HW8_REG( gpio_base + 0x18) & gpio_altfunc_mask) != (0x91   & gpio_altfunc_mask)) {err_code |= (1<<11); }
  if ((HW8_REG( gpio_base + 0x19) & (gpio_altfunc_mask>>8)) != (0x66 & (gpio_altfunc_mask>>8))) {err_code |= (1<<12); }
  HW16_REG(gpio_base + 0x1C) = 0xFFFF;
  HW8_REG(gpio_base + 0x18) = 0x45;
  HW8_REG(gpio_base + 0x19) = 0x23;
  HW8_REG(gpio_base + 0x1A) = 0x77;
  HW8_REG(gpio_base + 0x1B) = 0x77;
  if ((CMSDK_GPIO->ALTFUNCSET & gpio_altfunc_mask) != (0x2345 & gpio_altfunc_mask)) {err_code |= (1<<13); }
  if ((HW8_REG( gpio_base + 0x18) & gpio_altfunc_mask) != (0x45   & gpio_altfunc_mask)) {err_code |= (1<<14); }
  if ((HW8_REG( gpio_base + 0x19) & (gpio_altfunc_mask>>8)) != (0x23 & (gpio_altfunc_mask>>8))) {err_code |= (1<<15); }

  CMSDK_GPIO->ALTFUNCSET = 0x5488;
  CMSDK_GPIO->ALTFUNCCLR = 0x2201;
  if ((CMSDK_GPIO->ALTFUNCSET & gpio_altfunc_mask) != (0x55CC & gpio_altfunc_mask)) {err_code |= (1<<16); }
  CMSDK_GPIO->ALTFUNCCLR = 0x00CC;
  CMSDK_GPIO->ALTFUNCSET = 0xAA33;
  if ((CMSDK_GPIO->ALTFUNCSET & gpio_altfunc_mask) != (0xFF33 & gpio_altfunc_mask)) {err_code |= (1<<17); }
  CMSDK_GPIO->ALTFUNCCLR = 0xFFFF;
  CMSDK_GPIO->ALTFUNCSET = saved_altfunc; /* Restore ALTFUNC */

  if (err_code != 0) {
    printf ("ERROR : Read/write #2 failed(0x%x)\n", err_code);
    return_val=1;
    err_code = 0;
    }

  // Write to every location

  /* Test INTEN register read/write - offset 0x20  */
  CMSDK_GPIO->INTENSET = 0x00FF;
  if ((CMSDK_GPIO->INTENSET)      != 0x00FF){err_code |= (1<<0); }
  if ( HW8_REG(gpio_base + 0x20)!= 0xFF  ){err_code |= (1<<1); }
  if ( HW8_REG(gpio_base + 0x21)!= 0x00  ){err_code |= (1<<2); }
  if (HW16_REG(gpio_base + 0x22)!= 0x0000){err_code |= (1<<3); }
  CMSDK_GPIO->INTENSET = 0xFF00;
  CMSDK_GPIO->INTENCLR = 0x00FF;
  if ((CMSDK_GPIO->INTENSET)      != 0xFF00){err_code |= (1<<4); }
  if (HW16_REG(gpio_base + 0x20)!= 0xFF00){err_code |= (1<<5); }
  if ( HW8_REG(gpio_base + 0x20)!= 0x00  ){err_code |= (1<<6); }
  if ( HW8_REG(gpio_base + 0x21)!= 0xFF  ){err_code |= (1<<7); }

  CMSDK_GPIO->INTENCLR = 0x5500;
  CMSDK_GPIO->INTENSET = 0x0055;
  if ((CMSDK_GPIO->INTENSET) != 0xAA55)     {err_code |= (1<<8); }
  CMSDK_GPIO->INTENCLR = 0xFFFF;
  CMSDK_GPIO->INTENSET = 0x33BB;
  if ((CMSDK_GPIO->INTENSET) != 0x33BB)     {err_code |= (1<<9); }
  HW8_REG(gpio_base + 0x20) = 0x44;
  HW8_REG(gpio_base + 0x24) = 0xBA;
  HW8_REG(gpio_base + 0x25) = 0x10;
  HW8_REG(gpio_base + 0x26) = 0x99;
  HW8_REG(gpio_base + 0x27) = 0x99;
  if ((CMSDK_GPIO->INTENSET) != 0x2345)     {err_code |= (1<<10); }
  HW16_REG(gpio_base + 0x20) = 0x5892;
  HW16_REG(gpio_base + 0x22) = 0x1111;
  HW16_REG(gpio_base + 0x24) = 0x0240;
  if ((CMSDK_GPIO->INTENSET) != 0x7997)     {err_code |= (1<<11); }

  CMSDK_GPIO->INTENCLR = 0xFFFF;
  if ((CMSDK_GPIO->INTENSET) != 0x0000) {err_code |= (1<<12); }

  /* Test INTTYPE register read/write  - offset 0x14 */
  CMSDK_GPIO->INTTYPESET = 0x3A6B;
  if ((CMSDK_GPIO->INTTYPESET) != 0x3A6B) {err_code |= (1<<19); }
  if ( HW8_REG(gpio_base + 0x28)!= 0x6B  ){err_code |= (1<<20); }
  if ( HW8_REG(gpio_base + 0x29)!= 0x3A  ){err_code |= (1<<21); }
  if ( HW8_REG(gpio_base + 0x2A)!= 0x00  ){err_code |= (1<<22); }
  if ( HW8_REG(gpio_base + 0x2B)!= 0x00  ){err_code |= (1<<23); }

  HW16_REG(gpio_base + 0x2C) = 0xFFFF;
  HW8_REG(gpio_base + 0x28) = 0x7C;
  HW8_REG(gpio_base + 0x29) = 0x4B;
  if ((CMSDK_GPIO->INTTYPESET) != 0x4B7C)  {err_code |= (1<<24); }
  if (HW16_REG(gpio_base + 0x28)!= 0x4B7C ){err_code |= (1<<25); }

  HW16_REG(gpio_base + 0x2C) = 0x4964;
  HW16_REG(gpio_base + 0x28) = 0x8001;
  HW16_REG(gpio_base + 0x2A) = 0xFFFF;
  if ((CMSDK_GPIO->INTTYPESET) != 0x8219) {err_code |= (1<<26); }
  CMSDK_GPIO->INTTYPESET = 0xFFFF;
  if ((CMSDK_GPIO->INTTYPESET) != 0xFFFF) {err_code |= (1<<27); }
  CMSDK_GPIO->INTTYPECLR = 0xFFFF;
  if ((CMSDK_GPIO->INTTYPESET) != 0x0000) {err_code |= (1<<28); }

  /* Generate return value */
  if (err_code != 0) {
    printf ("ERROR : Read/write #3 failed(0x%x)\n", err_code);
    return_val=1;
    err_code = 0;
    }


  /* Test INTPOL register read/write  - offset 0x18 */
  CMSDK_GPIO->INTPOLSET = 0x3B6A;
  if ((CMSDK_GPIO->INTPOLSET) != 0x3B6A) {err_code |= (1<<0); }
  if ( HW8_REG(gpio_base + 0x30)!= 0x6A  ){err_code |= (1<<1); }
  if ( HW8_REG(gpio_base + 0x31)!= 0x3B  ){err_code |= (1<<2); }
  if ( HW8_REG(gpio_base + 0x32)!= 0x00  ){err_code |= (1<<3); }
  if ( HW8_REG(gpio_base + 0x33)!= 0x00  ){err_code |= (1<<4); }
  if (HW16_REG(gpio_base + 0x30)!= 0x3B6A){err_code |= (1<<5); }
  HW16_REG(gpio_base + 0x34) = 0xFFFF;
  HW16_REG(gpio_base + 0x30) = 0x4B7C;
  if ((CMSDK_GPIO->INTPOLSET) != 0x4B7C) {err_code |= (1<<6); }
  HW16_REG(gpio_base + 0x32)= 0x3333;
  if ((CMSDK_GPIO->INTPOLSET) != 0x4B7C) {err_code |= (1<<7); }
  HW8_REG( gpio_base + 0x34) = 0xFF;
  HW8_REG( gpio_base + 0x30) = 0x95;
  if ((CMSDK_GPIO->INTPOLSET) != 0x4B95) {err_code |= (1<<8); }
  HW8_REG( gpio_base + 0x35) = 0xFF;
  HW8_REG( gpio_base + 0x31) = 0x32;
  if ((CMSDK_GPIO->INTPOLSET) != 0x3295) {err_code |= (1<<9); }
  HW16_REG(gpio_base + 0x36)= 0xFFFF;
  HW8_REG( gpio_base + 0x32) = 0x66;
  HW8_REG( gpio_base + 0x33) = 0x66;
  if ((CMSDK_GPIO->INTPOLSET) != 0x3295) {err_code |= (1<<10); }
  HW16_REG(gpio_base + 0x34)= 0xFFFF;
  CMSDK_GPIO->INTPOLSET = 0x8219;
  if (HW16_REG(gpio_base + 0x30) != 0x8219) {err_code |= (1<<11); }
  CMSDK_GPIO->INTPOLSET = 0xFFFF;
  if ((CMSDK_GPIO->INTPOLSET) != 0xFFFF) {err_code |= (1<<12); }
  CMSDK_GPIO->INTPOLCLR = 0xFFFF;
  if ((CMSDK_GPIO->INTPOLSET) != 0x0000) {err_code |= (1<<13); }

  /* Generate return value */
  if (err_code != 0) {
    printf ("ERROR : Read/write #4 failed(0x%x)\n", err_code);
    return_val=1;
    err_code = 0;
    }
  puts("- INTSTATUS/INTCLEAR r/w");

  /* Test INTSTATUS/INTCLEAR register read/write - offset 0x1C */
  /* It was not set because when INTEN was high, input was high, but INTPOL was low (low level trigger) */
  if ((CMSDK_GPIO->INTSTATUS) != 0x0000) {err_code |= (1<<0); }
  if ( HW8_REG(gpio_base + 0x38)!= 0x00  ){err_code |= (1<<1); }
  if ( HW8_REG(gpio_base + 0x39)!= 0x00  ){err_code |= (1<<2); }
  if ( HW8_REG(gpio_base + 0x3A)!= 0x00  ){err_code |= (1<<3); }
  if ( HW8_REG(gpio_base + 0x3B)!= 0x00  ){err_code |= (1<<4); }
  if (HW16_REG(gpio_base + 0x38)!= 0x0000){err_code |= (1<<5); }

  /* Now we need to force INTSTATUS to 0xFFFF */
  CMSDK_GPIO->INTENSET = 0xFFFF;
  CMSDK_GPIO->INTPOLSET= 0xFFFF;
  /* It set INTSTATUS to 0xFFFF because INTEN was set, DATA input was pulled up and INTPOL was 0xFFFF */

  /* Disable interrupt from generating continuously */
  CMSDK_GPIO->INTENCLR = 0xFFFF;
  CMSDK_GPIO->INTPOLCLR= 0xFFFF;

  /* Test clearing of INTSTATUS */
  if ((CMSDK_GPIO->INTSTATUS & gpio_datamask) != (0xFFFF & gpio_datamask)) {err_code |= (1<<6); }
  if (( HW8_REG(gpio_base + 0x38) & gpio_datamask )!= (0xFF & gpio_datamask) ) {err_code |= (1<<7); }
  if (( HW8_REG(gpio_base + 0x39) & gpio_datamask )!= (0xFF & gpio_datamask) ) {err_code |= (1<<8); }
  if (( HW8_REG(gpio_base + 0x3A) & gpio_datamask )!= (0x00 & gpio_datamask) ) {err_code |= (1<<9); }
  if (( HW8_REG(gpio_base + 0x3B) & gpio_datamask )!= (0x00 & gpio_datamask) ) {err_code |= (1<<10); }
  if ((HW16_REG(gpio_base + 0x38) & gpio_datamask )!= (0xFFFF & gpio_datamask) ) {err_code |= (1<<11); }
  if ((HW16_REG(gpio_base + 0x3A) & gpio_datamask )!= (0x0000 & gpio_datamask) ) {err_code |= (1<<12); }

  CMSDK_GPIO->INTCLEAR = 0x0000;
  if ((CMSDK_GPIO->INTSTATUS & gpio_datamask) != (0xFFFF & gpio_datamask)) {err_code |= (1<<13); }

  CMSDK_GPIO->INTCLEAR = 0x0001;
  if ((CMSDK_GPIO->INTSTATUS & gpio_datamask) != (0xFFFE & gpio_datamask)) {err_code |= (1<<14); }
  if (( HW8_REG(gpio_base + 0x38) & gpio_datamask )!= (0xFE & gpio_datamask) ) {err_code |= (1<<15); }
  if (( HW8_REG(gpio_base + 0x39) & gpio_datamask )!= (0xFF & gpio_datamask) ) {err_code |= (1<<16); }

  HW16_REG(gpio_base + 0x38) = 0x0002;
  if ((CMSDK_GPIO->INTSTATUS & gpio_datamask) != (0xFFFC & gpio_datamask)) {err_code |= (1<<17); }
  HW16_REG(gpio_base + 0x38) = 0x0004;
  if ((CMSDK_GPIO->INTSTATUS & gpio_datamask) != (0xFFF8 & gpio_datamask)) {err_code |= (1<<18); }
  HW8_REG(gpio_base  + 0x38) = 0x0008;
  if ((CMSDK_GPIO->INTSTATUS & gpio_datamask) != (0xFFF0 & gpio_datamask)) {err_code |= (1<<19); }
  HW8_REG(gpio_base  + 0x39) = 0x0001;
  if ((CMSDK_GPIO->INTSTATUS & gpio_datamask) != (0xFEF0 & gpio_datamask)) {err_code |= (1<<20); }
  if ((HW16_REG(gpio_base + 0x38) & gpio_datamask )!= (0xFEF0 & gpio_datamask) ) {err_code |= (1<<21); }
  CMSDK_GPIO->INTCLEAR = 0x000E; /* Clear a bit that is already cleared has no effect */
  if ((HW16_REG(gpio_base + 0x38) & gpio_datamask )!= (0xFEF0 & gpio_datamask) ) {err_code |= (1<<22); }
  HW8_REG(gpio_base  + 0x3A) = 0x000F; /* ignored */
  if ((CMSDK_GPIO->INTSTATUS & gpio_datamask) != (0xFEF0 & gpio_datamask)) {err_code |= (1<<23); }
  HW16_REG(gpio_base + 0x3A) = 0xFFFF; /* ignored */
  if ((CMSDK_GPIO->INTSTATUS & gpio_datamask) != (0xFEF0 & gpio_datamask)) {err_code |= (1<<24); }
  CMSDK_GPIO->INTCLEAR = 0x3330;
  if ((CMSDK_GPIO->INTSTATUS & gpio_datamask) != (0xCCC0 & gpio_datamask)) {err_code |= (1<<25); }
  CMSDK_GPIO->INTCLEAR = 0xCCC0;
  if ((CMSDK_GPIO->INTSTATUS & gpio_datamask) != (0x0000 & gpio_datamask)) {err_code |= (1<<26); }

  /* Generate return value */
  if (err_code != 0) {
    printf ("ERROR : INTSTATUS/INTCLEAR r/w failed (0x%x)\n", err_code);
    return_val=1;
    err_code = 0;
    }

  /* Clean up */
  CMSDK_GPIO->OUTENABLECLR = 0xFFFFFFFF;
  CMSDK_GPIO->INTENCLR     = 0xFFFFFFFF;
  CMSDK_GPIO->INTPOLCLR    = 0xFFFFFFFF;
  CMSDK_GPIO->INTCLEAR  = 0x0000FFFF;
  gpio0_irq_occurred = 0;
  gpio1_irq_occurred = 0;
  gpio0_irq_expected = 0;
  gpio1_irq_expected = 0;

   /* Test whether the PORT0_0_IRQn is available */
   /* Note : GPIO Interrupt is pended in previous test stage */
  __disable_irq(); /* Use setting of Interrupt enable to detect if
    we have 32 interrupts, but we don't want the exact interrupt to
    take place, so we disable interrupt using PRIMASK during this checking */
  NVIC_EnableIRQ(PORT0_15_IRQn);
  if ((NVIC->ISER[0] & 0x80000000U)==0) {
    /* Number of IRQ less than 32, skip individual IRQ test */
    puts("Less then 32 IRQ in design\n");
    test_individual_irq = 0;
    }
  NVIC_DisableIRQ(PORT0_15_IRQn);
  __enable_irq();


  /* Check and Clear interrupt pending status in NVIC */
  if (CMSDK_GPIO==CMSDK_GPIO0) {

      if (test_individual_irq == 1) {
        for (i=0;i<16;i++){
          if (NVIC_GetPendingIRQ(((IRQn_Type)(((int)PORT0_0_IRQn) + i))) == 0) { /* Should have been set */
            err_code = err_code | (1<<i);
          }
          NVIC_ClearPendingIRQ(((IRQn_Type)(((int)PORT0_0_IRQn) + i)));
        }
       }
       if (NVIC_GetPendingIRQ(PORT0_ALL_IRQn) == 0) {/* Should have been set */
          err_code = err_code | (1<<16);
       }
  }

  if (CMSDK_GPIO==CMSDK_GPIO1) {
    if (NVIC_GetPendingIRQ(PORT1_ALL_IRQn) == 0) {/* Should have been set */
      err_code = err_code | (1<<17);
      }
    }

  NVIC_ClearPendingIRQ(PORT0_ALL_IRQn);
  NVIC_ClearPendingIRQ(PORT1_ALL_IRQn);

  /* Generate return value */
  if (err_code != 0) {
    printf ("ERROR : IRQ pending failed (0x%x)\n", err_code);
    return_val =1;
    err_code = 0;
  }
  else puts("Register Read/Write Test Passed");

  return(return_val);
}

/* --------------------------------------------------------------- */
/*  GPIO Unused Addresses access tests                             */
/* --------------------------------------------------------------- */
/* Test just one address - should read 0, write should be ignored */
int gpio_unused_addresses_test_single(unsigned int address)
{
  if (HW32_REG(address) != 0) return (1);
  HW32_REG(address) = 0xFFFFFFFF;
  if (HW32_REG(address) != 0) return (1);
  else return (0);
}

/* Test unused address in GPIO */
int gpio_unused_addresses_test(CMSDK_GPIO_TypeDef *CMSDK_GPIO){
  int return_val=0;
  unsigned int err_code=0;

  unsigned int gpio_base;

  puts("GPIO Unused address test");

  if (CMSDK_GPIO==CMSDK_GPIO0) {
    gpio_base  = CMSDK_GPIO0_BASE;
    }
  else if (CMSDK_GPIO==CMSDK_GPIO1) {
    gpio_base  = CMSDK_GPIO1_BASE;
    }
  else {
    puts ("ERROR: Input parameter invalid in function 'gpio_unused_addresses_test'.");
    return 1;
    }

  if (gpio_unused_addresses_test_single(gpio_base + 0x008)) err_code |= 1 << 0;
  if (gpio_unused_addresses_test_single(gpio_base + 0x00C)) err_code |= 1 << 1;
  if (gpio_unused_addresses_test_single(gpio_base + 0x03C)) err_code |= 1 << 2;
  if (gpio_unused_addresses_test_single(gpio_base + 0x040)) err_code |= 1 << 3;
  if (gpio_unused_addresses_test_single(gpio_base + 0x3FC)) err_code |= 1 << 4;
  if (gpio_unused_addresses_test_single(gpio_base + 0xC00)) err_code |= 1 << 5;
  if (gpio_unused_addresses_test_single(gpio_base + 0xD00)) err_code |= 1 << 6;
  if (gpio_unused_addresses_test_single(gpio_base + 0xE00)) err_code |= 1 << 7;
  if (gpio_unused_addresses_test_single(gpio_base + 0xF00)) err_code |= 1 << 8;
  if (gpio_unused_addresses_test_single(gpio_base + 0xFC0)) err_code |= 1 << 9;
  if (gpio_unused_addresses_test_single(gpio_base + 0xFC4)) err_code |= 1 << 10;
  if (gpio_unused_addresses_test_single(gpio_base + 0xFC8)) err_code |= 1 << 11;
  if (gpio_unused_addresses_test_single(gpio_base + 0xFCC)) err_code |= 1 << 12;


  /* Generate return value */
  if (err_code != 0) {
    printf ("ERROR : Unused addresses failed (0x%x)\n", err_code);
    return_val =1;
    err_code = 0;
  }
  else puts("- Unused addresses Test Passed");

  return(return_val);
}

/* --------------------------------------------------------------- */
/*  GPIO interrupt tests                                           */
/* --------------------------------------------------------------- */

int gpio_interrupt_test(CMSDK_GPIO_TypeDef *CMSDK_GPIO){
  int return_val=0;
  int err_code=0;
  int i;        /* loop variable */
  int irq_num;  /* loop variable */
  unsigned int irq_mask; /* one hot bit mask for IRQ being tested in the loop */

  unsigned int gpio_datamask;   /* Data bit mask */
  int test_individual_irq = 1;  /* Test individual IRQ in GPIO #0 by default */

  if (CMSDK_GPIO==CMSDK_GPIO0) {
    /* GPIO #0 is not shared in example system */
    gpio_datamask        = 0xFFFFFFFF; /* All bit can be tested */
    }
  else if (CMSDK_GPIO==CMSDK_GPIO1) {
    /* GPIO #1 is shared with peripherals in example system */
    /* bit 5 is used by stdout                             */
    gpio_datamask        = ~((1<<5)|(1<<4)|(1<<2)|(1<<0)); /* skip testing for these bits */
    }
  else {
    puts ("ERROR: Input parameter invalid in function 'gpio_interrupt_test'.");
    return 1;
    }

  for (i=0;i<16;i++){
    gpio0_irq_flags[i]=0;
    }
  puts("GPIO interrupt test");

  /* Check interrupt configurations */
  if (CMSDK_GPIO==CMSDK_GPIO0) {
    NVIC_EnableIRQ(PORT0_ALL_IRQn);
    if ((NVIC->ISER[0] & (1<<(PORT0_ALL_IRQn)))==0) {
      puts("Interrupt PORT0_ALL_IRQn not available.");
      return_val =1;
      }
    NVIC_DisableIRQ(PORT0_ALL_IRQn);
    }
  if (CMSDK_GPIO==CMSDK_GPIO1) {
    NVIC_EnableIRQ(PORT1_ALL_IRQn);
    if ((NVIC->ISER[0] & (1<<(PORT1_ALL_IRQn)))==0) {
      puts("Interrupt PORT1_ALL_IRQn not available.");
      return_val =1;
      }
    NVIC_DisableIRQ(PORT1_ALL_IRQn);
    }
  NVIC_EnableIRQ(PORT0_15_IRQn);
  if ((NVIC->ISER[0] & 0x80000000U)==0) {
    /* Number of IRQ less than 32, skip individual IRQ test */
    puts("Less then 32 IRQ in design\n");
    test_individual_irq = 0;
    }
  NVIC_DisableIRQ(PORT0_15_IRQn);

  /* Start test loop */
  for (irq_num = 0 ; irq_num <16; irq_num ++) {
    if (((gpio_datamask>>irq_num) & 0x1)!=0) {
      /* test required */

      CMSDK_GPIO->INTENCLR     = 0xFFFFFFFF;
      CMSDK_GPIO->INTPOLCLR    = 0xFFFFFFFF;
      CMSDK_GPIO->INTTYPECLR   = 0xFFFFFFFF;
      CMSDK_GPIO->INTCLEAR     = 0x0000FFFF;
      CMSDK_GPIO->OUTENABLECLR = 0xFFFFFFFF;
      gpio0_irq_occurred = 0;
      gpio1_irq_occurred = 0;
      gpio0_irq_expected = 0;
      gpio1_irq_expected = 0;
      gpio0_irq_level_irq = 1;
      gpio1_irq_level_irq = 1;
      multi_irq_test = 0;
      /* ------------------------------ */
      /*  Active low interrupt          */
      /* ------------------------------ */
      CMSDK_GPIO->DATAOUT     = 0xFFFFFFFF; /* Inactive */
      CMSDK_GPIO->INTTYPECLR  = 0xFFFFFFFF; /* Low level */
      CMSDK_GPIO->INTPOLCLR   = 0xFFFFFFFF; /* Low level */
      irq_mask                 = 1<<irq_num;
      CMSDK_GPIO->OUTENABLESET = irq_mask;
      if (CMSDK_GPIO==CMSDK_GPIO0) {
        gpio0_irq_expected   = irq_mask;
        NVIC_EnableIRQ(PORT0_ALL_IRQn);
        }
      if (CMSDK_GPIO==CMSDK_GPIO1) {
        gpio1_irq_expected   = irq_mask;
        NVIC_EnableIRQ(PORT1_ALL_IRQn);
        }
      /* -- interrupt enable  --        */
      gpio0_irq_level_irq    = 1;
      CMSDK_GPIO->INTENSET  = irq_mask;
      //medium_delay();
      if (CMSDK_GPIO==CMSDK_GPIO0) {
        /* Should not trigger IRQ yet because signal is high */
        if (gpio0_irq_occurred!=0) { err_code |= (1<<0);}
        /* Trigger combined IRQ now */
        CMSDK_GPIO->DATAOUT   = ~irq_mask; /* active */
        medium_delay();
        if (gpio0_irq_occurred==0) { err_code |= (1<<1);}
        }
      if (CMSDK_GPIO==CMSDK_GPIO1) {
        /* Should not trigger IRQ yet because signal is high */
        if (gpio1_irq_occurred!=0) { err_code |= (1<<0);}
        /* Trigger combined IRQ now */
        CMSDK_GPIO->DATAOUT   = ~irq_mask; /* active */
        medium_delay();
        if (gpio1_irq_occurred==0) { err_code |= (1<<1);}
        }
      /* -- interrupt disable --        */
      CMSDK_GPIO->INTENCLR  = 0xFFFFFFFF;
      gpio0_irq_expected     = 0;
      gpio1_irq_expected     = 0;
      gpio0_irq_occurred     = 0;
      gpio1_irq_occurred     = 0;
      if (CMSDK_GPIO==CMSDK_GPIO0) {
        CMSDK_GPIO->DATAOUT   = ~irq_mask; /* active */
        medium_delay();
        if (gpio0_irq_occurred!=0) { err_code |= (1<<2);}
        CMSDK_GPIO->DATAOUT   = 0xFFFFFFFF; /* Inactive */
        medium_delay();
        if (CMSDK_GPIO->INTSTATUS!=0){ err_code |= (1<<3);}
        }
      if (CMSDK_GPIO==CMSDK_GPIO1) {
        CMSDK_GPIO->DATAOUT   = ~irq_mask; /* active */
        medium_delay();
        if (gpio1_irq_occurred!=0) { err_code |= (1<<2);}
        CMSDK_GPIO->DATAOUT   = 0xFFFFFFFF; /* Inactive */
        medium_delay();
        if (CMSDK_GPIO->INTSTATUS!=0){ err_code |= (1<<3);}
        }

      /* -- separated irq --            */
      if ((CMSDK_GPIO==CMSDK_GPIO0) && (test_individual_irq!=0)) {
        gpio0_irq_flags[irq_num]=0;
        NVIC_DisableIRQ(PORT0_ALL_IRQn);
        NVIC_ClearPendingIRQ(((IRQn_Type)(((int)PORT0_0_IRQn) + irq_num)));
        NVIC_EnableIRQ(((IRQn_Type)(((int)PORT0_0_IRQn) + irq_num)));
        gpio0_irq_expected     = irq_mask;
        gpio0_irq_level_irq    = 1;
        CMSDK_GPIO->INTENSET  = irq_mask;
        CMSDK_GPIO->DATAOUT   = ~irq_mask; /* active */
        medium_delay();
        if (gpio0_irq_occurred==0) { err_code |= (1<<4);}
        if (gpio0_irq_flags[irq_num]==0) { err_code |= (1<<5);}
        CMSDK_GPIO->DATAOUT   = 0xFFFFFFFF; /* Inactive */

        /* Clean up */
        NVIC_ClearPendingIRQ(PORT0_ALL_IRQn);

        NVIC_DisableIRQ(((IRQn_Type)(((int)PORT0_0_IRQn) + irq_num)));
        gpio0_irq_flags[irq_num]= 0;
        gpio0_irq_expected      = 0;
        gpio0_irq_occurred      = 0;
        }

      /* -- multiple irq --             */
      if (CMSDK_GPIO==CMSDK_GPIO0) {
        gpio0_irq_expected   = irq_mask;
        gpio0_irq_occurred   = 0;
        NVIC_EnableIRQ(PORT0_ALL_IRQn);
        }
      if (CMSDK_GPIO==CMSDK_GPIO1) {
        gpio1_irq_expected   = irq_mask;
        gpio1_irq_occurred   = 0;
        NVIC_EnableIRQ(PORT1_ALL_IRQn);
        }
      multi_irq_test = 3; /* IRQ trigger 3 times */
      CMSDK_GPIO->INTENSET  = irq_mask;
      CMSDK_GPIO->DATAOUT   = ~irq_mask; /* active */
      medium_delay();
      if (CMSDK_GPIO==CMSDK_GPIO0) {
        if (gpio0_irq_occurred!=3) { err_code |= (1<<6);}
        }
      if (CMSDK_GPIO==CMSDK_GPIO1) {
        if (gpio1_irq_occurred!=3) { err_code |= (1<<6);}
        }
      CMSDK_GPIO->DATAOUT   = 0xFFFFFFFF; /* Inactive */
      CMSDK_GPIO->INTENCLR     = 0xFFFFFFFF; /* Disable */
      multi_irq_test = 0; /* Disable multiple IRQ test */
      /* ------------------------------ */
      /*  Active high interrupt         */
      /* ------------------------------ */
      CMSDK_GPIO->INTPOLSET = irq_mask; /* High level */
      CMSDK_GPIO->DATAOUT   = 0x00000000; /* Inactive */
      if (CMSDK_GPIO==CMSDK_GPIO0) {
        gpio0_irq_expected   = irq_mask;
        gpio0_irq_occurred   = 0;
        }

      if (CMSDK_GPIO==CMSDK_GPIO1) {
        gpio1_irq_expected   = irq_mask;
        gpio1_irq_occurred   = 0;
        }
      /* -- interrupt enable  --        */
      CMSDK_GPIO->INTENSET     = irq_mask; /* Enable */
      if (CMSDK_GPIO==CMSDK_GPIO0) {
        /* Should not trigger IRQ yet because signal is low */
        if (gpio0_irq_occurred!=0) { err_code |= (1<<7);}
        /* Trigger combined IRQ now */
        CMSDK_GPIO->DATAOUT   = irq_mask; /* active */
        medium_delay();
        if (gpio0_irq_occurred==0) { err_code |= (1<<8);}
        }
      if (CMSDK_GPIO==CMSDK_GPIO1) {
        /* Should not trigger IRQ yet because signal is low */
        if (gpio1_irq_occurred!=0) { err_code |= (1<<7);}
        /* Trigger combined IRQ now */
        CMSDK_GPIO->DATAOUT   = irq_mask; /* active */
        medium_delay();
        if (gpio1_irq_occurred==0) { err_code |= (1<<8);}
        }

      /* -- interrupt disable --        */
      CMSDK_GPIO->INTENCLR  = 0xFFFFFFFF;
      gpio0_irq_expected     = 0;
      gpio1_irq_expected     = 0;
      gpio0_irq_occurred     = 0;
      gpio1_irq_occurred     = 0;
      if (CMSDK_GPIO==CMSDK_GPIO0) {
        CMSDK_GPIO->DATAOUT   = irq_mask; /* active */
        medium_delay();
        if (gpio0_irq_occurred!=0) { err_code |= (1<<9);}
        CMSDK_GPIO->DATAOUT   = 0x00000000; /* Inactive */
        medium_delay();
        if (CMSDK_GPIO->INTSTATUS!=0){ err_code |= (1<<10);}
        }

      if (CMSDK_GPIO==CMSDK_GPIO1) {
        CMSDK_GPIO->DATAOUT   = irq_mask; /* active */
        medium_delay();
        if (gpio1_irq_occurred!=0) { err_code |= (1<<9);}
        CMSDK_GPIO->DATAOUT   = 0x00000000; /* Inactive */
        medium_delay();
        if (CMSDK_GPIO->INTSTATUS!=0){ err_code |= (1<<10);}
        }

      /* -- separated irq --            */
      if ((CMSDK_GPIO==CMSDK_GPIO0) && (test_individual_irq!=0)) {
        gpio0_irq_flags[irq_num]=0;
        NVIC_DisableIRQ(PORT0_ALL_IRQn);
        NVIC_ClearPendingIRQ(((IRQn_Type)(((int)PORT0_0_IRQn) + irq_num)));
        NVIC_EnableIRQ(((IRQn_Type)(((int)PORT0_0_IRQn) + irq_num)));
        gpio0_irq_expected     = irq_mask;
        gpio0_irq_level_irq    = 1;
        CMSDK_GPIO->INTENSET  = irq_mask;
        CMSDK_GPIO->DATAOUT   = irq_mask; /* active */
        medium_delay();
        if (gpio0_irq_occurred==0) { err_code |= (1<<11);}
      if (gpio0_irq_flags[irq_num]==0) { err_code |= (1<<12);}
        CMSDK_GPIO->DATAOUT   = 0x0; /* Inactive */
        /* Clean up */
        NVIC_ClearPendingIRQ(PORT0_ALL_IRQn);
        NVIC_DisableIRQ(((IRQn_Type)(((int)PORT0_0_IRQn) + irq_num)));
        gpio0_irq_flags[irq_num]= 0;
        gpio0_irq_expected      = 0;
        gpio0_irq_occurred      = 0;
        }
      /* -- multiple irq --             */
      if (CMSDK_GPIO==CMSDK_GPIO0) {
        gpio0_irq_expected   = irq_mask;
        gpio0_irq_occurred   = 0;
        NVIC_EnableIRQ(PORT0_ALL_IRQn);
        }
      if (CMSDK_GPIO==CMSDK_GPIO1) {
        gpio1_irq_expected   = irq_mask;
        gpio1_irq_occurred   = 0;
        NVIC_EnableIRQ(PORT1_ALL_IRQn);
        }
      multi_irq_test = 3; /* IRQ trigger 3 times */
      CMSDK_GPIO->INTENSET  = irq_mask;
      CMSDK_GPIO->DATAOUT   = irq_mask; /* active */
      medium_delay();
      if (CMSDK_GPIO==CMSDK_GPIO0) {
        if (gpio0_irq_occurred!=3) { err_code |= (1<<13);}
        }
      if (CMSDK_GPIO==CMSDK_GPIO1) {
        if (gpio1_irq_occurred!=3) { err_code |= (1<<13);}
        }
      CMSDK_GPIO->DATAOUT   = 0x00000000; /* Inactive */
      CMSDK_GPIO->INTENCLR  = 0xFFFFFFFF; /* Disable */
      multi_irq_test = 0; /* Disable multiple IRQ test */
      /* ------------------------------ */
      /*  Active rising edge interrupt  */
      /* ------------------------------ */

      CMSDK_GPIO->INTPOLSET    = irq_mask; /* Rising edge */
      CMSDK_GPIO->INTTYPESET   = irq_mask; /* Rising edge */
      CMSDK_GPIO->DATAOUT      = 0x00000000; /* Inactive */
      if (CMSDK_GPIO==CMSDK_GPIO0) {
        gpio0_irq_expected   = irq_mask;
        gpio0_irq_occurred   = 0;
        gpio0_irq_level_irq  = 0;
        }
      if (CMSDK_GPIO==CMSDK_GPIO1) {
        gpio1_irq_expected   = irq_mask;
        gpio1_irq_occurred   = 0;
        gpio1_irq_level_irq  = 0;
        }
      /* -- interrupt enable  --        */
      CMSDK_GPIO->INTENSET     = irq_mask; /* Enable */
      if (CMSDK_GPIO==CMSDK_GPIO0) {
        /* Should not trigger IRQ yet because signal is low */
        if (gpio0_irq_occurred!=0) { err_code |= (1<<14);}
        /* Trigger combined IRQ now */
        CMSDK_GPIO->DATAOUT   = irq_mask; /* active */
        medium_delay();
        if (gpio0_irq_occurred==0) { err_code |= (1<<15);}
        gpio0_irq_occurred   = 0;
        CMSDK_GPIO->DATAOUT   = 0x00000000; /* falling edge, should not trigger */
        medium_delay();
        if (gpio0_irq_occurred!=0) { err_code |= (1<<16);}

        }
      if (CMSDK_GPIO==CMSDK_GPIO1) {
        /* Should not trigger IRQ yet because signal is low */
        if (gpio1_irq_occurred!=0) { err_code |= (1<<14);}
        /* Trigger combined IRQ now */
        CMSDK_GPIO->DATAOUT   = irq_mask; /* active */
        medium_delay();
        if (gpio1_irq_occurred==0) { err_code |= (1<<15);}
        gpio1_irq_occurred   = 0;
        CMSDK_GPIO->DATAOUT   = 0x00000000; /* falling edge, should not trigger */
        medium_delay();
        if (gpio1_irq_occurred!=0) { err_code |= (1<<16);}
        }
      /* -- interrupt disable --        */
      CMSDK_GPIO->INTENCLR     = 0xFFFFFFFF; /* Disable */
      if (CMSDK_GPIO==CMSDK_GPIO0) {
        gpio0_irq_occurred   = 0;
        /* Should not trigger IRQ yet because signal is low */
        if (gpio0_irq_occurred!=0) { err_code |= (1<<17);}
        /* Rising edge - but IRQ is disabled */
        CMSDK_GPIO->DATAOUT   = irq_mask; /* active */
        medium_delay();
        if (gpio0_irq_occurred!=0) { err_code |= (1<<18);}
        gpio0_irq_occurred   = 0;
        CMSDK_GPIO->DATAOUT   = 0x00000000; /* falling edge, should not trigger */
        medium_delay();
        if (gpio0_irq_occurred!=0) { err_code |= (1<<19);}

        }
      if (CMSDK_GPIO==CMSDK_GPIO1) {
        gpio1_irq_occurred   = 0;
        /* Should not trigger IRQ yet because signal is low */
        if (gpio1_irq_occurred!=0) { err_code |= (1<<17);}
        /* Rising edge - but IRQ is disabled */
        CMSDK_GPIO->DATAOUT   = irq_mask; /* active */
        medium_delay();
        if (gpio1_irq_occurred!=0) { err_code |= (1<<18);}
        gpio1_irq_occurred   = 0;
        CMSDK_GPIO->DATAOUT   = 0x00000000; /* falling edge, should not trigger */
        medium_delay();
        if (gpio1_irq_occurred!=0) { err_code |= (1<<19);}
        }
      /* -- separated irq --            */
      if ((CMSDK_GPIO==CMSDK_GPIO0) && (test_individual_irq!=0)) {
        gpio0_irq_flags[irq_num]=0;
        NVIC_DisableIRQ(PORT0_ALL_IRQn);
        NVIC_ClearPendingIRQ(((IRQn_Type)(((int)PORT0_0_IRQn) + irq_num)));
        NVIC_EnableIRQ(((IRQn_Type)(((int)PORT0_0_IRQn) + irq_num)));
        gpio0_irq_expected     = irq_mask;
        CMSDK_GPIO->INTENSET     = irq_mask;
        CMSDK_GPIO->DATAOUT   = irq_mask; /* active */
        medium_delay();
        if (gpio0_irq_occurred==0) { err_code |= (1<<20);}
        if (gpio0_irq_flags[irq_num]==0) { err_code |= (1<<21);}
        gpio0_irq_occurred = 0;
        CMSDK_GPIO->DATAOUT   = 0x0; /* Inactive */
        medium_delay();
        if (gpio0_irq_occurred!=0) { err_code |= (1<<22);}

        /* Clean up */
        NVIC_ClearPendingIRQ(PORT0_ALL_IRQn);
        NVIC_DisableIRQ(((IRQn_Type)(((int)PORT0_0_IRQn) + irq_num)));
        gpio0_irq_flags[irq_num]= 0;
        gpio0_irq_expected      = 0;
        gpio0_irq_occurred      = 0;
        }
      /* ------------------------------ */
      /*  Active falling edge interrupt */
      /* ------------------------------ */
      CMSDK_GPIO->INTENCLR     = 0xFFFFFFFF;
      CMSDK_GPIO->INTPOLCLR    = 0xFFFFFFFF;        /* Falling edge */
      CMSDK_GPIO->INTTYPESET   = irq_mask; /* Falling edge */
      CMSDK_GPIO->DATAOUT      = irq_mask; /* Inactive */
      if (CMSDK_GPIO==CMSDK_GPIO0) {
        gpio0_irq_expected   = irq_mask;
        gpio0_irq_occurred   = 0;
        gpio0_irq_level_irq  = 0;
        NVIC_EnableIRQ(PORT0_ALL_IRQn);
        }
      if (CMSDK_GPIO==CMSDK_GPIO1) {
        gpio1_irq_expected   = irq_mask;
        gpio1_irq_occurred   = 0;
        gpio1_irq_level_irq  = 0;
        NVIC_EnableIRQ(PORT1_ALL_IRQn);
        }
      /* -- interrupt enable  --        */
      CMSDK_GPIO->INTENSET     = irq_mask; /* Enable */
      if (CMSDK_GPIO==CMSDK_GPIO0) {
        /* Should not trigger IRQ yet because signal is high */
        if (gpio0_irq_occurred!=0) { err_code |= (1<<23);}
        /* Trigger combined IRQ now */
        CMSDK_GPIO->DATAOUT   = 0x00000000; /* active */
        medium_delay();
        if (gpio0_irq_occurred==0) { err_code |= (1<<24);}
        gpio0_irq_occurred   = 0;
        CMSDK_GPIO->DATAOUT   = irq_mask; /* rising edge, should not trigger */
        medium_delay();
        if (gpio0_irq_occurred!=0) { err_code |= (1<<25);}

        }
      if (CMSDK_GPIO==CMSDK_GPIO1) {
        /* Should not trigger IRQ yet because signal is low */
        if (gpio1_irq_occurred!=0) { err_code |= (1<<23);}
        /* Trigger combined IRQ now */
        CMSDK_GPIO->DATAOUT   = 0x00000000; /* active */
        medium_delay();
        if (gpio1_irq_occurred==0) { err_code |= (1<<24);}
        gpio1_irq_occurred   = 0;
        CMSDK_GPIO->DATAOUT   = irq_mask; /* rising edge, should not trigger */
        medium_delay();
        if (gpio1_irq_occurred!=0) { err_code |= (1<<25);}
        }
      /* -- interrupt disable --        */
      CMSDK_GPIO->INTENCLR     = 0xFFFFFFFF; /* Disable */
      if (CMSDK_GPIO==CMSDK_GPIO0) {
        gpio0_irq_occurred   = 0;
        /* Should not trigger IRQ yet because signal is low */
        if (gpio0_irq_occurred!=0) { err_code |= (1<<26);}
        /* falling edge - but IRQ is disabled */
        CMSDK_GPIO->DATAOUT   = 0x00000000; /* active */
        medium_delay();
        if (gpio0_irq_occurred!=0) { err_code |= (1<<27);}
        gpio0_irq_occurred   = 0;
        CMSDK_GPIO->DATAOUT   = irq_mask; /* rising edge, should not trigger */
        medium_delay();
        if (gpio0_irq_occurred!=0) { err_code |= (1<<28);}

        }
      if (CMSDK_GPIO==CMSDK_GPIO1) {
        gpio1_irq_occurred   = 0;
        /* Should not trigger IRQ yet because signal is low */
        if (gpio1_irq_occurred!=0) { err_code |= (1<<26);}
        /* falling edge - but IRQ is disabled */
        CMSDK_GPIO->DATAOUT   = 0x00000000; /* active */
        medium_delay();
        if (gpio1_irq_occurred!=0) { err_code |= (1<<27);}
        gpio1_irq_occurred   = 0;
        CMSDK_GPIO->DATAOUT   = irq_mask; /* rising edge, should not trigger */
        medium_delay();
        if (gpio1_irq_occurred!=0) { err_code |= (1<<28);}
        }
      /* -- separated irq --            */
      if ((CMSDK_GPIO==CMSDK_GPIO0) && (test_individual_irq!=0)) {
        gpio0_irq_flags[irq_num]=0;
        NVIC_DisableIRQ(PORT0_ALL_IRQn);
        NVIC_ClearPendingIRQ(((IRQn_Type)(((int)PORT0_0_IRQn) + irq_num)));
        NVIC_EnableIRQ(((IRQn_Type)(((int)PORT0_0_IRQn) + irq_num)));
        gpio0_irq_expected     = irq_mask;
        CMSDK_GPIO->INTENSET  = irq_mask;
        CMSDK_GPIO->DATAOUT   = 0x00000000; /* active */
        medium_delay();
        if (gpio0_irq_occurred==0) { err_code |= (1<<29);}
      if (gpio0_irq_flags[irq_num]==0) { err_code |= (1<<30);}
        gpio0_irq_occurred = 0;
        CMSDK_GPIO->DATAOUT   = irq_mask; /* Inactive */
        medium_delay();
        if (gpio0_irq_occurred!=0) { err_code |= 0x80000000UL;}
        CMSDK_GPIO->INTENCLR      = 0xFFFFFFFF;
        /* Clean up */
        NVIC_ClearPendingIRQ(PORT0_ALL_IRQn);
        NVIC_DisableIRQ(((IRQn_Type)(((int)PORT0_0_IRQn) + irq_num)));
        gpio0_irq_flags[irq_num]= 0;
        gpio0_irq_expected      = 0;
        gpio0_irq_occurred      = 0;
        }

        if (err_code != 0) {
        printf ("ERROR : Interrupt failed (0x%x)\n", err_code);
        return_val =1;
        err_code = 0;
        }

      /* Cleaning up */
      CMSDK_GPIO->INTENCLR     = 0xFFFFFFFF;
      CMSDK_GPIO->INTTYPECLR   = 0xFFFFFFFF;
      CMSDK_GPIO->INTPOLCLR    = 0xFFFFFFFF;
      CMSDK_GPIO->OUTENABLECLR = 0xFFFFFFFF;
      CMSDK_GPIO->DATAOUT      = 0x00000000;
      CMSDK_GPIO->INTCLEAR     = 0x0000FFFF;

      } /* end if test required */
    }   /* end for irq_num */


  /* Clean up */
  gpio0_irq_occurred = 0;
  gpio1_irq_occurred = 0;
  gpio0_irq_expected = 0;
  gpio1_irq_expected = 0;

  NVIC_DisableIRQ(PORT0_ALL_IRQn);
  NVIC_DisableIRQ(PORT1_ALL_IRQn);
  if (CMSDK_GPIO==CMSDK_GPIO0) {
    NVIC_ClearPendingIRQ(PORT0_ALL_IRQn);
    for (i=0;i<16;i++){
      NVIC_DisableIRQ(((IRQn_Type)(((int)PORT0_0_IRQn) + i)));
      NVIC_ClearPendingIRQ(PORT0_ALL_IRQn);
      }
  } else {
    NVIC_ClearPendingIRQ(PORT1_ALL_IRQn);
  }

  /* Generate return value */
  if (err_code != 0) {
    printf ("ERROR : Interrupt failed (0x%x)\n", err_code);
    return_val =1;
    err_code = 0;
    }
    else puts("Interrupt Test Passed");

  return(return_val);
}
/* --------------------------------------- */
/* Combined interrupt handler for GPIO #0  */
/*                                         */
void PORT0_COMB_Handler(void)
{
  if (gpio0_irq_expected==0) {
    puts ("ERROR : Unexpected GPIO 0 interrupt occurred.\n");
    UartEndSimulation();
    while (1);
    }
  else if (gpio0_irq_level_irq!=0) {
    if (multi_irq_test > 1) {
      multi_irq_test--; /* twice here : 3->2, 2->1 */
      } else {          /* once here  */
      /* Deassert interrupt source */
      CMSDK_GPIO0->DATAOUT = CMSDK_GPIO0->DATAOUT ^ gpio0_irq_expected;
      medium_delay();
      }
    }
  CMSDK_GPIO0->INTCLEAR = gpio0_irq_expected;
  gpio0_irq_occurred++;
  return;
}

/* --------------------------------------- */
/* Combined interrupt handler for GPIO #1  */
/*                                         */
void PORT1_COMB_Handler(void)
{
  if (gpio1_irq_expected==0) {
    puts ("ERROR : Unexpected GPIO 1 interrupt occurred.\n");
    UartEndSimulation();
    while (1);
    }
  else if (gpio1_irq_level_irq!=0) {
    if (multi_irq_test > 1) {
      multi_irq_test--; /* twice here : 3->2, 2->1 */
      } else {          /* once here  */
      /* Deassert interrupt source */
      CMSDK_GPIO1->DATAOUT = CMSDK_GPIO1->DATAOUT ^ gpio1_irq_expected;
      medium_delay();
      }
    }
  CMSDK_GPIO1->INTCLEAR = gpio1_irq_expected;
  gpio1_irq_occurred++;
  return;
}

/* ----------------------------------------- */
/* Individual interrupt handler for GPIO #0  */
/*                                           */
void PORT0_0_Handler(void){
  PORT0_COMB_Handler();
  gpio0_irq_flags[0]++;
  return;
}
void PORT0_1_Handler(void){
  PORT0_COMB_Handler();
  gpio0_irq_flags[1]++;
  return;
}
void PORT0_2_Handler(void){
  PORT0_COMB_Handler();
  gpio0_irq_flags[2]++;
  return;
}
void PORT0_3_Handler(void){
  PORT0_COMB_Handler();
  gpio0_irq_flags[3]++;
  return;
}
void PORT0_4_Handler(void){
  PORT0_COMB_Handler();
  gpio0_irq_flags[4]++;
  return;
}
void PORT0_5_Handler(void){
  PORT0_COMB_Handler();
  gpio0_irq_flags[5]++;
  return;
}
void PORT0_6_Handler(void){
  PORT0_COMB_Handler();
  gpio0_irq_flags[6]++;
  return;
}
void PORT0_7_Handler(void){
  PORT0_COMB_Handler();
  gpio0_irq_flags[7]++;
  return;
}
void PORT0_8_Handler(void){
  PORT0_COMB_Handler();
  gpio0_irq_flags[8]++;
  return;
}
void PORT0_9_Handler(void){
  PORT0_COMB_Handler();
  gpio0_irq_flags[9]++;
  return;
}
void PORT0_10_Handler(void){
  PORT0_COMB_Handler();
  gpio0_irq_flags[10]++;
  return;
}
void PORT0_11_Handler(void){
  PORT0_COMB_Handler();
  gpio0_irq_flags[11]++;
  return;
}
void PORT0_12_Handler(void){
  PORT0_COMB_Handler();
  gpio0_irq_flags[12]++;
  return;
}
void PORT0_13_Handler(void){
  PORT0_COMB_Handler();
  gpio0_irq_flags[13]++;
  return;
}
void PORT0_14_Handler(void){
  PORT0_COMB_Handler();
  gpio0_irq_flags[14]++;
  return;
}
void PORT0_15_Handler(void){
  PORT0_COMB_Handler();
  gpio0_irq_flags[15]++;
  return;
}

