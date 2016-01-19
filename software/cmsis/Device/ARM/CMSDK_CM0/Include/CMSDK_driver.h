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
/*************************************************************************//**
 * @file     CMSDK_driver.h
 * @brief    CMSDK Example Device Driver Header File
 * @version  $State:$
 * @date     $Date: 2012-05-28 18:02:18 +0100 (Mon, 28 May 2012) $
 *
 ******************************************************************************/


/** @addtogroup CMSIS_CM0_CMSDK_Driver_definitions CMSDK Driver definitions
  This file defines all CMSDK Driver functions for CMSIS core for the following modules:
    - Timer
    - UART
    - GPIO
  @{
 */

 #include "CMSDK_CM0.h"


 /*UART Driver Declarations*/

  /**
   * @brief Initializes UART module.
   */

 extern uint32_t CMSDK_uart_init(CMSDK_UART_TypeDef *CMSDK_UART, uint32_t divider, uint32_t tx_en,
                           uint32_t rx_en, uint32_t tx_irq_en, uint32_t rx_irq_en, uint32_t tx_ovrirq_en, uint32_t rx_ovrirq_en);

  /**
   * @brief Returns whether the UART RX Buffer is Full.
   */

 extern uint32_t CMSDK_uart_GetRxBufferFull(CMSDK_UART_TypeDef *CMSDK_UART);

  /**
   * @brief Returns whether the UART TX Buffer is Full.
   */

 extern uint32_t CMSDK_uart_GetTxBufferFull(CMSDK_UART_TypeDef *CMSDK_UART);

  /**
   * @brief Sends a character to the UART TX Buffer.
   */


 extern void CMSDK_uart_SendChar(CMSDK_UART_TypeDef *CMSDK_UART, char txchar);

  /**
   * @brief Receives a character from the UART RX Buffer.
   */

 extern char CMSDK_uart_ReceiveChar(CMSDK_UART_TypeDef *CMSDK_UART);

  /**
   * @brief Returns UART Overrun status.
   */

 extern uint32_t CMSDK_uart_GetOverrunStatus(CMSDK_UART_TypeDef *CMSDK_UART);

  /**
   * @brief Clears UART Overrun status Returns new UART Overrun status.
   */

 extern uint32_t CMSDK_uart_ClearOverrunStatus(CMSDK_UART_TypeDef *CMSDK_UART);

  /**
   * @brief Returns UART Baud rate Divider value.
   */

 extern uint32_t CMSDK_uart_GetBaudDivider(CMSDK_UART_TypeDef *CMSDK_UART);

  /**
   * @brief Return UART TX Interrupt Status.
   */

 extern uint32_t CMSDK_uart_GetTxIRQStatus(CMSDK_UART_TypeDef *CMSDK_UART);

  /**
   * @brief Return UART RX Interrupt Status.
   */

 extern uint32_t CMSDK_uart_GetRxIRQStatus(CMSDK_UART_TypeDef *CMSDK_UART);

  /**
   * @brief Clear UART TX Interrupt request.
   */

 extern void CMSDK_uart_ClearTxIRQ(CMSDK_UART_TypeDef *CMSDK_UART);

  /**
   * @brief Clear UART RX Interrupt request.
   */

 extern void CMSDK_uart_ClearRxIRQ(CMSDK_UART_TypeDef *CMSDK_UART);

  /**
   * @brief Set CMSDK Timer for multi-shoot mode with internal clock
   */

 /*Timer Driver Declarations*/

 extern void CMSDK_timer_Init_IntClock(CMSDK_TIMER_TypeDef *CMSDK_TIMER, uint32_t reload,
 uint32_t irq_en);

  /**
   * @brief Set CMSDK Timer for multi-shoot mode with external enable
   */

 extern void CMSDK_timer_Init_ExtClock(CMSDK_TIMER_TypeDef *CMSDK_TIMER, uint32_t reload,
 uint32_t irq_en);


  /**
   * @brief Set CMSDK Timer for multi-shoot mode with external clock
   */

 extern void CMSDK_timer_Init_ExtEnable(CMSDK_TIMER_TypeDef *CMSDK_TIMER, uint32_t reload,
 uint32_t irq_en);

  /**
   * @brief CMSDK Timer interrupt clear
   */


 extern void CMSDK_timer_ClearIRQ(CMSDK_TIMER_TypeDef *CMSDK_TIMER);

  /**
   * @brief Returns timer IRQ status
   */

 uint32_t  CMSDK_timer_StatusIRQ(CMSDK_TIMER_TypeDef *CMSDK_TIMER);

  /**
   * @brief Returns Timer Reload value.
   */

 extern uint32_t CMSDK_timer_GetReload(CMSDK_TIMER_TypeDef *CMSDK_TIMER);

  /**
   * @brief Sets Timer Reload value.
   */

 extern void CMSDK_timer_SetReload(CMSDK_TIMER_TypeDef *CMSDK_TIMER, uint32_t value);

  /**
   * @brief Returns Timer current value.
   */

 uint32_t CMSDK_timer_GetValue(CMSDK_TIMER_TypeDef *CMSDK_TIMER);

  /**
   * @brief Sets Timer current value.
   */

 extern void CMSDK_timer_SetValue(CMSDK_TIMER_TypeDef *CMSDK_TIMER, uint32_t value);

  /**
   * @brief Stops CMSDK Timer.
   */

 extern void CMSDK_timer_StopTimer(CMSDK_TIMER_TypeDef *CMSDK_TIMER);

  /**
   * @brief Starts CMSDK Timer.
   */

 extern void CMSDK_timer_StartTimer(CMSDK_TIMER_TypeDef *CMSDK_TIMER);

  /**
   * @brief Enables CMSDK Timer Interrupt requests.
   */

 extern void CMSDK_timer_EnableIRQ(CMSDK_TIMER_TypeDef *CMSDK_TIMER);

  /**
   * @brief Disables CMSDK Timer Interrupt requests.
   */

 extern void CMSDK_timer_DisableIRQ(CMSDK_TIMER_TypeDef *CMSDK_TIMER);

  /**
   * @brief Set CMSDK GPIO Output Enable.
   */

 /*GPIO Driver Declarations*/

 extern void CMSDK_gpio_SetOutEnable(CMSDK_GPIO_TypeDef *CMSDK_GPIO, uint32_t outenableset);

  /**
   * @brief Clear CMSDK GPIO Output Enable.
   */

 extern void CMSDK_gpio_ClrOutEnable(CMSDK_GPIO_TypeDef *CMSDK_GPIO, uint32_t outenableclr);

  /**
   * @brief Returns CMSDK GPIO Output Enable.
   */

 extern uint32_t CMSDK_gpio_GetOutEnable(CMSDK_GPIO_TypeDef *CMSDK_GPIO);

  /**
   * @brief Set CMSDK GPIO Alternate function Enable.
   */

 extern void CMSDK_gpio_SetAltFunc(CMSDK_GPIO_TypeDef *CMSDK_GPIO, uint32_t AltFuncset);

  /**
   * @brief Clear CMSDK GPIO Alternate function Enable.
   */

 extern void CMSDK_gpio_ClrAltFunc(CMSDK_GPIO_TypeDef *CMSDK_GPIO, uint32_t AltFuncclr);

  /**
   * @brief Returns CMSDK GPIO Alternate function Enable.
   */

 extern uint32_t CMSDK_gpio_GetAltFunc(CMSDK_GPIO_TypeDef *CMSDK_GPIO);

  /**
   * @brief Clear CMSDK GPIO Interrupt request.
   */

 extern uint32_t CMSDK_gpio_IntClear(CMSDK_GPIO_TypeDef *CMSDK_GPIO, uint32_t Num);

  /**
   * @brief Enable CMSDK GPIO Interrupt request.
   */

 extern uint32_t CMSDK_gpio_SetIntEnable(CMSDK_GPIO_TypeDef *CMSDK_GPIO, uint32_t Num);

  /**
   * @brief Disable CMSDK GPIO Interrupt request.
   */

 extern uint32_t CMSDK_gpio_ClrIntEnable(CMSDK_GPIO_TypeDef *CMSDK_GPIO, uint32_t Num);

  /**
   * @brief Setup CMSDK GPIO Interrupt as high level.
   */

 extern void CMSDK_gpio_SetIntHighLevel(CMSDK_GPIO_TypeDef *CMSDK_GPIO, uint32_t Num);

   /**
   * @brief Setup CMSDK GPIO Interrupt as rising edge.
   */

 extern void CMSDK_gpio_SetIntRisingEdge(CMSDK_GPIO_TypeDef *CMSDK_GPIO, uint32_t Num);

     /**
   * @brief Setup CMSDK GPIO Interrupt as low level.
   */

 extern void CMSDK_gpio_SetIntLowLevel(CMSDK_GPIO_TypeDef *CMSDK_GPIO, uint32_t Num);

    /**
   * @brief Setup CMSDK GPIO Interrupt as falling edge.
   */

 extern void CMSDK_gpio_SetIntFallingEdge(CMSDK_GPIO_TypeDef *CMSDK_GPIO, uint32_t Num);

    /**
   * @brief Setup CMSDK GPIO output value using Masked access.
   */

 extern void CMSDK_gpio_MaskedWrite(CMSDK_GPIO_TypeDef *CMSDK_GPIO, uint32_t value, uint32_t mask);

  /*@}*/ /* end of group CMSIS_CM0_CMSDK_Driver_definitions CMSDK Driver definitions */


