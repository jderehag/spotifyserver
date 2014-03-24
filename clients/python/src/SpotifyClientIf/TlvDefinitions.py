'''
Copyright (c) 2012, Jesper Derehag
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
  * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
  * Neither the name of the <organization> nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL JESPER DEREHAG BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
'''

'''     
#define PROTOCOL_VERSION_MAJOR 1 /*increase when not backwards compatible*/
#define PROTOCOL_VERSION_MINOR 0 /*increase when adding new messages and/or TLV's*/
'''
        
class TlvMessageType:
    REQ_BIT             = int('0x00000000', 16)
    RSP_BIT             = int('0x80000000', 16)
    IND_BIT             = int('0x40000000', 16)
    
    # Session
    HELLO_REQ           = int('0x101', 16) | REQ_BIT
    HELLO_RSP           = int('0x101', 16) | RSP_BIT
    
    # Metadata
    GET_PLAYLISTS_REQ   = int('0x201', 16) | REQ_BIT
    GET_PLAYLISTS_RSP   = int('0x201', 16) | RSP_BIT
    GET_TRACKS_REQ      = int('0x202', 16) | REQ_BIT
    GET_TRACKS_RSP      = int('0x202', 16) | RSP_BIT
    GET_IMAGE_REQ       = int('0x203', 16) | REQ_BIT
    GET_IMAGE_RSP       = int('0x203', 16) | RSP_BIT
    GENERIC_SEARCH_REQ  = int('0x204', 16) | REQ_BIT
    GENERIC_SEARCH_RSP  = int('0x204', 16) | RSP_BIT
    
    # Playback
    PLAY_REQ            = int('0x302', 16) | REQ_BIT
    PLAY_RSP            = int('0x302', 16) | RSP_BIT
    PLAY_CONTROL_REQ    = int('0x303', 16) | REQ_BIT
    PLAY_CONTROL_RSP    = int('0x303', 16) | RSP_BIT
    SET_VOLUME_REQ      = int('0x304', 16) | REQ_BIT
    SET_VOLUME_RSP      = int('0x304', 16) | RSP_BIT
    
    # Status
    GET_STATUS_REQ      = int('0x401', 16) | REQ_BIT
    GET_STATUS_RSP      = int('0x401', 16) | RSP_BIT
    STATUS_IND          = int('0x401', 16) | IND_BIT
    
    CREATE_AUDIO_ENDPOINT_REQ  = int('0x1001', 16) | REQ_BIT
    CREATE_AUDIO_ENDPOINT_RSP  = int('0x1001', 16) | RSP_BIT
    DELETE_AUDIO_ENDPOINT_REQ  = int('0x1002', 16) | REQ_BIT
    DELETE_AUDIO_ENDPOINT_RSP  = int('0x1002', 16) | RSP_BIT
    GET_AUDIO_ENDPOINTS_REQ    = int('0x1003', 16) | REQ_BIT
    GET_AUDIO_ENDPOINTS_RSP    = int('0x1003', 16) | RSP_BIT
    AUDIO_ENDPOINTS_UPDATED_IND = int('0x1004', 16) | IND_BIT

    ADD_AUDIO_ENDPOINTS_REQ         = int('0x1021', 16) | REQ_BIT
    ADD_AUDIO_ENDPOINTS_RSP         = int('0x1021', 16) | RSP_BIT
    REM_AUDIO_ENDPOINTS_REQ         = int('0x1022', 16) | REQ_BIT
    REM_AUDIO_ENDPOINTS_RSP         = int('0x1022', 16) | RSP_BIT
    GET_CURRENT_AUDIO_ENDPOINTS_REQ = int('0x1024', 16) | REQ_BIT
    GET_CURRENT_AUDIO_ENDPOINTS_RSP = int('0x1024', 16) | RSP_BIT
    
    AUDIO_DATA_IND          = int('0x1011', 16) | IND_BIT


