/*
 * Copyright (c) 2012, Jens Nielsen
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


#include "stm32f4_discovery.h"
#include "stm32f4x7_eth_bsp.h"
#include "netconf.h"
#include "Platform/Threads/Runnable.h"
#include "Platform/Threads/Messagebox.h"
#include "UIEmbedded.h"
#include "buttonHandler.h"
#include "FreeRTOS.h"
#include "task.h"
//#include "portable.h"
#include "applog.h"
#include "MessageFactory/Message.h"

#if 1
extern "C" void  *malloc(size_t size)
{
  return pvPortMalloc(size);
}

//............................................................................
extern "C" void *calloc(size_t count, size_t size)
{
  return pvPortMalloc(count*size);
}

#include <string.h>
extern "C" void *realloc(void *p, size_t size)
{
    if ( p == NULL )
        return pvPortMalloc(size);
    else
    {
        void* p2 = pvPortMalloc(size);
        if ( p2 != NULL )
        {
            memcpy(p2, p, size);
            vPortFree(p);
        }
        return p2;
    }
}
//............................................................................
extern "C" void free(void *p)
{
    vPortFree(p);
}

//override new and delete operators for c++
void *operator new(size_t size)
{
    return pvPortMalloc(size);
}

void *operator new[](size_t size)
{
    return pvPortMalloc(size);
}

void operator delete(void *p){
	STM_EVAL_LEDToggle(LED5);
	vPortFree( p );
}

void operator delete[](void *p){
	vPortFree( p );
}
#endif

class LedFlasher : public Platform::Runnable
{
public:
    LedFlasher();
    virtual ~LedFlasher();

    virtual void run();
    virtual void destroy();
};

LedFlasher::LedFlasher() : Platform::Runnable(false)
{
    startThread();
}
LedFlasher::~LedFlasher()
{
}

void LedFlasher::destroy()
{

}

void LedFlasher::run()
{
    portTickType delay = 500 / portTICK_RATE_MS;
    portTickType t = xTaskGetTickCount();

    while( isCancellationPending() == false )
    {
        vTaskDelayUntil( &t, delay );
        STM_EVAL_LEDToggle( LED3 );
    }
}


int main(void)
{
    STM_EVAL_LEDInit(LED3);
    STM_EVAL_LEDInit(LED4);
    STM_EVAL_LEDInit(LED5);
    STM_EVAL_LEDInit(LED6);
    STM_EVAL_PBInit(BUTTON_USER, BUTTON_MODE_EXTI);


    ConfigHandling::LoggerConfig* cfg = new ConfigHandling::LoggerConfig;
    cfg->setLogTo(ConfigHandling::LoggerConfig::NOWHERE);
    Logger::Logger* l = new Logger::Logger(*cfg);

    LedFlasher* fl = new LedFlasher;
    Messenger* m = new Messenger("192.168.5.198");
    UIEmbedded* ui = new UIEmbedded(*m);

    buttonHandler_setUI(ui);



    /* configure ethernet (GPIOs, clocks, MAC, DMA) */
    //ETH_BSP_Config();

    /* Initilaize the LwIP stack */
    //LwIP_Init();

    vTaskStartScheduler();

    while (1);
}

