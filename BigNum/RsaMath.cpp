﻿#include <stdexcept>

#include "RsaMath.h"

// Adapted from binary extended GCD algorithm given in section 14.4.3 in Handbook of Applied
// Cryptography. Designed specifically for computing inverse of the RSA modulus N such that
// NN' + RR' = 1 given that R > N and that R is a power of two, specifically a power of the
// BigNum radix. In particular, this will find the inverse N' such that NN' = 1 (mod b) where
// b is the BigNum radix. Expected to be used when performing Montgomery exponentiation.
//
// Under these assumptions, we know N is odd because practical application of RSA enforces it
// to be a product of two large primes. Our radix b is determined at compile time and is even.
//
BigNum::digit_t compute_montgomery_exponentiation_inverse( const BigNum & n )
{
    // Represent our radix, which is 2^(DigitBits). For purposes of corresponding with variables
    // given in HAC, n and x here refer to x and y in HAC, respectively.
    BigNum b;
    b = 1;
    b <<= DigitBits;

    BigNum u( n );
    BigNum v( b );

    BigNum A;
    BigNum B;
    BigNum C;
    BigNum D;

    A = 1;
    D = 1;

    do
    {
        while( u.isEven() )
        {
            u.divideByTwo();

            if( A.isOdd() || B.isOdd() )
            {
                A += b;
                B -= n;
            }

            A.divideByTwo();
            B.divideByTwo();
        }

        while( v.isEven() )
        {
            v.divideByTwo();

            if( C.isOdd() || D.isOdd() )
            {
                C += b;
                D -= n;
            }

            C.divideByTwo();
            D.divideByTwo();
        }

        if( u.compare( v ) != Comparison::LessThan )
        {
            u -= v;
            A -= C;
            B -= D;
        }
        else
        {
            v -= u;
            C -= A;
            D -= B;
        }
    } while( !u.isZero() );

    // If v is not 1, then we somehow picked a bad RSA modulus, because it means that n and b
    // are NOT coprime, which is a precondition for this function.
    BigNum one;
    one = 1;
    if( v.compare( one ) != Comparison::Equal )
        throw std::invalid_argument( "n must be coprime to be" );

    // Make sure computed modulus is in the range [0, b).
    const BigNum zero;
    while( C.compare( zero ) != Comparison::GreaterThan )
        C += b;

    while( C.compare( b ) != Comparison::LessThan )
        C -= b;

    // Since we reduced mod b, the final answer is in the least significant digit of C.
    return C.getDigit( 0 );
}

// Based on Algorithm 14.36 in Handbook of Applied Cryptography.
BigNum montgomery_multiply( const BigNum & x, const BigNum & y,
    const BigNum & m, BigNum::digit_t mInv, const BigNum & r )
{
    const size_t numberDigits = m.numberDigits();
    BigNum a( m.numberDigits() );

    const auto y0 = static_cast<BigNum::word_t>(y.getDigit( 0 ));
    const auto mInvWord = static_cast<BigNum::word_t>(mInv);
    constexpr const auto digitMask = static_cast<const BigNum::word_t>(DigitMask);

    // These are used to hold the result of xi * y and ui * m inside the following loop.
    BigNum xiy( y.numberDigits() + 1 );
    BigNum uim( m.numberDigits() + 1 );

    for( size_t iDigit = 0; iDigit < numberDigits; ++iDigit )
    {
        const auto a0 = static_cast<BigNum::word_t>(a.getDigit( 0 ));
        const auto xi = static_cast<BigNum::word_t>(x.getDigit( iDigit ));

        // Compute ui = (a0 + xi * y0) * m' (mod b). Perform the mod b operation at each step to
        // avoid overflowing the double precision word.
        auto ui = static_cast<BigNum::digit_t>((((a0 + (xi * y0) & digitMask) & digitMask) * mInv) & digitMask);

        // Compute A = (A + xi * y + ui * m) / b.
        xiy = y * static_cast<BigNum::digit_t>(xi);
        uim = m * ui;
        a += (xiy + uim);
        a.rightDigitShift( 1 );
    }

    if( a.compare( m ) != Comparison::LessThan )
        a -= m;

    return a;
}
