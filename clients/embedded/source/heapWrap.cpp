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

#include "FreeRTOS.h"
#include <string.h>

/*
 * C stuff
 */

extern "C" void* malloc( size_t size )
{
    return pvPortMalloc(size);
}

extern "C" void* calloc( size_t count, size_t size )
{
    void* p = pvPortMalloc( count * size );
    if ( p != NULL )
        memset( p, 0, count * size );
    return p;
}

extern "C" void* realloc( void *p, size_t size )
{
    /* if size is 0, realloc shall free */
    if ( size == 0 )
    {
        vPortFree( p );
        return NULL;
    }
    /* if p is NULL, realloc is malloc */
    if ( p == NULL )
    {
        return pvPortMalloc( size );
    }
    else
    {
        void* p2 = pvPortMalloc( size );
        if ( p2 != NULL )
        {
            /* preserve content. we may read out of bounds here if new size is smaller than old, but we don't know the old size so... */
            memcpy( p2, p, size );
            vPortFree( p );
        }
        return p2;
    }
}

extern "C" void free( void* p )
{
    vPortFree( p );
}


/*
 * C++ stuff
 */

void* operator new( size_t size )
{
    return pvPortMalloc( size );
}

void* operator new[]( size_t size )
{
    return pvPortMalloc( size );
}

void operator delete( void* p )
{
    vPortFree( p );
}

void operator delete[]( void* p )
{
    vPortFree( p );
}

