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
  A simple test to check the functionalities of gpio functions in the example device driver.
*/

#include "CMSDK_CM0.h"
#define ALTMASK 0x2A
#include <stdio.h>
#include <stdlib.h>
#include "uart_stdout.h"
#include "CMSDK_driver.h"

/* Random seed used by GPIO_Mask_OP function */
#define RANDOM_NUMBER 47563745

/* Using ISB instruction to create a three cycles delay */
#define small_delay  __ISB

volatile int intstatus[16] = { 0 }; /* Detected interrupt operation from GPIO IRQ handler */
volatile int N = 0;         /* Bit number being test - Data shared between GPIO_IRQ and PORT0_COMB_Handler */
volatile uint32_t data = 0; /* Previous data in GPIO - Data shared between GPIO_IRQ and PORT0_COMB_Handler */

/* Declare test functions */
int GPIO_OPEN(void);       /*  tests the CMSDK GPIO output enable driver functions */
int GPIO_AltFuncEN(void);  /*  tests the CMSDK GPIO Alternate Function driver functions */
int GPIO_IRQ(void);        /*  tests the CMSDK GPIO IRQ driver functions */
int GPIO_Mask_OP(void);    /*  tests the CMSDK GPIO Masked Accesses driver functions */

int gpio0_id_check(void);
int gpio1_id_check(void);

int main (void)
{
  int result = 0;

  // UART init
  UartStdOutInit();

  // Test banner message and revision number
  puts("Cortex Microcontroller System Design Kit - GPIO Driver Test - revision $Revision: 242484 $\n");

  if (gpio0_id_check()!=0) {
    puts ("GPIO 0 not present. Test skipped.");
    UartEndSimulation();
    return 0;}
  if (gpio1_id_check()!=0) {
    puts ("GPIO 1 not present. Test skipped.");
    UartEndSimulation();
    return 0;}

  result |= GPIO_OPEN();
  result |= GPIO_AltFuncEN();
  result |= GPIO_IRQ();
  result |= GPIO_Mask_OP();

  if(result == 0) {
    puts("** TEST PASSED **\n");
  } else {
    printf("** TEST FAILED **, Error code: (0x%x)\n", result);
  }

  UartEndSimulation();
  return result;
}

/* GPIO ID check functions - detect if part number match expected value.
   If not, the GPIO might not be available and therefore the test should be skipped.
*/
#define HW32_REG(ADDRESS)  (*((volatile unsigned long  *)(ADDRESS)))
#define HW8_REG(ADDRESS)   (*((volatile unsigned char  *)(ADDRESS)))
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

/* GPIO_OPEN - This function tests the CMSDK GPIO output enable driver functions
  - Set Output enable (OUTENABLESET)
    - set Dataout to 0x0, due to the simulation pull up resistors,
      read back value is 0xFFFF when all the Output Enable control bits
      are not set.
    - for each pin on the port set the OUTENABLE, each in turn and check the status of the register
    - if the OUTENABLE for the specified pin is incorrect then change err_code
    - this will set the value of the specified pin to 0, keep 0 value for subsequent pin tests
    - if the Data-in for the port is incorrect then change err_code
    - else test passed for specified pin
    - if all pins pass, test passes

  - Clear Alternate Function (OUTENABLECLR)
    - using the current value of the Outen register all Outens will be cleared
    - for each pin on the port clear the OUTENABLE, each in turn and check the status of the register
    - if the OUTENABLE for the specified pin is incorrect then change err_code
    - if the Data-in for the port is incorrect then change err_code i.e != pins enabled and 0xFFFF
    - else test passed for specified pin
    - if the OUTENABLE register is cleared then the test is classed as passed
    */

