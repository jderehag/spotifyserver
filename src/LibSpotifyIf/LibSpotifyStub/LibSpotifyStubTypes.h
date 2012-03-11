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

#ifndef LIBSPOTIFYSTUBTYPES_H_
#define LIBSPOTIFYSTUBTYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <libspotify/api.h>

struct sp_search
{
	const char* did_you_mean;
	sp_track* tracks_array;
	int number_of_tracks;
};

struct sp_link
{
	char string[256];
	sp_track* pTrack;
};

struct sp_artist
{
    char name[256];
    char url[256];
};

struct sp_album
{
    char name[256];
    char url[256];
};

struct sp_track
{
	char name[256];
	char url[256];
	int duration;
	bool is_starred;
	bool is_local;
	bool is_autolinked;
	int ref_count;
	int num_artist;
	sp_artist* artist_array;
	sp_album album;
	//sp_link* link;
};

struct sp_playlist
{
	char name[256];
	char url[256];
	sp_playlist_type type;
	sp_uint64 folder_id;
	bool is_collaborative;
	int ref_count;
	int num_tracks;
	sp_track* track_array;
};

struct sp_playlistcontainer
{
	sp_playlistcontainer_callbacks callbacks;
	int num_playlists;
	sp_playlist* playlist_array;
};

struct sp_image
{
    char data[100];
};

struct sp_user
{
	char name[256];
	char canonicalName[256];
	bool is_logged_in;

};


struct sp_session
{
	char username[256];
	char password[256];
	bool remember_me;
	sp_connectionstate state;
	sp_user user;
	int user_country;
	sp_connectionstate connectionstate;
	sp_playlistcontainer playlistcontainer;
};


#ifdef __cplusplus
}
#endif

#endif /* LIBSPOTIFYSTUBTYPES_H_ */
