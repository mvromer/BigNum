#ifndef __RSA_MATH_H__
#define __RSA_MATH_H__

#include "BigNum.h"

BigNum::digit_t compute_montgomery_exponentiation_inverse( const BigNum & n );

#endif