int GPIO_OPEN(void)       /*output enable function*/
{
  int err_code = 0;
  uint32_t i = 0;
  uint32_t mask = 0;

  puts("Stage 1 - GPIO Output Enable Test\n");

  puts("- Stage 1a - Set GPIO Output Enable\n");

  /* Test sequence :
  Data output is set to 0x0000. Since output enable is 0x0000 by default,
  the actual I/O port status is determined by external pull up so it is
  0xFFFF at starting.  By enabling output enable bit by bit,
  the read back value of I/O port data become 0xFFFE, 0xFFFC, 0xFFF8, etc.
  */

  CMSDK_GPIO0->DATAOUT = 0x0000;     /* set data on all pins to logic 0 for the pull up resistor*/

  for(i = 1; i < 65536; i <<= 1){

    /* enable every pin starting at pin0 to pin15 while keeping the previous ones enabled */
    /* so that the transition of the output enable register is as follows: 0000 -> 0001 -> 0011 -> 0111 -> 1111 */

    mask |= i; /* mask is the expected value of OUTENABLE */
    CMSDK_gpio_SetOutEnable(CMSDK_GPIO0, i);    /* set specified bit in out enable register */

    if(CMSDK_gpio_GetOutEnable(CMSDK_GPIO0) != mask) err_code += 1;   /*check if the output enable register is not the*/
    if(CMSDK_GPIO0->DATA != (0xFFFF & (~mask))) err_code += 1;         /*same as the mask i.e. set up to and including*/
  }                                                                     /*the bit being set then increment error code */

  /* OUTENABLE should be 0xFFFF */
  printf("Output Enable Status: 0x%x\n\n", CMSDK_gpio_GetOutEnable(CMSDK_GPIO0));
  mask = 0;

  if(err_code) printf("Set Enable Test Failed, Error code: (0x%x\n)", err_code);
  else puts("Set Enable Test Passed\n");

  puts("- Stage 1b - Clear GPIO Output Enable\n");

  for(i = 1; i < 65536; i <<= 1){

    /* disable every pin starting at pin0 to pin15 while keeping the previously disabled ones disabled */
    /* and previously enabled ones enabled starting at all enabled so that the transition of the output*/
    /* enable register is as follows: 1111 -> 1110 -> 1100 -> 1000 -> 0000 */

    mask |= i;
    CMSDK_gpio_ClrOutEnable(CMSDK_GPIO0, i);  /*clear specified bit in out enable register*/

    if(~(CMSDK_gpio_GetOutEnable(CMSDK_GPIO0) | 0xFFFF0000) != mask)  /*check if the output enable register is not the*/
      err_code += 1;                                                    /*same as the mask i.e. clear up to and including*/
    if(CMSDK_GPIO0->DATA != (0xFFFF & mask))                           /*the bit being set then increment error code */
      err_code += 1;
  }

  printf("Output Enable Status: 0x%x\n\n", CMSDK_gpio_GetOutEnable(CMSDK_GPIO0));

  if(err_code) printf("Clear Output Enable Test Failed, Error code: (0x%x)\n", err_code);
  else puts("Clear Output Enable Test Passed\n");

  if(!err_code) return 0;
  else return 1;
}

/* GPIO_AltFuncEN - This function tests the CMSDK GPIO Alternate Function driver functions
  GPIO Port 1 is used for this test because GPIO pins in port 0 are not shared (AltFunc not implemented).
  - Set Alternate Function (AltFunc)
    - clear the alternate function register for all pins including TX2 (TUBE for puts)
    - for each pin on the port set the AltFunc, each in turn and check the status of the register
    - if the AltFunc for the specified pin is incorrect then change err_code
    - else test passed for specified pin
    - if the AltFunc register is set to 0x2A (M0) or 0xF82A (M3) then the test is classed as passed
    - NOTE: it is these exact combinations as only AltFunc outputs can be set in the AltFunc register
            as inputs have no effect as they are just routed to two places and pins without an AltFunc
            the value is not affected so is zero constantly
  - Clear Alternate Function (AltFunc)
    - using the current vale of the AltFunc register all AltFuncs will be cleared apart for vital ones
    - for each pin on the port clear the AltFunc, each in turn and check the status of the register
    - if the AltFunc for the specified pin is incorrect then change err_code unless it is TX2 as this
      is used for stdout so must remain set
    - else test passed for specified pin
    - if the AltFunc register is set to 0x20 (TX2 - pin 5) then the test is classed as passed
    */

