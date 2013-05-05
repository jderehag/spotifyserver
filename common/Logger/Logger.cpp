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
#include <stdio.h>
#include <stdarg.h>
#include <iostream>
#include <stdlib.h>
#include <assert.h>

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

Logger::Logger(const ConfigHandling::LoggerConfig& config) : config_(config)
{
    /* there should only exist 1 instance of logger! */
    assert(::logger == NULL);
    ::logger = this;
}
Logger::~Logger(){ }

void Logger::logAppend(LogLevel level, const char* functionName, const char* log)
{
    mtx_.lock();
    switch (config_.getLogTo())
    {
        case ConfigHandling::LoggerConfig::FILE:
        {
            std::filebuf fb;
            std::ostream os(&fb);
            if(fb.open(config_.getLogFile().c_str(), std::ios::out|std::ios::app) != NULL)
            {
                os << "[" << level_strings[level] << "] " << functionName << ": " << log << std::endl;
                fb.close();
            }
            else
            {
                std::cerr << "Unable to open output logfile " << config_.getLogFile();
                fb.close();
                exit(-1);
            }
        }
        break;

        case ConfigHandling::LoggerConfig::STDOUT:
        {
            std::cout << "[" << level_strings[level] << "] " << functionName << ": " << log << std::endl;
        }
        break;

        default:
            break;
    }
    mtx_.unlock();
}

LoggerStreamBuffer* Logger::logAppend(LogLevel level, const char* functionName)
{
    LoggerStreamBuffer* buff = new LoggerStreamBuffer(*this);
    *buff << "[" << level_strings[level] << "] " << functionName << ": ";
    return buff;
}
void Logger::flush(std::stringstream* buff)
{
    mtx_.lock();
    switch (config_.getLogTo())
    {
        case ConfigHandling::LoggerConfig::FILE:
        {
            std::fstream fstream(config_.getLogFile().c_str(), std::ios::out|std::ios::app);
            fstream << buff->str() << std::endl;
            fstream.close();
        }
        break;

        case ConfigHandling::LoggerConfig::STDOUT:
        {
            std::cout << buff->str() << std::endl;
        }
        break;

        default:
            break;
    }
    mtx_.unlock();
}

LogLevel Logger::getConfiguredLogLevel() const
{
    return config_.getLogLevel();
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
