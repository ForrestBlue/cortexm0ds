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
 *      Checked In          : $Date: 2012-05-28 18:02:18 +0100 (Mon, 28 May 2012) $
 *
 *      Revision            : $Revision: 210377 $
 *
 *      Release Information : Cortex-M0 DesignStart-r1p0-00rel0
 *-----------------------------------------------------------------------------
 */
/*********************************************************************//******
 * @file     CMSDK_driver.c
 * @brief    CMSDK Example Device Driver C File
 * @version  $State:$
 * @date     $Date: 2012-05-28 18:02:18 +0100 (Mon, 28 May 2012) $
 *
 ******************************************************************************/

#include "CMSDK_CM0.h"



/** \mainpage ARM CMSDK LIBRARY
 *
 *
 * This user manual describes the ARM Corex M Series CMSDK Library which utilises the
 * Cortex Microcontroller Software Interface Standard (CMSIS). it also includes drivers
 * for the following modules:
 *
 *    - UART
 *    - Timer
 *    - GPIO
 *
 * The library contains C and assembly functions that have been ported and tested on the MDK
 * toolchain.
 */


/**
 *
 * @param *CMSDK_TIMER Timer Pointer
 * @return none
 *
 * @brief  Enable the microcontroller timer interrupts.
 */

 void CMSDK_timer_EnableIRQ(CMSDK_TIMER_TypeDef *CMSDK_TIMER)
 {
       CMSDK_TIMER->CTRL |= CMSDK_TIMER_CTRL_IRQEN_Msk;
 }

/**
 *
 * @param *CMSDK_TIMER Timer Pointer
 * @return none
 *
 * @brief  Disable the microcontroller timer interrutps.
 */

 void CMSDK_timer_DisableIRQ(CMSDK_TIMER_TypeDef *CMSDK_TIMER)
 {
       CMSDK_TIMER->CTRL &= ~CMSDK_TIMER_CTRL_IRQEN_Msk;
 }

/**
 *
 * @param *CMSDK_TIMER Timer Pointer
 * @return none
 *
 * @brief  Start the Timer.
 */

 void CMSDK_timer_StartTimer(CMSDK_TIMER_TypeDef *CMSDK_TIMER)
 {
       CMSDK_TIMER->CTRL |= CMSDK_TIMER_CTRL_EN_Msk;
 }

/**
 *
 * @param *CMSDK_TIMER Timer Pointer
 * @return none
 *
 * @brief  Stop the Timer.
 */

 void CMSDK_timer_StopTimer(CMSDK_TIMER_TypeDef *CMSDK_TIMER)
 {
       CMSDK_TIMER->CTRL &= ~CMSDK_TIMER_CTRL_EN_Msk;
 }

/**
 *
 * @param *CMSDK_TIMER Timer Pointer
 * @return TimerValue
 *
 * @brief  Returns the current value of the timer.
 */

 uint32_t CMSDK_timer_GetValue(CMSDK_TIMER_TypeDef *CMSDK_TIMER)
 {
       return CMSDK_TIMER->VALUE;
 }

/**
 *
 * @param *CMSDK_TIMER Timer Pointer
 * @param value the value to which the timer is to be set
 * @return TimerValue
 *
 * @brief  Sets the timer to the specified value.
 */

 void CMSDK_timer_SetValue(CMSDK_TIMER_TypeDef *CMSDK_TIMER, uint32_t value)
 {
       CMSDK_TIMER->VALUE = value;
 }

/**
 *
 * @param *CMSDK_TIMER Timer Pointer
 * @return TimerReload
 *
 * @brief  Returns the reload value of the timer. The reload value is the value which the timer is set to after an underflow occurs.
 */

 uint32_t CMSDK_timer_GetReload(CMSDK_TIMER_TypeDef *CMSDK_TIMER)
 {
       return CMSDK_TIMER->RELOAD;
 }

/**
 *
 * @param *CMSDK_TIMER Timer Pointer
 * @param value Value to be loaded
 * @return none
 *
 * @brief  Sets the reload value of the timer to the specified value. The reload value is the value which the timer is set to after an underflow occurs.
 */

 void CMSDK_timer_SetReload(CMSDK_TIMER_TypeDef *CMSDK_TIMER, uint32_t value)
 {
       CMSDK_TIMER->RELOAD = value;
 }

