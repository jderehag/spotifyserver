/*
 * Copyright (c) 2014, Jens Nielsen
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

#include "../Encryption.h"

#include "aes.h"
#include <string.h>

void EncryptionInit()
{
}

static aes_context aesctx;

int Encrypt( const uint8_t* key,
             const uint8_t* iv,
             const uint8_t* indata,
             uint32_t indatalen,
             uint8_t* outdata,
             uint32_t outdatasize )
{
    aes_setkey_enc( &aesctx, key, 256 );
    uint8_t myiv[16];
    memcpy( myiv, iv, 16 );

    aes_crypt_cbc( &aesctx, AES_ENCRYPT, indatalen + (16 - indatalen%16), myiv, indata, outdata );
    return indatalen + (16 - indatalen%16);
}


int Decrypt( const uint8_t* key,
             const uint8_t* iv,
             const uint8_t* indata,
             uint32_t indatalen,
             uint8_t* outdata,
             uint32_t outdatasize )
{
    aes_setkey_dec( &aesctx, key, 256 );
    uint8_t myiv[16];
    memcpy( myiv, iv, 16 );

    aes_crypt_cbc( &aesctx, AES_DECRYPT, indatalen + (16 - indatalen%16), myiv, indata, outdata );
    return indatalen + (16 - indatalen%16);
}

