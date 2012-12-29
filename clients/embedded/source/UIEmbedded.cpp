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


#include "UIEmbedded.h"
#include "applog.h"
#include "stm32f4_discovery.h"

UIEmbedded::UIEmbedded( MediaInterface& m ) : m_(m)
{
    m_.registerForCallbacks( *this );
    itPlaylists_ = playlists.begin();
}

UIEmbedded::~UIEmbedded()
{
    m_.unRegisterForCallbacks( *this );
}

void UIEmbedded::shortButtonPress()
{
    switch( m_.getCurrentPlaybackState() )
    {
        case PLAYBACK_IDLE:
            if( itPlaylists_ != playlists.end() )
                m_.play((*itPlaylists_).getLink()); // -> PLAYING
            break;

        case PLAYBACK_PAUSED:
            m_.resume(); // -> PLAYING
            break;

        case PLAYBACK_PLAYING:
            m_.pause(); // -> PAUSED
            break;
    }
}
void UIEmbedded::longButtonPress()
{
    switch( m_.getCurrentPlaybackState() )
    {
        case PLAYBACK_IDLE:
        case PLAYBACK_PAUSED:
            if( !playlists.empty() )
            {
                itPlaylists_++;

                if( itPlaylists_ == playlists.end())
                    itPlaylists_ = playlists.begin();

                m_.play((*itPlaylists_).getLink()); // -> PLAYING
            }
            break;

        case PLAYBACK_PLAYING:
            m_.next(); // -> PLAYING
            break;
    }
}

void UIEmbedded::rootFolderUpdatedInd( Folder& f )
{
    playlists.clear();

    for( LibSpotify::FolderContainer::iterator it = f.getFolders().begin(); it != f.getFolders().end() ; it++)
        playlists.insert( playlists.end(), (*it).getPlaylists().begin(), (*it).getPlaylists().end());

    playlists.insert( playlists.end(), f.getPlaylists().begin(), f.getPlaylists().end());

    itPlaylists_ = playlists.begin();
}

void UIEmbedded::connectionState( bool up )
{
    if ( up )
    {
        /*new connection, check status and get playlists*/
        m_.getStatus();
        m_.getPlaylists();

        /*make sure we shuffle (should be controlled by button though..)*/
        m_.setShuffle(true);

        //addAudio();
        STM_EVAL_LEDOn( LED4 );
    }
    else
    {
        STM_EVAL_LEDOff( LED4 );
    }
}

void UIEmbedded::getTracksResponse( unsigned int reqId, const std::deque<Track>& tracks )
{
}


