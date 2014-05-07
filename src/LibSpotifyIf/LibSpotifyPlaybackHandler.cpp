/*
 * Copyright (c) 2012, Jesper Derehag
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
 * DISCLAIMED. IN NO EVENT SHALL JESPER DEREHAG BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "LibSpotifyPlaybackHandler.h"
#include "LibSpotifyIf.h"
#include <algorithm>
#include <ctime>
#include <random>
#include <cassert>
#include "applog.h"

namespace LibSpotify
{

LibSpotifyPlaybackHandler::LibSpotifyPlaybackHandler(LibSpotifyIf& libspotify) : libSpotifyIf_(libspotify),
                                                                                 historyQueue_(HISTORY_QUEUE_DEPTH),
                                                                                 isPlayingQueuedTrack(false),
                                                                                 isShuffle(false),
                                                                                 isRepeat(false),
                                                                                 shuffleGenerator((unsigned int)time(0))
{
    playQueueIter_ = playQueue_.end();
}

LibSpotifyPlaybackHandler::~LibSpotifyPlaybackHandler() { }

void LibSpotifyPlaybackHandler::doPlayTrack( TrackQueue::iterator t )
{
    libSpotifyIf_.playTrack(*t);
}

void LibSpotifyPlaybackHandler::playTrack(const Track& track)
{
    mtx_.lock();
    enquedQueue_.clear();
    playQueue_.clear();
    currentlyPlayingFromName_ = track.getName();
    currentlyPlayingFromUri_ = track.getLink();
    currentlyPlayingFromType_ = TRACK;
    playQueue_.push_back(track);
    playQueueIter_ = playQueue_.begin();
    if( playQueueIter_ != playQueue_.end() )
        doPlayTrack(playQueueIter_);
    else
        libSpotifyIf_.stop();
    isPlayingQueuedTrack = false;
    mtx_.unlock();
}

void LibSpotifyPlaybackHandler::playFolder(const Folder& folder)
{
    mtx_.lock();
    enquedQueue_.clear();
    playQueue_.clear();
    currentlyPlayingFromName_ = folder.getName();
    currentlyPlayingFromUri_ = ""; //Unknown so far as how to get the folder link..
    currentlyPlayingFromType_ = FOLDER;
    folder.getAllTracks(playQueue_);
    playQueueIter_ = playQueue_.begin();
    if( playQueueIter_ != playQueue_.end() )
        doPlayTrack(playQueueIter_);
    else
        libSpotifyIf_.stop();
    isPlayingQueuedTrack = false;
    mtx_.unlock();
}

void LibSpotifyPlaybackHandler::playPlaylist( const Playlist& playlist, int startIndex )
{
    mtx_.lock();
    currentlyPlayingFromType_ = PLAYLIST;
    loadPlaylist( playlist, startIndex );
    if( playQueueIter_ != playQueue_.end() )
        doPlayTrack(playQueueIter_);
    else
        libSpotifyIf_.stop();
    isPlayingQueuedTrack = false;
    mtx_.unlock();

}

void LibSpotifyPlaybackHandler::playAlbum( const Album& album, int startIndex )
{
    mtx_.lock();
    currentlyPlayingFromType_ = ALBUM;
    loadPlaylist( album, startIndex );
    if( playQueueIter_ != playQueue_.end() )
        doPlayTrack(playQueueIter_);
    else
        libSpotifyIf_.stop();
    isPlayingQueuedTrack = false;
    mtx_.unlock();
}

void LibSpotifyPlaybackHandler::loadPlaylist( const Playlist& playlist, int startIndex )
{
    enquedQueue_.clear();
    playQueue_.clear();
    currentlyPlayingFromName_ = playlist.getName();
    currentlyPlayingFromUri_ = playlist.getLink();
    // copy all available tracks to play queue
    for( std::deque<Track>::const_iterator it = playlist.getTracks().begin(); it != playlist.getTracks().end(); it++ )
    {
        if ( (*it).isAvailable() )
        {
            playQueue_.push_back( *it );
        }
        else if ( startIndex >= 0 && (*it).getIndex() == startIndex )
        {
            // requested track is unavailable
            if ( isShuffle )
                startIndex = -1; // if shuffle, just use any other track
            else
                startIndex++; // else try next one
        }
    }
    playQueueIter_ = playQueue_.begin();
    if ( startIndex >= 0 )
    {
        while ( ( playQueueIter_ != playQueue_.end() ) && ( (*playQueueIter_).getIndex() != startIndex ) )
            playQueueIter_++;

        if ( playQueueIter_ != playQueue_.end() && isShuffle )
        {
            Track first = *playQueueIter_;
            playQueue_.erase(playQueueIter_);
            shuffle();
            playQueue_.push_front(first);
            playQueueIter_ = playQueue_.begin();
        }
    }
    else if ( isShuffle )
    {
        shuffle();
        playQueueIter_ = playQueue_.begin();
    }
}

void LibSpotifyPlaybackHandler::playSearchResult(const std::string& searchString,
                                                 const std::string& searchLink,
                                                 const TrackQueue& searchResult)
{
    mtx_.lock();
    playQueue_.clear();
    currentlyPlayingFromName_ = searchString;
    currentlyPlayingFromUri_ = searchLink;
    currentlyPlayingFromType_ = SEARCH;
    playQueue_ = searchResult;
    playQueueIter_ = playQueue_.begin();
    doPlayTrack(playQueueIter_);
    isPlayingQueuedTrack = false;
    mtx_.unlock();
}

void LibSpotifyPlaybackHandler::enqueueTrack(const Track& track)
{
    mtx_.lock();
    enquedQueue_.push_back(track);
    mtx_.unlock();
}

void LibSpotifyPlaybackHandler::playPrevious( unsigned int currentTrackProgress )
{
    mtx_.lock();
    if ( !isPlayingQueuedTrack )
    {
        if ( !playQueue_.empty() )
        {
            if ( ( playQueueIter_ == playQueue_.begin() && !isRepeat ) || // it's the first track without repeat or..
                 ( ((float)currentTrackProgress / (*playQueueIter_).getDurationMillisecs()) > 0.02 ) ) //.. we've played this track for a while
            {
                doPlayTrack(playQueueIter_); //..restart this track
            }
            else // else back one step
            {
                if ( playQueueIter_ == playQueue_.begin() ) //this can only happen if repeat is on
                    playQueueIter_ = playQueue_.end();
                playQueueIter_--;
                doPlayTrack(playQueueIter_);
            }
        }
    }
    else
    {
        assert(!enquedQueue_.empty());
        if ( playQueue_.empty() || ((float)currentTrackProgress / (enquedQueue_.front()).getDurationMillisecs()) > 0.02 ) //..if we've played this track for a while
        {
            doPlayTrack(enquedQueue_.begin()); //..restart this track
        }
        else // else scrap this queued track and find previous one on play queue
        {
            enquedQueue_.pop_front();
            if ( playQueueIter_ == playQueue_.begin() && isRepeat )
                playQueueIter_ = playQueue_.end();

            if ( playQueueIter_ != playQueue_.begin() )
                playQueueIter_--;
            doPlayTrack(playQueueIter_);

            isPlayingQueuedTrack = false;
        }
    }
    mtx_.unlock();
}

void LibSpotifyPlaybackHandler::shuffle()
{
    std::shuffle( playQueue_.begin(), playQueue_.end(), shuffleGenerator );
}

void LibSpotifyPlaybackHandler::playNext()
{
    mtx_.lock();

    /* 1. Move to history queue and step applicable queue to next item */
    if ( !isPlayingQueuedTrack )
    {
        if ( playQueueIter_ != playQueue_.end() ) // make sure we have a current track
        {
            historyQueue_.push_back(*playQueueIter_);

            playQueueIter_++;
        }
    }
    else
    {
        assert(!enquedQueue_.empty());
        historyQueue_.push_back(enquedQueue_.front());
        enquedQueue_.pop_front();
    }

    /* 3. Play next item */
    if ( enquedQueue_.empty() == false )
    {
        doPlayTrack( enquedQueue_.begin() );
        isPlayingQueuedTrack = true;
    }
    else
    {
        if ( !playQueue_.empty() )
        {
            if ( playQueueIter_ != playQueue_.end() )
            {
                doPlayTrack( playQueueIter_ );
            }
            else
            {
                /* wrap around */
                playQueueIter_ = playQueue_.begin();
                if ( isRepeat )
                    doPlayTrack( playQueueIter_ );
            }
        }
        isPlayingQueuedTrack = false;
    }

    mtx_.unlock();
}

