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
                                 settingsLocation_("./tmp/")
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

void SpotifyConfig::setCacheLocation(std::string& cacheLocation)
{
    if(!cacheLocation.empty())cacheLocation_ = cacheLocation;
}

void SpotifyConfig::setPassword(std::string& password)
{
    if(!password.empty())password_ = password;
}

void SpotifyConfig::setSettingsLocation(std::string& settingsLocation)
{
    if(!settingsLocation.empty())settingsLocation_ = settingsLocation;
}

void SpotifyConfig::setUsername(std::string& username)
{
    if(!username.empty())username_ = username;
}
} /* namespace ConfigHandling */
