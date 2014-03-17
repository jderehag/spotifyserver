/*
 * Copyright (c) 2014, Jens Nielsen
 * All rights reserved.

 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of the <organization> nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL JENS NIELSEN BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include "stm32f4xx_hal.h"
#include "stm32f4_discovery_eth.h"
#include "FreeRTOS.h"
#include "semphr.h"

#define LAN8720_PHY_ADDRESS       0x00 /* For STM32F4-DIS-BB Board */

/* descriptors and receive buffers for DMA */
__ALIGN_BEGIN ETH_DMADescTypeDef  DMARxDscrTab[ETH_RXBUFNB] __ALIGN_END;/* Ethernet Rx MA Descriptor */
__ALIGN_BEGIN ETH_DMADescTypeDef  DMATxDscrTab[ETH_TXBUFNB] __ALIGN_END;/* Ethernet Tx DMA Descriptor */
__ALIGN_BEGIN uint8_t Rx_Buff[ETH_RXBUFNB][ETH_RX_BUF_SIZE] __ALIGN_END; /* Ethernet Receive Buffer */
__ALIGN_BEGIN uint8_t Tx_Buff[ETH_TXBUFNB][ETH_TX_BUF_SIZE] __ALIGN_END; /* Ethernet Transmit Buffer */

/* semaphore for synchronisation between interrupt and receive task */
static xSemaphoreHandle s_xSemaphore = NULL;
/* reference to global ethernet handle, needed in interrupt */
static ETH_HandleTypeDef* handle = NULL;

static void BSP_ETH_GPIO_Config(void);


uint8_t BSP_ETH_Init( ETH_HandleTypeDef* EthHandle )
{
    uint8_t ret = 0;
    uint8_t macaddress[6]= { MAC_ADDR0, MAC_ADDR1, MAC_ADDR2, MAC_ADDR3, MAC_ADDR4, MAC_ADDR5 };

    handle = EthHandle;

    handle->Instance = ETH;
    handle->Init.MACAddr = macaddress;
    handle->Init.AutoNegotiation = ETH_AUTONEGOTIATION_ENABLE;
    handle->Init.Speed = ETH_SPEED_100M;
    handle->Init.DuplexMode = ETH_MODE_FULLDUPLEX;
    handle->Init.MediaInterface = ETH_MEDIA_INTERFACE_RMII;
    handle->Init.RxMode = ETH_RXINTERRUPT_MODE;
    handle->Init.ChecksumMode = ETH_CHECKSUM_BY_HARDWARE;
    handle->Init.PhyAddress = LAN8720_PHY_ADDRESS;

    /* configure ethernet peripheral (GPIOs, clocks, MAC, DMA) */
    if ( HAL_ETH_Init( handle ) == HAL_OK )
    {
        ret = 1;

        /* Initialize Tx Descriptors list: Chain Mode */
        HAL_ETH_DMATxDescListInit( handle, DMATxDscrTab, &Tx_Buff[0][0], ETH_TXBUFNB);

        /* Initialize Rx Descriptors list: Chain Mode  */
        HAL_ETH_DMARxDescListInit( handle, DMARxDscrTab, &Rx_Buff[0][0], ETH_RXBUFNB);
    }

    /* create a binary semaphore used for synchronisation */
    s_xSemaphore = xSemaphoreCreateCounting(5,0);

    return ret;
}

/**
  * @brief  Initializes the ETH MSP. Called from ETH_Init()
  * @param  heth: ETH handle
  * @retval None
  */
void HAL_ETH_MspInit( ETH_HandleTypeDef *heth )
{
  (void)heth;
  /* Configure the GPIO ports for ethernet pins */
  BSP_ETH_GPIO_Config();
  
  /* Config NVIC for Ethernet */
  HAL_NVIC_SetPriority(ETH_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY, 0);
  HAL_NVIC_EnableIRQ(ETH_IRQn);


  __ETH_CLK_ENABLE();

  __ETHMAC_FORCE_RESET();
  __ETHMAC_RELEASE_RESET();
}


/**
  * @brief  DeInitializes ETH MSP.
  * @param  heth: ETH handle
  * @retval None
  */
void HAL_ETH_MspDeInit(ETH_HandleTypeDef *heth)
{
}


