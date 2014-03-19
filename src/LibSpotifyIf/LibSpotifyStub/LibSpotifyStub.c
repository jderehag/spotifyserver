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
#include "LibSpotifyStubTypes.h"
#include <libspotify/api.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>

static sp_session_config g_config;

static sp_artist artist_array_1[] =
{
        {
                .name ="Oasis",
                .url = "spotify:artist:Oasis",
        }
};

static sp_artist artist_array_2[] =
{
        {
                .name ="Oasis",
                .url = "spotify:artist:Oasis",
        },

        {
                .name ="Bon Jovi",
                .url = "spotify:artist:BonJovi",
        }

};

static sp_album best_oasis_album =
{
        .name = "Oasis, the second coming..",
        .url = "spotify:album:Oasisthesecondcoming",
        .releaseYear = 2012,
};

static sp_track track_array_1[] =
{
		{
		.name = "Wonderwall",
		.is_starred = 1,
		.is_local = 1,
		.is_autolinked = 0,
		.ref_count = 0,
		.url = "spotify:track:Wonderwall",
		.album = &best_oasis_album,
		.artist_array = artist_array_1,
		.num_artist = sizeof(artist_array_1) / sizeof(artist_array_1[0]),
        //sp_link* link;
		},
		{
		.name = "Wonderwall 2, the better version",
		.is_starred = 1,
		.is_local = 1,
		.is_autolinked = 0,
		.ref_count = 0,
		.url = "spotify:track:Wonderwall2",
		.album = &best_oasis_album,
		.artist_array = artist_array_1,
        .num_artist = sizeof(artist_array_1) / sizeof(artist_array_1[0]),
		//sp_link* link;
		}
};

static sp_album second_coming_album =
{
        .name = "Oasis, the second coming..",
        .url = "spotify:album:Oasisthesecondcoming",
        .releaseYear = 2012,
};

static sp_track track_array_2[] =
{
		{
		.name = "Wonderwall 3, slightly worse than Wonderwall 2, but still good!",
		.is_starred = 1,
		.is_local = 1,
		.is_autolinked = 0,
		.ref_count = 0,
		.url = "spotify:track:Wonderwall3",
		.album = &second_coming_album,
		.artist_array = artist_array_2,
        .num_artist = sizeof(artist_array_2) / sizeof(artist_array_2[0]),
		//sp_link* link;
		},
		{
		.name = "Wonderwall 4",
		.is_starred = 1,
		.is_local = 1,
		.is_autolinked = 0,
		.ref_count = 0,
		.url = "spotify:track:Wonderwall4",
		.album = &second_coming_album,
		.artist_array = artist_array_2,
        .num_artist = sizeof(artist_array_2) / sizeof(artist_array_2[0]),
		//sp_link* link;
		}
};

static sp_playlist playlist_array[] =
{
		{
				.name = "Best of oasis",
				.type = SP_PLAYLIST_TYPE_PLAYLIST,
				.folder_id = 1,
				.is_collaborative = 0,
				.num_tracks = sizeof(track_array_1)/sizeof(track_array_1[0]),
				.track_array = track_array_1,
				.url = "spotify:playlist:BestOfOasis",
		},
		{
				.name = "Oasis, now even better!",
				.type = SP_PLAYLIST_TYPE_PLAYLIST,
				.folder_id = 1,
				.is_collaborative = 0,
				.num_tracks = sizeof(track_array_2)/sizeof(track_array_2[0]),
				.track_array = track_array_2,
				.url = "spotify:playlist:OasisNowEvenBetter!",
		},
		{
				.name = "The rise and fall of Oasis",
				.type = SP_PLAYLIST_TYPE_PLAYLIST,
				.folder_id = 1,
				.is_collaborative = 0,
				.num_tracks = 0,
				.track_array = 0,
				.url = "spotify:playlist:TheRiseAndFallOfOasis",

		}

};

static sp_album album_search_result = {"Oasis, the searchresult album", "spotify:album:Theoasissearchresult", 2012};

static sp_track track_search_result[] =
{
		{
			.name = "Wonderwall - The search result",
			.is_starred = 1,
			.is_local = 0,
			.is_autolinked = 0,
			.ref_count = 0,
			.url = "spotify:track:WonderwallSearchResult",
		    .artist_array = artist_array_1,
			.num_artist = sizeof(artist_array_1)/sizeof(artist_array_1[0]),
		    .album = &album_search_result,
			//sp_link* link;
		}
};
static sp_search result =
{
		.did_you_mean = "You better not have meant Blur!",
		.tracks_array = track_search_result,
		.number_of_tracks = sizeof(track_search_result)/sizeof(track_search_result[0])
};

