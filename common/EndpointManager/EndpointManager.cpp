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

#include "EndpointManager.h"
#include "applog.h"
#include <assert.h>

EndpointManager::EndpointManager( AudioProvider& m ) : m_(m)
{
}

EndpointManager::~EndpointManager()
{
}

void EndpointManager::registerId( EndpointIdIf& appId )
{
    std::string id = appId.getId();
    EndpointContainer::iterator it = find( id );
    if ( it != endpoints.end() )
    {
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
        } while ( find( id ) != endpoints.end() );
        (*it).epId.rename(id);
    }
    endpoints.push_back( EndpointItem( appId ) );
}

void EndpointManager::unregisterId( EndpointIdIf& appId )
{
    EndpointContainer::iterator it = find( appId.getId() );
    if ( it != endpoints.end() )
        endpoints.erase( it );
}

void EndpointManager::getEndpoints( IEndpointCtrlCallbackSubscriber* subscriber, void* userData )
{
    EndpointInfoList result;
    EndpointContainer::const_iterator it = endpoints.begin();
    for( ; it != endpoints.end(); it++ )
    {
        result.push_back( (*it).epId.getId() );
    }

    result.sort();

    subscriber->getEndpointsResponse( result, userData );
}

void EndpointManager::renameEndpoint( const std::string& from, const std::string& to, IEndpointCtrlCallbackSubscriber* subscriber, void* userData )
{
    EndpointContainer::iterator toEp = find( to );
    EndpointContainer::iterator fromEp = find( from );
    if ( fromEp == endpoints.end() || toEp != endpoints.end() )
    {
        /* no can do, doesn't exist or new name already taken */
        // todo, error response
        return;
    }

    (*fromEp).epId.rename( to );
    if ( (*fromEp).audioEp != NULL )
        doAudioEndpointsUpdatedNotification();

    subscriber->renameEndpointResponse( userData );
}

void EndpointManager::createAudioEndpoint( Platform::AudioEndpoint& ep, IEndpointCtrlCallbackSubscriber* subscriber, void* userData )
{
    EndpointContainer::iterator it = find( ep.getId() );
    if ( it != endpoints.end() )
    {
        assert( (*it).audioEp== NULL );
        (*it).audioEp =  &ep;

        //todo: subscriber->createEndpointResponse();

        doAudioEndpointsUpdatedNotification(); //todo only if didn't exist already?
    }
}

void EndpointManager::deleteAudioEndpoint( Platform::AudioEndpoint& ep, IEndpointCtrlCallbackSubscriber* subscriber, void* userData )
{
    EndpointContainer::iterator it = find( ep.getId() );
    if ( it != endpoints.end() && (*it).audioEp != NULL )
    {
        assert( (*it).audioEp == &ep );
        if ( (*it).audioSource != NULL )
        {
            (*it).audioSource->removeAudioEndpoint(ep);
            (*it).audioSource = NULL;
        }
        (*it).audioEp = NULL;

        doAudioEndpointsUpdatedNotification();
    }

    //todo: subscriber->deleteEndpointResponse();
}

void EndpointManager::addAudioEndpoint( const std::string& id, IEndpointCtrlCallbackSubscriber* subscriber, void* userData )
{
    EndpointContainer::iterator it = find( id );
    if ( it != endpoints.end() && (*it).audioEp != NULL )
    {
        m_.addAudioEndpoint( *(*it).audioEp );
        (*it).audioSource = &m_;

        doAudioEndpointsUpdatedNotification();
    }
    //todo: subscriber->addEndpointResponse();
}

void EndpointManager::removeAudioEndpoint( const std::string& id, IEndpointCtrlCallbackSubscriber* subscriber, void* userData )
{
    EndpointContainer::iterator it = find( id );
    if ( it != endpoints.end() && (*it).audioEp != NULL )
    {
        m_.removeAudioEndpoint( *(*it).audioEp );
        (*it).audioSource = NULL;

        doAudioEndpointsUpdatedNotification();
    }
    //todo: subscriber->removeEndpointResponse();
}

void EndpointManager::getAudioEndpoints( IEndpointCtrlCallbackSubscriber* subscriber, void* userData )
{
    AudioEndpointInfoList result;

    EndpointContainer::const_iterator it = endpoints.begin();
    for( ; it != endpoints.end(); it++ )
    {
        if ( (*it).audioEp != NULL )
            result.push_back( AudioEndpointInfo((*it).epId.getId(), (*it).audioSource != NULL, (*it).audioEp->getRelativeVolume()) );
    }

    result.sort();

    subscriber->getAudioEndpointsResponse( result, userData );
}


Platform::AudioEndpoint* EndpointManager::getAudioEndpoint( const std::string& id )
{
    EndpointContainer::const_iterator it = endpoints.begin();
    for( ; it != endpoints.end(); it++ )
    {
        if ( (*it).epId.getId().compare( id ) == 0 )
        {
            return (*it).audioEp;
        }
    }
    return NULL;
}

EndpointManager::EndpointContainer::iterator EndpointManager::find( const std::string& id )
{
    EndpointContainer::iterator it = endpoints.begin();
    for ( ; it != endpoints.end(); it++ )
    {
        if ( (*it).epId.getId().compare( id ) == 0 )
            break;
    }
    return it;
}

void EndpointManager::setRelativeVolume( const std::string& id, uint8_t volume )
{
    Platform::AudioEndpoint* ep = getAudioEndpoint( id );
    if ( ep && volume != ep->getRelativeVolume() )
    {
        ep->setRelativeVolume( volume );

        doAudioEndpointsUpdatedNotification();
    }
}

void EndpointManager::getStatistics( const std::string& id, IEndpointCtrlCallbackSubscriber* subscriber, void* userData )
{
    Platform::AudioEndpoint* ep = getAudioEndpoint( id );
    if ( ep )
    {
        const Counters& c = ep->getStatistics();
        CounterList stats;
        for ( uint8_t i = 0; i < c.getNrofCounters(); i++ )
        {
            stats.push_back(CounterListItem(c.getCounterName(i), c.getCounterValue(i)));
        }

        subscriber->getStatisticsResponse( id, stats, userData );
    }
}