/**
 *
 * @param *CMSDK_TIMER Timer Pointer
 * @return none
 *
 * @brief  Clears the timer IRQ if set.
 */

 void CMSDK_timer_ClearIRQ(CMSDK_TIMER_TypeDef *CMSDK_TIMER)
 {
       CMSDK_TIMER->INTCLEAR = CMSDK_TIMER_INTCLEAR_Msk;
 }

/**
 *
 * @param *CMSDK_TIMER Timer Pointer
 * @return none
 *
 * @brief  Returns the IRQ status of the timer in question.
 */

 uint32_t  CMSDK_timer_StatusIRQ(CMSDK_TIMER_TypeDef *CMSDK_TIMER)
 {
       return CMSDK_TIMER->INTSTATUS;
 }

/**
 *
 * @param *CMSDK_TIMER Timer Pointer
 * @param reload The value to which the timer is to be set after an underflow has occurred
 * @param irq_en Defines whether the timer IRQ is to be enabled
 * @return none
 *
 * @brief  Initialises the timer to use the internal clock and specifies the timer reload value and whether IRQ is enabled or not.
 */

  void CMSDK_timer_Init_IntClock(CMSDK_TIMER_TypeDef *CMSDK_TIMER, uint32_t reload,
 uint32_t irq_en)
 {
       CMSDK_TIMER->CTRL = 0;
       CMSDK_TIMER->VALUE = reload;
       CMSDK_TIMER->RELOAD = reload;
       if (irq_en!=0)                                                                          /* non zero - enable IRQ */
         CMSDK_TIMER->CTRL = (CMSDK_TIMER_CTRL_IRQEN_Msk | CMSDK_TIMER_CTRL_EN_Msk);
       else{                                                                                   /* zero - do not enable IRQ */
         CMSDK_TIMER->CTRL = ( CMSDK_TIMER_CTRL_EN_Msk);                                       /* enable timer */
        }
 }

/**
 *
 * @param *CMSDK_TIMER Timer Pointer
 * @param reload The value to which the timer is to be set after an underflow has occurred
 * @param irq_en Defines whether the timer IRQ is to be enabled
 * @return none
 *
 * @brief  Initialises the timer to use the external clock and specifies the timer reload value and whether IRQ is enabled or not.
 */

 void CMSDK_timer_Init_ExtClock(CMSDK_TIMER_TypeDef *CMSDK_TIMER, uint32_t reload,
 uint32_t irq_en)
 {
       CMSDK_TIMER->CTRL = 0;
       CMSDK_TIMER->VALUE = reload;
       CMSDK_TIMER->RELOAD = reload;
       if (irq_en!=0)                                                                                  /* non zero - enable IRQ */
            CMSDK_TIMER->CTRL = (CMSDK_TIMER_CTRL_IRQEN_Msk |
                                   CMSDK_TIMER_CTRL_SELEXTCLK_Msk |CMSDK_TIMER_CTRL_EN_Msk);
       else  {                                                                                         /* zero - do not enable IRQ */
            CMSDK_TIMER->CTRL = ( CMSDK_TIMER_CTRL_EN_Msk |
                                    CMSDK_TIMER_CTRL_SELEXTCLK_Msk);                                   /* enable timer */
         }
 }

/**
 *
 * @brief  Initialises the timer to use the internal clock but with an external enable. It also specifies the timer reload value and whether IRQ is enabled or not.
 *
 * @param *CMSDK_TIMER Timer Pointer
 * @param reload The value to which the timer is to be set after an underflow has occurred
 * @param irq_en Defines whether the timer IRQ is to be enabled
 * @return none
 *
 *
 */

 void CMSDK_timer_Init_ExtEnable(CMSDK_TIMER_TypeDef *CMSDK_TIMER, uint32_t reload,
 uint32_t irq_en)
 {
       CMSDK_TIMER->CTRL = 0;
       CMSDK_TIMER->VALUE = reload;
       CMSDK_TIMER->RELOAD = reload;
       if (irq_en!=0)                                                                                  /* non zero - enable IRQ */
            CMSDK_TIMER->CTRL = (CMSDK_TIMER_CTRL_IRQEN_Msk |
                                   CMSDK_TIMER_CTRL_SELEXTEN_Msk | CMSDK_TIMER_CTRL_EN_Msk);
       else  {                                                                                         /* zero - do not enable IRQ */
            CMSDK_TIMER->CTRL = ( CMSDK_TIMER_CTRL_EN_Msk |
                                    CMSDK_TIMER_CTRL_SELEXTEN_Msk);                                    /* enable timer */
         }
 }


  /*UART driver functions*/

