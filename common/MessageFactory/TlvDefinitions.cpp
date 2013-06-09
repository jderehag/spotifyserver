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
        STR(HELLO_REQ);
        STR(HELLO_RSP);

        STR(GET_PLAYLISTS_REQ);
        STR(GET_PLAYLISTS_RSP);
        STR(GET_ALBUM_REQ);
        STR(GET_ALBUM_RSP);
        STR(GET_ARTIST_REQ);
        STR(GET_ARTIST_RSP);
        STR(GET_TRACKS_REQ);
        STR(GET_TRACKS_RSP);
        STR(GET_IMAGE_REQ);
        STR(GET_IMAGE_RSP);

        STR(PLAY_TRACK_REQ);
        STR(PLAY_TRACK_RSP);
        STR(GENERIC_SEARCH_REQ);
        STR(GENERIC_SEARCH_RSP);
        STR(GET_STATUS_REQ);
        STR(GET_STATUS_RSP);
        STR(STATUS_IND);
        STR(PLAY_REQ);
        STR(PLAY_RSP);
        STR(PLAY_CONTROL_REQ);
        STR(PLAY_CONTROL_RSP);

        STR(ADD_AUDIO_ENDPOINTS_REQ);
        STR(ADD_AUDIO_ENDPOINTS_RSP);
        STR(REM_AUDIO_ENDPOINTS_REQ);
        STR(REM_AUDIO_ENDPOINTS_RSP);
        STR(GET_CURRENT_AUDIO_ENDPOINTS_REQ);
        STR(GET_CURRENT_AUDIO_ENDPOINTS_RSP);

        STR(CREATE_AUDIO_ENDPOINT_REQ);
        STR(CREATE_AUDIO_ENDPOINT_RSP);
        STR(DELETE_AUDIO_ENDPOINT_REQ);
        STR(DELETE_AUDIO_ENDPOINT_RSP);
        STR(GET_AUDIO_ENDPOINTS_REQ);
        STR(GET_AUDIO_ENDPOINTS_RSP);

        STR(AUDIO_DATA_IND);

        default:
            return "Unknown MessageType";
    }

}

const char* tlvTypeToString(const TlvType_t type)
{
    switch(type)
    {
        STR(TLV_FOLDER);
        STR(TLV_PLAYLIST);
        STR(TLV_TRACK);
        STR(TLV_ARTIST);
        STR(TLV_ALBUM);
        STR(TLV_CLIENT);

        STR(TLV_TRACK_DURATION);
        STR(TLV_TRACK_INDEX );
        
        STR(TLV_SEARCH_QUERY);
        STR(TLV_STATE);
        STR(TLV_PROGRESS);
        STR(TLV_VOLUME);
        STR(TLV_PLAY_OPERATION);

        STR(TLV_PLAY_MODE_SHUFFLE);
        STR(TLV_PLAY_MODE_REPEAT);

        STR(TLV_LINK);
        STR(TLV_NAME);
        STR(TLV_IP_ADDRESS);
        STR(TLV_PORT);

        STR(TLV_IMAGE);
        STR(TLV_IMAGE_FORMAT);
        STR(TLV_IMAGE_DATA);
        
        STR(TLV_ALBUM_RELEASE_YEAR);
        STR(TLV_ALBUM_REVIEW);
        STR(TLV_ALBUM_IS_AVAILABLE);
        STR(TLV_ALBUM_TYPE);

        STR(TLV_LOGIN_USERNAME);
        STR(TLV_LOGIN_PASSWORD);
        STR(TLV_PROTOCOL_VERSION_MAJOR);
        STR(TLV_PROTOCOL_VERSION_MINOR);
        STR(TLV_FAILURE);

        STR(TLV_AUDIO_DATA);
        STR(TLV_AUDIO_CHANNELS);
        STR(TLV_AUDIO_RATE);
        STR(TLV_AUDIO_NOF_SAMPLES);

        STR(TLV_AUDIO_EP_PROTOCOL);

        default:
            return "Unknown TlvType";
    }

}

const char* failureCauseToString(const FailureCause_t type)
{
    switch(type)
    {
        STR(FAIL_GENERAL_ERROR);
        STR(FAIL_BAD_LOGIN);
        STR(FAIL_PROTOCOL_MISMATCH);
        STR(FAIL_UNKNOWN_REQUEST);
        STR(FAIL_MISSING_TLV);
        default:
            return "Unknown FailureCause";
    }

}
const char* playbackStateToString(const PlaybackState_t type)
{
    switch(type)
    {
        STR(PLAYBACK_IDLE);
        STR(PLAYBACK_PLAYING);
        STR(PLAYBACK_PAUSED);
        default:
            return "Unknown PlaybackState";
    }
}





