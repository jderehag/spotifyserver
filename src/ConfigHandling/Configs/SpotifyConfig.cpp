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

#include "../ConfigHandler.h"

namespace ConfigHandling
{

SpotifyConfig::SpotifyConfig() : username_(""),
                                 password_(""),
                                 cacheLocation_("./tmp/"),
                                 settingsLocation_("./tmp/"),
                                 rememberMe_(false),
                                 repeat_(false),
                                 shuffle_(false)
{ }

const std::string& SpotifyConfig::getCacheLocation() const
{
    return cacheLocation_;
}

const std::string& SpotifyConfig::getPassword() const
{
    return password_;
}

const std::string& SpotifyConfig::getSettingsLocation() const
{
    return settingsLocation_;
}

const std::string& SpotifyConfig::getUsername() const
{
    return username_;
}

bool SpotifyConfig::getRememberMe() const
{
    return rememberMe_;
}

const std::string SpotifyConfig::getRememberMeString() const
{
    return rememberMe_ ? "1" : "0";
}

bool SpotifyConfig::getRepeat() const
{
    return repeat_;
}

const std::string SpotifyConfig::getRepeatString() const
{
    return repeat_ ? "1" : "0";
}

bool SpotifyConfig::getShuffle() const
{
    return shuffle_;
}

const std::string SpotifyConfig::getShuffleString() const
{
    return shuffle_ ? "1" : "0";
}

void SpotifyConfig::setCacheLocation(std::string& cacheLocation)
{
    if(!cacheLocation.empty())cacheLocation_ = cacheLocation;
}

void SpotifyConfig::setPassword(std::string& password)
{
    password_ = password;
    if(writer) writer->writeConfigFile();
}

void SpotifyConfig::setSettingsLocation(std::string& settingsLocation)
{
    if(!settingsLocation.empty())settingsLocation_ = settingsLocation;
}

void SpotifyConfig::setUsername(std::string& username)
{
    username_ = username;
    if(writer) writer->writeConfigFile();
}

void SpotifyConfig::setRememberMe(bool rememberMe)
{
    rememberMe_ = rememberMe;
    if(writer) writer->writeConfigFile();
}

void SpotifyConfig::setRememberMe(std::string& rememberMe)
{
    if(!rememberMe.empty())
        rememberMe_ = rememberMe.compare("0") != 0 && rememberMe.compare("FALSE") != 0 && rememberMe.compare("false") != 0;
}

void SpotifyConfig::setRepeat(bool repeat)
{
    repeat_ = repeat;
    if(writer) writer->writeConfigFile();
}

void SpotifyConfig::setRepeat(std::string& repeat)
{
    if(!repeat.empty())
        repeat_ = repeat.compare("0") != 0 && repeat.compare("FALSE") != 0 && repeat.compare("false") != 0;
}

void SpotifyConfig::setShuffle(bool shuffle)
{
    shuffle_ = shuffle;
    if(writer) writer->writeConfigFile();
}

void SpotifyConfig::setShuffle(std::string& shuffle)
{
    if(!shuffle.empty())
        shuffle_ = shuffle.compare("0") != 0 && shuffle.compare("FALSE") != 0 && shuffle.compare("false") != 0;
}
} /* namespace ConfigHandling */