/**
 *
 * @brief  Initialises the UART specifying the UART Baud rate divider value and whether the send and recieve functionality is enabled. It also specifies which of the various interrupts are enabled.
 *
 * @param *CMSDK_UART UART Pointer
 * @param divider The value to which the UART baud rate divider is to be set
 * @param tx_en Defines whether the UART transmit is to be enabled
 * @param rx_en Defines whether the UART receive is to be enabled
 * @param tx_irq_en Defines whether the UART transmit buffer full interrupt is to be enabled
 * @param rx_irq_en Defines whether the UART receive buffer full interrupt is to be enabled
 * @param tx_ovrirq_en Defines whether the UART transmit buffer overrun interrupt is to be enabled
 * @param rx_ovrirq_en Defines whether the UART receive buffer overrun interrupt is to be enabled
 * @return 1 if initialisation failed, 0 if successful.
 */

 uint32_t CMSDK_uart_init(CMSDK_UART_TypeDef *CMSDK_UART, uint32_t divider, uint32_t tx_en,
                           uint32_t rx_en, uint32_t tx_irq_en, uint32_t rx_irq_en, uint32_t tx_ovrirq_en, uint32_t rx_ovrirq_en)
 {
       uint32_t new_ctrl=0;

       if (tx_en!=0)        new_ctrl |= CMSDK_UART_CTRL_TXEN_Msk;
       if (rx_en!=0)        new_ctrl |= CMSDK_UART_CTRL_RXEN_Msk;
       if (tx_irq_en!=0)    new_ctrl |= CMSDK_UART_CTRL_TXIRQEN_Msk;
       if (rx_irq_en!=0)    new_ctrl |= CMSDK_UART_CTRL_RXIRQEN_Msk;
       if (tx_ovrirq_en!=0) new_ctrl |= CMSDK_UART_CTRL_TXORIRQEN_Msk;
       if (rx_ovrirq_en!=0) new_ctrl |= CMSDK_UART_CTRL_RXORIRQEN_Msk;

       CMSDK_UART->CTRL = 0;         /* Disable UART when changing configuration */
       CMSDK_UART->BAUDDIV = divider;
       CMSDK_UART->CTRL = new_ctrl;  /* Update CTRL register to new value */

       if((CMSDK_UART->STATE & (CMSDK_UART_STATE_RXOR_Msk | CMSDK_UART_STATE_TXOR_Msk))) return 1;
       else return 0;
 }

/**
 *
 * @param *CMSDK_UART UART Pointer
 * @return RxBufferFull
 *
 * @brief  Returns whether the RX buffer is full.
 */

 uint32_t CMSDK_uart_GetRxBufferFull(CMSDK_UART_TypeDef *CMSDK_UART)
 {
        return ((CMSDK_UART->STATE & CMSDK_UART_STATE_RXBF_Msk)>> CMSDK_UART_STATE_RXBF_Pos);
 }

/**
 *
 * @param *CMSDK_UART UART Pointer
 * @return TxBufferFull
 *
 * @brief  Returns whether the TX buffer is full.
 */

 uint32_t CMSDK_uart_GetTxBufferFull(CMSDK_UART_TypeDef *CMSDK_UART)
 {
        return ((CMSDK_UART->STATE & CMSDK_UART_STATE_TXBF_Msk)>> CMSDK_UART_STATE_TXBF_Pos);
 }

/**
 *
 * @param *CMSDK_UART UART Pointer
 * @param txchar Character to be sent
 * @return none
 *
 * @brief  Sends a character to the TX buffer for transmission.
 */

 void CMSDK_uart_SendChar(CMSDK_UART_TypeDef *CMSDK_UART, char txchar)
 {
       while(CMSDK_UART->STATE & CMSDK_UART_STATE_TXBF_Msk);
       CMSDK_UART->DATA = (uint32_t)txchar;
 }

