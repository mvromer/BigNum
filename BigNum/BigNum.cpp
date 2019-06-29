#include <algorithm>
#include <climits>

#include "BigNum.h"

namespace
{

constexpr size_t BaseCapacity = 4;

}

BigNum::BigNum() : BigNum( BaseCapacity ) { }

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

BigNum & BigNum::multiplyByTwo()
{
    const size_t minCapacity = m_numDigitsUsed + 1;
    if( m_digits.size() < minCapacity )
        grow( minCapacity );

    const size_t oldNumDigitsUsed = m_numDigitsUsed;
    size_t iDigit;
    digit_t carry = 0;

    for( iDigit = 0; iDigit < m_numDigitsUsed; ++iDigit )
    {
        constexpr digit_t nextCarryShift = DigitBits - DigitOne;
        const digit_t nextCarry = m_digits[iDigit] >> nextCarryShift;
        m_digits[iDigit] = ((m_digits[iDigit] << DigitOne) | carry) & DigitMask;
        carry = nextCarry;
    }

    if( carry != 0 )
    {
        m_digits[iDigit] = carry;
        ++m_numDigitsUsed;
    }

    if( m_numDigitsUsed < (oldNumDigitsUsed - 1) )
    {
        for( iDigit = m_numDigitsUsed; iDigit < oldNumDigitsUsed; ++iDigit )
            m_digits[iDigit] = 0;
    }

    return *this;
}

BigNum & BigNum::divideByTwo()
{
    const size_t oldNumDigitsUsed = m_numDigitsUsed;
    size_t iDigit;
    size_t riDigit;
    digit_t carry = 0;

    for( iDigit = 0, riDigit = m_numDigitsUsed - 1;
        iDigit < m_numDigitsUsed;
        ++iDigit, --riDigit )
    {
        constexpr digit_t carryShift = DigitBits - DigitOne;
        const digit_t nextCarry = m_digits[riDigit] & DigitOne;
        m_digits[riDigit] = (m_digits[riDigit] >> DigitOne) | (carry << carryShift);
        carry = nextCarry;
    }

    if( m_numDigitsUsed < (oldNumDigitsUsed - 1) )
    {
        for( iDigit = m_numDigitsUsed; iDigit < oldNumDigitsUsed; ++iDigit )
            m_digits[iDigit] = 0;
    }

    clamp();
    return *this;
}

BigNum & BigNum::leftDigitShift( size_t numDigits )
{
    if( numDigits == 0 )
        return *this;

    const size_t minCapacity = m_numDigitsUsed + numDigits;
    if( m_digits.size() < minCapacity )
        grow( minCapacity );

    m_numDigitsUsed += numDigits;
    size_t iLead = m_numDigitsUsed - 1;
    size_t iTrail = iLead - numDigits;

    for( size_t iStep = m_numDigitsUsed - 1; iStep >= numDigits; --iStep )
    {
        m_digits[iLead] = m_digits[iTrail];
        --iLead;
        --iTrail;
    }

    for( size_t iDigit = 0; iDigit < numDigits; ++iDigit )
        m_digits[iDigit] = 0;

    return *this;
}

BigNum & BigNum::rightDigitShift( size_t numDigits )
{
    if( numDigits == 0 )
        return *this;

    if( m_numDigitsUsed <= numDigits )
    {
        zero();
        return *this;
    }

    size_t iTrail = 0;
    size_t iLead = numDigits;
    const size_t newNumDigitsUsed = m_numDigitsUsed - numDigits;

    for( size_t iStep = 0; iStep < newNumDigitsUsed; ++iStep )
    {
        m_digits[iTrail] = m_digits[iLead];
        --iTrail;
        --iLead;
    }

    for( size_t iDigit = newNumDigitsUsed; iDigit < m_numDigitsUsed; ++iDigit )
        m_digits[iDigit] = 0;

    m_numDigitsUsed = newNumDigitsUsed;
    return *this;
}

