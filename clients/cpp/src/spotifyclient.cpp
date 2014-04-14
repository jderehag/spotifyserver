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

#include "SocketHandling/SocketClient.h"
#include "RemoteMediaInterface.h"
#include "UIConsole.h"
#include "Platform/Utils/Utils.h"
#include "Platform/AudioEndpoints/AudioEndpointLocal.h"
#include "AudioEndpointManager/RemoteAudioEndpointManager.h"
#include "Platform/Timers/TimerFramework.h"
#include "applog.h"
#include "LoggerImpl.h"

int main(int argc, char *argv[])
{
    std::string configFile = std::string("spotifyclient.conf");

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
                //printUsage();
                return -1;
            }
        }
        else
        {
            std::cout << "Unknown option: " << arg << std::endl;
            //printUsage();
            return -1;
        }
    }

    Platform::initTimers();

    ConfigHandling::ConfigHandler ch(configFile);
    ch.parseConfigFile();

    ConfigHandling::LoggerConfig cfg = ch.getLoggerConfig();
    cfg.setLogTo(ConfigHandling::LoggerConfig::STDOUT);
    Logger::LoggerImpl l(cfg);

    Platform::AudioEndpointLocal audioEndpoint(ch.getAudioEndpointConfig());

    SocketClient sc(ch.getNetworkConfig());
    RemoteMediaInterface m(sc);

    RemoteAudioEndpointManager audioMgr(sc);
    audioMgr.createEndpoint(audioEndpoint, NULL, NULL);

    UIConsole ui( m, audioMgr );

    /* wait for ui thread to exit */
    ui.joinThread();

    std::cout << "Exiting" << std::endl;

    /* cleanup */
    ui.destroy();
    sc.destroy();

#if AUDIO_SERVER
    audioserver.destroy();
#endif

    Platform::deinitTimers();

    return 0;
}