/**
 *
 * @param *CMSDK_UART UART Pointer
 * @return rxchar
 *
 * @brief  returns the character from the RX buffer which has been received.
 */

 char CMSDK_uart_ReceiveChar(CMSDK_UART_TypeDef *CMSDK_UART)
 {
       while(!(CMSDK_UART->STATE & CMSDK_UART_STATE_RXBF_Msk));
       return (char)(CMSDK_UART->DATA);
 }

/**
 *
 * @param *CMSDK_UART UART Pointer
 * @return 0 - No overrun
 * @return 1 - TX overrun
 * @return 2 - RX overrun
 * @return 3 - TX & RX overrun
 *
 * @brief  returns the current overrun status of both the RX & TX buffers.
 */


 uint32_t CMSDK_uart_GetOverrunStatus(CMSDK_UART_TypeDef *CMSDK_UART)
 {
        return ((CMSDK_UART->STATE & (CMSDK_UART_STATE_RXOR_Msk | CMSDK_UART_STATE_TXOR_Msk))>>CMSDK_UART_STATE_TXOR_Pos);
 }

/**
 *
 * @param *CMSDK_UART UART Pointer
 * @return 0 - No overrun
 * @return 1 - TX overrun
 * @return 2 - RX overrun
 * @return 3 - TX & RX overrun
 *
 * @brief  Clears the overrun status of both the RX & TX buffers and then returns the current overrun status.
 */

 uint32_t CMSDK_uart_ClearOverrunStatus(CMSDK_UART_TypeDef *CMSDK_UART)
 {
       CMSDK_UART->STATE = (CMSDK_UART_STATE_RXOR_Msk | CMSDK_UART_STATE_TXOR_Msk);
        return ((CMSDK_UART->STATE & (CMSDK_UART_STATE_RXOR_Msk | CMSDK_UART_STATE_TXOR_Msk))>>CMSDK_UART_STATE_TXOR_Pos);
 }

/**
 *
 * @param *CMSDK_UART UART Pointer
 * @return BaudDiv
 *
 * @brief  Returns the current UART Baud rate divider. Note that the Baud rate divider is the difference between the clock frequency and the Baud frequency.
 */

 uint32_t CMSDK_uart_GetBaudDivider(CMSDK_UART_TypeDef *CMSDK_UART)
 {
       return CMSDK_UART->BAUDDIV;
 }

 /**
 *
 * @param *CMSDK_UART UART Pointer
 * @return TXStatus
 *
 * @brief  Returns the TX interrupt status.
 */

 uint32_t CMSDK_uart_GetTxIRQStatus(CMSDK_UART_TypeDef *CMSDK_UART)
 {
       return ((CMSDK_UART->INTSTATUS & CMSDK_UART_CTRL_TXIRQ_Msk)>>CMSDK_UART_CTRL_TXIRQ_Pos);
 }

/**
 *
 * @param *CMSDK_UART UART Pointer
 * @return RXStatus
 *
 * @brief  Returns the RX interrupt status.
 */

 uint32_t CMSDK_uart_GetRxIRQStatus(CMSDK_UART_TypeDef *CMSDK_UART)
 {
       return ((CMSDK_UART->INTSTATUS & CMSDK_UART_CTRL_RXIRQ_Msk)>>CMSDK_UART_CTRL_RXIRQ_Pos);
 }

 /**
 *
 * @param *CMSDK_UART UART Pointer
 * @return none
 *
 * @brief  Clears the TX buffer full interrupt status.
 */

 void CMSDK_uart_ClearTxIRQ(CMSDK_UART_TypeDef *CMSDK_UART)
 {
       CMSDK_UART->INTCLEAR = CMSDK_UART_CTRL_TXIRQ_Msk;
 }

/**
 *
 * @param *CMSDK_UART UART Pointer
 * @return none
 *
 * @brief  Clears the RX interrupt status.
 */

 void CMSDK_uart_ClearRxIRQ(CMSDK_UART_TypeDef *CMSDK_UART)
 {
       CMSDK_UART->INTCLEAR = CMSDK_UART_CTRL_RXIRQ_Msk;
 }


 /*GPIO driver functions*/

