/*
 * Copyright (c) 2012, Jens Nielsen
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


#include "UIEmbedded.h"
#include "applog.h"
#include "stm32f4_discovery.h"
#include "stm32f4_discovery_lcd.h"
#include <string.h>
#include <sstream>

static void progressTimerCb( xTimerHandle tmr )
{
    UIEmbedded* ui = (UIEmbedded*) pvTimerGetTimerID( tmr );

    ui->progressUpdateTick();
}

UIEmbedded::UIEmbedded( MediaInterface& m ) : m_(m), playbackState(PLAYBACK_IDLE), progress_(0), currentTrackDuration_(0)
{
    m_.registerForCallbacks( *this );
    itPlaylists_ = playlists.begin();

    progressTimer = xTimerCreate("prgTmr", 1000/portTICK_RATE_MS, pdTRUE, this, progressTimerCb );
    drawDefault();
}

UIEmbedded::~UIEmbedded()
{
    m_.unRegisterForCallbacks( *this );
}

void UIEmbedded::shortButtonPress()
{
    /* TODO: do a context switch before going to MediaInterface, this function is called from
     * ISR and the MessageBox implementation assumes non-ISR */
    switch( playbackState )
    {
        case PLAYBACK_IDLE:
            if( itPlaylists_ != playlists.end() )
                m_.play( (*itPlaylists_).getLink(), this, NULL ); // -> PLAYING
            break;

        case PLAYBACK_PAUSED:
            m_.resume(); // -> PLAYING
            break;

        case PLAYBACK_PLAYING:
            m_.pause(); // -> PAUSED
            break;
    }
}
void UIEmbedded::longButtonPress()
{
    /* TODO: do a context switch before going to MediaInterface, this function is called from
     * ISR and the MessageBox implementation assumes non-ISR */
    switch( playbackState )
    {
        case PLAYBACK_IDLE:
        case PLAYBACK_PAUSED:
            if( !playlists.empty() )
            {
                itPlaylists_++;

                if( itPlaylists_ == playlists.end())
                    itPlaylists_ = playlists.begin();

                m_.play( (*itPlaylists_).getLink(), this, NULL ); // -> PLAYING
            }
            break;

        case PLAYBACK_PLAYING:
            m_.next(); // -> PLAYING
            break;
    }
}

void UIEmbedded::rootFolderUpdatedInd()
{
}

void UIEmbedded::connectionState( bool up )
{
    if ( up )
    {
        /*new connection, check status and get playlists*/
        m_.getStatus( this, NULL );
        //m_.getPlaylists( this, NULL);

        /*make sure we shuffle (should be controlled by button though..)*/
        m_.setShuffle(true);

        //addAudio();
#ifndef WITH_LCD
        STM_EVAL_LEDOn( LED4 );
    }
    else
    {
        STM_EVAL_LEDOff( LED4 );
#endif
    }
}

void UIEmbedded::getPlaylistsResponse( const Folder& rootfolder, void* userData )
{
    playlists.clear();

    for( LibSpotify::FolderContainer::const_iterator it = rootfolder.getFolders().begin(); it != rootfolder.getFolders().end() ; it++)
        playlists.insert( playlists.end(), (*it).getPlaylists().begin(), (*it).getPlaylists().end());

    playlists.insert( playlists.end(), rootfolder.getPlaylists().begin(), rootfolder.getPlaylists().end());

    itPlaylists_ = playlists.begin();
}

void UIEmbedded::getTracksResponse( const std::deque<Track>& tracks, void* userData )
{}
void UIEmbedded::getImageResponse( const void* data, size_t dataSize, void* userData )
{}
void UIEmbedded::getAlbumResponse( const Album& album, void* userData )
{}
void UIEmbedded::genericSearchCallback( const std::deque<Track>& listOfTracks, const std::string& didYouMean, void* userData )
{}

#define TEXT_COL White
#define BACK_COL ASSEMBLE_RGB(55,55,55) //ASSEMBLE_RGB(71,71,71)
#define PROGBAR_COL White
#define FONT Font8x8
#define PROGBAR_WIDTH (LCD_PIXEL_WIDTH - (2 * ( 4 + 5*8 + 4 )))
#define PROGBAR_POS (LCD_PIXEL_HEIGHT - 12)

static void printText( uint16_t ypos, uint16_t xpos, const char* text, sFONT* font )
{
    LCD_DisplayStringLineCol( ypos, xpos, text, TEXT_COL, BACK_COL, font );
}

