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
#include "RemoteMediaInterface.h"
#include "buttonHandler.h"
#include "powerHandler.h"
#include "SocketHandling/SocketClient.h"
#include "AudioEndpointManager/RemoteAudioEndpointManager.h"
#include "AudioEndpointRemoteSocketServer.h"
#include "Platform/AudioEndpoints/AudioEndpointLocal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "applog.h"
#include "LoggerEmbedded.h"

#ifdef WITH_LCD
#include "stm32f4_discovery_lcd.h"
#include <stdio.h>
#endif

#include "Platform/Threads/Runnable.h"

class LedFlasher : public Platform::Runnable
{
public:
    LedFlasher();
    virtual ~LedFlasher();

    virtual void run();
    virtual void destroy();
};

LedFlasher::LedFlasher() : Platform::Runnable(false, SIZE_SMALL, PRIO_VERY_LOW)
{
    startThread();
}
LedFlasher::~LedFlasher()
{
}

void LedFlasher::destroy()
{

}
#ifdef DEBUG_COUNTERS
extern int lasttimetoplay;
extern int lasttimestamp;
extern int lastmissingsamples;
extern int lastpacketsamples;
extern int lastpacketms;
extern int lastpadsamples;
extern int totalsamples;
extern int totaltimems;
extern int totalpadsamples;
extern int totalservertimems;

extern int servertimeplay;
extern int clienttimeplay;
extern uint32_t totalrecsamples;
#endif
extern int clockDrift;
void LedFlasher::run()
{
    portTickType delay = 500 / portTICK_RATE_MS;
    portTickType t = xTaskGetTickCount();
    int count = 0;

    while( isCancellationPending() == false )
    {
#ifdef WITH_LCD
        sFONT* font = &Font8x8;
        char clockdiff[10];
        LCD_DisplayChar( 0,0, (count & 1) ? '.' : ' ', White, Black, font );
        sprintf( clockdiff, "%5d", clockDrift );
        LCD_DisplayStringLineCol( 0, 2*font->Width, clockdiff, White, Black, font );
#ifdef DEBUG_COUNTERS
        log(LOG_NOTICE) <<  missingsamples << " " << totalsamples << " " << totalpadsamples;
#endif
#else
        STM_EVAL_LEDToggle( LED5 );
#endif
        vTaskDelayUntil( &t, delay );
        count++;
    }
}

int main(void)
{
    STM_EVAL_PBInit(BUTTON_USER, BUTTON_MODE_EXTI);
    STM_EVAL_LEDInit(LED4);
#ifndef WITH_LCD
    STM_EVAL_LEDInit(LED3);
    STM_EVAL_LEDInit(LED5);
    STM_EVAL_LEDInit(LED6);
#endif

#ifdef WITH_LCD
    STM32f4_Discovery_LCD_Init();
#endif

    Logger::LoggerEmbedded* l = new Logger::LoggerEmbedded(LOG_NOTICE);
    LedFlasher* fl = new LedFlasher;

#if 0
    SocketClient* sc = new SocketClient("192.168.5.98", "7788");
#else
    SocketClient* sc = new SocketClient("", "7788");
#endif

    ConfigHandling::AudioEndpointConfig* audiocfg = new ConfigHandling::AudioEndpointConfig;
    Platform::AudioEndpointLocal* audioEndpoint = new Platform::AudioEndpointLocal( *audiocfg );
    RemoteAudioEndpointManager* audioMgr = new RemoteAudioEndpointManager( *sc );
    audioMgr->addEndpoint( *audioEndpoint, NULL, NULL );

    RemoteMediaInterface* m = new RemoteMediaInterface( *sc );
    UIEmbedded* ui = new UIEmbedded(*m);

    pwrInit();
    buttonHandler_setUI(ui);

    /* configure ethernet (GPIOs, clocks, MAC, DMA) */
    ETH_BSP_Config();

    /* Initialise the LwIP stack */
    LwIP_Init();

    vTaskStartScheduler();

    while (1);
}