/**
 *
 * @param *CMSDK_GPIO GPIO Pointer
 * @param outenable Bit pattern to be used to set output enable register
 * @return none
 *
 * @brief  Sets pins on a port as an output. Set the bit corresponding to the pin number to 1 for output i.e. Set bit 1 of outenable to 1 to set pin 1 as an output. This function is thread safe.
 */

 void CMSDK_gpio_SetOutEnable(CMSDK_GPIO_TypeDef *CMSDK_GPIO, uint32_t outenableset)
 {
       CMSDK_GPIO->OUTENABLESET = outenableset;
 }

/**
 *
 * @param *CMSDK_GPIO GPIO Pointer
 * @param outenable Bit pattern to be used to set output enable register
 * @return none
 *
 * @brief  Sets pins on a port as an input. Set the bit corresponding to the pin number to 1 for input i.e. Set bit 1 of outenable to 1 to set pin 1 as an input. This function is thread safe.
 */

 void CMSDK_gpio_ClrOutEnable(CMSDK_GPIO_TypeDef *CMSDK_GPIO, uint32_t outenableclr)
 {
       CMSDK_GPIO->OUTENABLECLR = outenableclr;
 }

/**
 *
 * @param *CMSDK_GPIO GPIO Pointer
 * @return outputstatus
 *
 * @brief  returns a uint32_t which defines the whether pins on a port are set as inputs or outputs i.e. if bit 1 of the returned uint32_t is set to 1 then this means that pin 1 is an output.
 */

 uint32_t CMSDK_gpio_GetOutEnable(CMSDK_GPIO_TypeDef *CMSDK_GPIO)
 {
       return CMSDK_GPIO->OUTENABLESET;
 }

/**
 *
 * @param *CMSDK_GPIO GPIO Pointer
 * @param AltFunc uint32_t to specify whether the alternate function for the pins on the port is enabled
 * @return none
 *
 * @brief  enables the alternative function for pins. Set the bit corresponding to the pin number to 1 for alternate function i.e. Set bit 1 of ALtFunc to 1 to set pin 1 to its alternative function. This function is thread safe.
 */

 void CMSDK_gpio_SetAltFunc(CMSDK_GPIO_TypeDef *CMSDK_GPIO, uint32_t AltFuncset)
 {
       CMSDK_GPIO->ALTFUNCSET = AltFuncset;
 }

/**
 *
 * @param *CMSDK_GPIO GPIO Pointer
 * @param AltFunc uint32_t to specify whether the alternate function for the pins on the port is enabled
 * @return none
 *
 * @brief  disables the alternative function for pins. Set the bit corresponding to the pin number to 1 to disable alternate function i.e. Set bit 1 of ALtFunc to 1 to set pin 1 to the orignal output function. This function is thread safe.
 */

 void CMSDK_gpio_ClrAltFunc(CMSDK_GPIO_TypeDef *CMSDK_GPIO, uint32_t AltFuncclr)
 {
       CMSDK_GPIO->ALTFUNCCLR = AltFuncclr;
 }

/**
 *
 * @param *CMSDK_GPIO GPIO Pointer
 * @return AltFuncStatus
 *
 * @brief  returns a uint32_t which defines the whether pins on a port are set to their alternative or their original output functionality i.e. if bit 1 of the returned uint32_t is set to 1 then this means that pin 1 is set to its alternative function.
 */

 uint32_t CMSDK_gpio_GetAltFunc(CMSDK_GPIO_TypeDef *CMSDK_GPIO)
 {
       return CMSDK_GPIO->ALTFUNCSET;
 }

/**
 *
 * @param *CMSDK_GPIO GPIO Pointer
 * @param Num The pin number for which to clear the Interrupt
 * @return NewIntStatus
 *
 * @brief  Clears the interrupt flag for the specified pin and then returns the new interrupt status of the pin. This function is thread safe.
 */

 uint32_t CMSDK_gpio_IntClear(CMSDK_GPIO_TypeDef *CMSDK_GPIO, uint32_t Num)
 {
       CMSDK_GPIO->INTCLEAR = (1 << Num);

       return CMSDK_GPIO->INTSTATUS;
 }

