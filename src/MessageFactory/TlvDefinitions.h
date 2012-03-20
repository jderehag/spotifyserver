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

#ifndef TLVDEFINITIONS_H_
#define TLVDEFINITIONS_H_

#define PROTOCOL_VERSION_MAJOR 1 /*increase when not backwards compatible*/
#define PROTOCOL_VERSION_MINOR 0 /*increase when adding new messages and/or TLV's*/

#define RSP_BIT 0x80000000
#define IND_BIT 0x40000000

#define REQ(name, value) name##_REQ = value, name##_RSP = (value | RSP_BIT),
#define IND(name, value) name##_IND = (value | IND_BIT),

typedef enum
{
    /* Session */
    REQ( HELLO,            0x101 ) /*tbd*/

    /* Metadata */
    REQ( GET_PLAYLISTS,    0x201 )
    REQ( GET_TRACKS,       0x202 )
    REQ( GET_IMAGE,        0x203 )
    REQ( GENERIC_SEARCH,   0x204 )

    /* Playback */
    REQ( PLAY_TRACK,       0x301 ) /* obsolete, use PLAY_REQ */
    REQ( PLAY,             0x302 )
    REQ( PLAY_CONTROL,     0x303 )

    /* Status */
    REQ( GET_STATUS,       0x401 )
    IND( STATUS,           0x401 )
}MessageType_t;


typedef enum
{
    /*Container TLV's*/
    TLV_FOLDER   = 0x01,
    TLV_PLAYLIST = 0x02,
    TLV_TRACK    = 0x03,

    TLV_IMAGE    = 0x08,
    TLV_ALBUM    = 0x09,
    TLV_ARTIST   = 0x0a,

    /*Folder TLV's*/
    /* TLV_FOLDER_NAME = 0x101, deprecated */

    /*Playlist TLV's*/
    /* TLV_PLAYLIST_NAME = 0x201, deprecated */
    /* TLV_PLAYLIST_LINK = 0x202,  deprecated */

    /*Track TLV's*/
    /* TLV_TRACK_LINK     = 0x301, deprecated */
    /* TLV_TRACK_NAME       = 0x302, deprecated */
    /* TLV_TRACK_ARTIST     = 0x303, deprecated */
    TLV_TRACK_DURATION   = 0x304,
    /* TLV_TRACK_ALBUM      = 0x305, deprecated */
    /* TLV_TRACK_ALBUM_LINK = 0x306, deprecated */

    /* Search TLV's */
    TLV_SEARCH_QUERY = 0x401,

    /* Status TLV's */
    TLV_STATE    = 0x501, /* PlaybackState_t */
    TLV_PROGRESS = 0x502,

    /* Playback control */
    TLV_PLAY_MODE      = 0x601, /* PlayMode_t */
    TLV_VOLUME         = 0x602,
    TLV_PLAY_OPERATION = 0x603, /* PlayOp_t */

    /* Generic data items */
    TLV_LINK      = 0x701,
    TLV_NAME      = 0x702,

    /* Image TLV's */
    TLV_IMAGE_FORMAT = 0x801, /* ImageFormat_t */
    TLV_IMAGE_DATA   = 0x802,

    /* Album TLV's */
    /* reserved 0x900 range */

    /* Artist TLV's */
    /* reserved 0xa00 range */



    /* Session TLV's */
    TLV_LOGIN_USERNAME         = 0x1001,
    TLV_LOGIN_PASSWORD         = 0x1002,
    TLV_PROTOCOL_VERSION_MAJOR = 0x1003,
    TLV_PROTOCOL_VERSION_MINOR = 0x1004,

    /* Error handling TLV's */
    TLV_FAILURE                = 0x1101,
}TlvType_t;


typedef enum
{
    /*generic codes*/
    FAIL_GENERAL_ERROR     = 0x01, /* = Some error I don't want to add a new cause for */

    /*session codes*/
    FAIL_BAD_LOGIN         = 0x11, /* = Wrong username or password */
    FAIL_PROTOCOL_MISMATCH = 0x12, /* = Protocol versions not compatible */

    /*message errors*/
    FAIL_UNKNOWN_REQUEST   = 0x21, /* = Unknown MessageType_t */
    FAIL_MISSING_TLV       = 0x22, /* = Mandatory parameter was missing in request */
}FailureCause_t;

typedef enum
{
    PLAYBACK_IDLE,
    PLAYBACK_PLAYING,
    PLAYBACK_PAUSED
}PlaybackState_t;

typedef enum
{
    PLAY_MODE_SHUFFLE,
    PLAY_MODE_REPEAT,
    PLAY_MODE_WONDERWALL_ONLY,
}PlayMode_t;

typedef enum
{
    PLAY_OP_PAUSE,
    PLAY_OP_RESUME,
    PLAY_OP_NEXT,
    PLAY_OP_PREV,
}PlayOp_t;

typedef enum /*sp_imageformat*/
{
    IMAGE_FORMAT_UNKNOWN, /*what to do with this?*/
    IMAGE_FORMAT_JPEG,
}ImageFormat_t;


const char* failureCauseToString(const FailureCause_t type);
const char* messageTypeToString(const MessageType_t type);
const char* tlvTypeToString(const TlvType_t type);
const char* playbackStateToString(const PlaybackState_t type);
const char* playModeToString(const PlayMode_t type);



#endif /* TLVDEFINITIONS_H_ */
