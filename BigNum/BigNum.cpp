#include <algorithm>
#include <stdexcept>

#include "BigNum.h"

namespace
{

constexpr size_t BaseCapacity = 4;

}

BigNum::biterator::biterator( const BigNum & number ) : m_number( number )
{
    if( m_number.isZero() )
    {
        m_iCurrentDigit = 0;
        m_currentBitInDigitMask = 0;
    }
    else
    {
        m_iCurrentDigit = m_number.m_numDigitsUsed - 1;

        // If this is zero, it really means there are DigitBits number of bits in the leading digit.
        size_t numLeadingDigitBits = m_number.numberBits() % DigitBits;
        if( numLeadingDigitBits == 0 )
            numLeadingDigitBits = DigitBits;

        m_currentBitInDigitMask = DigitOne << (numLeadingDigitBits - 1);
    }
}

BigNum::digit_t BigNum::biterator::nextBit()
{
    digit_t value = m_number.m_digits[m_iCurrentDigit] & m_currentBitInDigitMask;
    m_currentBitInDigitMask >>= 1;

    if( m_currentBitInDigitMask == 0 && m_iCurrentDigit > 0 )
    {
        --m_iCurrentDigit;
        m_currentBitInDigitMask = DigitOne << (DigitBits - 1);
    }

    return value;
}


BigNum::BigNum() : BigNum( BaseCapacity ) { }

BigNum::BigNum( size_t capacity ) :
    m_negative( false ),
    m_numDigitsUsed( 0 ),
    m_digits( 0 )
{
    grow( capacity );
}

BigNum::BigNum( const std::vector<uint8_t> & digitData ) :
    BigNum( digitData.data(), digitData.size() ) { }

BigNum::BigNum( const uint8_t * digitData, size_t numberBytes,
    bool swizzle, size_t swizzleSize ) :
    BigNum()
{
    constexpr bool preZero = true;
    loadBytes( digitData, numberBytes, preZero, swizzle, swizzleSize );
}

size_t BigNum::numberBits() const
{
    if( isZero() )
        return 0;

    size_t numberBits = (m_numDigitsUsed - 1) * DigitBits;
    digit_t mostSigDigit = m_digits[m_numDigitsUsed - 1];

    while( mostSigDigit > 0 )
    {
        ++numberBits;
        mostSigDigit >>= 1;
    }

    return numberBits;
}

size_t BigNum::numberBytes() const
{
    size_t bits = numberBits();
    return (bits / 8) + (bits % 8 == 0 ? 0 : 1);
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

void BigNum::loadBytes( const uint8_t * bytes, size_t count, bool preZero,
    bool swizzle, size_t swizzleSize )
{
    if( bytes == nullptr )
        return;

    if( count % swizzleSize != 0 )
        throw std::invalid_argument( "Swizzle size must be multiple of load size." );

    if( preZero )
        zero();

    const auto computeByteOffset = swizzle ? computeByteOffsetSwizzle : computeByteOffsetNoSwizzle;
    
    for( size_t iByte = 0; iByte <= (count - swizzleSize); iByte += swizzleSize )
    {
        for( size_t swizzleOffset = 0; swizzleOffset < swizzleSize; ++swizzleOffset )
        {
            *this <<= 8;
            const size_t iRead = iByte + computeByteOffset( swizzleSize, swizzleOffset );
            m_digits[0] |= bytes[iRead];
            ++m_numDigitsUsed;
        }
    }

    clamp();
}

void BigNum::storeBytes( uint8_t * bytes, size_t count,
    bool swizzle, size_t swizzleSize )
{
    if( bytes == nullptr )
        return;

    if( count % swizzleSize != 0 )
        throw std::invalid_argument( "Swizzle size must be multiple of store size." );

    BigNum x( *this );
    const auto computeByteOffset = swizzle ? computeByteOffsetSwizzle : computeByteOffsetNoSwizzle;

    for( size_t iByte = 0; iByte <= (count - swizzleSize); iByte += swizzleSize )
    {
        for( size_t swizzleOffset = 0; swizzleOffset < swizzleSize; ++swizzleOffset )
        {
            const size_t iWrite = iByte + computeByteOffset( swizzleSize, swizzleOffset );
            bytes[count - 1 - iWrite] = static_cast<uint8_t>(x.m_digits[0] & 0xFF);
            x >>= 8;
        }
    }
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
        ++iTrail;
        ++iLead;
    }

    for( size_t iDigit = newNumDigitsUsed; iDigit < m_numDigitsUsed; ++iDigit )
        m_digits[iDigit] = 0;

    m_numDigitsUsed = newNumDigitsUsed;
    return *this;
}

BigNum & BigNum::mod( const BigNum & modulus )
{
    BigNum q;
    BigNum r;
    divide( modulus, q, r );

    // Remainder could be negative depending on the signs of our inputs. To reduce mod our modulus,
    // add back the modulus.
    if( r.m_negative )
        r += modulus;

    *this = r;
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
    // This only supports the baseline multiplier from BigNum Math and none of the fancier methods.
    m_negative = (m_negative != rhs.m_negative);
    baselineMultiply( rhs, m_numDigitsUsed + rhs.m_numDigitsUsed + 1 );
    return *this;
}