static sp_image image =
{
        .data = "binary stuff",
};

static sp_albumbrowse albumbrowse_result =
{
        .album = &best_oasis_album,
        .num_tracks = sizeof(track_array_1)/sizeof(track_array_1[0]),
        .track_array = track_array_1,
        .review = "The best album ever. Angels are weeping rainbows.",
};

/*************************************
 * Session stubs
 * *************************************/

sp_error sp_session_create(const sp_session_config *config, sp_session **sess)
{
	g_config = *config;
	*sess = malloc(sizeof(sp_session));
	return SP_ERROR_OK;
}

sp_error sp_session_login(sp_session *session, const char *username, const char *password, bool remember_me, const char *blob)
{
	/* session */
	strcpy(session->username, username);
	strcpy(session->password, password);
	session->remember_me = remember_me;
	session->user_country = 1;

	/* user */
	strcpy(session->user.name, username);
	strcpy(session->user.canonicalName, username);
	session->user.is_logged_in = 1;

	session->playlistcontainer.playlist_array = playlist_array;
	session->playlistcontainer.num_playlists = sizeof(playlist_array)/sizeof(playlist_array[0]);


	session->connectionstate = SP_CONNECTION_STATE_LOGGED_IN;
	g_config.callbacks->log_message(session, "Logged in!\n");
	g_config.callbacks->logged_in(session, SP_ERROR_OK);
	return SP_ERROR_OK;
}

sp_error sp_session_relogin(sp_session *session)
{
	g_config.callbacks->logged_in(session, SP_ERROR_OK);
	return SP_ERROR_OK;
}

sp_error sp_session_logout(sp_session *session)
{
	session->connectionstate = SP_CONNECTION_STATE_LOGGED_OUT;
	g_config.callbacks->log_message(session, "Logged out!\n");
	g_config.callbacks->logged_out(session);
	return SP_ERROR_OK;
};
sp_error sp_session_release(sp_session *sess) { return SP_ERROR_OK; }
sp_user* sp_session_user(sp_session *session) { return &session->user; }
int sp_session_user_country(sp_session *session) { return session->user_country; }
sp_connectionstate sp_session_connectionstate(sp_session *session) { return session->connectionstate; }
sp_error sp_session_process_events(sp_session *session, int *next_timeout){ *next_timeout = 1; return SP_ERROR_OK; }
sp_error sp_session_preferred_bitrate (sp_session *session, sp_bitrate bitrate){ return SP_ERROR_OK; }


/* ************************************
 * User stubs
 * *************************************/
/* user is statically declared, so this is always true */
bool sp_user_is_loaded(sp_user *user){ return 1; }
const char* sp_user_display_name(sp_user *user){ return user->name; }
const char* sp_user_canonical_name(sp_user *user){ return user->canonicalName;}





/* ************************************
 * Playlist stubs
 * *************************************/
sp_playlistcontainer* sp_session_playlistcontainer(sp_session *session) { return &session->playlistcontainer; }
int sp_playlistcontainer_num_playlists(sp_playlistcontainer *pc) { return pc->num_playlists; }

sp_playlist* sp_playlist_create(sp_session* session, sp_link* link)
{
    /*TODO: for now, only return first playlist in container */
    return sp_playlistcontainer_playlist(sp_session_playlistcontainer(session), 0);
}

sp_playlist* sp_playlistcontainer_playlist(sp_playlistcontainer *pc, int index)
{
	assert(index >= 0 && index < pc->num_playlists);
	return &pc->playlist_array[index];
}

const char * sp_playlist_name(sp_playlist *playlist)
{
	return playlist->name;
}
sp_playlist_type sp_playlistcontainer_playlist_type(sp_playlistcontainer *pc, int index)
{
	assert(index >= 0 && index < pc->num_playlists);
	return pc->playlist_array[index].type;
}

