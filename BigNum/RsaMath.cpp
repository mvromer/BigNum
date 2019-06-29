#include <stdexcept>

#include "RsaMath.h"

// Adapted from section 14.4.3 in Handbook of Applied Cryptography. Designed specifically for
// computing inverse of the RSA modulus N such that NN' + RR' = 1 given that R > N and that
// R is a power of two, specifically a power of the BigNum radix. In particular, this will
// find the inverse N' such that NN' = 1 (mod b) where b is the BigNum radix.
//
// Under these assumptions, we know N is odd because practical application of RSA enforces it
// to be a product of two large primes. Our radix b is determined at compile time and is even.
//
BigNum compute_rsa_inverse( const BigNum & n )
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

    return C;
}
