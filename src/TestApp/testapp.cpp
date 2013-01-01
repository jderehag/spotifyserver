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
#include "AudioEndpointRemoteSocketServer.h"
#include "RemoteMediaInterface.h"
#include "UIConsole.h"
#include "Platform/Utils/Utils.h"
#include "applog.h"


int main(int argc, char *argv[])
{
    std::string servaddr("127.0.0.1");
    ConfigHandling::LoggerConfig cfg;
    cfg.setLogTo(ConfigHandling::LoggerConfig::STDOUT);
    Logger::Logger logger(cfg);
    ConfigHandling::NetworkConfig audioepservercfg;
    audioepservercfg.setPort("7789");
    ConfigHandling::AudioEndpointConfig audiocfg;

    AudioEndpointRemoteSocketServer audioserver( audiocfg, audioepservercfg );


    if(argc > 1)
        servaddr = std::string(argv[1]);

    SocketClient sc(servaddr, 7788);
    RemoteMediaInterface m(sc);
    UIConsole ui(m);

    /* wait for ui thread to exit */
    ui.joinThread();

    std::cout << "Exiting" << std::endl;

    /* cleanup */
    ui.destroy();
    sc.destroy();
    audioserver.destroy();

    return 0;
}