sp_error sp_playlistcontainer_playlist_folder_name(sp_playlistcontainer *pc, int index, char *buffer, int buffer_size)
{
	/* validation */
	sp_playlist* pl = sp_playlistcontainer_playlist(pc, index);
	assert(SP_PLAYLIST_TYPE_START_FOLDER == sp_playlistcontainer_playlist_type(pc, index));
	if(buffer_size < sizeof(pl->name))return SP_ERROR_INDEX_OUT_OF_RANGE;

	strcpy(buffer, pl->name);
	return SP_ERROR_OK;
}

sp_uint64 sp_playlistcontainer_playlist_folder_id(sp_playlistcontainer *pc, int index)
{
	sp_playlist* pl = sp_playlistcontainer_playlist(pc, index);
	return pl->folder_id;
}

int sp_playlist_num_tracks(sp_playlist *playlist)
{
	return playlist->num_tracks;
}

sp_track* sp_playlist_track(sp_playlist *playlist, int index)
{
	assert(sp_playlist_num_tracks(playlist) > index);
	return &playlist->track_array[index];
}

bool sp_playlist_is_collaborative(sp_playlist *playlist)
{
	return playlist->is_collaborative;
}

sp_error sp_playlist_add_ref(sp_playlist *playlist)
{
	playlist->ref_count++;
    return SP_ERROR_OK;
}

sp_error sp_playlist_release(sp_playlist *playlist)
{
	playlist->ref_count--;
    return SP_ERROR_OK;
}

bool sp_playlist_is_loaded(sp_playlist *playlist)
{
	return 1;
}


/* ************************************
 * Track stubs
 * * *********************************/
const char* sp_track_name(sp_track *track)
{
	return track->name;
}

int sp_track_num_artists(sp_track* track)
{
    return track->num_artist;
}

sp_artist* sp_track_artist(sp_track *track, int index)
{
    assert(index < track->num_artist);
    return &track->artist_array[index];
}

sp_album* sp_track_album(sp_track *track)
{
    return track->album;
}

int sp_track_duration(sp_track *track)
{
    return track->duration;
}

bool sp_track_is_starred(sp_session *session, sp_track *track)
{
	return track->is_starred;
}

bool sp_track_is_local(sp_session *session, sp_track *track)
{
	return track->is_local;
}

bool sp_track_is_autolinked(sp_session *session, sp_track *track)
{
	return track->is_autolinked;
}

sp_error sp_track_add_ref(sp_track *track)
{
	track->ref_count++;
    return SP_ERROR_OK;
}

sp_error sp_track_release(sp_track *track)
{
	track->ref_count--;
	return SP_ERROR_OK;
}


/* ************************************
 * Artist stubs
 * * *********************************/
const char* sp_artist_name(sp_artist *artist)
{
    return artist->name;
}

/* ************************************
 * Album stubs
 * * *********************************/
const char * sp_album_name(sp_album *album)
{
    return album->name;
}

bool sp_album_is_loaded(sp_album* album)
{
    return 1;
}

const byte* sp_album_cover(sp_album *album, sp_image_size size)
{
    return (const byte*) "an image reference";
}

int sp_album_year(sp_album *album)
{
    return album->releaseYear;
}

bool sp_album_is_available(sp_album *album)
{
    return 1;
}

/* ************************************
 * Albumbrowse stubs
 * * *********************************/

sp_albumbrowse * sp_albumbrowse_create(sp_session *session, sp_album *album, albumbrowse_complete_cb *callback, void *userdata)
{
    callback(&albumbrowse_result, userdata);
    return &albumbrowse_result;
}

sp_error sp_albumbrowse_release(sp_albumbrowse *alb)
{
    return SP_ERROR_OK;
}

sp_album * sp_albumbrowse_album(sp_albumbrowse *alb)
{
    return alb->album;
}

int sp_albumbrowse_num_tracks(sp_albumbrowse *alb)
{
    return alb->num_tracks;
}

sp_track* sp_albumbrowse_track(sp_albumbrowse *alb, int index)
{
    assert(sp_albumbrowse_num_tracks(alb) > index);
    return &alb->track_array[index];
}

const char* sp_albumbrowse_review(sp_albumbrowse *alb)
{
    return alb->review;
}

sp_artist* sp_albumbrowse_artist(sp_albumbrowse* alb)
{
    return &artist_array_1[0];
}

/* ************************************
 * Image stubs
 * * *********************************/
sp_image* sp_image_create(sp_session *session, const byte image_id[20])
{
    return &image;
}

sp_error sp_image_error(sp_image *image)
{
    return SP_ERROR_OK;
}

