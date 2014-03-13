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

#include "ClockSyncServer.h"
#include "Platform/Utils/Utils.h"
#include "applog.h"

ClockSyncServer::ClockSyncServer()
{
}

ClockSyncServer::~ClockSyncServer()
{
}

#if 0
int
default_get_resolution(void)
{
    //struct timeval tp;
    //struct timezone tzp;
    long last;
    int i;
    long diff;
    long val;
    int minsteps = MINLOOPS;    /* need at least this many steps */

    //gettimeofday(&tp, &tzp);
    last = getTick_ms() * 1000;
    for (i = - --minsteps; i< MAXLOOPS; i++) {
        //gettimeofday(&tp, &tzp);
        uint32_t now = getTick_ms() * 1000;
        diff = now - last;
        if (diff < 0) diff += DUSECS;
        if (diff > MINSTEP) if (minsteps-- <= 0) break;
        last = now;
    }

    log(LOG_NOTICE) << "resolution = " <<diff<< " usec after " << i << " loops";

    diff = (diff *3)/2;
    if (i >= MAXLOOPS) {
        log(LOG_NOTICE) <<
            "     (Boy this machine is fast ! " << MAXLOOPS << " %d loops without a step)";
        diff = 1; /* No STEP, so FAST machine */
    }
    if (i == 0) {
        log(LOG_NOTICE) << "The resolution is less than the time to read the clock -- Assume 1us";
        diff = 1; /* time to read clock >= resolution */
    }
    diff = 1000;
    for (i=0, val=HUSECS; val>0; i--, val >>= 1) if (diff >= val) return i;
    log(LOG_NOTICE) << "     (Oh dear -- that wasn't expected ! I'll guess !)";
    return -99 /* Something's BUST, so lie ! */;
}
#endif

Message* ClockSyncServer::handleRequest( Message* req )
{
    Message* rsp = NULL;

    if ( req->getType() == AUDIO_SYNC_REQ )
    {
        IntTlv* clientClockTlv = (IntTlv*) req->getTlv( TLV_CLIENT_CLOCK );
        if ( clientClockTlv )
        {
            rsp = req->createResponse();
            rsp->addTlv( new IntTlv(*clientClockTlv) );
            rsp->addTlv( TLV_SERVER_CLOCK, getTick_ms() );
        }
    }
    return rsp;
}


