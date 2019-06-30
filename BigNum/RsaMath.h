#ifndef __RSA_MATH_H__
#define __RSA_MATH_H__

#include "BigNum.h"

BigNum::digit_t compute_montgomery_inverse( const BigNum & n );

BigNum montgomery_multiply( const BigNum & x, const BigNum & y,
    const BigNum & m, BigNum::digit_t mInv );

BigNum montgomery_exponentiation( const BigNum & x, const BigNum & e,
    const BigNum & m, BigNum::digit_t mInv,
    const BigNum & r, const BigNum & r2 );

void rsaEncrypt( const uint8_t * input, size_t inputLength, uint8_t * output, size_t outputLength,
    const BigNum & n, const BigNum & e, BigNum::digit_t nInv,
    const BigNum & r, const BigNum & r2 );

#endif