void UIEmbedded::drawProgress()
{
    uint16_t progbarfill = currentTrackDuration_ ? (PROGBAR_WIDTH * progress_) / currentTrackDuration_ : 0;
    LCD_DrawFullRect( 4 + 5*8 + 4, PROGBAR_POS, PROGBAR_WIDTH , 10, PROGBAR_COL, ASSEMBLE_RGB(20,20,20));
    LCD_DrawFullRect( 4 + 5*8 + 4, PROGBAR_POS, progbarfill, 10, PROGBAR_COL, PROGBAR_COL );
    //LCD_DrawFullCircle( 4 + 5*8 + 4 + progbarfill, PROGBAR_POS + 5, 5, White );

    {
        std::stringstream out;
        out << (progress_/60 < 10 ? " " : "") << progress_/60 <<":" << (progress_%60 < 10 ? "0" : "") << progress_%60;
        printText(PROGBAR_POS, 4, out.str().c_str(), &FONT);
    }

    {
        std::stringstream out;
        out << currentTrackDuration_/60 <<":" << (currentTrackDuration_%60 < 10 ? "0" : "") << currentTrackDuration_%60;// << " "; // last space is just to clear any remaining character from screen
        printText(PROGBAR_POS, LCD_PIXEL_WIDTH - 4 - 5*8, out.str().c_str(), &FONT);
    }
}

void UIEmbedded::drawDefault()
{
    uint16_t ypos = 24*8;
    LCD_DrawLine( 0, ypos, LCD_PIXEL_WIDTH, LCD_DIR_HORIZONTAL, White );
    ypos++;
    LCD_DrawFullRect( 0, ypos, LCD_PIXEL_WIDTH , LCD_PIXEL_HEIGHT - 24*8, BACK_COL, BACK_COL);

    drawProgress();
}

void UIEmbedded::progressUpdateTick()
{
    if ( playbackState == PLAYBACK_PLAYING )
    {
        /* avoid drawing outside box if we lose connection or miss transition to stopped */
        if ( progress_ < currentTrackDuration_ )
        {
            progress_++;
            drawProgress();
        }
    }
}

void UIEmbedded::statusUpdateInd( PlaybackState_t state, bool repeatStatus, bool shuffleStatus, const Track& currentTrack, unsigned int progress )
{
    playbackState = state;
    progress_ = progress/1000;
    currentTrackDuration_ = currentTrack.getDurationMillisecs()/1000;

    if ( playbackState == PLAYBACK_PLAYING )
    {
        xTimerStart( progressTimer, 0 );
        /* print to log when new track starts to get some history, cheap but history isn't implemented.. */
        if ( progress == 0 )
            log( LOG_NOTICE ) << "Playing " << (*(currentTrack.getArtists().begin())).getName() << " - " << currentTrack.getName();
    }
    else
    {
        xTimerStop( progressTimer, 0 );
    }
#ifdef WITH_LCD
    drawDefault();

    unsigned int ypos = 24*8;
    ypos += 3;

    printText( ypos, 4, currentTrack.getName().c_str(), &Font12x12 );
    ypos += 13;

    std::stringstream out;
    for ( std::vector<Artist>::const_iterator it = currentTrack.getArtists().begin();
            it != currentTrack.getArtists().end(); it++ )
    {
        out << (*it).getName();
        if ( it+1 != currentTrack.getArtists().end() )
            out << ", ";
    }

    printText(ypos, 4, out.str().c_str(), &FONT);
    ypos += 9;

    printText( ypos, 4, currentTrack.getAlbum().c_str(), &FONT );
#endif

}

void UIEmbedded::statusUpdateInd( PlaybackState_t state, bool repeatStatus, bool shuffleStatus )
{
    playbackState = state;
    progress_ = 0;
    currentTrackDuration_ = 0;
#ifdef WITH_LCD
    drawDefault();
#endif
    if ( playbackState == PLAYBACK_PLAYING )
    {
        xTimerStart( progressTimer, 0 );
    }
    else
    {
        xTimerStop( progressTimer, 0 );
    }
}

void UIEmbedded::getStatusResponse( PlaybackState_t state, bool repeatStatus, bool shuffleStatus, const Track& currentTrack, unsigned int progress, void* userData )
{
    statusUpdateInd( state, repeatStatus, shuffleStatus, currentTrack, progress );
}
void UIEmbedded::getStatusResponse( PlaybackState_t state, bool repeatStatus, bool shuffleStatus, void* userData )
{
    statusUpdateInd( state, repeatStatus, shuffleStatus );
}