BigNum & BigNum::mod2b( size_t b )
{
    if( b == 0 )
    {
        zero();
        return *this;
    }

    if( b > m_numDigitsUsed * DigitBits )
        return *this;

    // Zero out all digits that are completely outside the modulus.
    const size_t iFirstToZero = (b / DigitBits) + ((b % DigitBits) == 0 ? 0 : 1);
    for( size_t iDigit = iFirstToZero; iDigit < m_numDigitsUsed; ++iDigit )
        m_digits[iDigit] = 0;

    // Clear out the appropriate bits in the digit that is not completely in/out of the modulus.
    const size_t iBoundaryDigit = b / DigitBits;
    const size_t residualMask = (DigitOne << (b % DigitBits)) - DigitOne;
    m_digits[iBoundaryDigit] &= residualMask;

    clamp();
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

BigNum & BigNum::operator+=( const BigNum & rhs )
{
    if( m_negative == rhs.m_negative )
    {
        unsignedAddEquals( rhs );
    }
    else if( compareMagnitude( rhs ) == Comparison::LessThan )
    {
        *this = unsignedSubtract( rhs, *this );
    }
    else
    {
        unsignedSubtractEquals( rhs );
    }

    return *this;
}

BigNum & BigNum::operator-=( const BigNum & rhs )
{
    if( m_negative != rhs.m_negative )
    {
        unsignedAddEquals( rhs );
    }
    else if( compareMagnitude( rhs ) != Comparison::LessThan )
    {
        unsignedSubtractEquals( rhs );
    }
    else
    {
        const bool newNegative = !m_negative;
        *this = unsignedSubtract( rhs, *this );
        m_negative = newNegative;
    }

    return *this;
}

BigNum & BigNum::operator*=( const BigNum & rhs )
{
    m_negative = (m_negative == rhs.m_negative);
    baselineMultiply( rhs, m_numDigitsUsed + rhs.m_numDigitsUsed + 1 );
    return *this;
}

BigNum & BigNum::operator<<=( size_t numBits )
{
    const size_t newCapacity = m_numDigitsUsed + numBits / DigitBits + 1;
    if( m_digits.size() < newCapacity )
        grow( newCapacity );

    // Shift by whole digits first.
    if( numBits >= DigitBits )
        leftDigitShift( numBits / DigitBits );

    // Shift the remaining number of bits not covered previously by the digit shift.
    numBits %= DigitBits;

    if( numBits != 0 )
    {
        const digit_t mask = (DigitOne << numBits) - DigitOne;
        const digit_t carryShift = DigitBits - numBits;
        digit_t carry = 0;

        for( size_t iDigit = 0; iDigit < m_numDigitsUsed; ++iDigit )
        {
            const digit_t nextCarry = (m_digits[iDigit] >> carryShift) & mask;
            m_digits[iDigit] = ((m_digits[iDigit] << numBits) | carry) & DigitMask;
            carry = nextCarry;
        }

        if( carry > 0 )
        {
            m_digits[m_numDigitsUsed] = carry;
            ++m_numDigitsUsed;
        }
    }

    return *this;
}

BigNum & BigNum::operator>>=( size_t numBits )
{
    if( numBits == 0 )
        return *this;

    // Shift by whole digits first.
    if( numBits >= DigitBits )
        rightDigitShift( numBits / DigitBits );

    // Shift the remaining number of bits not covered by the previous digit shift.
    numBits %= DigitBits;

    if( numBits != 0 )
    {
        const digit_t mask = (DigitOne << numBits) - DigitOne;
        const digit_t carryShift = DigitBits - numBits;
        digit_t carry = 0;

        for( size_t iDigit = 0, riDigit = m_numDigitsUsed - 1;
            iDigit < m_numDigitsUsed;
            ++iDigit, --riDigit )
        {
            const digit_t nextCarry = m_digits[iDigit] & mask;
            m_digits[iDigit] = (m_digits[iDigit] >> numBits) | (carry << carryShift);
            carry = nextCarry;
        }
    }

    clamp();
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

    const size_t oldNumDigitsUsed = m_numDigitsUsed;
    m_numDigitsUsed = maxUsed + 1;
    digit_t carry = 0;

    size_t iDigit;
    for( iDigit = 0; iDigit < minUsed; ++iDigit )
    {
        // Sum the current digits from both addends plus the current carry.
        m_digits[iDigit] += rhs.m_digits[iDigit] + carry;

        // Get the carry bit for this iteration's sum.
        carry = m_digits[iDigit] >> DigitBits;

        // Clear out the carry bit from this iteration's sum.
        m_digits[iDigit] &= DigitMask;
    }

    if( minUsed != maxUsed )
    {
        for( ; iDigit < maxUsed; ++iDigit )
        {
            // Sum the current digit from longer addend plus the current carry.
            m_digits[iDigit] = maxNum->m_digits[iDigit] + carry;
        
            // Get the carry bit for this iteration's sum.
            carry = m_digits[iDigit] >> DigitBits;
            
            // Clear out the carry bit from this iteration's sum.
            m_digits[iDigit] &= DigitMask;
        }
    }

    m_digits[maxUsed] = carry;

    if( oldNumDigitsUsed > maxUsed )
    {
        // If needed, zero out the most significant digits that don't contribute to the sum.
        //
        // NOTE: I haven't verified this, but since this is implemented as basically a +=
        // operation, and thus (I'm pretty sure) oldNumDigitsUsed will either be less than or
        // equal to maxUsed, I think this loop can't get executed. Similarly, I'm not convinced
        // the final clamp operation is needed. This would need additional checking though.
        //
        for( iDigit = maxUsed + 1; iDigit < oldNumDigitsUsed; ++iDigit )
            m_digits[iDigit] = 0;
    }

    clamp();
    return *this;
}

BigNum & BigNum::unsignedSubtractEquals( const BigNum & rhs )
{
    size_t minUsed = rhs.m_numDigitsUsed;
    size_t maxUsed = m_numDigitsUsed;

    if( m_digits.size() < maxUsed )
        grow( maxUsed );

    const size_t oldNumDigitsUsed = maxUsed;
    digit_t carry = 0;

    size_t iDigit;
    for( iDigit = 0; iDigit < minUsed; ++iDigit )
    {
        // Subtract the subtrahend from the minuend and subtract out the previous carry.
        m_digits[iDigit] = m_digits[iDigit] - rhs.m_digits[iDigit] - carry;

        // Get the carry bit for this iteration's difference. This optimization from BigNum Math
        // is an optimization that pulls out the carry bit that propagated to the most significant
        // bit in the computed difference. This only works on machines that perform 2's complement
        // arithmetic, which is valid for x86/x64 and RISC-V.
        carry = m_digits[iDigit] >> (DigitBitSize - DigitOne);

        // Clear out the carry bits from this iteration's difference.
        m_digits[iDigit] &= DigitMask;
    }

    if( minUsed < maxUsed )
    {
        for( ; iDigit < maxUsed; ++iDigit )
        {
            // This routine assumes this number is equal to or greater in magnitude than the right
            // hand side, so we always subtract out the carry from this number's upper digits.
            m_digits[iDigit] -= carry;

            // Get the carry bit for this iteration's difference. This optimization from BigNum Math
            // is an optimization that pulls out the carry bit that propagated to the most significant
            // bit in the computed difference. This only works on machines that perform 2's complement
            // arithmetic, which is valid for x86/x64 and RISC-V.
            carry = m_digits[iDigit] >> (DigitBitSize - DigitOne);

            // Clear out the carry bits from this iteration's difference.
            m_digits[iDigit] &= DigitMask;
        }
    }

    if( oldNumDigitsUsed > maxUsed )
    {
        // I believe this loop can't be executed simply because this basically implements a -=
        // operation, and this number is assumed to be one with a larger magnitude. Similarly with
        // the final clamp operation. This would need additional verification.
        for( ; iDigit < oldNumDigitsUsed; ++iDigit )
            m_digits[iDigit] = 0;
    }

    clamp();
    return *this;
}

BigNum & BigNum::baselineMultiply( const BigNum & rhs, size_t numDigits )
{
    BigNum temp( numDigits );
    temp.m_numDigitsUsed = numDigits;

    for( size_t iDigitThis = 0; iDigitThis < m_numDigitsUsed; ++iDigitThis )
    {
        digit_t carry = 0;
        const size_t numDigitsRhs = std::min( rhs.m_numDigitsUsed, numDigits - iDigitThis );

        if( numDigitsRhs < 1 )
            break;

        for( size_t iDigitRhs = 0; iDigitRhs < numDigitsRhs; ++iDigitRhs )
        {
            const size_t iDigitTemp = iDigitThis + iDigitRhs;
            const word_t product = (static_cast<word_t>(temp.m_digits[iDigitTemp]) +
                static_cast<word_t>(m_digits[iDigitThis]) *
                static_cast<word_t>(rhs.m_digits[iDigitRhs]) +
                static_cast<word_t>(carry));

            temp.m_digits[iDigitTemp] = product & static_cast<word_t>(DigitMask);
            carry = static_cast<digit_t>(product >> static_cast<word_t>(DigitBits));
        }

        if( (iDigitThis + numDigitsRhs) < numDigits )
            temp.m_digits[iDigitThis + numDigitsRhs] = carry;
    }

    temp.clamp();
    m_numDigitsUsed = temp.m_numDigitsUsed;
    m_digits = std::move( temp.m_digits );

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

BigNum multiplyByTwo( const BigNum & x )
{
    BigNum y( x );
    y.multiplyByTwo();
    return y;
}

BigNum divideByTwo( const BigNum & x )
{
    BigNum y( x );
    y.divideByTwo();
    return y;
}

BigNum leftDigitShift( const BigNum & x, size_t numDigits )
{
    BigNum y( x );
    y.leftDigitShift( numDigits );
    return y;
}

BigNum rightDigitShift( const BigNum & x, size_t numDigits )
{
    BigNum y( x );
    y.rightDigitShift( numDigits );
    return y;
}

BigNum mod2b( const BigNum & x, size_t b )
{
    BigNum y( x );
    y.mod2b( b );
    return y;
}
