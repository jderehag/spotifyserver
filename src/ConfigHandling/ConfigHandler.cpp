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

#include "ConfigHandler.h"
#include "ConfigParser.h"
#include "ConfigGenerator.h"

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <cstring>


namespace ConfigHandling
{

ConfigHandler::ConfigHandler(const std::string& pathToConfigFile) : configFilePath_(pathToConfigFile) { }
ConfigHandler::~ConfigHandler() { }


void ConfigHandler::writeConfigFile()
{
    /* General */
    applicationId = generalConfig_.getId();
    /* Spotify Section*/
    spotifyUsername = spotifyConfig_.getUsername();
    spotifyPassword = spotifyConfig_.getPassword();
    spotifyCacheLocation = spotifyConfig_.getCacheLocation();
    spotifySettingsLocation = spotifyConfig_.getSettingsLocation();
    spotifyRememberMe = spotifyConfig_.getRememberMeString();
    spotifyRepeat = spotifyConfig_.getRepeatString();
    spotifyShuffle = spotifyConfig_.getShuffleString();
    /* Network Section*/
    networkBindType = networkConfig_.getBindTypeString();
    networkIp = networkConfig_.getIp();
    networkPort = networkConfig_. getPort();
    networkDevice = networkConfig_.getDevice();
    networkUsername = networkConfig_.getUsername();
    networkPassword = networkConfig_.getPassword();
    /* AudioEndpoint Section*/
    audioEndpointType = audioEndpointConfig_.getEndpointTypeString();
    audioEndpointAlsaDevice = audioEndpointConfig_.getDevice();
    /* Logger Section */
    loggerLogLevel = loggerConfig_.getLogLevelString();
    loggerLogFile = loggerConfig_.getLogFile();

    std::list<std::string> outConfig;
    std::list<SectionAttributes> listOfAttributes = getConfigAttributes();
    generateConfig(config, outConfig, listOfAttributes);

    std::ofstream configFile(configFilePath_.c_str());

    if(configFile.is_open())
    {
        std::list<std::string>::iterator it = outConfig.begin();
        while( it != outConfig.end() )
        {
            configFile << *it;
            it++;
            if ( it != outConfig.end() )
                configFile << '\n';
        }
    }
}


std::list<SectionAttributes> ConfigHandler::getConfigAttributes()
{
    std::list<SectionAttributes> listOfSectionAttributes;

    /* General Section*/
    listOfSectionAttributes.push_back(SectionAttributes ( 0,  TYPE_SECTION,      "General"));
    listOfSectionAttributes.push_back(SectionAttributes ( 1,  TYPE_ATTRIBUTE,    "ID",                &applicationId            ));

    /* Spotify Section*/
    listOfSectionAttributes.push_back(SectionAttributes ( 0,  TYPE_SECTION,      "Spotify"));
    listOfSectionAttributes.push_back(SectionAttributes ( 1,  TYPE_ATTRIBUTE,    "Username",          &spotifyUsername            ));
    listOfSectionAttributes.push_back(SectionAttributes ( 1,  TYPE_ATTRIBUTE,    "Password",          &spotifyPassword            ));
    listOfSectionAttributes.push_back(SectionAttributes ( 1,  TYPE_ATTRIBUTE,    "RememberMe",        &spotifyRememberMe          ));
    listOfSectionAttributes.push_back(SectionAttributes ( 1,  TYPE_ATTRIBUTE,    "Repeat",            &spotifyRepeat              ));
    listOfSectionAttributes.push_back(SectionAttributes ( 1,  TYPE_ATTRIBUTE,    "Shuffle",           &spotifyShuffle             ));
    listOfSectionAttributes.push_back(SectionAttributes ( 1,  TYPE_ATTRIBUTE,    "CacheLocation",     &spotifyCacheLocation       ));
    listOfSectionAttributes.push_back(SectionAttributes ( 1,  TYPE_ATTRIBUTE,    "SettingsLocation",  &spotifySettingsLocation    ));

    /* Network Section*/
    listOfSectionAttributes.push_back(SectionAttributes ( 0,  TYPE_SECTION,      "Network",           NULL                        ));
    listOfSectionAttributes.push_back(SectionAttributes ( 1,  TYPE_ATTRIBUTE,    "BindType",          &networkBindType            ));
    listOfSectionAttributes.push_back(SectionAttributes ( 1,  TYPE_ATTRIBUTE,    "Ip",                &networkIp                  ));
    listOfSectionAttributes.push_back(SectionAttributes ( 1,  TYPE_ATTRIBUTE,    "Port",              &networkPort                ));
    listOfSectionAttributes.push_back(SectionAttributes ( 1,  TYPE_ATTRIBUTE,    "Device",            &networkDevice              ));
    listOfSectionAttributes.push_back(SectionAttributes ( 1,  TYPE_ATTRIBUTE,    "Username",          &networkUsername            ));
    listOfSectionAttributes.push_back(SectionAttributes ( 1,  TYPE_ATTRIBUTE,    "Password",          &networkPassword            ));

    /* AudioEndpoint Section*/
    listOfSectionAttributes.push_back(SectionAttributes ( 0,  TYPE_SECTION,      "AudioEndpoint",     NULL                        ));
    listOfSectionAttributes.push_back(SectionAttributes ( 1,  TYPE_ATTRIBUTE,    "Type",              &audioEndpointType          ));
    listOfSectionAttributes.push_back(SectionAttributes ( 1,  TYPE_SUBSECTION,   "ALSA",              NULL                        ));
    listOfSectionAttributes.push_back(SectionAttributes ( 2,  TYPE_ATTRIBUTE,    "Device",            &audioEndpointAlsaDevice    ));

    /* Logger Section*/
    listOfSectionAttributes.push_back(SectionAttributes ( 0,  TYPE_SECTION,      "Logger",            NULL                        ));
    listOfSectionAttributes.push_back(SectionAttributes ( 1,  TYPE_ATTRIBUTE,    "LogLevel",          &loggerLogLevel             ));
    listOfSectionAttributes.push_back(SectionAttributes ( 1,  TYPE_ATTRIBUTE,    "LogFile",           &loggerLogFile              ));

    return listOfSectionAttributes;
}

void ConfigHandler::parseConfigFile()
{
    readFromFile();

    std::list<SectionAttributes> listOfAttributes = getConfigAttributes();
    parseConfig(config, listOfAttributes);

    /* General */
    generalConfig_.setId( applicationId );

    /* Spotify */
    spotifyConfig_.setUsername(spotifyUsername);
    spotifyConfig_.setPassword(spotifyPassword);
    spotifyConfig_.setCacheLocation(spotifyCacheLocation);
    spotifyConfig_.setSettingsLocation(spotifySettingsLocation);
    spotifyConfig_.setRememberMe(spotifyRememberMe);
    spotifyConfig_.setRepeat(spotifyRepeat);
    spotifyConfig_.setShuffle(spotifyShuffle);

    /* Network */
    networkConfig_.setBindType(networkBindType);
    networkConfig_.setIp(networkIp);
    networkConfig_.setPort(networkPort);
    networkConfig_.setDevice(networkDevice);
    networkConfig_.setUsername(networkUsername);
    networkConfig_.setPassword(networkPassword);

    /* AudioEndpoint */
    audioEndpointConfig_.setEndpointType(audioEndpointType);
    audioEndpointConfig_.setDevice(audioEndpointAlsaDevice);

    /* Logger */
    loggerConfig_.setLogLevel(loggerLogLevel);
    loggerConfig_.setLogFile(loggerLogFile);

    generalConfig_.setWriteIf( this );
    spotifyConfig_.setWriteIf( this );
}

void ConfigHandler::readFromFile()
{
    config.clear();
    std::ifstream configFile(configFilePath_.c_str());

    if(configFile.is_open())
    {
        while(configFile.good())
        {
            std::string line;
            getline(configFile, line);

            config.push_back( line );
        }
    }
    else
    {
        std::cerr << "Failed to open config file " << configFilePath_ << ", using default parameters" << std::endl;
        return;
    }
    configFile.close();
}

const std::string& ConfigHandler::getConfigFilePath() const
{
    return configFilePath_;
}

GeneralConfig& ConfigHandler::getGeneralConfig()
{
    return generalConfig_;
}

const AudioEndpointConfig& ConfigHandler::getAudioEndpointConfig() const
{
    return audioEndpointConfig_;
}

const LoggerConfig& ConfigHandler::getLoggerConfig() const
{
    return loggerConfig_;
}

const NetworkConfig& ConfigHandler::getNetworkConfig() const
{
    return networkConfig_;
}

SpotifyConfig& ConfigHandler::getSpotifyConfig()
{
    return spotifyConfig_;
}

} /* namespace ConfigHandling */
