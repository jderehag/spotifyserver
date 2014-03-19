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

#ifndef LIBSPOTIFYIFCALLBACKWRAPPER_H_
#define LIBSPOTIFYIFCALLBACKWRAPPER_H_

#include <libspotify/api.h>

namespace LibSpotify
{
class LibSpotifyIfCallbackWrapper
{
private:


public:
	LibSpotifyIfCallbackWrapper(class LibSpotifyIf& libSpotifyIf);
	~LibSpotifyIfCallbackWrapper();

	sp_session_callbacks* getRegisteredSessionCallbacks();
	sp_playlistcontainer_callbacks* getRegisteredPlaylistContainerCallbacks();

	/* Session Callbacks */
	static void SP_CALLCONV loggedInCb(sp_session *session, sp_error error);
	static void SP_CALLCONV loggedOutCb(sp_session *session);
	static void SP_CALLCONV connectionErrorCb(sp_session *session, sp_error error);
	static void SP_CALLCONV notifyLibSpotifyMainThreadCb(sp_session *session);
	static void SP_CALLCONV logMessageCb(sp_session *session, const char *data);
	static void SP_CALLCONV metadataUpdatedCb(sp_session *sess);
	static int SP_CALLCONV musicDeliveryCb(sp_session *session, const sp_audioformat *format,
	                          const void *frames, int num_frames);
    static void SP_CALLCONV playTokenLostCb(sp_session *session);
	static void SP_CALLCONV endOfTrackCb(sp_session *session);

	/* Playlist callbacks */
	void SP_CALLCONV playlistAdded(sp_playlistcontainer *pc, sp_playlist *playlist, int position, void *userdata);
	void SP_CALLCONV playlistRemoved(sp_playlistcontainer *pc, sp_playlist *playlist, int position, void *userdata);
	void SP_CALLCONV playlistMoved(sp_playlistcontainer *pc, sp_playlist *playlist, int position, int new_position, void *userdata);
	void SP_CALLCONV playlistContainerLoaded(sp_playlistcontainer *pc, void *userdata);

	/* search callbacks */
	static void SP_CALLCONV genericSearchCallback(sp_search *search, void *userdata);
	static void SP_CALLCONV imageLoadedCallback(sp_image *search, void *userdata);
	static void SP_CALLCONV albumLoadedCallback(sp_albumbrowse *result, void *userdata);

};
}

#endif /* LIBSPOTIFYIFCALLBACKWRAPPER_H_ */
