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

#include "Logger.h"
#include "ConfigHandling/ConfigHandler.h"
#include <ostream>
#include <stdarg.h>
#include <iostream>
#include <assert.h>
#include <time.h>

Logger::Logger* logger = NULL;

namespace Logger
{
static const char* level_strings[] =

{
    "EMERG",
    "WARN",
    "NOTICE",
    "DEBUG"
};
#ifdef _WIN32
#define localtime_r(a, b) localtime_s(b, a) /*goddammit microsoft!*/
#endif

std::string logprefix( LogLevel level, const char* functionName )
{
    time_t rawtime;
    struct tm timeinfo;
    std::stringstream str;
    char tmp[10] = {0};

    time (&rawtime);
    localtime_r (&rawtime, &timeinfo);
    strftime( tmp, sizeof(tmp), "%H:%M:%S", &timeinfo );

    str << tmp << " [" << level_strings[level] << "] " << functionName << ": ";
    return str.str();
}

Logger::Logger(LogLevel level) : level_(level)
{
    /* there should only exist 1 instance of logger! */
    assert(::logger == NULL);
    ::logger = this;
}
Logger::~Logger(){ }


LoggerStreamBuffer* Logger::logAppend(LogLevel level, const char* functionName)
{
    LoggerStreamBuffer* buff = new LoggerStreamBuffer(*this);
    *buff << logprefix(level, functionName);
    return buff;
}


LogLevel Logger::getConfiguredLogLevel() const
{
    return level_;
}

void Logger::operator<<=(LoggerStreamBuffer& buff) { buff.flush(); }


void LogAppendCBinder(LogLevel level, const char* functionName, const char* format, ...)
{
    char buffer[1000];
    va_list vl;
    va_start(vl,format);
    vsprintf(buffer, format, vl);
    va_end(vl);
    logger->logAppend(level, functionName, buffer);
}
LogLevel getConfiguredLogLevelCBinder()
{
    return logger->getConfiguredLogLevel();
}

/************************
 * LoggerStreamBuffer
 ************************/
LoggerStreamBuffer::LoggerStreamBuffer(Logger& logger) : log_(logger), ss_(new std::stringstream) { }
LoggerStreamBuffer::~LoggerStreamBuffer() { delete ss_; }
void LoggerStreamBuffer::flush()
{
    log_.flush(ss_);
    delete ss_;
    ss_ = NULL;
    delete this;
};

}
