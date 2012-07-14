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


#include "buttonHandler.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

static UIEmbedded* ui = NULL;

void buttonHandler_setUI(UIEmbedded* ui_)
{
    ui = ui_;
}

static void timerCb( xTimerHandle xTimer )
{
    (void) xTimer;
    /*fire long press*/
    if (ui)
    {
        ui->longButtonPress();
    }
}

void buttonHandler_action(uint8_t pressed)
{
    static portTickType pressTime = 0;
    static xTimerHandle tmr = xTimerCreate( (const signed char *)"btn",1000/portTICK_RATE_MS,pdFALSE, NULL, timerCb );
    if ( pressed )
    {
        portBASE_TYPE higherPrioTaskWoken;
        pressTime = xTaskGetTickCountFromISR();
        /*launch timer for long press detection*/
        xTimerStartFromISR( tmr, &higherPrioTaskWoken );

        if ( higherPrioTaskWoken )
        {
           // vPortYieldFromISR();
        }
    }
    else if ( xTimerIsTimerActive( tmr ) )
    {
        portBASE_TYPE higherPrioTaskWoken;
        xTimerStopFromISR( tmr, &higherPrioTaskWoken );
        portTickType totalTime = xTaskGetTickCountFromISR() - pressTime;

        if ( totalTime > 20 )
        {
            /*fire short press*/
            if (ui)
            {
                ui->shortButtonPress();
            }
        }
        else
        {
            /*just a bounce, do nothing*/
        }

        if ( higherPrioTaskWoken )
        {
           // vPortYieldFromISR();
        }
    }
}