void LibSpotifyPlaybackHandler::rootFolderUpdatedInd()
{
    /* TODO: Should update the playQueue/Folder lists.. */
}

static bool playQueueCompareTrack (Track a, Track b) { return ( a.getIndex() < b.getIndex() ); }

void LibSpotifyPlaybackHandler::setShuffle(bool shuffleOn)
{
    if ( isShuffle == shuffleOn )
        return; /*client out of sync, just nop()*/

    mtx_.lock();

    if ( shuffleOn )
    {
        if ( playQueueIter_ != playQueue_.end() )
        {
            Track currentTrack = *playQueueIter_;
            playQueue_.erase( playQueueIter_ );
            shuffle();
            playQueue_.push_front( currentTrack );
            playQueueIter_ = playQueue_.begin();
        }
        else
            shuffle();
    }
    else
    {
        /*get the index of the current track (if we have one)*/
        int currentIndex = -1;
        if ( playQueueIter_ != playQueue_.end() ) /* don't care if playing queued track, we'll just keep playQueueIter_ at the same track as it was */
            currentIndex = (*playQueueIter_).getIndex();

        /*put playqueue back in order*/
        sort( playQueue_.begin(), playQueue_.end(), playQueueCompareTrack );

        /*find the current track again (if we had one)*/
        playQueueIter_ = playQueue_.begin();
        if ( currentIndex >= 0 )
        {
            while ( ( playQueueIter_ != playQueue_.end() ) && ( (*playQueueIter_).getIndex() != currentIndex ) )
                playQueueIter_++;
        }
    }

    isShuffle = shuffleOn;
    mtx_.unlock();
}

void LibSpotifyPlaybackHandler::setRepeat(bool repeatOn)
{
    isRepeat = repeatOn;
}

bool LibSpotifyPlaybackHandler::getShuffle() { return isShuffle; }
bool LibSpotifyPlaybackHandler::getRepeat()  { return isRepeat; }

} /* namespace LibSpotify */
