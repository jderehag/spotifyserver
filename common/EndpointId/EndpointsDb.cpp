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

#include "EndpointsDb.h"
#ifdef __CYGWIN__
#include <sstream>
#endif
EndpointsDb::EndpointsDb( const AudioEndpointManager& audioMgr_ ) : audioMgr(audioMgr_)
{

}

EndpointsDb::~EndpointsDb()
{
}

void EndpointsDb::registerId( EndpointIdIf* appId )
{
    DbContainer::iterator it = find( appId->getId() );
    if ( it != epIds.end() )
    {
        std::string id = (*it)->getId();
        do
        {
            size_t dashPos = id.find_last_of('-');
            std::string numstr = id.substr(dashPos+1, std::string::npos);
            if ( !numstr.empty() && numstr.find_first_not_of("0123456789") == std::string::npos )
            {
                long num = strtol( numstr.c_str(), NULL, 10 );
                num++;
                id = id.substr(0, dashPos+1);
#ifndef __CYGWIN__
                id += std::to_string( num );
#else
                {
                    std::ostringstream portStr;
                    portStr << num;
                    id += portStr.str();
                }
#endif
            }
            else
            {
                id += "-0";
            }
        } while ( find( id ) != epIds.end() );
        (*it)->rename(id);
    }
    epIds.push_back( appId );
}

void EndpointsDb::unregisterId( EndpointIdIf* appId )
{
    epIds.remove( appId );
}

void EndpointsDb::getIds( EndpointsDbSubscriber* subscriber, void* userData )
{
    std::set<std::string> out;
    DbContainer::iterator it = epIds.begin();
}

void EndpointsDb::rename( std::string from, std::string to )
{
    DbContainer::iterator it = find( from );
    if ( it != epIds.end() )
    {
        (*it)->rename( to );
    }
}

EndpointsDb::DbContainer::iterator EndpointsDb::find( const std::string& id )
{
    DbContainer::iterator it = epIds.begin();
    for ( ; it != epIds.end(); it++ )
    {
        if ( (*it)->getId().compare( id ) == 0 )
            break;
    }
    return it;
}

