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
#include <cstddef>

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


struct TimerEntry_t
{
    Timer* t;
    bool isPeriodic;
    unsigned int period;
    unsigned int nextTimeout;
};

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
    // make sure it's not already on the list
    RemoveTimer( t );

    // list is sorted by timeout, find position to put this timer
    std::list<TimerEntry_t>::iterator it = timerList.begin();
    while( it != timerList.end() && ((*it).nextTimeout - entry.nextTimeout) > 0x80000000 )
        it++;

    // if this one will be first in list, wake up thread to re-evaluate sleep delay
    if ( it == timerList.begin() )
        cond.signal();

    // put on list, we have the mutex so the thread shouldn't wake up before this
    timerList.insert( it, entry );

    mtx.unlock();
}

void TimerThread::CancelTimer( Timer* t )
{
    mtx.lock();
    RemoveTimer( t );
    mtx.unlock();
}

// internal function to remove timer from list, assumes mutex is already locked
void TimerThread::RemoveTimer( Timer* t )
{
    std::list<TimerEntry_t>::iterator it = timerList.begin();
    while( it != timerList.end() )
    {
        if ( (*it).t == t )
        {
            //just remove it, no need to wake up thread since the next timeout will be equal to or later than this one if this one was first
            timerList.erase(it);
            return;
        }
        it++;
    }
}

bool TimerThread::IsTimerRunning( Timer* t )
{
    bool ret = false;
    mtx.lock();
    // iterate list and check if this one is on it
    std::list<TimerEntry_t>::iterator it = timerList.begin();
    while( it != timerList.end() )
    {
        if ( (*it).t == t )
        {
            ret = true;
            break;
        }
        it++;
    }
    mtx.unlock();
    return ret;
}

void TimerThread::run()
{
    uint32_t timeToNext = 0;

    mtx.lock();
    while(!isCancellationPending())
    {
        if ( timerList.empty() )
            cond.wait( mtx );
        else
            cond.timedWait( mtx, timeToNext );

        bool isExpired = false;
        do
        {
            if ( timerList.empty() )
            {
                // bail, no more timers
                timeToNext = 0;
                break;
            }

            timeToNext = timerList.front().nextTimeout - getTick_ms();
            // todo allow timeToNext == 0 here as well but evaluate what to do with timers with timeout 0
            isExpired = ( timeToNext >= 0xFFFFF000 ); // "negative" number if expired, assuming we check within 0xFFF ms from actual timeout. This should also handle wrap, I think..
            if ( isExpired )
            {
                TimerEntry_t t = timerList.front();
                timerList.pop_front();
                t.t->Expired();

                if ( t.isPeriodic )
                {
                    // periodic timer, update timeout and put back at correct position in list
                    t.nextTimeout += t.period;
                    std::list<TimerEntry_t>::iterator it = timerList.begin();
                    while( it != timerList.end() && (*it).nextTimeout <= t.nextTimeout )
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