class TlvType:
        
    # Container TLV's
    TLV_FOLDER                  = int('0x1', 16)
    TLV_PLAYLIST                = int('0x2', 16)
    TLV_TRACK                   = int('0x3', 16)
    
    TLV_IMAGE                   = int('0x8',16)
    TLV_ALBUM                   = int('0x9',16)
    TLV_ARTIST                  = int('0xa',16)
    
    TLV_CLIENT                  = int('0x21',16)

    # Track TLV's
    TLV_TRACK_DURATION          = int('0x304', 16)
    TLV_TRACK_INDEX             = int('0x307', 16)
    
    # Search TLV's */
    TLV_SEARCH_QUERY            = int('0x401', 16)
    
    # Status TLV's */
    TLV_STATE                   = int('0x501', 16)
    TLV_PROGRESS                = int('0x502', 16)
    
    # Playback control */
    TLV_PLAY_MODE               = int('0x601', 16)
    TLV_VOLUME                  = int('0x602', 16)
    TLV_PLAY_OPERATION          = int('0x603', 16)
    
    # Generic data items */
    TLV_LINK                    = int('0x701', 16)
    TLV_NAME                    = int('0x702', 16)
    TLV_IP_ADDRESS              = int('0x703', 16)
    TLV_PORT                    = int('0x704', 16)
    
    # Image TLV's */
    TLV_IMAGE_FORMAT            = int('0x801', 16)
    TLV_IMAGE_DATA              = int('0x802', 16)

    # Session TLV's
    TLV_LOGIN_USERNAME         = int('0x1001', 16)
    TLV_LOGIN_PASSWORD         = int('0x1002', 16)
    TLV_PROTOCOL_VERSION_MAJOR = int('0x1003', 16)
    TLV_PROTOCOL_VERSION_MINOR = int('0x1004', 16)

    # Error handling TLV's
    TLV_FAILURE                = int('0x1101', 16)
    
    #Audio data TLV's
    TLV_AUDIO_DATA             = int('0x2001', 16)
    TLV_AUDIO_CHANNELS         = int('0x2002', 16)
    TLV_AUDIO_RATE             = int('0x2003', 16)
    TLV_AUDIO_NOF_SAMPLES      = int('0x2004', 16)
    #TLV_AUDIO_DESTINATION_PORT = int('0x2005', 16)
    #TLV_AUDIO_PROTOCOL_TYPE    = int('0x2006', 16)
    
    #Audio endpoint TLV's
    TLV_AUDIO_EP_PROTOCOL      = int('0x2102', 16)


class TlvFailureCause:
    # generic codes
    FAIL_GENERAL_ERROR     = int('0x01', 16) # /* = Some error I don't want to add a new cause for
    
    # session codes
    FAIL_BAD_LOGIN         = int('0x11', 16) # = Wrong username or password
    FAIL_PROTOCOL_MISMATCH = int('0x12', 16) # = Protocol versions not compatible

    # message errors
    FAIL_UNKNOWN_REQUEST   = int('0x21', 16) # = Unknown MessageType_t
    FAIL_MISSING_TLV       = int('0x22', 16) # = Mandatory parameter was missing in request

class TlvPlaybackState:
    PLAYBACK_IDLE           = int('0x0', 16)
    PLAYBACK_PLAYING        = int('0x1', 16)
    PLAYBACK_PAUSED         = int('0x2', 16)

class TlvPlayModeType:
    PLAY_MODE_SHUFFLE       = int('0x0', 16)
    PLAY_MODE_REPEAT        = int('0x1', 16)
    PLAY_MODE_WONDERWALL_ONLY = int('0x2', 16)
        
class TlvPlayOperation:
    PLAY_OP_PAUSE           = int('0x0', 16)
    PLAY_OP_RESUME          = int('0x1', 16)
    PLAY_OP_NEXT            = int('0x2', 16)
    PLAY_OP_PREV            = int('0x3', 16)
        
class TlvImageFormat:
    IMAGE_FORMAT_UNKNOWN        = int('0x0', 16)
    IMAGE_FORMAT_JPEG           = int('0x1', 16)
    
class TlvAudioEndpointProtocolType:
    LIGHTWEIGHT_UDP         = int('0x0', 16)
    RTP                     = int('0x1', 16) # Not supported yet!



        

