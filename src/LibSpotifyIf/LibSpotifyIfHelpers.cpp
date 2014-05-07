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

#include "LibSpotifyIfHelpers.h"
#include "LibSpotifyIf.h"
#include "applog.h"

#include <assert.h>
#include <string.h>



/// The application key is specific to each project, and allows Spotify
/// to produce statistics on how their service is used.
extern "C"
{
extern const uint8_t g_appkey[];
extern const size_t g_appkey_size;
}
namespace LibSpotify
{

Track spotifyGetTrack(std::string& track_uri, sp_session* session)
{
    sp_link* link = sp_link_create_from_string(track_uri.c_str());
    if(link)
    {
        assert(sp_link_type(link) == SP_LINKTYPE_TRACK);

        sp_track* track = sp_link_as_track(link);
        sp_link_release(link);
        /* TODO: Could return errors! (both link & track) */
        return spotifyGetTrack(track, session);
    }
    log(LOG_WARN) << "Could not create track for " << track_uri;
    return Track("", "");
}

Track spotifyGetTrack(sp_track* track, sp_session* session)
{
    char uri[MAX_LINK_NAME_LENGTH];
    sp_link* link = sp_link_create_from_track(track,0);
    if(link)
    {
        sp_link_as_string(link, uri, sizeof(uri));
        sp_link_release(link);
    }

    Track trackObj(sp_track_name(track), uri);

    for(int artistIndex = 0; artistIndex < sp_track_num_artists(track); artistIndex++)
    {
        sp_artist* artist = sp_track_artist(track, artistIndex);
        sp_link* artistlink = sp_link_create_from_artist(artist);
        uri[0] = '\0';
        if(artistlink)
        {
            sp_link_as_string(artistlink, uri, sizeof(uri));
            sp_link_release(artistlink);
        }
        Artist artistObj(sp_artist_name(artist), uri);

        trackObj.addArtist(artistObj);
    }

    //TODO: JESPER, setAlbum crashes sometimes due to that the album object is not loaded yet.
    sp_album* album = sp_track_album(track);
    if(album)
    {
        sp_link* albumlink = sp_link_create_from_album(album);
        uri[0] = '\0';
        if(albumlink)
        {
            sp_link_as_string(albumlink, uri, sizeof(uri));
            sp_link_release(albumlink);
        }
        trackObj.setAlbum(sp_album_name(album), uri);
    }
    trackObj.setDurationMillisecs(sp_track_duration(track));
    trackObj.setIsStarred(sp_track_is_starred(session, track));
    trackObj.setIsLocal(sp_track_is_local(session, track));
    trackObj.setIsAutoLinked(sp_track_is_autolinked(session, track));
    trackObj.setIsAvailable(sp_track_get_availability(session, track) == SP_TRACK_AVAILABILITY_AVAILABLE);
    return trackObj;
}

Playlist spotifyGetPlaylist(std::string& playlist_uri, sp_session* session)
{
    sp_link* link = sp_link_create_from_string(playlist_uri.c_str());
    if(link)
    {
        assert(sp_link_type(link) == SP_LINKTYPE_PLAYLIST);

        sp_playlist* pl = sp_playlist_create(session, link);
        /* TODO: Could return errors! (both link & playlist) */
        Playlist playlistObj(spotifyGetPlaylist(pl, session));
        sp_playlist_release(pl);
        sp_link_release(link);
        return playlistObj;
    }
    log(LOG_WARN) << "Could not create playlist for " << playlist_uri;
    return Playlist("","");

}

Playlist spotifyGetPlaylist(sp_playlist* playlist, sp_session* session)
{
    char uri[MAX_LINK_NAME_LENGTH];
    uri[0] = '\0';
    sp_link* link = sp_link_create_from_playlist(playlist);
    if(link)
    {
        sp_link_as_string(link, uri, sizeof(uri));
        sp_link_release(link);
    }

    Playlist playlistObj(sp_playlist_name(playlist), uri);
    playlistObj.setIsCollaborative(sp_playlist_is_collaborative(playlist));

    for (int trackIndex = 0; trackIndex < sp_playlist_num_tracks(playlist); ++trackIndex)
    {
        Track trackObj(spotifyGetTrack(sp_playlist_track(playlist, trackIndex), session));
        trackObj.setIndex(trackIndex);
        playlistObj.addTrack(trackObj);
    }
    //log(LOG_NOTICE) << "Loaded playlist " << sp_playlist_name(playlist) << " - " << uri << " with " << sp_playlist_num_tracks(playlist) << " tracks";
    return playlistObj;
}

Album spotifyGetAlbum(sp_album* album, sp_session* session)
{
    char uri[MAX_LINK_NAME_LENGTH];
    sp_link* link = sp_link_create_from_album(album);

    sp_link_as_string(link, uri, sizeof(uri));
    sp_link_release(link);

    Album albumObj( sp_album_name(album), uri );

    albumObj.setYear( sp_album_year( album ) );
    albumObj.setIsAvailable( sp_album_is_available( album ) );

    sp_artist* artist = sp_album_artist( album );
    sp_link* artistlink = sp_link_create_from_artist( artist );
    sp_link_as_string( artistlink, uri, sizeof( uri ) );
    sp_link_release( artistlink );
    Artist artistObj( sp_artist_name( artist ), uri );
    albumObj.setArtist(artistObj);

    return albumObj;
}

void spotifyAddAlbumBrowseInfo( Album& album, sp_albumbrowse* albumbrowse, sp_session* session )
{
    album.setReview( sp_albumbrowse_review( albumbrowse ) );

    for (int trackIndex = 0; trackIndex < sp_albumbrowse_num_tracks(albumbrowse); ++trackIndex)
    {
        Track trackObj(spotifyGetTrack(sp_albumbrowse_track(albumbrowse, trackIndex), session));
        trackObj.setIndex(trackIndex);
        album.addTrack(trackObj);
    }
}

Album spotifyGetAlbum(sp_albumbrowse* albumbrowse, sp_session* session)
{
    sp_album* album = sp_albumbrowse_album( albumbrowse );

    Album albumObj = spotifyGetAlbum( album, session );
    spotifyAddAlbumBrowseInfo( albumObj, albumbrowse, session );

    return albumObj;
}


Artist spotifyGetArtist(sp_artistbrowse* artistbrowse, sp_session* session)
{
    sp_artist* artist = sp_artistbrowse_artist( artistbrowse );

    char uri[MAX_LINK_NAME_LENGTH];
    sp_link* link = sp_link_create_from_artist(artist);

    sp_link_as_string(link, uri, sizeof(uri));
    sp_link_release(link);

    Artist artistObj( sp_artist_name(artist), uri );

    for ( int i = 0; i < sp_artistbrowse_num_albums(artistbrowse); i++ )
    {
        sp_album* album = sp_artistbrowse_album( artistbrowse, i );
        // todo maybe filter out "appears on" albums as well?
        if ( sp_album_is_available( album ) ) // there's a ridiculous amount of albums not available
            artistObj.addAlbum( spotifyGetAlbum( album, session ) );
    }

    return artistObj;
}

void LibSpotifyIf::libSpotifySessionCreate()
{
    sp_session_config config;
    sp_error error;

    memset(&config, 0, sizeof(config));

    // Always do this. It allows libspotify to check for
    // header/library inconsistencies.
    config.api_version = SPOTIFY_API_VERSION;

    // The path of the directory to store the cache. This must be specified.
    // Please read the documentation on preferred values.
    config.cache_location = config_.getCacheLocation().c_str();

    // The path of the directory to store the settings.
    // This must be specified.
    // Please read the documentation on preferred values.
    config.settings_location = config_.getSettingsLocation().c_str();

    // The key of the application. They are generated by Spotify,
    // and are specific to each application using libspotify.
    config.application_key = g_appkey;
    config.application_key_size = g_appkey_size;

    // This identifies the application using some
    // free-text string [1, 255] characters.
    config.user_agent = "spotifyserver";

    // Register the callbacks.
    config.callbacks = itsCallbackWrapper_.getRegisteredSessionCallbacks();

    error = sp_session_create(&config, &spotifySession_);
    if (SP_ERROR_OK != error) {
        std::cerr << "failed to create session: " << sp_error_message(error);
        assert(0);
    }
}

} /* namespace LibSpotify */
