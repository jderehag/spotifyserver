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

#include <openssl/aes.h>
#include <openssl/evp.h>


void EncryptionInit()
{
}

int Encrypt( const uint8_t* key,
             const uint8_t* iv,
             const uint8_t* indata,
             uint32_t indatalen,
             uint8_t* outdata,
             uint32_t outdatasize )
{
    const int blockSize = 16;
    EVP_CIPHER_CTX ctx;
    int res = 0;
    int len = 0;
    uint32_t outdatalen = 0;

    if ( outdatasize < indatalen + blockSize )
        return -1;

    // create encryption context
    EVP_CIPHER_CTX_init( &ctx );
    EVP_EncryptInit_ex( &ctx, EVP_aes_256_cbc(), 0, key, iv );

    // encrypt bulk
    res = EVP_EncryptUpdate(
        &ctx,
        outdata,
        &len,
        indata,
        indatalen);

    outdatalen = len;

    if ( res != 1 )
        return -1;

    // last block and padding:
    res = EVP_EncryptFinal(
        &ctx,
        outdata + len,
        &len);

    outdatalen += len;

    if ( res != 1 )
        return -1;

    return outdatalen; // Success
}

int Decrypt( const uint8_t* key,
             const uint8_t* iv,
             const uint8_t* indata,
             uint32_t indatalen,
             uint8_t* outdata,
             uint32_t outdatasize )
{
    const int blockSize = 16;
    EVP_CIPHER_CTX ctx;
    int res = 0;
    int len = 0;
    uint32_t decodedLen = 0;

    if ( outdatasize < indatalen + blockSize )
        return -1;

    // create encryption context
    EVP_CIPHER_CTX_init( &ctx );
    EVP_DecryptInit_ex( &ctx, EVP_aes_256_cbc(), 0, key, iv );

    // decrypt bulk
    res = EVP_DecryptUpdate(
        &ctx,
        outdata,
        &len,
        indata,
        indatalen );

    decodedLen = len;

    if ( res != 1 )
        return -1;

    // last block and padding:
    res = EVP_DecryptFinal(
        &ctx,
        outdata + len,
        &len );

    decodedLen += len;

    if ( res != 1 )
        return -1;

    return decodedLen; // Success
}

