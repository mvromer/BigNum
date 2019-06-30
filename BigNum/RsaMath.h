#ifndef __RSA_MATH_H__
#define __RSA_MATH_H__

#include "BigNum.h"

BigNum::digit_t compute_montgomery_inverse( const BigNum & n );

BigNum montgomery_multiply( const BigNum & x, const BigNum & y,
    const BigNum & m, BigNum::digit_t mInv, const BigNum & r );

#endif