int GPIO_AltFuncEN(void)
{
  int err_code = 0;
  uint32_t i = 0;
  uint32_t mask = 0;

  puts("Stage 2 - GPIO Alternative Function Enable Test\n");

  puts("- Stage 2a - Set GPIO Alternative Function Enable\n");

  CMSDK_gpio_ClrAltFunc(CMSDK_GPIO1, 0xFFFF);         /* Clear alternate function on all GPIO Port 1 pins to zero */

  for(i = 1; i < 65536; i <<= 1){

  //Set alternate function on all GPIO Port 1 pins which have an alternate function output port
  //will not affect those with either no alternate function or those which the pin is an input for
  //the alternate function so when read back the value with all alternate function is 0x2A for M0
  //and 0xf82A for M3, because the only pins that the alternate function affects is the UART
  //TX0 (pin 1), TX1 (pin 3), TX2 (pin 5)

    mask |= i;
    CMSDK_gpio_SetAltFunc(CMSDK_GPIO1, i);                     //set alternate function register for the specified bit
    if(CMSDK_gpio_GetAltFunc(CMSDK_GPIO1) != (ALTMASK & mask)) /*check if the AltFunc register is not the same as the*/
      err_code += i;                                             /*AltFunc mask i.e. all pins which have one, up to and including*/
  }                                                              /*the bit being set then if not increment error code */

  printf("\nAlternate Function Enable Status: 0x%x\n\n", CMSDK_gpio_GetAltFunc(CMSDK_GPIO1));

  if(err_code) printf("Set Enable Test Failed, Error code: (0x%x\n)", err_code);
  else puts("Set Enable Test Passed\n");    //if the port did not set the AltFunc as expected display error*/

  puts("- Stage 2b - Clear GPIO Alternative Function Enable\n");

  mask = 0;

  for(i = 1; i < 65536; i <<= 1){

    /* Clear AltFunc bit by bit apart from bit 5 for stdout */

    //Clear alternate function on all GPIO Port 1 pins which have an alternate function output port
    //will not affect those with either no alternate function or those which the pin is an input for
    //the alternate function so when read back the value with all alternate function is 0x20 for M0
    //and M3 because must leave the TX2 enabled for the TUBE

    mask |= i;

    /*clear alternate function register for the specified bit except for UART2 TX for tube*/

    CMSDK_gpio_ClrAltFunc(CMSDK_GPIO1, i & (~0x20));

    if(CMSDK_gpio_GetAltFunc(CMSDK_GPIO1) != ((ALTMASK & (~mask )) | 0x20)) /*check if the AltFunc register is not the same as the*/
      err_code += i;                                                        /*AltFunc mask i.e. all pins which have one, up to and including*/
                                                                            /*the bit being set then if not increment error code */
  }

  printf("Alternate Function Enable Status: 0x%x\n\n", CMSDK_gpio_GetAltFunc(CMSDK_GPIO1)); /*display Port's AltFunc Setting*/

  if(err_code) printf("Clear Enable Test Failed, Error code: (0x%x)\n", err_code);
  else puts("Clear Enable Test Passed\n");  /*if the port did not clear the AltFunc as expected display error*/

  if(!err_code) return 0;
  else return 2;
}

/* GPIO_IRQ - This function tests the CMSDK GPIO IRQ driver functions

  - Test High level IRQ
    - Set DataOut to 0x0000 ready for high level test
    - Enable all pins as output
    - Set all pins as a high level interrupt and clear all pending IRQs
    - Global enable (NVIC) the interrupts on the port
    - for each pin on the port enable the CMSDK GPIO interrupt for that pin then set the Dataout on the specified pin high
    - Check the inten register for the specified pin to check that it's set
    - in ISR, record what type of interrupt it's meant to be according to the intpol and inttype registers
    - if either the inten, intpol or inttype are incorrect then change err_code
    - else test passed for specified pin
    - if test on each pin passes the test as a whole passes and return 0 to main
    - else return an error

  - Test Rising Edge IRQ
    - Set DataOut to 0x0000 ready for rising edge test
    - Set all pins as a Rising Edge interrupt and clear all pending IRQs
    - for each pin on the port enable the CMSDK GPIO interrupt for that pin
    - set the Dataout on the specified pin high and keep high for subsequent pin tests (so can move on to falling edge tests)
    - Check the inten register for the specified pin to check that it's set
    - in ISR, record what type of interrupt it's meant to be according to the intpol and inttype registers
    - if either the inten, intpol or inttype are incorrect then change err_code
    - else test passed for specified pin
    - if test on each pin passes the test as a whole passes and return 0 to main
    - else return an error

  - Test Falling Edge IRQ
    - DataOut is already set to 0xFFFF from rising edge test
    - Set all pins as a Falling Edge interrupt and clear all pending IRQs
    - for each pin on the port enable the CMSDK GPIO interrupt for that pin
    - set the Dataout on the specified pin low and keep low for subsequent pin tests
    - Check the inten register for the specified pin to check that it's set
    - in ISR, record what type of interrupt it's meant to be according to the intpol and inttype registers
    - if either the inten, intpol or inttype are incorrect then change err_code
    - else test passed for specified pin
    - if test on each pin passes the test as a whole passes and return 0 to main
    - else return an error

  - Test Low level IRQ
    - Set DataOut to 0xFFFF ready for low level test
    - Enable all pins as output
    - Set all pins as a low level interrupt and clear all pending IRQs
    - for each pin, in turn, on the port enable the CMSDK GPIO interrupt for that pin
    - set the Dataout on the specified pin low whilst the interrupts are enabled then back high before clearing it
    - Check the inten register for the specified pin to check that it's set
    - in ISR, record what type of interrupt it's meant to be according to the intpol and inttype registers
    - if either the inten, intpol or inttype are incorrect then change err_code
    - else test passed for specified pin
    - if test on each pin passes the test as a whole passes and return 0 to main
    - else return an error
    */

