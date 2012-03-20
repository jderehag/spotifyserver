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

#include "TlvDefinitions.h"

#define STR(n) case n: return #n

const char* messageTypeToString(const MessageType_t type)
{
    switch (type)
    {
        case GET_PLAYLISTS_REQ:
            return "GET_PLAYLISTS_REQ";
        case GET_PLAYLISTS_RSP:
            return "GET_PLAYLISTS_RSP";
        case GET_TRACKS_REQ:
            return "GET_TRACKS_REQ";
        case GET_TRACKS_RSP:
            return "GET_TRACKS_RSP";
        case PLAY_TRACK_REQ:
            return "PLAY_TRACK_REQ";
        case PLAY_TRACK_RSP:
            return "PLAY_TRACK_RSP";
        case GENERIC_SEARCH_REQ:
            return "GENERIC_SEARCH_REQ";
        case GENERIC_SEARCH_RSP:
            return "GENERIC_SEARCH_RSP";
        case GET_STATUS_REQ:
            return "GET_STATUS_REQ";
        case GET_STATUS_RSP:
            return "GET_STATUS_RSP";
        case STATUS_IND:
            return "STATUS_IND";
        case PLAY_REQ:
            return "PLAY_REQ";
        case PLAY_RSP:
            return "PLAY_RSP";
        case PLAY_CONTROL_REQ:
            return "PLAY_CONTROL_REQ";
        case PLAY_CONTROL_RSP:
            return "PLAY_CONTROL_RSP";
        case GET_IMAGE_REQ:
            return "GET_IMAGE_REQ";
        case GET_IMAGE_RSP:
            return "GET_IMAGE_RSP";

        STR(HELLO_REQ);
        STR(HELLO_RSP);
    }
    return "Unknown MessageType";
}

const char* tlvTypeToString(const TlvType_t type)
{
    switch(type)
    {
        case TLV_FOLDER:
            return "TLV_FOLDER";
        case TLV_PLAYLIST:
            return "TLV_PLAYLIST";
        case TLV_TRACK:
            return "TLV_TRACK";
        case TLV_ARTIST:
            return "TLV_ARTIST";
        case TLV_ALBUM:
            return "TLV_ALBUM";
        case TLV_TRACK_DURATION:
            return "TLV_TRACK_DURATION";
        case TLV_SEARCH_QUERY:
            return "TLV_SEARCH_QUERY";
        case TLV_STATE:
            return "TLV_STATE";
        case TLV_PROGRESS:
            return "TLV_PROGRESS";
        case TLV_PLAY_MODE:
            return "TLV_PLAY_MODE";
        case TLV_VOLUME:
            return "TLV_VOLUME";
        case TLV_PLAY_OPERATION:
            return "TLV_PLAY_OPERATION";
        case TLV_LINK:
            return "TLV_LINK";
        case TLV_NAME:
            return "TLV_NAME";
        case TLV_IMAGE:
            return "TLV_IMAGE";
        case TLV_IMAGE_FORMAT:
            return "TLV_IMAGE_FORMAT";
        case TLV_IMAGE_DATA:
            return "TLV_IMAGE_DATA";

        STR( TLV_LOGIN_USERNAME );
        STR( TLV_LOGIN_PASSWORD );
        STR( TLV_PROTOCOL_VERSION_MAJOR );
        STR( TLV_PROTOCOL_VERSION_MINOR );
        STR( TLV_FAILURE );
    }
    return "Unknown TlvType";
}

const char* failureCauseToString(const FailureCause_t type)
{
    switch(type)
    {
        STR( FAIL_GENERAL_ERROR );
        STR( FAIL_BAD_LOGIN );
        STR( FAIL_PROTOCOL_MISMATCH );
        STR( FAIL_UNKNOWN_REQUEST );
        STR( FAIL_MISSING_TLV );
    }
    return "Unknown FailureCause";
}
const char* playbackStateToString(const PlaybackState_t type)
{
    switch(type)
    {
        case PLAYBACK_IDLE:
            return "PLAYBACK_IDLE";
        case PLAYBACK_PLAYING:
            return "PLAYBACK_PLAYING";
        case PLAYBACK_PAUSED:
            return "PLAYBACK_PAUSED";
    }
    return "Unknown PlaybackState";
}

const char* playModeToString(const PlayMode_t type)
{
    switch(type)
    {
        case PLAY_MODE_SHUFFLE:
            return "PLAY_MODE_SHUFFLE";
        case PLAY_MODE_REPEAT:
            return "PLAY_MODE_REPEAT";
        case PLAY_MODE_WONDERWALL_ONLY:
            return "PLAY_MODE_WONDERWALL_ONLY";
    }
    return "Unknown PlayMode";
}




