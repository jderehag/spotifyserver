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
#include "../Runnable.h"

#include <Windows.h>
#include <assert.h>

namespace Platform
{

typedef struct ThreadHandle_t
{
    HANDLE handle_;
    DWORD  id_;     // thread id
    CRITICAL_SECTION cancellationMutex_;
}ThreadHandle_t;

/* Wrapper for pointing out the correct instance for the runnable implementation */
DWORD WINAPI runnableWrapper(void* arg)
{
    Runnable* runnable = reinterpret_cast<Runnable*> (arg);
    runnable->run();
    return 0;
}

Runnable::Runnable(bool isJoinable, Size size, Prio prio) : isCancellationPending_(false), 
                                                            isJoinable_(isJoinable), 
                                                            size_(size), 
                                                            prio_(prio)
{
    threadHandle_ = new ThreadHandle_t;
    InitializeCriticalSection(&threadHandle_->cancellationMutex_);

    if(isJoinable_)
    {
        /*nop?*/
    }
}
Runnable::~Runnable()
{
    DeleteCriticalSection(&threadHandle_->cancellationMutex_);
    delete threadHandle_;
}


void Runnable::startThread()
{
    threadHandle_->handle_ = CreateThread(NULL, 0, runnableWrapper, this, 0, &threadHandle_->id_);
    if( prio_ == PRIO_LOW )
        SetThreadPriority( threadHandle_->handle_, THREAD_PRIORITY_BELOW_NORMAL );
    else if( prio_ == PRIO_HIGH )
        SetThreadPriority( threadHandle_->handle_, THREAD_PRIORITY_HIGHEST );
}

void Runnable::joinThread()
{
    WaitForSingleObject(threadHandle_->handle_, INFINITE);
}

bool Runnable::isCancellationPending()
{
    EnterCriticalSection(&threadHandle_->cancellationMutex_);
    bool isCancellationPending = isCancellationPending_;
    LeaveCriticalSection(&threadHandle_->cancellationMutex_);
    return isCancellationPending;
}
void Runnable::cancelThread()
{
    EnterCriticalSection(&threadHandle_->cancellationMutex_);
    isCancellationPending_ = true;
    LeaveCriticalSection(&threadHandle_->cancellationMutex_);
}



} /* namespace Platform */
