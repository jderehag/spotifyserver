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
#include "stm32f4xx_hal_rng.h"
#include "stm32f4xx_hal_rcc.h"

namespace Platform
{
struct random
{
    RNG_HandleTypeDef RngHandle;
};

RandomNumberGenerator::RandomNumberGenerator()
{
    rng_ = new random;
    __RNG_CLK_ENABLE();
    rng_->RngHandle.Instance = RNG;
    HAL_RNG_Init(&rng_->RngHandle);
}

RandomNumberGenerator::~RandomNumberGenerator()
{
    delete rng_;
}

uint32_t RandomNumberGenerator::min() { return 0; }
uint32_t RandomNumberGenerator::max() { return 0xFFFFFFFFUL; }
uint32_t RandomNumberGenerator::operator()()
{
    return HAL_RNG_GetRandomNumber(&rng_->RngHandle);
}
uint32_t RandomNumberGenerator::operator()(uint32_t n)
{
    return (*this)() % n;
}
}
