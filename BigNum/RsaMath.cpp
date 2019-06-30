#include <stdexcept>

#include "RsaMath.h"

// Adapted from binary extended GCD algorithm given in section 14.4.3 in Handbook of Applied
// Cryptography. This is designed specifically for computing a value N' = -N^-1 mod b for an
// RSA modulus N (i.e., the product of two large primes). Here, b is the BigNum radix, which
// is a power of two.
//
// The value N' is guaranteed to exist because N and b are coprime. By extension, this means
// N and R = b^l are also coprime, where l is the number of base-b digits in N. Consequently,
// the value returned by this function is sufficient for use in Montgomery multiplication and,
// by extension, Montgomery exponentiation.
//
BigNum::digit_t compute_montgomery_inverse( const BigNum & n )
{
    // Represent our radix, which is 2^(DigitBits). For purposes of corresponding with variables
    // given in HAC, n and b here refer to x and y in HAC, respectively.
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

    // Montgomery multiplication requires N' to be -N^-1 mod b. C contains our computed inverse
    // for N, though it may not be reduced mod b. Negate the inverse and reduce it mod b. Since
    // the result is reduced mod b, the final answer is in the least significant digit of C.
    return C.negate().mod( b ).getDigit( 0 );
}

// Based on Algorithm 14.36 in Handbook of Applied Cryptography.
BigNum montgomery_multiply( const BigNum & x, const BigNum & y,
    const BigNum & m, BigNum::digit_t mInv )
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
        auto ui = static_cast<BigNum::digit_t>(
            (((a0 + (xi * y0) & digitMask) & digitMask) * mInv) & digitMask
        );

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

// Based on HAC algorithm 14.94.
BigNum montgomery_exponentiation( const BigNum & x, const BigNum & e,
    const BigNum & m, BigNum::digit_t mInv,
    const BigNum & r, const BigNum & r2 )
{
    const BigNum xBar( montgomery_multiply( x, r2, m, mInv ) );
    BigNum a( r );

    auto iExponentBits = e.createBiterator();
    while( iExponentBits.hasBits() )
    {
        a = montgomery_multiply( a, a, m, mInv );
        if( iExponentBits.nextBit() != 0 )
            a = montgomery_multiply( a, xBar, m, mInv );
    }

    BigNum one;
    one = 1;

    return montgomery_multiply( a, one, m, mInv );
}
