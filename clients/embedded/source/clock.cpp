/*
 * clock.c
 *
 *  Created on: 24 Feb 2014
 *      Author: Jesse
 */

#include "clock.h"
#include <assert.h>
#include "Platform/Threads/Mutex.h"

static bool inited_ = false;
static time_t currenttime = 0;
static Platform::Mutex* mtx;

void clockInit()
{
    assert( !inited_ );
    inited_ = true;
    mtx = new Platform::Mutex();
}

void clockTick()
{
    assert( inited_ );
    // need lock here, this is two operations
    mtx->lock();
    currenttime++;
    mtx->unlock();
}

time_t getTime()
{
    assert( inited_ );
    // no need to lock, operation is atomic and not important if performed before or after write op
    return currenttime;
}

void setTime( time_t newTime )
{
    assert( inited_ );
    // this is one operation but we need to lock since it would be bad if this occurred between read and write op in tick
    mtx->lock();
    currenttime = newTime;
    mtx->unlock();
}

