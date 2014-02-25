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

#include "NtpClient.h"
#include "Platform/Socket/Socket.h"
#include "Platform/Utils/Utils.h"
#include "applog.h"
#include "clock.h"

#include <string.h>

NtpClient::NtpClient() : Platform::Runnable(false, SIZE_SMALL, PRIO_VERY_LOW)
{
    startThread();
}

NtpClient::~NtpClient()
{
}

void NtpClient::run()
{
    uint8_t buf[48];

    while( !isCancellationPending() )
    {
        Socket s( SOCKTYPE_DATAGRAM );
        std::set<Socket*> readset;
        bool success = false;

        log( LOG_NOTICE ) << "NTP: Updating time";

        if ( s.Connect( "pool.ntp.org", "123" ) == 0 )
        {
            memset( buf, 0, sizeof(buf) );
            buf[0] = 0x23; // no warning, version 4, I'm a client
            if ( s.Send( buf, sizeof(buf) ) == sizeof( buf ) )
            {
                readset.insert( &s );
                if ( select( &readset, NULL, NULL, 5000 ) == 1 )
                {
                    if ( s.Receive( buf, sizeof(buf) ) == sizeof(buf) )
                    {
                        uint32_t ntptime;
                        memcpy( &ntptime, &buf[40], sizeof(ntptime) );
                        ntptime = Ntohl( ntptime );

                        /* convert to 1970 epoch and set */
                        /* todo some day maybe:
                         * this is directly towards the embedded client wall clock implementation,
                         * since there is no platform agnostic way to set time and it's a PITA to
                         * handle all the different types used to define time..
                         */
                        setTime( ntptime - 2208988800UL );

                        /* I guess would be something like this for linux
                        tv.tv_sec = ntptime - 2208988800UL;
                        tv.tv_usec = the next 4 bytes;
                        settimeofday( &tv, NULL );*/

                        log( LOG_NOTICE ) << "NTP: Success";

                        success = true;
                    }
                    else
                    {
                        log( LOG_NOTICE ) << "NTP: Failed to receive";
                    }
                }
                else
                {
                    log( LOG_NOTICE ) << "NTP: Nothing on select";
                }
            }
            else
            {
                log( LOG_NOTICE ) << "NTP: Failed to send";
            }

            s.Close();
        }
        else
        {
            log( LOG_NOTICE ) << "Failed to connect";
        }

        sleep_ms(success ? 39600000 : 30*60*1000); //get time every 11 hours, or try again in 30 minutes if failed
    }
}

void NtpClient::destroy()
{
    cancelThread();
}
