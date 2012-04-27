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

#ifndef LIBSPOTIFYIFHELPERS_H_
#define LIBSPOTIFYIFHELPERS_H_

#include "MediaContainers/Track.h"
#include "MediaContainers/Playlist.h"
#include "MediaContainers/Album.h"
#include <libspotify/api.h>
#include <string>

namespace LibSpotify
{
    Track spotifyGetTrack(std::string* track_uri, sp_session* session);
    Track spotifyGetTrack(sp_track* track, sp_session* session);

    Playlist spotifyGetPlaylist(std::string* playlist_uri, sp_session* session);
    Playlist spotifyGetPlaylist(sp_playlist* playlist, sp_session* session);

    //Album spotifyGetAlbum(std::string* album_uri, sp_session* session);
    Album spotifyGetAlbum(sp_albumbrowse* album, sp_session* session);

} /* namespace LibSpotify */
#endif /* LIBSPOTIFYIFHELPERS_H_ */
