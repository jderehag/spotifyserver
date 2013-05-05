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

#include "../Runnable.h"

#include <pthread.h>
#include <assert.h>

namespace Platform
{

typedef struct ThreadHandle_t
{
	pthread_t thread;
	pthread_attr_t attr;
	pthread_mutex_t cancellationMutex_;
}ThreadHandle_t;

/* Wrapper for pointing out the correct instance for the runnable implementation */
static void* runnableWrapper(void* arg)
{
	Runnable* runnable = reinterpret_cast<Runnable*> (arg);
	runnable->run();
	return NULL;
}

Runnable::Runnable(bool isJoinable, Size size, Prio prio) : isCancellationPending_(false),
                                                            isJoinable_(isJoinable),
                                                            size_(size),
                                                            prio_(prio)
{
	threadHandle_ = new ThreadHandle_t;
	pthread_attr_init(&threadHandle_->attr);
	pthread_mutex_init(&threadHandle_->cancellationMutex_, NULL);

	if(isJoinable_)
	{
		pthread_attr_setdetachstate(&threadHandle_->attr, PTHREAD_CREATE_JOINABLE);
	}
}
Runnable::~Runnable()
{
	pthread_mutex_destroy(&threadHandle_->cancellationMutex_);
	pthread_attr_destroy(&threadHandle_->attr);
	delete threadHandle_;
}


void Runnable::startThread()
{
	pthread_create(&threadHandle_->thread, &threadHandle_->attr, runnableWrapper, this);
}

void Runnable::joinThread()
{
	void *status;
	assert(isJoinable_);
	pthread_join(threadHandle_->thread, &status);
}

bool Runnable::isCancellationPending()
{
	pthread_mutex_lock(&threadHandle_->cancellationMutex_);
	bool isCancellationPending = isCancellationPending_;
	pthread_mutex_unlock(&threadHandle_->cancellationMutex_);
	return isCancellationPending;
}
void Runnable::cancelThread()
{
	pthread_mutex_lock(&threadHandle_->cancellationMutex_);
	isCancellationPending_ = true;
	pthread_mutex_unlock(&threadHandle_->cancellationMutex_);
}



} /* namespace LibSpotify */