int GPIO_IRQ(void)
{
  uint32_t err_code = 0, return_val = 0;
  int i = 0;
  int irq_counter = 0;

  puts("Stage 3 - GPIO Pin IRQ Test\n");

  puts("- Stage 3a - GPIO High Level IRQ Test\n");

  CMSDK_GPIO0->DATAOUT = 0x0000;                         //set data out to 0 to test high level IRQ
  CMSDK_gpio_SetOutEnable(CMSDK_GPIO0, 0xFFFF);         //set output enable to output on all ports of GPIO 0

  for(i = 0; i < 16; i++)
  {
    CMSDK_gpio_SetIntHighLevel(CMSDK_GPIO0, i);         //set all pins to high level interrupt and clear any
    CMSDK_gpio_IntClear(CMSDK_GPIO0, i);                //outstanding set interrupts
  }

  NVIC_ClearPendingIRQ(PORT0_ALL_IRQn);                   //clear all global NVIC PORT0 pending interrupts

  NVIC_EnableIRQ(PORT0_ALL_IRQn);                         //enable NVIC interrupts on PORT0

  for(i = 0; i < 16; i++){

    CMSDK_gpio_SetIntEnable(CMSDK_GPIO0, i);            //enable interrupt for the pin to be tested

    if(!((CMSDK_GPIO0->INTENSET) & (1 << i))) err_code = (1 << (2*i));//check to see if the interrupt enable register is
                                                                      //set for the pin which it should have just been
    N = i;

    CMSDK_GPIO0->DATAOUT = (0x0001 << i);                //set data out on pin0 high to cause a high level interrupt
                                                          //repeat for all other pins 0001 -> 0010 -> 0100 -> 1000
    CMSDK_gpio_ClrIntEnable(CMSDK_GPIO0, i );           /*clear interrupt enable*/

    if((CMSDK_GPIO0->INTENSET) & ~(1 << i)) err_code = (1 << (2*i +1));//check to see if the interrupt enable register is
                                                                        //clear for the pin which it should have just been

    /* check to see whether intstatus, for the specified pin, is 2, which corresponds to a high level interrupt */

    if(intstatus[i] == 2) irq_counter++;
    else err_code |= (1 << i);
  }

  if((irq_counter == 16) && (err_code == 0)) puts("High Level IRQ Tests Passed Successfully\n");
  else{
    printf("\nHigh Level IRQ Tests Failed, Error Code: (0x%x)\n", err_code);
    err_code = 0;       /*if the port did not have 1 HL IRQ for each pin as expected then display error*/
    return_val = 8;
  }

  puts("- Stage 3b - GPIO Rising Edge IRQ Test\n");

  irq_counter = 0;

  CMSDK_GPIO0->DATAOUT = 0x0000;   //set data out to 0 to test rising edge IRQ

  for(i = 0; i < 16; i++) CMSDK_gpio_SetIntRisingEdge(CMSDK_GPIO0, i); //set all pins to rising edge interrupts

  for(i = 0; i < 16; i++){

    CMSDK_gpio_SetIntEnable(CMSDK_GPIO0, i);    //enable interrupts on specified pin then set data out to 1
                                                  //for pin 0 then repeat for all subsequent pins whilst still

    if(!((CMSDK_GPIO0->INTENSET) & (1 << i))) err_code = (1 <<(2*i));//check to see if the interrupt enable register is
                                                                      //set for the pin which it should have just been
    N = i;

    CMSDK_GPIO0->DATAOUT = (0x0001 << i) | CMSDK_GPIO0->DATA; //keeping all other pins high 0000->0001->0011->0111->1111

    CMSDK_gpio_ClrIntEnable(CMSDK_GPIO0, i);                     //clear interrupt enable for specified pin

    if((CMSDK_GPIO0->INTENSET) & ~(1 << i)) err_code = (1 <<(2*i + 1));//check to see if the interrupt enable register is
                                                                        //clear for the pin which it should have just been

    /* check to see whether intstatus, for the specified pin, is 1, which corresponds to a rising edge interrupt */

    if(intstatus[i] == 1) irq_counter++;
    else err_code |= (1 << i);
  }

  if((irq_counter == 16) && (err_code == 0)) puts("Rising Edge IRQ Tests Passed Successfully\n");
  else{
    printf("\nRising Edge IRQ Tests Failed, Error Code: (0x%x\n", err_code);
    err_code = 0;     /*if the port did not have 1 RE IRQ for each pin as expected then display error*/
    return_val = 16;
  }

  puts("- Stage 3c - GPIO Falling Edge IRQ Test\n");

  irq_counter = 0;

  CMSDK_GPIO0->DATAOUT = 0xFFFF;

  for(i = 0; i < 16; i++) CMSDK_gpio_SetIntFallingEdge(CMSDK_GPIO0, i);   //set all pins to falling edge interrupts

  for(i = 0; i  < 16; i++){

    /* Counter change from 0->1->2 ...15 */

    CMSDK_gpio_SetIntEnable(CMSDK_GPIO0, i);    //enable interrupts on specified pin then set data out to
                                                  //0xFFFF and shiftright to cause a falling edge on
                                                  //pin 15 and then shift each time to cause a falling
                                                  // edge on pins 14 -> 0 1111 >> 0111 >> 0011 >> 0001 >> 0000

    if(!((CMSDK_GPIO0->INTENSET) & (1 << i)))    //check to see if the interrupt enable register is
      err_code = (1 << 2*i);                      //set for the pin which it should have just been

    N = i;

    CMSDK_GPIO0->DATAOUT = (CMSDK_GPIO0->DATA  & ~(1 << i));

    CMSDK_gpio_ClrIntEnable(CMSDK_GPIO0, i);    //disable interrupts on specified pin

    if((CMSDK_GPIO0->INTENSET) & ~(1 << i))      //check to see if the interrupt enable register is
      err_code = (1 << (2*i + 1));                //clear for the pin which it should have just been

    /* check to see whether intstatus, for the specified pin, is 3, which corresponds to a falling edge interrupt */

    if(intstatus[i] == 3) irq_counter++;
    else err_code |= (1 << i);
  }

  if((irq_counter == 16) && (err_code == 0)) puts("Falling Edge IRQ Tests Passed Successfully\n");
  else{
    printf("\nFalling Edge IRQ Tests Failed, Error Code: (0x%x)\n", err_code);
    err_code = 0; /*if the port did not have 1 FE IRQ for each pin as expected then display error*/
    return_val = 32;
  }

  puts("- Stage 3d - GPIO Low Level IRQ Test\n");

  irq_counter = 0;
  data = 0xFFFE;

  CMSDK_GPIO0->DATAOUT = 0xFFFF;           //set dataout to all ones ready to test low level interrupts

  for(i = 0; i < 16; i++) CMSDK_gpio_SetIntLowLevel(CMSDK_GPIO0, i);            //set all pins to low level interrupts

  for(i = 0; i < 16; i++){

    CMSDK_gpio_SetIntEnable(CMSDK_GPIO0, i);                          //enable interrupts on specified pin and set the
                                                                        //data output to 0xFFFF XOR with 1 shifted to the
    if(!((CMSDK_GPIO0->INTENSET) & (1 << i))) err_code = (1 << (2*i)); //pins position so that just the desired pin is
                                                                        //set to 0 1111 -> 1110 -> 1101 -> 1011 -> 1111
    N = i;

    CMSDK_GPIO0->DATAOUT = (0x0001 << i) ^ 0xFFFF;

    CMSDK_gpio_ClrIntEnable(CMSDK_GPIO0, i);                                    //disable interrupts on specified pin

    if((CMSDK_GPIO0->INTENSET) & ~(1 << i))                      //check to see if the interrupt enable register is
      err_code = (1 << (2*i + 1));                                //clear for the pin which it should have just been

    /* check to see whether intstatus, for the specified pin, is 4, which corresponds to a low level interrupt */

    if(intstatus[i] == 4) irq_counter++;
    else err_code |= (1 << i);
  }

  if((irq_counter == 16) && (err_code == 0)) puts("Low Level IRQ Tests Passed Successfully\n");
  else{
    printf("\nLow Level IRQ Tests Failed, Error Code: (0x%x)\n\n", err_code);
    /*if the port did not have 1 LL IRQ for each pin as expected then display error*/
    return_val = 64;
  }

  irq_counter = 0;

  NVIC_DisableIRQ(PORT0_ALL_IRQn);                                       //disable all GPIO interrupts on PORT 0

  NVIC_ClearPendingIRQ(PORT0_ALL_IRQn);                                  //clear any outstanding GPIO interrupts

  for(i = 0; i < 16; i++) CMSDK_gpio_IntClear(CMSDK_GPIO0, i);         //clear any outstanding GPIO interrupts

  return return_val;
}

