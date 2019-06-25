#include <algorithm>

#include "BigNum.h"

namespace
{

constexpr size_t BaseCapacity = 4;
constexpr BigNum::digit_t DigitBits = 31;
constexpr BigNum::digit_t DigitMask = (static_cast<BigNum::digit_t>(1) << DigitBits) - static_cast<BigNum::digit_t>(1);

}

BigNum::BigNum( size_t capacity ) :
    m_negative( false ),
    m_numDigitsUsed( 0 ),
    m_digits( 0 )
{
    grow( capacity );
}

void BigNum::grow( size_t newCapacity )
{
    if( m_digits.capacity() >= newCapacity )
        return;

    // Note: vector.resize zero-initializes newly inserted elements.
    newCapacity += (2 * BaseCapacity) - (newCapacity % BaseCapacity);
    m_digits.resize( newCapacity );
}

void BigNum::clamp()
{
    while( m_numDigitsUsed > 0 && m_digits[m_numDigitsUsed - 1] == 0 )
        --m_numDigitsUsed;

    if( m_numDigitsUsed == 0 )
        m_negative = false;
}

void BigNum::zero()
{
    m_numDigitsUsed = 0;
    m_negative = false;
    std::fill( m_digits.begin(), m_digits.end(), 0 );
}

BigNum & BigNum::abs()
{
    m_negative = false;
    return *this;
}

BigNum & BigNum::negate()
{
    // Only need to flip the sign for nonzero values. Otherwise enforce sign is nonnegative.
    m_negative = isZero() ? false : !m_negative;
    return *this;
}

Comparison BigNum::compareMagnitude( const BigNum & other ) const
{
    if( m_numDigitsUsed > other.m_numDigitsUsed )
        return Comparison::GreaterThan;

    if( m_numDigitsUsed < other.m_numDigitsUsed )
        return Comparison::LessThan;

    for( size_t iDigit = 0, riDigit = m_numDigitsUsed - 1;
        iDigit < m_numDigitsUsed;
        ++iDigit, --riDigit )
    {
        if( m_digits[riDigit] > other.m_digits[riDigit] )
            return Comparison::GreaterThan;

        if( m_digits[riDigit] < other.m_digits[riDigit] )
            return Comparison::LessThan;
    }

    return Comparison::Equal;
}

Comparison BigNum::compare( const BigNum & other ) const
{
    if( m_negative && !other.m_negative )
        return Comparison::LessThan;

    if( !m_negative && other.m_negative )
        return Comparison::GreaterThan;

    return m_negative ? other.compareMagnitude( *this ) : compareMagnitude( other );
}


BigNum & BigNum::operator=( const BigNum & other )
{
    if( this != &other )
    {
        if( m_digits.size() < other.m_numDigitsUsed )
            grow( other.m_numDigitsUsed );

        size_t iDigit = 0;
        for( ; iDigit < other.m_numDigitsUsed; ++iDigit )
            m_digits[iDigit] = other.m_digits[iDigit];

        for( ; iDigit < m_numDigitsUsed; ++iDigit )
            m_digits[iDigit] = 0;

        m_numDigitsUsed = other.m_numDigitsUsed;
        m_negative = other.m_negative;
    }

    return *this;
}

BigNum & BigNum::operator=( digit_t value )
{
    zero();
    m_digits[0] = value & DigitMask;
    m_numDigitsUsed = m_digits[0] != 0 ? 1 : 0;
    return *this;
}

BigNum & BigNum::unsignedAddEquals( const BigNum & rhs )
{
    size_t maxUsed;
    size_t minUsed;
    const BigNum * maxNum;

    if( m_numDigitsUsed > rhs.m_numDigitsUsed )
    {
        minUsed = rhs.m_numDigitsUsed;
        maxUsed = m_numDigitsUsed;
        maxNum = this;
    }
    else
    {
        minUsed = m_numDigitsUsed;
        maxUsed = rhs.m_numDigitsUsed;
        maxNum = &rhs;
    }

    if( m_digits.size() < maxUsed + 1 )
        grow( maxUsed + 1 );

    size_t oldNumDigitsUsed = m_numDigitsUsed;
    m_numDigitsUsed = maxUsed + 1;
    digit_t carry = 0;

    size_t iDigit;
    for( iDigit = 0; iDigit < minUsed; ++iDigit )
    {
        m_digits[iDigit] += rhs.m_digits[iDigit] + carry;
        carry = m_digits[iDigit] >> DigitBits;
        m_digits[iDigit] &= DigitMask;
    }

    if( minUsed != maxUsed )
    {
        for( ; iDigit < maxUsed; ++iDigit )
        {
            m_digits[iDigit] = maxNum->m_digits[iDigit] + carry;
            carry = m_digits[iDigit] >> DigitBits;
            m_digits[iDigit] &= DigitMask;
        }
    }

    m_digits[maxUsed] = carry;

    if( oldNumDigitsUsed > maxUsed )
    {
        for( ; iDigit < oldNumDigitsUsed; ++iDigit )
            m_digits[iDigit] = 0;
    }

    clamp();
    return *this;
}


BigNum abs( const BigNum & x )
{
    BigNum y( x );
    y.abs();
    return y;
}

BigNum negate( const BigNum & x )
{
    BigNum y( x );
    y.negate();
    return y;
}
