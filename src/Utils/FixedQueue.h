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
 *
 *      Very simple wrapper for deque, basically imposes a static max size for it.
 */

#ifndef FIXEDDEQUE_H_
#define FIXEDQUEUE_H_

#include <deque>

namespace Util
{
template <class T>
class FixedQueue
{
    typedef typename std::deque<T> QueueType;
    typedef unsigned int SizeType;
    typedef typename std::deque<T>::iterator iterator;
    typedef typename std::deque<T>::const_iterator const_iterator;

private:
    SizeType maxNumberOfElements_;
    QueueType queue_;
public:
    FixedQueue(SizeType maxNumberOfElements) : maxNumberOfElements_(maxNumberOfElements) {};

    iterator erase(iterator position) { return queue_.erase(position); }
    iterator erase(iterator first, iterator last) { return queue_.erase(first, last); }
    void clear() {queue_.clear(); }

    SizeType size() const { return queue_.size(); }
    bool empty() const {return queue_.empty(); }

    void pop_front() { queue_.pop_front(); }
    void pop_back() { queue_.pop_back(); }

    T& back() { return queue_.back(); }
    const T& back() const {return queue_.back(); }

    T& front() { return queue_.front(); }
    const T& front() const {return queue_.front(); }

    iterator begin() { return queue_.begin(); }
    const_iterator begin() const { return queue_.begin(); }
    iterator end() { return queue_.end(); }
    const_iterator end() const { return queue_.end(); }

    iterator rbegin() { return queue_.rbegin(); }
    const_iterator rbegin() const { return queue_.rbegin(); }
    iterator rend() { return queue_.rend(); }
    const_iterator rend() const { return queue_.rend(); }

    bool push_front(const T& x)
    {
        if(size() >= maxNumberOfElements_)return false;
        queue_.push_front(x);
        return true;
    }

    bool push_back(const T& x)
    {
        if(size() >= maxNumberOfElements_)return false;
        queue_.push_back(x);
        return true;
    }
};
}

#endif /* FIXEDDEQUE_H_ */