BigNum & BigNum::operator*=( digit_t rhs )
{
    const size_t oldNumDigitsUsed = m_numDigitsUsed;
    grow( oldNumDigitsUsed + 1 );

    m_numDigitsUsed = oldNumDigitsUsed + 1;
    digit_t carry = 0;
    constexpr auto digitMask = static_cast<word_t>(DigitMask);
    constexpr auto digitBits = static_cast<word_t>(DigitBits);

    for( size_t iDigit = 0; iDigit < oldNumDigitsUsed; ++iDigit )
    {
        const word_t r = static_cast<word_t>(carry) +
            static_cast<word_t>(m_digits[iDigit]) *
            static_cast<word_t>(rhs);

        m_digits[iDigit] = static_cast<digit_t>(r & digitMask);
        carry = static_cast<digit_t>(r >> digitBits);
    }

    m_digits[oldNumDigitsUsed] = carry;

    clamp();
    return *this;
}

BigNum & BigNum::operator/=( const BigNum & rhs )
{
    BigNum q;
    BigNum r;
    divide( rhs, q, r );
    *this = q;
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
            const digit_t nextCarry = m_digits[riDigit] & mask;
            m_digits[riDigit] = (m_digits[riDigit] >> numBits) | (carry << carryShift);
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

// Based on the BigNum Math's enhanced version of HAC's Algorithm 14.20.
void BigNum::divide( const BigNum & rhs, BigNum & q, BigNum & r )
{
    if( rhs.isZero() )
        throw std::invalid_argument( "Cannot divide by zero." );

    if( compareMagnitude( rhs ) == Comparison::LessThan )
    {
        r = *this;
        q.zero();
        return;
    }

    // Setup the quotient.
    q.grow( m_numDigitsUsed + 2 );
    q.m_numDigitsUsed = m_numDigitsUsed + 2;

    BigNum x( *this );
    x.abs();

    BigNum y( rhs );
    y.abs();

    const bool negative = (m_negative != rhs.m_negative);

    // Normalize inputs. Compute how much we need to shift the divisor by to have its most
    // significant bit in the DigitBits position of its leading digit. Use this amount to
    // shift both our divisor and dividend.
    size_t normShift = y.numberBits() & DigitMask;
    if( normShift < (DigitBits - 1) )
    {
        normShift = DigitBits - 1 - normShift;
        x <<= normShift;
        y <<= normShift;
    }
    else
    {
        normShift = 0;
    }

    // Find the leading digit in the quotient.
    size_t n = x.m_numDigitsUsed - 1;
    size_t t = y.m_numDigitsUsed - 1;

    y.leftDigitShift( n - t );
    while( x.compare( y ) != Comparison::LessThan )
    {
        q.m_digits[n - t] += 1;
        x -= y;
    }
    y.rightDigitShift( n - t );

    // Compute remaining digits of the quotient.
    BigNum temp1;
    BigNum temp2;

    for( size_t iDigit = n; iDigit > t; --iDigit )
    {
        if( iDigit > x.m_numDigitsUsed )
            continue;

        // Estimate the current quotient digit.
        auto & currentQuotientDigit = q.m_digits[iDigit - t - 1];

        if( x.m_digits[iDigit] == y.m_digits[iDigit] )
        {
            currentQuotientDigit = DigitRadix - 1;
        }
        else
        {
            constexpr auto digitMask = static_cast<word_t>(DigitMask);

            word_t r = static_cast<word_t>(x.m_digits[iDigit]) <<
                static_cast<word_t>(DigitBits);

            r |= static_cast<word_t>(x.m_digits[iDigit - 1]);
            r /= static_cast<word_t>(y.m_digits[t]);

            if( r > digitMask )
                r = digitMask;

            currentQuotientDigit = static_cast<digit_t>(r & digitMask);
        }

        currentQuotientDigit = (currentQuotientDigit + 1) & DigitMask;

        // Start to fix the quotient digit estimate.
        do
        {
            currentQuotientDigit = (currentQuotientDigit - 1) & DigitMask;

            temp1.zero();
            temp1.m_digits[0] = (t < 1) ? 0 : y.m_digits[t - 1];
            temp1.m_digits[1] = y.m_digits[t];
            temp1.m_numDigitsUsed = 2;
            temp1 *= currentQuotientDigit;

            temp2.m_digits[0] = (iDigit < 2) ? 0 : x.m_digits[iDigit - 2];
            temp2.m_digits[1] = (iDigit < 1) ? 0 : x.m_digits[iDigit - 1];
            temp2.m_digits[2] = x.m_digits[iDigit];
            temp2.m_numDigitsUsed = 3;
        } while( temp1.compareMagnitude( temp2 ) == Comparison::GreaterThan );

        temp1 = y * currentQuotientDigit;
        temp1.leftDigitShift( iDigit - t - 1 );

        x -= temp1;
        if( x.m_negative )
        {
            temp1 = y;
            temp1.leftDigitShift( iDigit - t - 1 );
            x += temp1;

            constexpr digit_t one = 1;
            currentQuotientDigit = (currentQuotientDigit - one) & DigitMask;
        }
    }

    // Quotient is computed in q, and remainder is in x. Need to set signs, clamp digits, and
    // denormalize results, and set outputs.
    q.clamp();
    q.m_negative = negative;

    x.m_negative = (x.m_numDigitsUsed == 0) ? false : m_negative;
    r = x >> normShift;
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