bool sp_image_is_loaded(sp_image *image)
{
    return 1;
}

const void* sp_image_data(sp_image *image, size_t *data_size)
{
    *data_size = sizeof(image->data);
    return image->data;
}

sp_error sp_image_release(sp_image *image)
{
    return SP_ERROR_OK;
}

sp_error sp_image_add_load_callback(sp_image *image, image_loaded_cb *callback, void *userdata)
{
    return SP_ERROR_OK;
}

/* ************************************
 * Link stubs
 * * *********************************/
sp_link* sp_link_create_from_string(const char *link)
{
	sp_link* ln = malloc(sizeof(sp_link));
	assert(strlen(link) < sizeof(ln->string));
	strcpy(ln->string, link);
	return ln;
}

sp_link* sp_link_create_from_playlist(sp_playlist* pl)
{
	sp_link* ln = malloc(sizeof(sp_link));
	strcpy(ln->string, pl->url);
	return ln;
}

sp_link* sp_link_create_from_track(sp_track* track, int offset)
{
	sp_link* ln = malloc(sizeof(sp_link));
	strcpy(ln->string, track->url);
	return ln;
}

sp_link* sp_link_create_from_album(sp_album* album)
{
    sp_link* ln = malloc(sizeof(sp_link));
    strcpy(ln->string, album->url);
    return ln;
}

sp_link* sp_link_create_from_artist(sp_artist* artist)
{
    sp_link* ln = malloc(sizeof(sp_link));
    strcpy(ln->string, artist->url);
    return ln;
}

int sp_link_as_string(sp_link *link, char *buffer, int buffer_size)
{
	assert(strlen(link->string) < buffer_size);
	strcpy(buffer,link->string);
	return strlen(buffer);
}

sp_error sp_link_release(sp_link *link)
{
	free(link);
	link = 0;
	return SP_ERROR_OK;
}

sp_track* sp_link_as_track(sp_link *link)
{
	return &track_array_1[0];//link->pTrack;
}

sp_album* sp_link_as_album(sp_link *link)
{
    return &best_oasis_album;
}

sp_linktype sp_link_type(sp_link *link)
{
    if(strncmp(link->string, "spotify:track", strlen("spotify:track")) == 0)
    {
        return SP_LINKTYPE_TRACK;
    }
    else if(strncmp(link->string, "spotify:album", strlen("spotify:album")) == 0)
    {
        return SP_LINKTYPE_ALBUM;
    }
    else if(strncmp(link->string, "spotify:artist", strlen("spotify:artist")) == 0)
    {
        return SP_LINKTYPE_ARTIST;
    }
    else if(strncmp(link->string, "spotify:playlist", strlen("spotify:playlist")) == 0)
    {
        return SP_LINKTYPE_PLAYLIST;
    }
    else
    {
        return SP_LINKTYPE_INVALID;
    }
}


/* ************************************
 * Playback stubs
 * * *********************************/
sp_error sp_session_player_load(sp_session *session, sp_track *track)
{
	/* callback!*/
	return SP_ERROR_OK;
}

sp_error sp_session_player_unload(sp_session *session)
{
    return SP_ERROR_OK;
}
/* playback not supported on stub */
sp_error sp_session_player_play(sp_session *session, bool play)
{
    /*g_config.callbacks->end_of_track(session);*/
    return SP_ERROR_OK;
}

sp_error sp_track_error(sp_track *track){ return SP_ERROR_OK; }


/* ************************************
 * Search stubs
 * * *********************************/
sp_search* sp_search_create(sp_session *session,
                            const char *query,
                            int track_offset,
                            int track_count,
                            int album_offset,
                            int album_count,
                            int artist_offset,
                            int artist_count,
                            int playlist_offset,
                            int playlist_count,
                            sp_search_type search_type,
                            search_complete_cb *callback,
                            void *userdata)
{
	callback(&result, userdata);
	return &result;
}

sp_track* sp_search_track(sp_search *search, int index)
{
	assert(index < search->number_of_tracks);
	return &(search->tracks_array[index]);
}

const char* sp_search_did_you_mean(sp_search* search)
{
	return search->did_you_mean;
}

int sp_search_total_tracks(sp_search *search)
{
	return search->number_of_tracks;
}

int sp_search_num_tracks(sp_search *search)
{
	return search->number_of_tracks;
}

sp_error sp_search_release(sp_search *search){ return SP_ERROR_OK; }
/* ************************************
 * Util stubs
 * * *********************************/
