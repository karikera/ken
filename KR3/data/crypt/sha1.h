
#pragma once

#include <stdint.h>

#define SHA1HashSize 20

class CHashSHA1
{
public: 
    void Reset();
    void Input(const uint8_t *,size_t length);
    void Result(uint8_t Message_Digest[SHA1HashSize]);
    
protected:
    uint32_t Intermediate_Hash[SHA1HashSize/4]; /* Message Digest  */

    uint32_t Length_Low;            /* Message length in bits      */
    uint32_t Length_High;           /* Message length in bits      */

                               /* Index into message block array   */
    int_least16_t Message_Block_Index;
    uint8_t Message_Block[64];      /* 512-bit message blocks      */

    /* Local Function Prototyptes */
    void _PadMessage();
    void _ProcessMessageBlock();
};

void sha1(void *out,const void * in,size_t size);