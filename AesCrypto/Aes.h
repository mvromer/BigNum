#ifndef __AES_H__
#define  __AES_H__

#include <cstdint>

void aesEncrypt( const uint8_t * input, size_t inputLength,
    const uint8_t * counter, const uint8_t * key, uint8_t * output );

#endif
