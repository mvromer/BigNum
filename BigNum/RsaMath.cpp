#include "RsaMath.h"

// Adapted from section 14.4.3 in Handbook of Applied Cryptography. Designed specifically for
// computing inverse of the RSA modulus N such that NN' + RR' = 1 given that R > N and that
// R is a power of two, specifically a power of the BigNum radix. In particular, this will
// find the inverse N' such that NN' = 1 (mod b) where b is the BigNum radix.
//
// Under these assumptions, we know N is odd because practical RSA enforces it to be a product
// of two large primes. Our radix b is determined at compile time and is even.
//
void compute_rsa_inverse( const BigNum & n, BigNum & nInv )
{
    BigNum a( n );

    // Represent our radix, which is 2^(DigitBits). For purposes of corresponding with variables
    // given in HAC, b and n here refer to x and y in HAC, respectively.
    BigNum b( n.numberDigits() );
    b = 1;
    b <<= DigitBits;

    BigNum x( a );
    BigNum y( b );

    BigNum u( x );
    BigNum v( y );

    BigNum A( a.numberDigits() );
    BigNum B( a.numberDigits() );
    BigNum C( a.numberDigits() );
    BigNum D( a.numberDigits() );

    A = 1;
    D = 1;

    do
    {
        while( u.isEven() )
        {
            u.divideByTwo();

            if( A.isOdd() || B.isOdd() )
            {
                A += y;
                B -= x;
            }

            A.divideByTwo();
            B.divideByTwo();
        }

        while( v.isEven() )
        {
            v.divideByTwo();

            if( C.isOdd() || D.isOdd() )
            {
                C += y;
                D -= x;
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

    const BigNum zero( C.numberDigits() );
    while( C.compare( zero ) != Comparison::GreaterThan )
        C += b;

    while( C.compare( b ) != Comparison::LessThan )
        C -= b;

    nInv = C;
}
