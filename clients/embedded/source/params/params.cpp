/*
 * Copyright (c) 2014, Jens Nielsen
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


#include "params.h"
#include "Platform/Threads/Mutex.h"
#include "stm32f4xx_hal_flash.h"
#include "applog.h"
#include <assert.h>
#include <string.h>

#define PARAMS_VALID 0xa5
#define PARAMS_VERSION 0

typedef struct
{
    uint8_t valid;
    uint8_t version;
    uint8_t pageHeaderSize;
    uint8_t entryHeaderSize;
    uint8_t seqNo;
    uint8_t pad[3];
} ParamsHeader_t;

typedef struct
{
    uint8_t valid;
    uint8_t id;
    uint16_t size;
} ParamsEntryHeader_t;

static uint8_t currentPage = 0;
static uint8_t sectors[] = { FLASH_SECTOR_1, FLASH_SECTOR_2 };
static uint32_t sectorStart[] = { 0x08004000, 0x08008000 };
static Platform::Mutex mtx;

static const ParamsHeader_t* getHeader( uint8_t page )
{
    return (ParamsHeader_t*) sectorStart[ page ];
}

static const ParamsEntryHeader_t* getFirstEntry( uint8_t page )
{
    const ParamsHeader_t* pageHeader = getHeader( page );
    return (ParamsEntryHeader_t*) ((uint8_t*)pageHeader + pageHeader->pageHeaderSize);
}

static const ParamsEntryHeader_t* getNextEntry( const ParamsEntryHeader_t* entry )
{
    return (ParamsEntryHeader_t*)(((uint8_t*) entry) + entry->size);
}

/* erase entire page, assumes flash is unlocked */
static bool erasePage( uint8_t page )
{
    uint32_t errorCode;
    FLASH_EraseInitTypeDef flashErase;
    flashErase.TypeErase = TYPEERASE_SECTORS;
    flashErase.NbSectors = 1;
    flashErase.Sector = sectors[page];
    flashErase.VoltageRange = VOLTAGE_RANGE_3; //or 4?
    HAL_FLASH_Unlock();
    return ( HAL_FLASHEx_Erase( &flashErase, &errorCode ) == HAL_OK );
}

/* write a chunk of data to flash, assumes area is erased and flash is unlocked already */
static void writeFlash( uint32_t addr, const uint8_t* data, uint16_t length )
{
    int i = 0;
    assert( length % 4 == 0 );
    for ( ; i < length; i+=4 )
    {
        uint32_t curData;
        memcpy( &curData, &data[i], 4 );
        HAL_FLASH_Program( TYPEPROGRAM_WORD, addr+i, curData );
    }
}

/* writes a new page header */
static void writeHeader( uint8_t page, uint8_t seqNo )
{
    ParamsHeader_t hdr;
    hdr.seqNo = seqNo;
    hdr.pageHeaderSize = sizeof(ParamsHeader_t);
    hdr.entryHeaderSize = sizeof(ParamsEntryHeader_t);
    hdr.version = PARAMS_VERSION;
    hdr.valid = PARAMS_VALID;
    writeFlash( sectorStart[page], (uint8_t*)&hdr, sizeof(ParamsHeader_t) );
}

/* makes a copy of all parameters in one page to the other page */
static void mirrorPage( uint8_t fromPage )
{
    uint8_t writePage = fromPage ^ 1;
    assert ( erasePage( writePage ) == true );

    const ParamsHeader_t* fromHdr = getHeader( fromPage );

    uint32_t addr = sectorStart[ writePage ] + sizeof(ParamsHeader_t);

    // write all parameters from 'from page' to new page, and make sure it's current version format on headers
    const ParamsEntryHeader_t* entry = getFirstEntry( fromPage );
    while( entry->valid == PARAMS_VALID )
    {
        const uint8_t* data = (const uint8_t*) entry + fromHdr->entryHeaderSize;
        uint16_t dataSize = entry->size - fromHdr->entryHeaderSize;

        // write header
        ParamsEntryHeader_t newEntry;
        newEntry.id = entry->id;
        newEntry.size = sizeof(ParamsEntryHeader_t) + dataSize;
        newEntry.valid = PARAMS_VALID;
        writeFlash( addr, (uint8_t*)&newEntry, sizeof(ParamsEntryHeader_t) );
        addr += sizeof(ParamsEntryHeader_t);

        // write data
        writeFlash( addr, data, dataSize );
        addr += dataSize;

        entry = getNextEntry( entry );
    }

    /* now it's safe to write the page header */
    writeHeader( writePage, fromHdr->seqNo + 1 );
}

// just for test
/*
void wastePrimaryPage()
{
    erasePage(currentPage);
}
*/



