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

#include "ClockSyncClient.h"
#include "Platform/Socket/Socket.h"
#include "Platform/Utils/Utils.h"
#include "applog.h"

#include <string.h>
//#include <tgmath.h>
#include <stdlib.h>

ClockSyncClient::ClockSyncClient() :    hasValidSync_(false),
                                        waitingForResponse_(false),
                                        lastSync_(0),
                                        lastResponse_(0),
                                        clockDiff_(0),
                                        nsamples(0),
                                        nextpt(0)
{
    memset( samples, 0, sizeof(samples) );
}

ClockSyncClient::~ClockSyncClient()
{
}


bool ClockSyncClient::hasValidSync()
{
    /* todo invalidate when old! */
    return hasValidSync_;
}
bool ClockSyncClient::timeToSync()
{
    uint32_t now = getTick_ms();
    if ( now - lastResponse_ > 100 && now - lastSync_ > 50)
    {
        lastSync_ = getTick_ms();
        return true;
    }
    return false;
}

Message* ClockSyncClient::createRequest()
{
    lastSync_ = getTick_ms();
    Message* req = new Message(AUDIO_SYNC_REQ);
    req->addTlv(TLV_CLIENT_CLOCK, lastSync_ - clockDiff_ );

    return req;
}

static uint32_t firstDiff = 0;
int clockDrift = 0;
void ClockSyncClient::handleResponse( Message* rsp )
{
    if ( rsp->getType() == AUDIO_SYNC_RSP )
    {
        IntTlv* clientClockTlv = (IntTlv*) rsp->getTlv( TLV_CLIENT_CLOCK );
        IntTlv* serverClockTlv = (IntTlv*) rsp->getTlv( TLV_SERVER_CLOCK );

        if ( clientClockTlv && serverClockTlv )
        {
            uint32_t now = getTick_ms();

            uint32_t t1 = clientClockTlv->getVal();
            uint32_t t2 = serverClockTlv->getVal();
            uint32_t t3 = t2; // assume T2 and T3 are equal
            uint32_t t4 = convertToServerTime( now );

            int64_t t21 = (int64_t)t2 - t1;
            int64_t t34 = (int64_t)t3 - t4;

            int64_t offset = ( t21 + t34 ) / 2;
            //int32_t delay = abs(t21 - t34); //t21 and t34 should be fairly equal so capping to 32 bit is ok

            samples[ nextpt ] = offset;
            nextpt = ( nextpt + 1) % SYNC_SHIFT;

            if ( nsamples < SYNC_SHIFT ) nsamples++;

            /* calculate an average over the stored offset samples */
            int64_t adjust = 0;
            for ( uint8_t i=0; i<nsamples; i++ )
            {
                adjust += samples[i];
            }
            adjust /= nsamples;

            /* adjust our clock */
            clockDiff_ -= adjust;

            if ( firstDiff == 0 ) firstDiff = clockDiff_;
            clockDrift = clockDiff_ - firstDiff;

            /* adjust the stored samples so they are relative to the new clockDiff_ and can be used again */
            for ( uint8_t i=0; i<nsamples; i++ )
            {
                samples[i] -= adjust;
            }

            //log(LOG_NOTICE) << "adjust   " << adjust << " to " << clockDiff_;

            hasValidSync_ = true;
            lastResponse_ = now;
        }
    }
}

uint32_t ClockSyncClient::convertToLocalTime( uint32_t serverTime )
{
    return serverTime + clockDiff_;
}

uint32_t ClockSyncClient::convertToServerTime( uint32_t localTime )
{
    return localTime - clockDiff_;
}

