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

#include "../Timer.h"
#include "FreeRTOS.h"
#include "timers.h"

namespace Platform
{

struct Timer_t
{
    TimerHandle_t tmr;
};

static void timerCb( TimerHandle_t xTimer )
{
    Timer* me = reinterpret_cast<Timer*>(pvTimerGetTimerID( xTimer ));
    me->Expired();
}

Timer::Timer() : cb_(NULL), arg_(NULL)
{
    timer_ = new Timer_t;
    timer_->tmr = NULL;
}

Timer::~Timer()
{
    delete timer_;
}

void Timer::Start( unsigned int timeout, bool isPeriodic, TimerCallbackFn cb, void* arg )
{
    cb_ = cb;
    arg_ = arg;
    if ( timer_->tmr != NULL )
        xTimerChangePeriod( timer_->tmr, timeout/portTICK_PERIOD_MS, 0 );
    else
        timer_->tmr = xTimerCreate("tmr", timeout/portTICK_PERIOD_MS, isPeriodic ? pdTRUE : pdFALSE, this, timerCb );
    xTimerStart( timer_->tmr, 0 );
}

void Timer::Cancel()
{
    xTimerStop( timer_->tmr, 0 );
}

void Timer::Expired()
{
    cb_(arg_);
}

bool Timer::IsRunning()
{
    return timer_->tmr ? ( xTimerIsTimerActive( timer_->tmr ) == pdTRUE ) : false;
}


// Empty "timer framework" functions since this is handled entirely by FreeRTOS
void initTimers()
{
}
void deinitTimers()
{
}


}
