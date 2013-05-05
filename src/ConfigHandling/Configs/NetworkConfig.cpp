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

#include <stdlib.h>
#include <iostream>
#include <sstream>

namespace ConfigHandling
{
NetworkConfig::NetworkConfig() : bindType_(NetworkConfig::IP),
                                 ip_("ANY"),
                                 device_(""),
                                 port_("7788")
{ }

NetworkConfig::BindType NetworkConfig::getBindType() const
{
    return bindType_;
}

const std::string& NetworkConfig::getDevice() const
{
    return device_;
}

const std::string& NetworkConfig::getIp() const
{
    return ip_;
}

const std::string& NetworkConfig::getPort() const
{
    return port_;
}

const std::string& NetworkConfig::getUsername() const
{
    return username_;
}

const std::string& NetworkConfig::getPassword() const
{
    return password_;
}

void NetworkConfig::setBindType(const std::string& bindType)
{
    if(!bindType.empty())
    {
        if(bindType == "IP")bindType_ = NetworkConfig::IP;
        else if(bindType == "DEVICE")bindType_ = NetworkConfig::DEVICE;
        else
        {
            std::cerr << "Unknown Network config BindType: " << bindType << std::endl;
            exit(-1);
        }
    }
}

void NetworkConfig::setDevice(const std::string& device)
{
    if(!device.empty())device_ = device;
}

void NetworkConfig::setIp(const std::string& ip)
{
    if(!ip.empty())ip_ = ip;
}

void NetworkConfig::setPort(const std::string& port)
{
    if(!port.empty())port_ = port;
}

void NetworkConfig::setUsername(std::string& username)
{
    if(!username.empty())username_ = username;
}

void NetworkConfig::setPassword(std::string& password)
{
    if(!password.empty())password_ = password;
}

}/* namespace ConfigHandling */
