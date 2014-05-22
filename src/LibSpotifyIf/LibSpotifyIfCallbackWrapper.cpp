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

/*
 * All callbacks needs unfortunatly to be static (since libspotify uses pure c callbacks)
 * could be solved by using boost::Thread, or keeping a list of all LibSpotify
 * instances on the heap, and then wrapping the callbacks to find the correct instance
 * For now, only support 1 instance callbacks..
 */

#include "LibSpotifyIfCallbackWrapper.h"
#include "LibSpotifyIf.h"
#include <libspotify/api.h>
#include <iostream>
#include "applog.h"


namespace LibSpotify
{

static LibSpotifyIf* libSpotifyIfSingleton;
static sp_session_callbacks libSpotifySessioncallbacks =
{
	&LibSpotifyIfCallbackWrapper::loggedInCb, 					//.logged_in
	&LibSpotifyIfCallbackWrapper::loggedOutCb,					//.logged_out
	&LibSpotifyIfCallbackWrapper::metadataUpdatedCb,			//.metadata_updated
	&LibSpotifyIfCallbackWrapper::connectionErrorCb,			//.connection_error
	NULL,														//.message_to_user
	&LibSpotifyIfCallbackWrapper::notifyLibSpotifyMainThreadCb, //.notify_main_thread
	&LibSpotifyIfCallbackWrapper::musicDeliveryCb,				//.music_delivery
	&LibSpotifyIfCallbackWrapper::playTokenLostCb,              //.play_token_lost
	&LibSpotifyIfCallbackWrapper::logMessageCb,					//.log_message
	&LibSpotifyIfCallbackWrapper::endOfTrackCb,					//.end_of_track
	NULL,														//.streaming_error
	NULL,														//.userinfo_updated
	NULL,														//.start_playback
	NULL,														//.stop_playback
	NULL,														//.get_audio_buffer_stats
	NULL,														//.offline_status_updated
    NULL,                                                       //.offline_error
    &LibSpotifyIfCallbackWrapper::credentialsBlobUpdated,       //.credentials_blob_updated
    NULL,                                                       //.connectionstate_updated
    NULL,                                                       //.scrobble_error
    NULL,                                                       //.private_session_mode_changed
};

/**
 * The playlist container callbacks
 */
static sp_playlistcontainer_callbacks libSpotifyContainerCallbacks = {
    &LibSpotifyIfCallbackWrapper::playlistAdded,                //.playlist_added
    &LibSpotifyIfCallbackWrapper::playlistRemoved,              //.playlist_removed
    &LibSpotifyIfCallbackWrapper::playlistMoved,                //.playlist_moved
    &LibSpotifyIfCallbackWrapper::playlistContainerLoaded,      //.container_loaded
};

/**
 * The playlist callbacks
 */
static sp_playlist_callbacks libSpotifyPlaylistCallbacks = {
    &LibSpotifyIfCallbackWrapper::tracksAdded,                   //.tracks_added
    &LibSpotifyIfCallbackWrapper::tracksRemoved,                 //.tracks_removed
    &LibSpotifyIfCallbackWrapper::tracksMoved,                   //.tracks_moved
    &LibSpotifyIfCallbackWrapper::playlistRenamed,               //.playlist_renamed
    &LibSpotifyIfCallbackWrapper::playlistStateChanged,          //.playlist_state_changed
    &LibSpotifyIfCallbackWrapper::playlistUpdateInProgress,      //.playlist_update_in_progress
    &LibSpotifyIfCallbackWrapper::playlistMetadataUpdated,       //.playlist_metadata_updated
    &LibSpotifyIfCallbackWrapper::trackCreatedChanged,           //.track_created_changed
    &LibSpotifyIfCallbackWrapper::trackSeenChanged,              //.track_seen_changed
    &LibSpotifyIfCallbackWrapper::descriptionChanged,            //.description_changed 
    &LibSpotifyIfCallbackWrapper::imageChanged,                  //.image_changed
    &LibSpotifyIfCallbackWrapper::trackMessageChanged,           //.track_message_changed
    &LibSpotifyIfCallbackWrapper::subscribersChanged,            //.subscribers_changed
};

LibSpotifyIfCallbackWrapper::LibSpotifyIfCallbackWrapper(LibSpotifyIf& libSpotifyIf)
{
	libSpotifyIfSingleton = &libSpotifyIf;
}

LibSpotifyIfCallbackWrapper::~LibSpotifyIfCallbackWrapper()
{
}

sp_session_callbacks* LibSpotifyIfCallbackWrapper::getRegisteredSessionCallbacks()
{
	return &libSpotifySessioncallbacks;
}

sp_playlistcontainer_callbacks* LibSpotifyIfCallbackWrapper::getRegisteredPlaylistContainerCallbacks()
{
    return &libSpotifyContainerCallbacks;
}

sp_playlist_callbacks* LibSpotifyIfCallbackWrapper::getRegisteredPlaylistCallbacks()
{
    return &libSpotifyPlaylistCallbacks;
}

/* *************************
 * Session Callbacks
 * **************************/
void SP_CALLCONV LibSpotifyIfCallbackWrapper::loggedInCb(sp_session *session, sp_error error)
{
	libSpotifyIfSingleton->loggedInCb(session, error);
}
void SP_CALLCONV LibSpotifyIfCallbackWrapper::metadataUpdatedCb(sp_session *session)
{
	libSpotifyIfSingleton->metadataUpdatedCb(session);
}
int SP_CALLCONV LibSpotifyIfCallbackWrapper::musicDeliveryCb(sp_session *session, const sp_audioformat *format,
	                          const void *frames, int num_frames)
{
	return libSpotifyIfSingleton->musicDeliveryCb(session, format, frames, num_frames);
}
void SP_CALLCONV LibSpotifyIfCallbackWrapper::playTokenLostCb(sp_session *session)
{
    // todo: should also display something to user
    libSpotifyIfSingleton->pause();
}
void SP_CALLCONV LibSpotifyIfCallbackWrapper::endOfTrackCb(sp_session *session)
{
	libSpotifyIfSingleton->endOfTrackCb(session);
}
void SP_CALLCONV LibSpotifyIfCallbackWrapper::loggedOutCb(sp_session *session)
{
	libSpotifyIfSingleton->loggedOutCb(session);
}
void SP_CALLCONV LibSpotifyIfCallbackWrapper::connectionErrorCb(sp_session *session, sp_error error)
{
	libSpotifyIfSingleton->connectionErrorCb(session, error);
}
void SP_CALLCONV LibSpotifyIfCallbackWrapper::notifyLibSpotifyMainThreadCb(sp_session *session)
{
	libSpotifyIfSingleton->notifyLibSpotifyMainThreadCb(session);
}
void SP_CALLCONV LibSpotifyIfCallbackWrapper::logMessageCb(sp_session *session, const char *data)
{
	libSpotifyIfSingleton->logMessageCb(session, data);
}

void SP_CALLCONV LibSpotifyIfCallbackWrapper::credentialsBlobUpdated(sp_session *session, const char *blob)
{
    libSpotifyIfSingleton->credentialsBlobUpdatedCb( session, blob );
}

/* **************************
 * Playlist Container Callbacks
 * **************************/
void SP_CALLCONV LibSpotifyIfCallbackWrapper::playlistAdded(sp_playlistcontainer *pc, sp_playlist *playlist, int position, void *userdata)
{
    libSpotifyIfSingleton->playlistAdded( playlist );
}
void SP_CALLCONV LibSpotifyIfCallbackWrapper::playlistRemoved(sp_playlistcontainer *pc, sp_playlist *playlist, int position, void *userdata)
{
    libSpotifyIfSingleton->playlistRemoved( playlist );
}
void SP_CALLCONV LibSpotifyIfCallbackWrapper::playlistMoved(sp_playlistcontainer *pc, sp_playlist *playlist, int position, int new_position, void *userdata)
{
    libSpotifyIfSingleton->playlistsMoved();
}
void SP_CALLCONV LibSpotifyIfCallbackWrapper::playlistContainerLoaded(sp_playlistcontainer *pc, void *userdata)
{
    libSpotifyIfSingleton->rootFolderLoaded();
}


void SP_CALLCONV LibSpotifyIfCallbackWrapper::tracksAdded(sp_playlist *pl, sp_track *const *tracks, int num_tracks, int position, void *userdata)
{
    libSpotifyIfSingleton->playlistContentsUpdated( pl );
}
void SP_CALLCONV LibSpotifyIfCallbackWrapper::tracksRemoved(sp_playlist *pl, const int *tracks, int num_tracks, void *userdata)
{
    libSpotifyIfSingleton->playlistContentsUpdated( pl );
}
void SP_CALLCONV LibSpotifyIfCallbackWrapper::tracksMoved(sp_playlist *pl, const int *tracks, int num_tracks, int new_position, void *userdata)
{
    libSpotifyIfSingleton->playlistContentsUpdated( pl );
}
void SP_CALLCONV LibSpotifyIfCallbackWrapper::playlistRenamed(sp_playlist *pl, void *userdata)
{
    libSpotifyIfSingleton->playlistRenamed( pl );
}
void SP_CALLCONV LibSpotifyIfCallbackWrapper::playlistStateChanged(sp_playlist *pl, void *userdata)
{
}
void SP_CALLCONV LibSpotifyIfCallbackWrapper::playlistUpdateInProgress(sp_playlist *pl, bool done, void *userdata)
{
}
void SP_CALLCONV LibSpotifyIfCallbackWrapper::playlistMetadataUpdated(sp_playlist *pl, void *userdata)
{
}
void SP_CALLCONV LibSpotifyIfCallbackWrapper::trackCreatedChanged(sp_playlist *pl, int position, sp_user *user, int when, void *userdata)
{
}
void SP_CALLCONV LibSpotifyIfCallbackWrapper::trackSeenChanged(sp_playlist *pl, int position, bool seen, void *userdata)
{
}
void SP_CALLCONV LibSpotifyIfCallbackWrapper::descriptionChanged(sp_playlist *pl, const char *desc, void *userdata)
{
}
void SP_CALLCONV LibSpotifyIfCallbackWrapper::imageChanged(sp_playlist *pl, const byte *image, void *userdata)
{
}
void SP_CALLCONV LibSpotifyIfCallbackWrapper::trackMessageChanged(sp_playlist *pl, int position, const char *message, void *userdata)
{
}
void SP_CALLCONV LibSpotifyIfCallbackWrapper::subscribersChanged(sp_playlist *pl, void *userdata)
{
}
/* **************************
 * Search Callbacks
 * **************************/
void SP_CALLCONV LibSpotifyIfCallbackWrapper::genericSearchCallback(sp_search *search, void *userdata)
{
	libSpotifyIfSingleton->genericSearchCb(search, userdata);
}

void SP_CALLCONV LibSpotifyIfCallbackWrapper::imageLoadedCallback(sp_image *image, void *userdata)
{
    libSpotifyIfSingleton->imageLoadedCb(image, userdata);
}

void SP_CALLCONV LibSpotifyIfCallbackWrapper::albumLoadedCallback(sp_albumbrowse *result, void *userdata)
{
    libSpotifyIfSingleton->albumLoadedCb(result, userdata);
}

void SP_CALLCONV LibSpotifyIfCallbackWrapper::artistLoadedCallback(sp_artistbrowse *result, void *userdata)
{
    libSpotifyIfSingleton->artistLoadedCb(result, userdata);
}
}