const char* sp_error_message(sp_error error)
{
	switch(error)
	{
		case SP_ERROR_OK:
			return "SP_ERROR_OK";
		case SP_ERROR_BAD_API_VERSION:
			return "SP_ERROR_BAD_API_VERSION";
		case SP_ERROR_API_INITIALIZATION_FAILED:
			return "SP_ERROR_API_INITIALIZATION_FAILED";
		case SP_ERROR_TRACK_NOT_PLAYABLE:
			return "SP_ERROR_TRACK_NOT_PLAYABLE";
		case SP_ERROR_BAD_APPLICATION_KEY:
			return "SP_ERROR_BAD_APPLICATION_KEY";
		case SP_ERROR_BAD_USERNAME_OR_PASSWORD:
			return "SP_ERROR_BAD_USERNAME_OR_PASSWORD";
		case SP_ERROR_USER_BANNED:
			return "SP_ERROR_USER_BANNED";
		case SP_ERROR_UNABLE_TO_CONTACT_SERVER:
			return "SP_ERROR_UNABLE_TO_CONTACT_SERVER";
		case SP_ERROR_CLIENT_TOO_OLD:
			return "SP_ERROR_CLIENT_TOO_OLD";
		case SP_ERROR_OTHER_PERMANENT:
			return "SP_ERROR_OTHER_PERMANENT";
		case SP_ERROR_BAD_USER_AGENT:
			return "SP_ERROR_BAD_USER_AGENT";
		case SP_ERROR_MISSING_CALLBACK:
			return "SP_ERROR_MISSING_CALLBACK";
		case SP_ERROR_INVALID_INDATA:
			return "SP_ERROR_INVALID_INDATA";
		case SP_ERROR_INDEX_OUT_OF_RANGE:
			return "SP_ERROR_INDEX_OUT_OF_RANGE";
		case SP_ERROR_USER_NEEDS_PREMIUM:
			return "SP_ERROR_USER_NEEDS_PREMIUM";
		case SP_ERROR_OTHER_TRANSIENT:
			return "SP_ERROR_OTHER_TRANSIENT";
		case SP_ERROR_IS_LOADING:
			return "SP_ERROR_IS_LOADING";
		case SP_ERROR_NO_STREAM_AVAILABLE:
			return "SP_ERROR_NO_STREAM_AVAILABLE";
		case SP_ERROR_PERMISSION_DENIED:
			return "SP_ERROR_PERMISSION_DENIED";
		case SP_ERROR_INBOX_IS_FULL:
			return "SP_ERROR_INBOX_IS_FULL";
		case SP_ERROR_NO_CACHE:
			return "SP_ERROR_NO_CACHE";
		case SP_ERROR_NO_SUCH_USER:
			return "SP_ERROR_NO_SUCH_USER";
		case SP_ERROR_NO_CREDENTIALS:
			return "SP_ERROR_NO_CREDENTIALS";
		case SP_ERROR_NETWORK_DISABLED:
			return "SP_ERROR_NETWORK_DISABLED";
		case SP_ERROR_INVALID_DEVICE_ID:
			return "SP_ERROR_INVALID_DEVICE_ID";
		case SP_ERROR_CANT_OPEN_TRACE_FILE:
			return "SP_ERROR_CANT_OPEN_TRACE_FILE";
		case SP_ERROR_APPLICATION_BANNED:
			return "SP_ERROR_APPLICATION_BANNED";
		case SP_ERROR_OFFLINE_TOO_MANY_TRACKS:
			return "SP_ERROR_OFFLINE_TOO_MANY_TRACKS";
		case SP_ERROR_OFFLINE_DISK_CACHE:
			return "SP_ERROR_OFFLINE_DISK_CACHE";
		case SP_ERROR_OFFLINE_EXPIRED:
			return "SP_ERROR_OFFLINE_EXPIRED";
		case SP_ERROR_OFFLINE_NOT_ALLOWED:
			return "SP_ERROR_OFFLINE_NOT_ALLOWED";
		case SP_ERROR_OFFLINE_LICENSE_LOST:
			return "SP_ERROR_OFFLINE_LICENSE_LOST";
		case SP_ERROR_OFFLINE_LICENSE_ERROR:
			return "SP_ERROR_OFFLINE_LICENSE_ERROR";
		default:
			return "Unknown error!";
	}
}


