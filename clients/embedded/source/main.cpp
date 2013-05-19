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
#include "UIEmbedded.h"
#include "TestApp/RemoteMediaInterface.h"
#include "buttonHandler.h"
#include "powerHandler.h"
#include "SocketHandling/SocketClient.h"
//#include "TestApp/AudioEndpointRemoteSocketServer.h"
#include "FreeRTOS.h"
#include "task.h"
#include "applog.h"


#include "Platform/Threads/Runnable.h"

class LedFlasher : public Platform::Runnable
{
public:
    LedFlasher();
    virtual ~LedFlasher();

    virtual void run();
    virtual void destroy();
};

LedFlasher::LedFlasher() : Platform::Runnable(false, SIZE_SMALL, PRIO_LOW)
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
        STM_EVAL_LEDToggle( LED5 );
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

#if 0
    ConfigHandling::NetworkConfig* audioepservercfg = new ConfigHandling::NetworkConfig;
    audioepservercfg->setPort("7789");
    ConfigHandling::AudioEndpointConfig* audiocfg = new ConfigHandling::AudioEndpointConfig;

    AudioEndpointRemoteSocketServer* audioserver = new AudioEndpointRemoteSocketServer( *audiocfg, *audioepservercfg );
#endif

    LedFlasher* fl = new LedFlasher;
#if 0
    SocketClient* sc = new SocketClient("192.168.5.98", "7788");
#else
    SocketClient* sc = new SocketClient("192.168.5.198", "7788");
#endif
    RemoteMediaInterface* m = new RemoteMediaInterface( *sc );
    UIEmbedded* ui = new UIEmbedded(*m);

    pwrInit();
    buttonHandler_setUI(ui);

    /* configure ethernet (GPIOs, clocks, MAC, DMA) */
    ETH_BSP_Config();

    /* Initilaize the LwIP stack */
    LwIP_Init();

    vTaskStartScheduler();

    while (1);
}

