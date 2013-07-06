/*
 * Copyright (c) 2013, Jens Nielsen
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
 * DISCLAIMED. IN NO EVENT SHALL JENS NIELSEN BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include "LcdLog.h"
#include "stm32f4_discovery_lcd.h"
#include <string.h>
#include <assert.h>


Logger::LcdLog* lcdLog = NULL;

namespace Logger
{

/* todo move this into class withoug exposing stm32f4_discovery_lcd.h */
#define MAX_WIDTH ( LCD_HORIZONTAL_MAX / 8 )
#define MAX_HEIGHT ( LCD_VERTICAL_MAX / 8 )
static char buffer[MAX_HEIGHT][MAX_WIDTH+1];
static int head = 0;


LcdLog::LcdLog( unsigned int firstLine, unsigned int firstColumn, unsigned int nLines, unsigned int width ) : //buffer_(nLines),
                                                                                                              firstLine_(firstLine),
                                                                                                              firstColumn_(firstColumn),
                                                                                                              nLines_(nLines),
                                                                                                              width_(width)
{
    /* provide global singleton to allow printing to screen without log prefix */
    assert( ::lcdLog == NULL );
    ::lcdLog = this;

    if ( width_ > MAX_WIDTH ) width_ = MAX_WIDTH;
    if ( nLines_ > MAX_HEIGHT ) nLines_ = MAX_HEIGHT;

    STM32f4_Discovery_LCD_Init();
    LCD_SetBackColor(Black);
    LCD_SetTextColor(White);
    LCD_SetFont( &Font8x8 );
}

LcdLog::~LcdLog()
{
}

void LcdLog::addLine( const std::string& line )
{
    size_t totlen = line.length();
    bufferMtx_.lock();
    for ( size_t pos = 0; pos < totlen ; )
    {
        size_t left = totlen - pos;
        size_t nextNewline = line.find('\n', pos) - pos;

        /* every line is padded with spaces to clear previous line when screen scrolls */
        memset( buffer[head], ' ', width_ );
        buffer[head][width_] = '\0';

        if ( nextNewline < left && nextNewline < width_ )
        {
            /* there's a newline in this line, take everything up until it and then skip past it */
            //buffer_.push_back( line.substr(pos, nextNewline ) );
            memcpy( buffer[head], &line[pos], nextNewline );
            pos += nextNewline + 1;
        }
        else
        {
            /* no newline, take full width or as much as we can get */
            size_t len = (left < width_) ? left : width_;
            //buffer_.push_back( line.substr(pos, len ) );
            memcpy( buffer[head], &line[pos], len );
            pos += len;
        }
        head = (head+1) % nLines_;
    }
    bufferMtx_.unlock();

    /* always update display for now, some day we may check if log is visible or something */
    updateDisplay();
}

void LcdLog::updateDisplay()
{
    unsigned int i = firstLine_;
    bufferMtx_.lock();
    //for( Util::CircularQueue<std::string>::const_iterator it = buffer_.begin(); it != buffer_.end() ; it++, i++ )
    for( unsigned int j = 0; j < nLines_; j++, i++ )
    {
        //LCD_ClearLine( LINE(i) );
        //LCD_DisplayStringLine( LINE(i), (*it).c_str() );
        LCD_DisplayStringLine( LINE(i), buffer[(head+j)%nLines_] );
    }
    bufferMtx_.unlock();
}

} /* namespace Logger */
