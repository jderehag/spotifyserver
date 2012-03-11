'''
Created on 12 feb 2012

@author: jeppe
'''     
        
class TlvMessageType:
    GET_PLAYLISTS_REQ   = int('0x1', 16)
    GET_PLAYLISTS_RSP   = int('0x2', 16)
    GET_TRACKS_REQ      = int('0x3', 16)
    GET_TRACKS_RSP      = int('0x4', 16)
    PLAY_TRACK_REQ      = int('0x5', 16)
    PLAY_TRACK_RSP      = int('0x6', 16)
    GENERIC_SEARCH_REQ  = int('0x7', 16)
    GENERIC_SEARCH_RSP  = int('0x8', 16)
    STATUS_IND          = int('0x9', 16)
    PLAY_REQ            = int('0xa', 16)
    PLAY_RSP            = int('0xb', 16)
    PLAY_CONTROL_REQ    = int('0xc', 16)
    PLAY_CONTROL_RSP    = int('0xd', 16)
    GET_STATUS_REQ      = int('0xe', 16)
    GET_STATUS_RSP      = int('0xf', 16)
    GET_IMAGE_REQ       = int('0x10', 16)
    GET_IMAGE_RSP       = int('0x11', 16)

class TlvContainerType:
        
    # Container TLV's
    TLV_FOLDER          = int('0x1', 16)
    TLV_PLAYLIST        = int('0x2', 16)
    TLV_TRACK           = int('0x3', 16)
    
    TLV_IMAGE           = int('0x8',16)
    TLV_ALBUM           = int('0x9',16)
    TLV_ARTIST          = int('0xa',16)
    
    # Track TLV's
    TLV_TRACK_DURATION  = int('0x304', 16)
    
    # Search TLV's */
    TLV_SEARCH_QUERY    = int('0x401', 16)
    
    # Status TLV's */
    TLV_STATE           = int('0x501', 16)
    TLV_PROGRESS        = int('0x502', 16)
    
    # Playback control */
    TLV_PLAY_MODE       = int('0x601', 16)
    TLV_VOLUME          = int('0x602', 16)
    TLV_PLAY_OPERATION  = int('0x603', 16)
    
    # Generic data items */
    TLV_LINK            = int('0x701', 16)
    TLV_NAME            = int('0x702', 16)
    
    # Image TLV's */
    TLV_IMAGE_FORMAT    = int('0x701', 16)
    TLV_IMAGE_DATA      = int('0x702', 16)



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


        