/**
 *
 * @param *CMSDK_GPIO GPIO Pointer
 * @param Num The pin number for which to enable the Interrupt
 * @return NewIntEnStatus
 *
 * @brief  Enables interrupts for the specified pin and then returns the new interrupt enable status of the pin. This function is thread safe.
 */

 uint32_t CMSDK_gpio_SetIntEnable(CMSDK_GPIO_TypeDef *CMSDK_GPIO, uint32_t Num)
 {
       CMSDK_GPIO->INTENSET = (1 << Num);

       return CMSDK_GPIO->INTENSET;
 }

/**
 *
 * @param *CMSDK_GPIO GPIO Pointer
 * @param Num The pin number for which to disable the Interrupt
 * @return NewIntEnStatus
 *
 * @brief  Disables interrupts for the specified pin and then returns the new interrupt enable status of the pin. This function is thread safe.
 */

  uint32_t CMSDK_gpio_ClrIntEnable(CMSDK_GPIO_TypeDef *CMSDK_GPIO, uint32_t Num)
 {
       CMSDK_GPIO->INTENCLR = (1 << Num);

       return CMSDK_GPIO->INTENCLR;
 }

/**
 *
 * @param *CMSDK_GPIO GPIO Pointer
 * @param Num The pin number for which to set the Interrupt type
 * @return none
 *
 * @brief  Changes the interrupt type for the specified pin to a high level interrupt. This function is thread safe.
 */

 void CMSDK_gpio_SetIntHighLevel(CMSDK_GPIO_TypeDef *CMSDK_GPIO, uint32_t Num)
 {
       CMSDK_GPIO->INTTYPECLR = (1 << Num); /* Clear INT TYPE bit */
       CMSDK_GPIO->INTPOLSET = (1 << Num);  /* Set INT POLarity bit */
 }

/**
 *
 * @param *CMSDK_GPIO GPIO Pointer
 * @param Num The pin number for which to set the Interrupt type
 * @return none
 *
 * @brief  Changes the interrupt type for the specified pin to a rising edge interrupt. This function is thread safe.
 */

 void CMSDK_gpio_SetIntRisingEdge(CMSDK_GPIO_TypeDef *CMSDK_GPIO, uint32_t Num)
 {
       CMSDK_GPIO->INTTYPESET = (1 << Num); /* Set INT TYPE bit */
       CMSDK_GPIO->INTPOLSET = (1 << Num);  /* Set INT POLarity bit */
 }

/**
 *
 * @param *CMSDK_GPIO GPIO Pointer
 * @param Num The pin number for which to set the Interrupt type
 * @return none
 *
 * @brief  Changes the interrupt type for the specified pin to a low level interrupt. This function is thread safe.
 */

 void CMSDK_gpio_SetIntLowLevel(CMSDK_GPIO_TypeDef *CMSDK_GPIO, uint32_t Num)
 {
       CMSDK_GPIO->INTTYPECLR = (1 << Num);  /* Clear INT TYPE bit */
       CMSDK_GPIO->INTPOLCLR = (1 << Num);   /* Clear INT POLarity bit */
 }

/**
 *
 * @param *CMSDK_GPIO GPIO Pointer
 * @param Num The pin number for which to set the Interrupt type
 * @return none
 *
 * @brief  Changes the interrupt type for the specified pin to a falling edge interrupt. This function is thread safe.
 */

 void CMSDK_gpio_SetIntFallingEdge(CMSDK_GPIO_TypeDef *CMSDK_GPIO, uint32_t Num)
 {
       CMSDK_GPIO->INTTYPESET = (1 << Num);  /* Set INT TYPE bit */
       CMSDK_GPIO->INTPOLCLR = (1 << Num);   /* Clear INT POLarity bit */
 }

/**
 *
 * @param *CMSDK_GPIO GPIO Pointer
 * @param mask The output port mask
 * @param value The value to output to the specified port
 * @return none
 *
 * @brief Outputs the specified value on the desired port using the user defined mask to perform Masked access.
 */

 void CMSDK_gpio_MaskedWrite(CMSDK_GPIO_TypeDef *CMSDK_GPIO, uint32_t value, uint32_t mask)
 {
       CMSDK_GPIO->LB_MASKED[0x00FF & mask] = value;
       CMSDK_GPIO->UB_MASKED[((0xFF00 & mask) >> 8)] = value;
 }

