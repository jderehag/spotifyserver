/**
 ******************************************************************************
 * @file    EXTI/stm32f4xx_it.c
 * @author  MCD Application Team
 * @version V1.0.0
 * @date    19-September-2011
 * @brief   Main Interrupt Service Routines.
 *          This file provides template for all exceptions handler and
 *          peripherals interrupt service routine.
 ******************************************************************************
 * @attention
 *
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
 * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_it.h"
#include "stm32f4_discovery.h"
#include "buttonHandler.h"
#include "stm32f4_discovery_eth.h"
#include "stm32f4xx_hal.h"
//#include "stm32f4x7_eth.h"

/* Scheduler includes */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* lwip includes */
#include "lwip/sys.h"

#include "applog.h"

/** @addtogroup STM32F4_Discovery_Peripheral_Examples
 * @{
 */

/** @addtogroup EXTI_Example
 * @{
 */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
 * @brief   This function handles NMI exception.
 * @param  None
 * @retval None
 */
void NMI_Handler(void)
{
}

/**
 * @brief  This function handles Hard Fault exception.
 * @param  None
 * @retval None
 */
#if 0
void HardFault_Handler(void)
{
#ifndef WITH_LCD
    STM_EVAL_LEDOn(LED6);
#else
    //STM_EVAL_LEDOn(LED4);
#endif
    /* Go to infinite loop when Hard Fault exception occurs */
    while (1)
    {
    }
}
#else
// From Joseph Yiu, minor edits by FVH
// hard fault handler in C,
// with stack frame location as input parameter
// called from HardFault_Handler
void hard_fault_handler_c (unsigned int * hardfault_args)
{
  unsigned int stacked_r0;
  unsigned int stacked_r1;
  unsigned int stacked_r2;
  unsigned int stacked_r3;
  unsigned int stacked_r12;
  unsigned int stacked_lr;
  unsigned int stacked_pc;
  unsigned int stacked_psr;

  stacked_r0 = ((unsigned long) hardfault_args[0]);
  stacked_r1 = ((unsigned long) hardfault_args[1]);
  stacked_r2 = ((unsigned long) hardfault_args[2]);
  stacked_r3 = ((unsigned long) hardfault_args[3]);

  stacked_r12 = ((unsigned long) hardfault_args[4]);
  stacked_lr = ((unsigned long) hardfault_args[5]);
  stacked_pc = ((unsigned long) hardfault_args[6]);
  stacked_psr = ((unsigned long) hardfault_args[7]);

  LogAppendSimpleCBinder( "\n\n[Hard fault handler - all numbers in hex]\n", 0 );
  LogAppendSimpleCBinder( "R0 = %x\n", stacked_r0);
  LogAppendSimpleCBinder( "R1 = %x\n", stacked_r1);
  LogAppendSimpleCBinder( "R2 = %x\n", stacked_r2);
  LogAppendSimpleCBinder( "R3 = %x\n", stacked_r3);
  LogAppendSimpleCBinder( "R12 = %x\n", stacked_r12);
  LogAppendSimpleCBinder( "LR [R14] = %x  subroutine call return address\n", stacked_lr);
  LogAppendSimpleCBinder( "PC [R15] = %x  program counter\n", stacked_pc);
  LogAppendSimpleCBinder( "PSR = %x\n", stacked_psr);
  LogAppendSimpleCBinder( "BFAR = %x\n", (*((volatile unsigned long *)(0xE000ED38))));
  LogAppendSimpleCBinder( "CFSR = %x\n", (*((volatile unsigned long *)(0xE000ED28))));
  LogAppendSimpleCBinder( "HFSR = %x\n", (*((volatile unsigned long *)(0xE000ED2C))));
  LogAppendSimpleCBinder( "DFSR = %x\n", (*((volatile unsigned long *)(0xE000ED30))));
  LogAppendSimpleCBinder( "AFSR = %x\n", (*((volatile unsigned long *)(0xE000ED3C))));
  LogAppendSimpleCBinder( "SCB_SHCSR = %x\n", SCB->SHCSR);

  while (1);
}
#endif
/**
 * @brief  This function handles Memory Manage exception.
 * @param  None
 * @retval None
 */
void MemManage_Handler(void)
{
    APP_LOG(LOG_EMERG,"MemManage!");
    /* Go to infinite loop when Memory Manage exception occurs */
    while (1)
    {
    }
}

/**
 * @brief  This function handles Bus Fault exception.
 * @param  None
 * @retval None
 */
void BusFault_Handler(void)
{
    APP_LOG(LOG_EMERG,"BusFault!");
    /* Go to infinite loop when Bus Fault exception occurs */
    while (1)
    {
    }
}

/**
 * @brief  This function handles Usage Fault exception.
 * @param  None
 * @retval None
 */
void UsageFault_Handler(void)
{
    APP_LOG(LOG_EMERG,"UsageFault!");
    /* Go to infinite loop when Usage Fault exception occurs */
    while (1)
    {
    }
}

/**
 * @brief  This function handles Debug Monitor exception.
 * @param  None
 * @retval None
 */
void DebugMon_Handler(void)
{
}



/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f4xx.s).                                               */
/******************************************************************************/

/**
 * @brief  This function handles External line 0 interrupt request.
 * @param  None
 * @retval None
 */
void EXTI0_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_0 );
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if( GPIO_Pin == GPIO_PIN_0 )
    {
        buttonHandler_action( (BSP_PB_GetState(BUTTON_KEY) != 0) ? 1 : 0 );
    }
}



void ETH_IRQHandler(void)
{
    BSP_ETH_IRQ_Handler();
}


/**
 * @}
 */

/**
 * @}
 */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
