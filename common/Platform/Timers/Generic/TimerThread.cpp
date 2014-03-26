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

#include "TimerThread.h"
#include "../Timer.h"
#include "Platform/Utils/Utils.h"
#include <assert.h>
#include <stdint.h>

namespace Platform
{
TimerThread* timers = NULL; //the timer framework singleton

void initTimers()
{
    timers = new TimerThread;
}

void deinitTimers()
{
    timers->destroy();
    delete timers;
    timers = NULL;
}


typedef struct TimerEntry_s
{
    Timer* t;
    bool isPeriodic;
    unsigned int period;
    unsigned int nextTimeout;
} TimerEntry_t;

TimerThread::TimerThread()
{
    assert( timers == NULL );
    startThread();
}

TimerThread::~TimerThread()
{
}

void TimerThread::AddTimer( Timer* t, bool isPeriodic, unsigned int timeout )
{
    TimerEntry_t entry = { t, isPeriodic, timeout, getTick_ms() + timeout };

    mtx.lock();
    std::list<TimerEntry_t>::iterator it = timerList.begin();
    while( it != timerList.end() && (*it).nextTimeout < entry.nextTimeout )
        it++;

    if ( it == timerList.begin() )
        cond.signal();

    timerList.insert( it, entry );

    mtx.unlock();
}

void TimerThread::CancelTimer( Timer* t )
{
    mtx.lock();
    std::list<TimerEntry_t>::iterator it = timerList.begin();
    while( it != timerList.end() )
    {
        if ( (*it).t == t )
        {
            if ( it == timerList.begin() )
                cond.signal();

            timerList.erase(it);
            mtx.unlock();
            return;
        }
        it++;
    }
    mtx.unlock();
}

bool TimerThread::IsTimerRunning( Timer* t )
{
    mtx.lock();
    std::list<TimerEntry_t>::iterator it = timerList.begin();
    while( it != timerList.end() )
    {
        if ( (*it).t == t )
        {
            mtx.unlock();
            return true;
        }
        it++;
    }
    mtx.unlock();
    return false;
}

void TimerThread::run()
{
    uint32_t timeToNext = 0;

    mtx.lock();
    while(!isCancellationPending())
    {
        if ( timerList.empty() )
            cond.wait(mtx);
        else
            cond.timedWait( mtx, timeToNext );

        bool isExpired = false;
        do
        {
            if ( timerList.empty() )
            {
                timeToNext = 0;
                break;
            }

            timeToNext = timerList.front().nextTimeout - getTick_ms();
            isExpired = ( timeToNext >= 0xFFFFFFF );
            if ( isExpired ) // "negative" number if expired, this should handle wrap, I think..
            {
                TimerEntry_t t = timerList.front();
                timerList.pop_front();
                t.t->Expired();

                if ( t.isPeriodic )
                {
                    t.nextTimeout += t.period;
                    std::list<TimerEntry_t>::iterator it = timerList.begin();
                    while( it != timerList.end() && (*it).nextTimeout < t.nextTimeout )
                        it++;

                    timerList.insert( it, t );
                }
            }
        } while( isExpired );
    }
    mtx.unlock();
}

void TimerThread::destroy()
{
    cancelThread();
    cond.signal();
    joinThread();
}



}
