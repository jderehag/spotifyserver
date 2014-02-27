
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "stm32f4xx_pwr.h"
#include "stm32f4xx_rcc.h"
//#include "stm32f4xx_exti.h"
//#include "stm32f4xx_rtc.h"

#include "stm32f4_discovery.h"

typedef enum
{
    PWR_ON,
    PWR_ON_LIGHTS_OUT,
    PWR_STANDBY,
}PowerLevel_t;

static PowerLevel_t powerLevel = PWR_ON;
static xTimerHandle tmr;
static int canPowerOff = 1;

static void timerCb( xTimerHandle xTimer );
static void SYSCLKConfig_STOP(void);

void pwrInit()
{
#if 0
    EXTI_InitTypeDef  EXTI_InitStructure;
    NVIC_InitTypeDef  NVIC_InitStructure;
    __IO uint32_t AsynchPrediv = 0, SynchPrediv = 0;

    /* Enable the PWR clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

    /* Allow access to RTC */
    PWR_BackupAccessCmd(ENABLE);

    /* The RTC Clock may varies due to LSI frequency dispersion. */
    /* Enable the LSI OSC */
    RCC_LSICmd(ENABLE);

    /* Wait till LSI is ready */
    while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
    {
    }

    /* Select the RTC Clock Source */
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);

    SynchPrediv = 0xFF;
    AsynchPrediv = 0x7C;

    /* Enable the RTC Clock */
    RCC_RTCCLKCmd(ENABLE);

    /* Wait for RTC APB registers synchronisation */
    RTC_WaitForSynchro();

    /* RTC Wakeup Interrupt Generation: Clock Source: RTCCLK_Div16, Wakeup Time Base: ~4s

       Wakeup Time Base = (16 / (LSE or LSI)) * WakeUpCounter
    */
    RTC_WakeUpClockConfig(RTC_WakeUpClock_RTCCLK_Div16);
    RTC_SetWakeUpCounter(0x1FFF);

    /* Enable the Wakeup Interrupt */
    RTC_ITConfig(RTC_IT_WUT, ENABLE);

    /* Connect EXTI_Line22 to the RTC Wakeup event */
    EXTI_ClearITPendingBit(EXTI_Line19);
    EXTI_InitStructure.EXTI_Line = EXTI_Line19;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* Enable the RTC Wakeup Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = RTC_WKUP_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
#endif
    powerLevel = PWR_ON;
    tmr = xTimerCreate( "sleep",10000/portTICK_RATE_MS,pdFALSE, NULL, timerCb );
    xTimerStart( tmr, 0 );
}
void vApplicationIdleHook( void )
{
    switch ( powerLevel )
    {
        case PWR_ON:
        case PWR_ON_LIGHTS_OUT:
            /* we're active but all tasks have done their stuff, sleep until next interrupt */
            //__WFI();  // <-- this breaks debugging! enable with care...

            break;
        case PWR_STANDBY:
            /* ok to go to cortex "stop mode" (more like standby to me) */
            /* todo turn off leds and stuff */
            /* todo disconnect socket? */
            PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);

            /* Configures system clock after wake-up from STOP: enable HSE, PLL and select
               PLL as system clock source (HSE and PLL are disabled in STOP mode) */
            SYSCLKConfig_STOP();
            break;
    }
}

void pwrKeepAlive()
{
    portBASE_TYPE bHigherPrioTaskWoken;
    xTimerResetFromISR( tmr, &bHigherPrioTaskWoken );
    powerLevel = PWR_ON;
}

void pwrCanPowerOff( int enabled )
{
    if ( enabled )
    {
        canPowerOff = 1;
        if ( powerLevel == PWR_ON_LIGHTS_OUT )
            powerLevel = PWR_ON_LIGHTS_OUT; //PWR_STANDBY;
    }
    else
    {
        canPowerOff = 0;
        if ( powerLevel == PWR_STANDBY )
            powerLevel = PWR_ON_LIGHTS_OUT;
    }
}

int pwrIsAlive()
{
    return powerLevel != PWR_STANDBY ? 1 : 0;
}

static void timerCb( xTimerHandle xTimer )
{
    (void) xTimer;
    powerLevel = canPowerOff != 0 ? PWR_ON_LIGHTS_OUT/*PWR_STANDBY*/ : PWR_ON_LIGHTS_OUT; /*no activity for a while, go to rest*/
}

/**
  * @brief  Configures system clock after wake-up from STOP: enable HSE, PLL
  *         and select PLL as system clock source.
  * @param  None
  * @retval None
  */
static void SYSCLKConfig_STOP(void)
{
  /* After wake-up from STOP reconfigure the system clock */
  /* Enable HSE */
  RCC_HSEConfig(RCC_HSE_ON);

  /* Wait till HSE is ready */
  while (RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET)
  {}

  /* Enable PLL */
  RCC_PLLCmd(ENABLE);

  /* Wait till PLL is ready */
  while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
  {}

  /* Select PLL as system clock source */
  RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

  /* Wait till PLL is used as system clock source */
  while (RCC_GetSYSCLKSource() != 0x08)
  {}
}
