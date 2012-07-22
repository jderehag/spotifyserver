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

#include "../Messagebox.h"
#include "FreeRTOS.h"
#include "queue.h"

namespace Platform
{


template <typename T>
struct messagebox_t
{
    xQueueHandle hdl;
};


template <typename T>
Messagebox<T>::Messagebox()
{
    mb_ = new messagebox_t<T>;
    mb_->hdl = xQueueCreate( 10, sizeof(T) );
}

template <typename T>
Messagebox<T>::~Messagebox()
{
    vQueueDelete( mb_->hdl );
    delete mb_;
}

template <typename T>
void Messagebox<T>::push_back(const T& item_)
{
    portBASE_TYPE higherPrioTaskWoken;
    xQueueSendToBackFromISR( mb_->hdl, &item_, &higherPrioTaskWoken ); //todo assert success

    if ( higherPrioTaskWoken )
    {
        vPortYieldFromISR();
    }
}

template <typename T>
T Messagebox<T>::pop_front()
{
    T ret;

    /*this will wait forever until there's something in the queue, just as with the stl containers you must check with empty() first*/
    xQueueReceive( mb_->hdl, &ret, portMAX_DELAY );

    return ret;
}

template <typename T>
bool Messagebox<T>::empty()
{
    return ( uxQueueMessagesWaiting( mb_->hdl ) == 0 );
}

}

template class Platform::Messagebox<class Message*>;