/* GPIO_Mask_OP - This function tests the CMSDK GPIO Masked Accesses driver functions
  - Normal test of all 16 pins
    - Firstly the data output is set to 0x0000 so as to ensure all comparisons are evaluated correctly
    - write 0xFFFF using masked access with the mask transitioning through the combinations
      0001->0010->0100->1000
    - evaluate whether the data outputted are correct i.e. is same as 0xFFFF and mask
    - if not amend err_code else test passed
    - clear the current array element corresponding to the mask else the data on the output port will accumulate
      i.e. mask of 0x0001 and mask of 0x0002 will give a mask of 0x0003.

  - Test of 16 random combinations
    - seed the random function with the #define RANDOM_NUMBER
    - use rand to get a number between 0 and 65535
    - perform masked access using the value 0xFFFF and the random number as the mask
    - evaluate whether the data outputted is correct i.e. is same as 0xFFFF and random mask
    - if not amend err_code else test passed
    - clear the current array element corresponding to the mask else the data on the output port will accumulate
      i.e. mask of 0x0001 and mask of 0x0002 will give a mask of 0x0003.

  - Test accumulation of masks
    - write 0xFFFF using masked access with the mask transitioning through the combinations
      0001->0010->0100->1000 but do not clear after evaluating so that the masked values accumulate
    - evaluate whether the data outputted is correct i.e. is same as 0xFFFF and the accumulated mask
    - if not amend err_code else test passed

  - Test accumulation of random masks and random values
    - gather both a random value and random mask using the rand function
    - store the random mask so that it can be cleared later
    - write value using masked access
    - amend expected value to be seen when the GPIO is read back condition gained from verilog RTL
      ((DATA[15:8] & mask[9:2])|(existing_data[15:8] & ~mask[9:2]));
    - repeat with a second random value and mask after the test has been evaluated
      clear the output port for the previous masks and reset both the previous masks and the expected value
    - repeat process again until it has occurred 8 times
*/

