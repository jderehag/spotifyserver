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

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <cstring>

namespace ConfigHandling
{
ConfigHandler::ConfigHandler(const std::string& pathToConfigFile) : configFilePath_(pathToConfigFile) { }
ConfigHandler::~ConfigHandler() { }

void ConfigHandler::parseConfigFile()
{
    std::string configString("");
    readFromFile(configString);
    /* Spotify Section*/
    std::string spotifyUsername;
    std::string spotifyPassword;
    std::string spotifyCacheLocation;
    std::string spotifySettingsLocation;
    /* Network Section*/
    std::string networkBindType;
    std::string networkIp;
    std::string networkPort;
    std::string networkDevice;
    /* AudioEndpoint Section*/
    std::string audioEndpointType;
    std::string audioEndpointAlsaDevice;
    /* Logger Section */
    std::string loggerLogLevel;
    std::string loggerLogFile;

    SectionAttributes parseDefintion[] = {

        /* Spotify Section*/
        {0,     TYPE_SECTION,                 "Spotify",               NULL                        },
        {1,     TYPE_ATTRIBUTE,               "Username",              &spotifyUsername            },
        {1,     TYPE_ATTRIBUTE,               "Password",              &spotifyPassword            },
        {1,     TYPE_ATTRIBUTE,               "CacheLocation",         &spotifyCacheLocation       },
        {1,     TYPE_ATTRIBUTE,               "SettingsLocation",      &spotifySettingsLocation    },

        /* Network Section*/
        {0,     TYPE_SECTION,                 "Network",               NULL                        },
        {1,     TYPE_ATTRIBUTE,               "BindType",              &networkBindType            },
        {1,     TYPE_ATTRIBUTE,               "Ip",                    &networkIp                  },
        {1,     TYPE_ATTRIBUTE,               "Port",                  &networkPort                },
        {1,     TYPE_ATTRIBUTE,               "Device",                &networkDevice              },

        /* AudioEndpoint Section*/
        {0,     TYPE_SECTION,                 "AudioEndpoint",         NULL                        },
        {1,     TYPE_ATTRIBUTE,               "Type",                  &audioEndpointType          },
        {1,     TYPE_SUBSECTION,              "ALSA",                  NULL                        },
        {2,     TYPE_ATTRIBUTE,               "Device",                &audioEndpointAlsaDevice    },

        /* Logger Section*/
        {0,     TYPE_SECTION,                 "Logger",                NULL                        },
        {1,     TYPE_ATTRIBUTE,               "LogLevel",              &loggerLogLevel             },
        {1,     TYPE_ATTRIBUTE,               "LogFile",               &loggerLogFile              }
	};

	parseConfig(configString, parseDefintion);

	/* Spotify */
	spotifyConfig_.setUsername(spotifyUsername);
	spotifyConfig_.setPassword(spotifyPassword);
	spotifyConfig_.setCacheLocation(spotifyCacheLocation);
	spotifyConfig_.setSettingsLocation(spotifySettingsLocation);

	/* Network */
	networkConfig_.setBindType(networkBindType);
	networkConfig_.setIp(networkIp);
	networkConfig_.setPort(networkPort);
	networkConfig_.setDevice(networkDevice);

	/* AudioEndpoint */
	audioEndpointConfig_.setEndpointType(audioEndpointType);
	audioEndpointConfig_.setDevice(audioEndpointAlsaDevice);

	/* Logger */
	loggerConfig_.setLogLevel(loggerLogLevel);
	loggerConfig_.setLogFile(loggerLogFile);

}

void ConfigHandler::readFromFile(std::string& configString)
{
	configString.clear();
	std::ifstream configFile(configFilePath_.c_str());

	if(configFile.is_open())
	{
		while(configFile.good())
		{
			char line[256];
			configFile.getline(line, sizeof(line));

			/* skip comments and empty lines, should perhaps be done in the parser, but is more efficient this way */
			if(line[0] != '#' &&
			   line[0] != '\r' &&
			   line[0] != '\n' &&
			   line[0] != '\0')
			{
				configString += line;
				configString += '\n';
			}
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

const SpotifyConfig& ConfigHandler::getSpotifyConfig() const
{
    return spotifyConfig_;
}

} /* namespace ConfigHandling */
