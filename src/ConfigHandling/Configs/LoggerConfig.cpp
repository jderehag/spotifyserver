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

namespace ConfigHandling
{

LoggerConfig::LoggerConfig() : logLevel_(LOG_DEBUG),
                               logFile_("./output.log"),
                               logTo_(FILE)
{ }

const std::string& LoggerConfig::getLogFile() const
{
    return logFile_;
}

LogLevel LoggerConfig::getLogLevel() const
{
    return logLevel_;
}

const std::string LoggerConfig::getLogLevelString() const
{
    switch( logLevel_ )
    {
    case LOG_DEBUG: return "DEBUG";
    case LOG_NOTICE: return "NOTICE";
    case LOG_WARN: return "WARN";
    case LOG_EMERG: return "EMERG";
    }
    return "";
}

LoggerConfig::LogTo LoggerConfig::getLogTo() const
{
    return logTo_;
}

void LoggerConfig::setLogFile(const std::string& logFile)
{
    if(!logFile.empty())logFile_ = logFile;
}

void LoggerConfig::setLogLevel(const std::string& logLevel)
{
    if(!logLevel.empty())
    {
        if(logLevel == "EMERG")logLevel_ = LOG_EMERG;
        else if(logLevel == "WARN")logLevel_ = LOG_WARN;
        else if(logLevel == "NOTICE")logLevel_ = LOG_NOTICE;
        else if(logLevel == "DEBUG")logLevel_ = LOG_DEBUG;
        else
        {
            std::cerr << "Unknown LogLevel type: " << logLevel << std::endl;
            exit(-1);
        }
    }
}

void LoggerConfig::setLogTo(LogTo logTo)
{
    logTo_ = logTo;
}

} /* namespace ConfigHandling */
