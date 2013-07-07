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

#ifndef LOGGER_H_
#define LOGGER_H_

#include "LogLevels.h"
#include <fstream>
#include <iostream>
#include <sstream>

namespace Logger
{

class Logger
{
private:
    LogLevel level_;

    friend class LoggerStreamBuffer;
    virtual void flush(std::stringstream* buff) = 0;

    /* Make non-copyable */
    Logger();
    Logger(const Logger&);
    Logger& operator=(const Logger&);

protected:
    Logger(LogLevel level);
public:
    virtual ~Logger();
    LogLevel getConfiguredLogLevel() const;

    virtual void logAppend(LogLevel level, const char* functionName, const char* log) = 0;
    class LoggerStreamBuffer* logAppend(LogLevel level, const char* functionName);
    void operator<<=(class LoggerStreamBuffer& buff);
};

class LoggerStreamBuffer
{
private:
    Logger& log_;
    void flush();

public:
    std::stringstream* ss_; /*todo: hide me! private doesn't compile on windows*/
    friend class Logger;
    template<class T> friend LoggerStreamBuffer& operator<<(LoggerStreamBuffer& buff, const T& rhs);

    LoggerStreamBuffer(Logger& logger);
    virtual ~LoggerStreamBuffer();
};

template<class T>
LoggerStreamBuffer& operator<<(LoggerStreamBuffer& buff, const T& rhs)
{
    *buff.ss_ << rhs;
    return buff;
}

std::string logprefix( LogLevel level, const char* functionName );

extern "C"
{
void LogAppendCBinder(LogLevel level, const char* functionName, const char* format, ...);
LogLevel getConfiguredLogLevelCBinder();
}

}
#endif
