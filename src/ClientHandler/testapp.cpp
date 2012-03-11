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
//============================================================================
// Name        : testapp.cpp
// Author      : Jens Nielsen
// Version     :
// Copyright   :
// Description : Some dumb testapp
//============================================================================

#include "MessageFactory/TlvDefinitions.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#define GET(x) (ntohl(*(u_int32_t*)x))
#define PUT(x, v) ((*(u_int32_t*)x) = htonl(v))

int main(int argc, char *argv[])
{
    int sockfd, n;
    struct sockaddr_in serv_addr;
    char buffer[1024];
    int seqnum=0;
    int len=0;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        std::cout << "ERROR opening socket";
        exit(1);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;

    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    //inet_aton("127.0.0.1", &serv_addr.sin_addr);
    serv_addr.sin_port = htons(7788);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
    {
        std::cout << "ERROR connecting";
        exit(1);
    }

    while(1)
    {
    	std::cout << "'g' get playlists\n"
    			     "'t' get tracks\n"
    			     "'p' play track\n"
    				 "'s' search using query\n\n";

    	char c = getchar();
    	len = 0;
    	switch(c)
    	{
    	case 'g':
    	{
    		PUT(&buffer[len], 12);
    		len+=4;
    		PUT(&buffer[len], GET_PLAYLISTS_REQ);
    		len+=4;
    		PUT(&buffer[len], seqnum++);
    		len+=4;
    		break;
    	}
    	case 't':
    	{
    		const char* playlist = "spotify:playlist:BestOfOasis";
    		int playlistlen = strlen(playlist);
    		if (playlistlen%4)
    			playlistlen += (4-(playlistlen%4));

    		PUT(&buffer[len], playlistlen + 12 + 8 + 8);
    		len+=4;
    		PUT(&buffer[len], GET_TRACKS_REQ);
    		len+=4;
    		PUT(&buffer[len], seqnum++);
    		len+=4;
    		PUT(&buffer[len], TLV_PLAYLIST);
    		len+=4;
    		PUT(&buffer[len], playlistlen + 8);
    		len+=4;
    		PUT(&buffer[len], TLV_LINK);
    		len+=4;
    		PUT(&buffer[len], playlistlen);
    		len+=4;
    		strncpy(&buffer[len], playlist, playlistlen);
    		len+=playlistlen;

    		break;
    	}
    	case 'p':
    	{
    		const char* track = "spotify:track:2CT3r93YuSHtm57mjxvjhH";
    		int tracklen = strlen(track);
    		if (tracklen%4)
    			tracklen += (4-(tracklen%4));

    		len+=4;
    		PUT(&buffer[len], PLAY_REQ);
    		len+=4;
    		PUT(&buffer[len], seqnum++);
    		len+=4;
    		PUT(&buffer[len], TLV_LINK);
    		len+=4;
    		PUT(&buffer[len], tracklen);
    		len+=4;
    		strncpy(&buffer[len], track, tracklen);
    		len+=tracklen;

    		PUT(buffer, len);

    		break;
    	}

    	case 's':
    	{
    		std::string query;
    		std::cout << "Write your search query, end with enter:" << std::endl;
    		std::cin >> query;
    		/*byte alignment?*/
    		int querypadding = (4-(query.length() % 4));
    		int messageLength = 0;
    		PUT(&buffer[len], 12/*dummy, overwrite later */); /* messagelength */
    		len +=4;
    		messageLength +=4;
    		PUT(&buffer[len], GENERIC_SEARCH_REQ);
    		len+=4;
    		messageLength +=4;
    		PUT(&buffer[len], seqnum++);
    		len+=4;
    		messageLength +=4;
    		PUT(&buffer[len], TLV_SEARCH_QUERY);
    		len+=4;
    		messageLength +=4;
    		PUT(&buffer[len], query.length() + querypadding);
    		len+=4;
    		messageLength +=4;
    		strncpy(&buffer[len], query.c_str(), query.length());
    		len+=query.length();
    		messageLength +=query.length();

    		/* pad with zeroes */
    		memset(&buffer[len], 0, querypadding);
    		len+=querypadding;
    		messageLength+=querypadding;

    		/* update the header with messagelength*/
    		PUT(&buffer[len - messageLength],messageLength);

    		break;
    	}
    	default:
    		continue;
    	}
    	std::cout << "Sending " << len << " bytes" << std::endl;

		for(int i=0; i<len; i++)
		{
			char s[3];

			//sprintf(s, "%.2x", buffer[i]);
			if(((buffer[i] >> 4) & 0xf) <= 9)
				s[0] = ((buffer[i] >> 4) & 0xf) + '0';
			else
				s[0] = ((buffer[i] >> 4) & 0xf) - 10 + 'a';

			if((buffer[i] & 0xf) <= 9)
				s[1] = (buffer[i] & 0xf) + '0';
			else
				s[1] = (buffer[i] & 0xf) - 10 + 'a';

			s[2]  = '\0';
			std::cout << s;

			if(i%32 == 31)
				std::cout << std::endl;
			else if(i%4 == 3)
				std::cout << " ";
		}
		std::cout << std::endl << std::endl;

    	n = send(sockfd,buffer,len,0);
    	if (n <= 0)
    	{
    		std::cout << "ERROR writing to socket";
    		exit(1);
    	}
    	bzero(buffer,1024);
    	n = recv(sockfd,buffer,1024,0);

    	if(n <= 0)
    	{
    		std::cout << "ERROR reading from socket";
    		exit(1);
    	}

    	std::cout << "Received " << n << " bytes" << std::endl;
		for(int i=0; i<n; i++)
		{
			char s[3];

			if(((buffer[i] >> 4) & 0xf) <= 9)
				s[0] = ((buffer[i] >> 4) & 0xf) + '0';
			else
				s[0] = ((buffer[i] >> 4) & 0xf) - 10 + 'a';

			if((buffer[i] & 0xf) <= 9)
				s[1] = (buffer[i] & 0xf) + '0';
			else
				s[1] = (buffer[i] & 0xf) - 10 + 'a';

			s[2]  = '\0';

			std::cout << s;

			if(i%32 == 31)
				std::cout << std::endl;
			else if(i%4 == 3)
				std::cout << " ";
		}

		std::cout << std::endl << std::endl;


    }

    close(sockfd);
    return 0;
}