void paramsInit()
{
    const ParamsHeader_t* header1 = getHeader( 0 );
    const ParamsHeader_t* header2 = getHeader( 1 );

    //sanity checks
    assert( sizeof(ParamsHeader_t) % 4 == 0 );
    assert( sizeof(ParamsEntryHeader_t) % 4 == 0 );

    /* check which params page to use */
    if( header1->valid == PARAMS_VALID && header2->valid == PARAMS_VALID )
    {
        /* both pages valid, use one with best seq num */
        int8_t d = header1->seqNo - header2->seqNo;
        if ( d > 0 )
        {
            currentPage = 0;
        }
        else
        {
            currentPage = 1;
        }
    }
    else if( header1->valid == PARAMS_VALID )
    {
        currentPage = 0;
    }
    else if( header2->valid == PARAMS_VALID )
    {
        currentPage = 1;
    }
    else
    {
        /* neither page is valid, just reset one */
        currentPage = 0;

        HAL_FLASH_Unlock();

        assert( erasePage( currentPage ) == true );
        writeHeader( currentPage, 0 );

        HAL_FLASH_Lock();
    }

    /* handle downgrades/upgrades */
    if ( getHeader( currentPage )->version != PARAMS_VERSION )
    {
        HAL_FLASH_Unlock();
        mirrorPage( currentPage );
        currentPage = currentPage ^ 1;
        HAL_FLASH_Lock();
    }

#if 0 // this is maybe not necessary...
    /* check if backup page is valid */
    if ( getHeader( currentPage^1 )->valid != PARAMS_VALID || getHeader( currentPage^1 )->version != PARAMS_VERSION )
    {
        /* backup page is incorrect, mirror current page */
        HAL_FLASH_Unlock();
        mirrorPage( currentPage );
        currentPage = currentPage ^ 1;
        HAL_FLASH_Lock();
    }
#endif
}

bool paramsGet( ParamId paramId, std::string& val )
{
    bool ret = false;
    mtx.lock();

#if 0 // maybe this is overkill, needs to be tested anyway
    const ParamsHeader_t* currentHdr = getHeader( currentPage );
    /* first make sure we're reading valid data */
    if ( currentHdr->valid != PARAMS_VALID )
    {
        /* this is bad! swap to backup page instead */
        log(LOG_EMERG) << "Current params flash page is corrupt!";
        currentPage = currentPage ^ 1;
        currentHdr = getHeader( currentPage );
        if ( currentHdr->valid != PARAMS_VALID )
        {
            log(LOG_EMERG) << "Backup params flash page is also corrupt!";
            mtx.unlock();
            return false;
        }
    }
#endif
    const ParamsEntryHeader_t* entry = getFirstEntry( currentPage );

    /* iterate params and look for matching id */
    while( entry->valid == PARAMS_VALID )
    {
        if ( entry->id == paramId )
        {
            /* found correct param, extract data */
            const char* data = (const char*)(entry+1);
            val = std::string(data);
            if ( val.length() < entry->size - sizeof(ParamsEntryHeader_t))
                ret = true;
            break;
        }
        entry = getNextEntry( entry );
    }
    mtx.unlock();
    return ret;
}

bool paramsSet( ParamId paramId, const std::string& val )
{
    /* don't write anything if new value is identical to current value */
    std::string currentVal;
    if ( paramsGet( paramId, currentVal ) && currentVal.compare( val ) == 0 )
        return true;

    uint16_t length = val.length() + 1; // string length plus null
    if ( length % 4 != 0 ) // make sure params are 4-byte aligned
        length += (4 - (length%4) );

    return paramsSet( paramId, (const uint8_t*)val.c_str(), length);
}

bool paramsSet( ParamId paramId, const uint8_t* data, const uint16_t length )
{
    bool success = false;
    mtx.lock();
    uint8_t writePage = currentPage ^ 1; // we'll write to the page that is currently inactive
    const ParamsHeader_t* currentHdr = getHeader( currentPage );
    uint8_t newSeqNo = currentHdr->seqNo + 1;

#if 0 // maybe this is overkill, needs to be tested anyway
    /* try to make sure we have valid data somewhere */
    if ( currentHdr->valid != PARAMS_VALID )
    {
        /* this is bad! swap to backup page and write to current instead */
        log(LOG_EMERG) << "Current params flash page is corrupt!";
        currentHdr = getHeader( writePage );
        writePage = currentPage;
        newSeqNo = 0;
    }
#endif
    // begin with erasing the sector
    HAL_FLASH_Unlock();
    if ( erasePage( writePage ) )
    {
        uint32_t addr = sectorStart[ writePage ] + sizeof(ParamsHeader_t);

        /* write new parameter at the beginning of entry area */
        // first entry header
        ParamsEntryHeader_t newEntry;
        newEntry.id = paramId;
        newEntry.size = sizeof(ParamsEntryHeader_t) + length;
        newEntry.valid = PARAMS_VALID;
        writeFlash( addr, (uint8_t*)&newEntry, sizeof(ParamsEntryHeader_t) );
        addr += sizeof(ParamsEntryHeader_t);

        // then data
        writeFlash( addr, data, length );
        addr += length;

        /* and then copy all the other parameters from current page to new page */
        if ( currentHdr->valid == PARAMS_VALID )
        {
            const ParamsEntryHeader_t* entry = getFirstEntry( currentPage );
            while( entry->valid == PARAMS_VALID )
            {
                if ( entry->id != paramId )
                {
                    writeFlash( addr, (uint8_t*)entry, entry->size );
                    addr += entry->size;
                }
                entry = getNextEntry( entry );
            }
        }

        /* and finally we can write the page header */
        writeHeader( writePage, newSeqNo );

        currentPage = writePage;
        success = true;
    }

    HAL_FLASH_Lock();
    mtx.unlock();

    return success;
}