int GPIO_Mask_OP(void)
{
  int err_code = 0, err_code_all = 0;
  uint32_t i = 0, random_mask = 0, exp_value = 0, random_value, prev_masks[2] = { 0 }, mask = 0;


  CMSDK_gpio_SetOutEnable(CMSDK_GPIO0, 0xFFFF);

  CMSDK_GPIO0->DATAOUT = 0x0000;

  puts("Stage 4 - GPIO Masked Output Test\n");

  puts("- Stage 4a - GPIO Masked Output - Deterministic Test\n");

  for(i = 0; i < 16; i++){

    CMSDK_gpio_MaskedWrite(CMSDK_GPIO0, 0xFFFF, (1 << i));   //test each pin on the GPIO to check that masked
    small_delay();                                             //access works for each pin individually
    if(CMSDK_GPIO0->DATA != (0xFFFF & (1 << i)))
      err_code |= (1 << i);
    CMSDK_gpio_MaskedWrite(CMSDK_GPIO0, 0x0000, (1 << i));   //clear value back to zero
    small_delay();
  }

  CMSDK_GPIO0->DATAOUT = 0;

  for(i = 0; i < 16; i++){
    CMSDK_gpio_MaskedWrite(CMSDK_GPIO0, 0xFFFF, (1 << i));   //test each pin on the GPIO to check that masked
    small_delay();                                             //access works for each pin individually
    mask |= (1 << i);
    if(CMSDK_GPIO0->DATA != (0xFFFF & mask))                  //test do not reset to ensure that the masked output
      err_code |= (1 << (i + 16));                             //goes through the transition 0001->0011->0111->1111
    small_delay();
  }

  if(!err_code) puts("Stage 4a Deterministic Masked Output Tests Passed Successfully\n");
  else {
    printf("\nStage 4a Deterministic Masked Output Tests Failed, Error Code: (0x%x)\n", err_code);
    err_code_all |= 1;
    }

  puts("- Stage 4b - GPIO Masked Output - Random Test\n");

  err_code = 0;

  CMSDK_GPIO0->DATAOUT = 0;

  srand(RANDOM_NUMBER);

  for(i = 0; i < 16; i++){                                         //repeat same process but with 16 random mask
    random_mask = (rand() % 65536);                                //values, the random function is seeded with a
    CMSDK_gpio_MaskedWrite(CMSDK_GPIO0, 0xFFFF, random_mask);    //#define at the top of the program called RANDOM_NUMBER
    if(CMSDK_GPIO0->DATA != (0xFFFF & random_mask))               //can be easily changed to perform tests on 16
      err_code |= (1 << i);                                 // more different random numbers
    CMSDK_gpio_MaskedWrite(CMSDK_GPIO0, 0x0000, random_mask);
  }

  CMSDK_GPIO0->DATAOUT = 0;

  mask = 0;

  for(i = 0; i < 16; i++){
                                                            //repeat same process but with 16 random mask values,
    random_mask = (rand() % 65536);                         //which are only cleared every other iteration so that
    random_value = (rand() % 65536);                        //they accumulate, the first value is outputted using
                                                            //masked access and then not cleared then a second value
    if(prev_masks[0] == 0) prev_masks[0] = random_mask;     //is outputted using another mask and then the resulting
    else prev_masks[1] = random_mask;                       //output is then checked against the expected output

    exp_value = ((random_mask & random_value) | (exp_value & ~random_mask));

    CMSDK_gpio_MaskedWrite(CMSDK_GPIO0, random_value, random_mask);

    if(CMSDK_GPIO0->DATA != exp_value)
      err_code |= (1 << (i + 16));

    /* For every two non-zero mask writes taken place,
    reset the data using the stored mask value and
    clear the exp_value back to 0 */
    if((prev_masks[0] != 0) && (prev_masks[1] != 0)){
      CMSDK_gpio_MaskedWrite(CMSDK_GPIO0, 0x0000, prev_masks[0]);
      CMSDK_gpio_MaskedWrite(CMSDK_GPIO0, 0x0000, prev_masks[1]);
      prev_masks[0] = 0;
      prev_masks[1] = 0;
      exp_value = 0;
    }
  }

  if(!err_code) puts("Stage 4b Random Masked Output Tests Passed Successfully\n");
  else {
    printf("\nStage 4b Random Masked Output Tests Failed, Error Code: (0x%x)\n", err_code);
    err_code_all |= 2;
    }


  if(!err_code_all) puts("\nAll Masked Output Tests Passed Successfully\n");

  if(!err_code_all) return 0;
  else return 4;
}

