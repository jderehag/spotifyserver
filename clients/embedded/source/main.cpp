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
#include "NtpClient.h"
#include "clock.h"
#include "params/params.h"

#ifdef WITH_LCD
#include "stm32f4_discovery_lcd.h"
#include <stdio.h>
#include <string.h>
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
        time_t rawtime;
        struct tm timeinfo;
        char tmp[10] = {0};
        sFONT* font = &Font8x8;

        /* flashing dot */
        LCD_DisplayChar( 0,0, (count & 1) ? '.' : ' ', White, Black, font );

        /* debug counters and stuff */
        sprintf( tmp, "%5d", clockDrift );
        LCD_DisplayStringLineCol( 0, 2*font->Width, strlen(tmp)*font->Width, tmp, White, Black, font );

        /* clock in top right corner */
        time (&rawtime);
        localtime_r (&rawtime, &timeinfo);
        strftime( tmp, sizeof(tmp), "%H:%M:%S", &timeinfo );
        LCD_DisplayStringLineCol( 0, 31*font->Width, 8*font->Width, tmp, White, Black, font );


#ifdef DEBUG_COUNTERS
        log(LOG_NOTICE) <<  missingsamples << " " << totalsamples << " " << totalpadsamples;
#endif
#else
        BSP_LED_Toggle( LED5 );
#endif
        vTaskDelayUntil( &t, delay );
        count++;
    }
}

class Main : public Platform::Runnable
{
public:
    Main();
    virtual ~Main();

    virtual void run();
    virtual void destroy();
};

Main::Main() : Platform::Runnable(false, SIZE_SMALL, PRIO_VERY_HIGH)
{
    startThread();
}

void Main::run()
{
    clockInit();

    Logger::LoggerEmbedded* l = new Logger::LoggerEmbedded(LOG_NOTICE);
    LedFlasher* fl = new LedFlasher;

    /* Initialise the LwIP stack */
    LwIP_Init();

#ifdef WITH_TIME
    NtpClient* nc = new NtpClient();
    putenv( "TZ=CET-1CEST-2,M3.5.0/2,M10.5.0/3" ); /* initialize current time zone to CET, with DST switch */
#endif

    std::string id;
    if ( paramsGet( PARAM_CLIENT_ID, id ) )
    {
        log(LOG_NOTICE) << "Hi! My name is (what?)";
        log(LOG_NOTICE) << "My name is (who?)";
        log(LOG_NOTICE) << "My name is (shika-shika) " << id;
    }

    EndpointId epId(id);
#if 0
    SocketClient* sc = new SocketClient("192.168.5.98", "7788", epId);
#else
    SocketClient* sc = new SocketClient("ANY", "7788", epId);
#endif

    ConfigHandling::AudioEndpointConfig* audiocfg = new ConfigHandling::AudioEndpointConfig;
    Platform::AudioEndpointLocal* audioEndpoint = new Platform::AudioEndpointLocal( *audiocfg, epId );
    RemoteAudioEndpointManager* audioMgr = new RemoteAudioEndpointManager( *sc );
    audioMgr->createEndpoint( *audioEndpoint, NULL, NULL );

    RemoteMediaInterface* m = new RemoteMediaInterface( *sc );
    UIEmbedded* ui = new UIEmbedded(*m);

    pwrInit();
    buttonHandler_setUI(ui);

    /* now die */
}

Main::~Main()
{
}

void Main::destroy()
{
}
extern "C" void InitializeAudio();
int main(void)
{
    Main* m = new Main();
    BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_EXTI);
    BSP_LED_Init(LED4);

    InitializeAudio();

#ifndef WITH_LCD
    BSP_LED_Init(LED3);
    BSP_LED_Init(LED5);
    BSP_LED_Init(LED6);
#else
    BSP_LCD_Init();
#endif

    paramsInit();

    vTaskStartScheduler();
}