static void BSP_ETH_GPIO_Config(void)
{
    volatile uint32_t i;
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Enable GPIOs clocks */
    __GPIOA_CLK_ENABLE();
    __GPIOB_CLK_ENABLE();
    __GPIOC_CLK_ENABLE();
    __GPIOE_CLK_ENABLE();

    /* Ethernet pins configuration ************************************************/
    /*
        ETH_MDIO --------------> PA2
        ETH_MDC ---------------> PC1
    
        ETH_RMII_REF_CLK-------> PA1

        ETH_RMII_CRS_DV -------> PA7
        ETH_MII_RX_ER   -------> PB10
        ETH_RMII_RXD0   -------> PC4
        ETH_RMII_RXD1   -------> PC5
        ETH_RMII_TX_EN  -------> PB11
        ETH_RMII_TXD0   -------> PB12
        ETH_RMII_TXD1   -------> PB13

        ETH_RST_PIN     -------> PE2
    */

    /* Configure PA1,PA2 and PA7 */
    GPIO_InitStructure.Pin = GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_7;
    GPIO_InitStructure.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStructure.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStructure.Alternate = GPIO_AF11_ETH;
    GPIO_InitStructure.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Configure PB10,PB11,PB12 and PB13 */
    GPIO_InitStructure.Pin = GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* Configure PC1, PC4 and PC5 */
    GPIO_InitStructure.Pin = GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* Configure the PHY RST  pin */
    GPIO_InitStructure.Pin = GPIO_PIN_2;
    GPIO_InitStructure.Speed = GPIO_SPEED_FAST;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Alternate = 0;
    GPIO_InitStructure.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStructure);

    HAL_GPIO_WritePin( GPIOE, GPIO_PIN_2, GPIO_PIN_RESET );
    for (i = 0; i < 20000; i++);
    HAL_GPIO_WritePin( GPIOE, GPIO_PIN_2, GPIO_PIN_SET );
    for (i = 0; i < 20000; i++);
}

uint8_t BSP_ETH_WaitForRx( uint8_t ms )
{
    if ( xSemaphoreTake( s_xSemaphore, ms/portTICK_RATE_MS ) == pdTRUE )
        return 1;
    else
        return 0;
}

/* this is basically a copy of the HAL IRQ handler, just to be able to handle xHigherPriorityTaskWoken */
void BSP_ETH_IRQ_Handler(void)
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    /* Frame received */
    if (__HAL_ETH_DMA_GET_FLAG(handle, ETH_DMA_FLAG_R))
    {
        /* Notify lwip */
        xSemaphoreGiveFromISR( s_xSemaphore, &xHigherPriorityTaskWoken );

        /* Clear the Eth DMA Rx IT pending bits */
        __HAL_ETH_DMA_CLEAR_IT(handle, ETH_DMA_IT_R);

        /* Set HAL State to Ready */
        handle->State = HAL_ETH_STATE_READY;

        /* Process Unlocked */
        //__HAL_UNLOCK(heth); what?
    }
    /* Frame transmitted */
    else if (__HAL_ETH_DMA_GET_FLAG(handle, ETH_DMA_FLAG_T))
    {
        /* Transfer complete callback */
        HAL_ETH_TxCpltCallback(handle);

        /* Clear the Eth DMA Tx IT pending bits */
        __HAL_ETH_DMA_CLEAR_IT(handle, ETH_DMA_IT_T);

        /* Set HAL State to Ready */
        handle->State = HAL_ETH_STATE_READY;

        /* Process Unlocked */
        __HAL_UNLOCK(handle);
    }

    /* Clear the interrupt flags */
    __HAL_ETH_DMA_CLEAR_IT(handle, ETH_DMA_IT_NIS);

    /* ETH DMA Error */
    if(__HAL_ETH_DMA_GET_FLAG(handle, ETH_DMA_FLAG_AIS))
    {
        /* Ethernet Error callback */
        HAL_ETH_ErrorCallback(handle);

        /* Clear the interrupt flags */
        __HAL_ETH_DMA_CLEAR_IT(handle, ETH_DMA_FLAG_AIS);

        /* Set HAL State to Ready */
        handle->State = HAL_ETH_STATE_READY;

        /* Process Unlocked */
        __HAL_UNLOCK(handle);
    }

    portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
}