//PORT 0 ISR - sets what type of interrupt it is according to the int polarity and type and
//if the result when checked is the one expected then function will pass the test or fail it, if it's not
//0 = undefined, 1 = rising edge, 2 = high level, 3 = falling edge, 4 = low level

void PORT0_COMB_Handler(void)
{

  if((CMSDK_GPIO0->INTPOLSET)){
    if((CMSDK_GPIO0->INTTYPESET)){
      if(CMSDK_GPIO0->DATA == ( data |= (1 << N) ))
        intstatus[N] = 1;                                                 /*rising edge*/
    }else{
      if(CMSDK_GPIO0->DATA == (1 << N)){
        intstatus[N] = 2;                                                   /*high level*/
        CMSDK_GPIO0->DATAOUT = 0x0000;
      }
    }
  }else{
    if((CMSDK_GPIO0->INTTYPESET)){
      if(CMSDK_GPIO0->DATA == (data &= ~(1 << N))) intstatus[N] = 3;       /*falling edge*/
    }else{
      if(CMSDK_GPIO0->DATA == 0xFFFF ^ (1 << N)){
        intstatus[N] = 4;                                                   /*low level*/
        CMSDK_GPIO0->DATAOUT = 0xFFFF;
      }
    }
  }
  CMSDK_gpio_IntClear(CMSDK_GPIO0, N);
}

