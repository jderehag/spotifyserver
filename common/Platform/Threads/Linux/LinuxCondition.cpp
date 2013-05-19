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
#include "../Condition.h"
#include "../Mutex.h"
#include "LinuxMutexPimpl.h"

#include <pthread.h>
#include <time.h>

namespace Platform
{
struct conditional
{
	pthread_cond_t cond;
};

Condition::Condition()
{
	cond_ = new conditional;
	pthread_cond_init(&cond_->cond, NULL);
}

Condition::~Condition()
{
	pthread_cond_destroy(&cond_->cond);
	delete cond_;
}

void Condition::wait(Mutex& mtx)
{
	pthread_cond_wait(&cond_->cond, &mtx.mtx_->mtx_);
}

void Condition::timedWait(Mutex& mtx, unsigned int milliSeconds)
{
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	ts.tv_sec += milliSeconds / 1000;
	ts.tv_nsec += (milliSeconds % 1000) * 1000000;

	pthread_cond_timedwait(&cond_->cond, &mtx.mtx_->mtx_, &ts);
}

void Condition::signal()
{
	pthread_cond_signal(&cond_->cond);
}

}
