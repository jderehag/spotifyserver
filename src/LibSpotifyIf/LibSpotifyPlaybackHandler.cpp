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

namespace LibSpotify
{

LibSpotifyPlaybackHandler::LibSpotifyPlaybackHandler(LibSpotifyIf& libspotify) : libSpotifyIf_(libspotify),
                                                                                 historyQueue_(HISTORY_QUEUE_DEPTH),
                                                                                 isShuffle(false),
                                                                                 isRepeat(false)
{
    playQueueIter_ = playQueue_.end();
}

LibSpotifyPlaybackHandler::~LibSpotifyPlaybackHandler() { }


void LibSpotifyPlaybackHandler::playTrack(const Track& track)
{
    mtx_.lock();
    playQueue_.clear();
    currentlyPlayingFromName_ = track.getName();
    currentlyPlayingFromUri_ = track.getLink();
    currentlyPlayingFromType_ = TRACK;
    playQueue_.push_back(track);
    playQueueIter_ = playQueue_.begin();
    if( playQueueIter_ != playQueue_.end() )
        libSpotifyIf_.playTrack(*playQueueIter_);
    else
        libSpotifyIf_.stop();
    mtx_.unlock();
}

void LibSpotifyPlaybackHandler::playFolder(const Folder& folder)
{
    mtx_.lock();
    playQueue_.clear();
    currentlyPlayingFromName_ = folder.getName();
    currentlyPlayingFromUri_ = ""; //Unknown so far as how to get the folder link..
    currentlyPlayingFromType_ = FOLDER;
    folder.getAllTracks(playQueue_);
    playQueueIter_ = playQueue_.begin();
    if( playQueueIter_ != playQueue_.end() )
        libSpotifyIf_.playTrack(*playQueueIter_);
    else
        libSpotifyIf_.stop();
    mtx_.unlock();
}

void LibSpotifyPlaybackHandler::playPlaylist( const Playlist& playlist, int startIndex )
{
    mtx_.lock();
    currentlyPlayingFromType_ = PLAYLIST;
    loadPlaylist( playlist, startIndex );
    if( playQueueIter_ != playQueue_.end() )
        libSpotifyIf_.playTrack(*playQueueIter_);
    else
        libSpotifyIf_.stop();
    mtx_.unlock();

}

void LibSpotifyPlaybackHandler::playAlbum( const Album& album, int startIndex )
{
    mtx_.lock();
    currentlyPlayingFromType_ = ALBUM;
    loadPlaylist( album, startIndex );
    if( playQueueIter_ != playQueue_.end() )
        libSpotifyIf_.playTrack(*playQueueIter_);
    else
        libSpotifyIf_.stop();
    mtx_.unlock();
}

void LibSpotifyPlaybackHandler::loadPlaylist( const Playlist& playlist, int startIndex )
{
    playQueue_.clear();
    currentlyPlayingFromName_ = playlist.getName();
    currentlyPlayingFromUri_ = playlist.getLink();
    playQueue_.insert(playQueue_.end(), playlist.getTracks().begin(), playlist.getTracks().end());
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
    libSpotifyIf_.playTrack(playQueue_.front());
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
    if(enquedQueue_.empty())
    {
        if ( ( playQueueIter_ == playQueue_.begin() && !isRepeat ) || // if first track without repeat or..
             ( ((float)currentTrackProgress / (*playQueueIter_).getDurationMillisecs()) > 0.02 ) ) //..if we've played this track for a while
        {
            libSpotifyIf_.playTrack(*playQueueIter_); //..restart this track
        }
        else // else back one step
        {
            if ( playQueueIter_ == playQueue_.begin() ) //this can only happen if repeat is on
                playQueueIter_ = playQueue_.end();
            playQueueIter_--;
            libSpotifyIf_.playTrack(*playQueueIter_);
        }
        
    }
    else
    {
        if ( ((float)currentTrackProgress / (*playQueueIter_).getDurationMillisecs()) > 0.02 ) //..if we've played this track for a while
        {
            libSpotifyIf_.playTrack(enquedQueue_.front()); //..restart this track
        }
        else // else scrap this queued track and go to the one we had on play queue
        {
            enquedQueue_.pop_front();
            libSpotifyIf_.playTrack(*playQueueIter_);
        }
    }
    mtx_.unlock();
}

void LibSpotifyPlaybackHandler::shuffle()
{
    random_shuffle( playQueue_.begin(), playQueue_.end() );
}

void LibSpotifyPlaybackHandler::playNext()
{
    mtx_.lock();

    /* First check where we are playing from, and move that track into the history queue,
     * then start playing the next item */

    /* 1. Move to history queue */
    if(enquedQueue_.empty())
    {
        if ( playQueueIter_ != playQueue_.end() ) // this is to catch a glitch, playlist was unloaded
        {
            historyQueue_.push_back(*playQueueIter_);
            playQueueIter_++;

            if( isRepeat && playQueueIter_ == playQueue_.end())
                playQueueIter_ = playQueue_.begin();
        }
    }
    else
    {
        historyQueue_.push_back(enquedQueue_.front());
        enquedQueue_.pop_front();
    }

    /* 2. Play next item */
    if ( ( enquedQueue_.empty() == false ) ||
         ( playQueue_.empty() == false && playQueueIter_ != playQueue_.end() ) )
    {
        if(enquedQueue_.empty())libSpotifyIf_.playTrack(*playQueueIter_);
        else libSpotifyIf_.playTrack(enquedQueue_.front());
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
            Track current = *playQueueIter_;
            playQueue_.erase(playQueueIter_);
            shuffle();
            playQueue_.push_front(current);
            playQueueIter_ = playQueue_.begin();
        }
        else
            shuffle();
    }
    else
    {
        /*get the index of the current track (if we have one)*/
        int currentIndex = -1;
        if ( playQueueIter_ != playQueue_.end() )
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
