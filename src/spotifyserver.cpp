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

#include <iostream>
#include <stdlib.h>
#include <stdio.h>

#include "LibSpotifyIf/LibSpotifyIf.h"
#include "AudioEndpointManager/AudioEndpointManager.h"
#include "ClientHandler/ClientHandler.h"
#include "Platform/AudioEndpoints/AudioEndpointLocal.h"
#include "ConfigHandling/ConfigHandler.h"
#include "Platform/Utils/Utils.h"
#include "Platform/Timers/TimerFramework.h"
#include "UIConsole.h"

#include "applog.h"
#include "LoggerImpl.h"




void printUsage()
{
    std::cout << "Usage: spotifyserver [-c config-file-path]" << std::endl;
}

int main(int argc, char *argv[])
{
    std::string configFile = std::string("spotifyserver.conf");
    
    /*no getopt for windows...*/
    for(int i = 1; i < argc; i++)
    {
        std::string arg = std::string(argv[i]);
        if (arg == "-c")
        {
            if (i+1 < argc)
            {
                configFile = std::string(argv[i+1]);
                i++;
            }
            else
            {
                std::cout << "No file specified for option: " << arg << std::endl;
                printUsage();
                return -1;
            }
        }
        else
        {
            std::cout << "Unknown option: " << arg << std::endl;
            printUsage();
            return -1;
        }
    }

    Platform::initTimers();

    ConfigHandling::ConfigHandler ch(configFile);
    ch.parseConfigFile();

    Logger::LoggerImpl l(ch.getLoggerConfig());

    ConfigHandling::SpotifyConfig spConfig = ch.getSpotifyConfig();
    if (spConfig.getUsername().empty())
    {
        std::string username;
        std::cout << "Enter username: ";
        std::cin >> username;
        spConfig.setUsername(username);
    }
    else
    {
        std::cout << "User " << spConfig.getUsername() << std::endl;
    }

    if (spConfig.getPassword().empty())
    {
        std::string pwd;
        std::cout << "Enter password: ";
        disableStdinEcho();
        std::cin >> pwd;
        enableStdinEcho();
        std::cout << std::endl;
        spConfig.setPassword(pwd);
    }


    LibSpotify::LibSpotifyIf libspotifyif(spConfig);
    libspotifyif.logIn();

    EndpointId serverId( ch.getGeneralConfig().getId() );
    Platform::AudioEndpointLocal audioEndpoint( ch.getAudioEndpointConfig(), serverId );
    AudioEndpointManager audioMgr( libspotifyif );
    audioMgr.createEndpoint( audioEndpoint, NULL, NULL );
    audioMgr.addEndpoint( audioEndpoint.getId(), NULL, NULL );

    EndpointsDb epDb = EndpointsDb( audioMgr );
    epDb.registerId( &serverId );
    ClientHandler clienthandler(ch.getNetworkConfig(), libspotifyif, audioMgr, epDb );

    UIConsole ui( libspotifyif, audioMgr );
    ui.joinThread();

    libspotifyif.logOut();

    /* cleanup */
    libspotifyif.destroy();
    clienthandler.destroy();
    audioEndpoint.destroy();

    Platform::deinitTimers();

    return 0;
}
