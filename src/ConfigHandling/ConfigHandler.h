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

#ifndef CONFIGHANDLER_H_
#define CONFIGHANDLER_H_

#include "Logger/LogLevels.h"
#include "ConfigDefs.h"
#include <string>
#include <list>

namespace ConfigHandling
{
class IConfigWriter
{
public:
    virtual void writeConfigFile() = 0;
};

class ConfigBase
{
protected:
    IConfigWriter* writer;
public:
    ConfigBase() : writer(NULL) {}
    void setWriteIf( IConfigWriter* writer_ ) { writer = writer_; }
};

class GeneralConfig : public ConfigBase
{
public:
    GeneralConfig() : id_("Server") {}
    const std::string& getId() const { return id_; }
    void setId(const std::string& id) { if ( !id.empty() ) { id_ = id; if(writer) writer->writeConfigFile(); } }

private:
    std::string id_;
};

class AudioEndpointConfig
{
public:
    typedef enum
    {
        ALSA
    }EndpointType;

    AudioEndpointConfig();
    const std::string& getDevice() const;
    EndpointType getEndpointType() const;
    const std::string getEndpointTypeString() const;
    void setDevice(const std::string& device);
    void setEndpointType(const std::string& endpointType);

private:
    EndpointType endpointType_;
    std::string device_;
};

class LoggerConfig
{
public:
    typedef enum
    {
        FILE,
        STDOUT,
        NOWHERE,
    }LogTo;

    LoggerConfig();
    const std::string& getLogFile() const;
    LogLevel getLogLevel() const;
    const std::string getLogLevelString() const;
    LogTo getLogTo() const;
    void setLogFile(const std::string& logFile);
    void setLogLevel(const std::string& logLevel);
    void setLogTo(LogTo logTo);

private:
    LogLevel logLevel_;
    std::string logFile_;
    LogTo logTo_;
};


class NetworkConfig
{
public:
    typedef enum
    {
        IP,
        DEVICE
    }BindType;

    NetworkConfig();

    BindType getBindType() const;
    const std::string getBindTypeString() const;
    const std::string& getDevice() const;
    const std::string& getIp() const;
    const std::string& getPort() const;
    const std::string& getUsername() const;
    const std::string& getPassword() const;
    void setBindType(const std::string& bindType);
    void setDevice(const std::string& device);
    void setIp(const std::string& ip);
    void setPort(const std::string& port);
    void setUsername(std::string& username);
    void setPassword(std::string& password);
private:
    BindType bindType_;
    // IP is kept as string for now since ip representation is different on different platforms
    std::string ip_;
    std::string device_;
    std::string port_;

    /*login stuff on client side*/
    std::string username_;
    std::string password_;
};


class SpotifyConfig
{
private:
    std::string username_;
    std::string password_;
    std::string cacheLocation_;
    std::string settingsLocation_;
public:
    SpotifyConfig();
    const std::string& getCacheLocation() const;
    const std::string& getPassword() const;
    const std::string& getSettingsLocation() const;
    const std::string& getUsername() const;
    void setCacheLocation(std::string& cacheLocation);
    void setPassword(std::string& password);
    void setSettingsLocation(std::string& settingsLocation);
    void setUsername(std::string& username);
};

class ConfigHandler : IConfigWriter
{
private:
    GeneralConfig generalConfig_;
    SpotifyConfig spotifyConfig_;
    NetworkConfig networkConfig_;
    AudioEndpointConfig audioEndpointConfig_;
    LoggerConfig loggerConfig_;

    std::string configFilePath_;
    std::list<std::string> config;
    void readFromFile();
    std::list<SectionAttributes> getConfigAttributes();

    /* General Section */
    std::string applicationId;
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
    std::string networkUsername;
    std::string networkPassword;
    /* AudioEndpoint Section*/
    std::string audioEndpointType;
    std::string audioEndpointAlsaDevice;
    /* Logger Section */
    std::string loggerLogLevel;
    std::string loggerLogFile;
public:
	ConfigHandler(const std::string& pathToConfigFile);
	virtual ~ConfigHandler();
	void parseConfigFile();
    void writeConfigFile();
    GeneralConfig& getGeneralConfig();
    const AudioEndpointConfig& getAudioEndpointConfig() const;
    const LoggerConfig& getLoggerConfig() const;
    const NetworkConfig& getNetworkConfig() const;
    const SpotifyConfig& getSpotifyConfig() const;
    const std::string& getConfigFilePath() const;
};

} /* namespace ConfigHandling */
#endif /* CONFIGHANDLER_H_ */
