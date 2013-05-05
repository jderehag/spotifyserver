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

#ifndef RUNNABLE_H_
#define RUNNABLE_H_

namespace Platform
{

class Runnable
{
public:
    typedef enum
    {
        SIZE_SMALL,
        SIZE_MEDIUM,
        SIZE_LARGE,
    }Size;

    typedef enum
    {
        PRIO_LOW,
        PRIO_MID,
        PRIO_HIGH,
    }Prio;

private:
	struct ThreadHandle_t* threadHandle_;
	bool isCancellationPending_;
	bool isJoinable_;
	Size size_;
	Prio prio_;
public:
	Runnable(bool isJoinable = 1, Size size = SIZE_LARGE, Prio prio = PRIO_HIGH);
	~Runnable();

	/* startThread needs to be called from the child class to start the actual thread,
	 * this is done separately from the constructor so that the child can initialize everything
	 * before starting the thread */
	void startThread();

	void joinThread();

	/* this is normally called from the child's destroy method to indicate that the thread should
	 * be canceled, it does not actually stop the thread, but simply sets the cancellationPending_
	 * attribute, cancellation is handled inline within the run method. */
	void cancelThread();
	bool isCancellationPending();

	/* Is the method that is executed in the new thread */
	virtual void run() = 0;

	/* Must always be called when destroying child class, this is so that
	 * threads will be canceled correctly in an asynchronous way,
	 * child class should have all destructor's private */
	virtual void destroy() = 0;

};

} /* namespace LibSpotify */
#endif /* RUNNABLE_H_ */
