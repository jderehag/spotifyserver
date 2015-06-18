/*
 * Copyright (c) 2015, Jens Nielsen
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

#include "../RandomNumberGenerator.h"
#include <random>
#include <ctime>

/* This is just a wrapper for the default_random_engine */
namespace Platform
{

struct random
{
    std::default_random_engine rng;
};

RandomNumberGenerator::RandomNumberGenerator()
{
    rng_ = new random;
    rng_->rng.seed((unsigned int)time(0));
    //rng_->rng = new std::default_random_engine((unsigned int)time(0));
}

RandomNumberGenerator::~RandomNumberGenerator()
{
    //delete rng_->rng;
    delete rng_;
}

uint32_t RandomNumberGenerator::min() { return std::default_random_engine::min(); }
uint32_t RandomNumberGenerator::max() { return std::default_random_engine::max(); }
uint32_t RandomNumberGenerator::operator()()
{
    //return (*(rng_->rng))();
    return rng_->rng();
}
uint32_t RandomNumberGenerator::operator()(uint32_t n)
{
    //return (*(rng_->rng))();
    return rng_->rng() % n;
}
}
