#ifndef __BIG_NUM_H__
#define __BIG_NUM_H__

#include <climits>
#include <vector>

enum class Comparison
{
    LessThan,
    GreaterThan,
    Equal
};

// Based on multi-precision algorithms described in the book BigNum Math by Tom St. Denis.
class BigNum
{
public:
    typedef uint32_t digit_t;
    typedef uint64_t word_t;

    struct biterator
    {
        explicit biterator( const BigNum & number );

        bool hasBits() const { return m_currentBitInDigitMask > 0; }

        digit_t nextBit();

    private:
        const BigNum & m_number;
        digit_t m_currentBitInDigitMask;
        size_t m_iCurrentDigit;
    };

public:
    BigNum();
    explicit BigNum( size_t capacity );

    digit_t & operator[]( std::size_t iDigit ) { return m_digits[iDigit]; }
    const digit_t & operator[]( std::size_t iDigit ) const { return m_digits[iDigit]; }

    size_t numberDigits() const { return m_numDigitsUsed;  }
    digit_t getDigit( size_t iDigit ) const { return m_digits[iDigit]; }

    size_t numberBits() const;

    biterator createBiterator() const { return biterator( *this ); }

    void grow( size_t newCapacity );
    void clamp();
    void zero();

    void loadBytes( const uint8_t * bytes, size_t count, bool preZero = true );
    void storeBytes( uint8_t * bytes, size_t count );

    bool isZero() const { return m_numDigitsUsed == 0; }
    bool isEven() const { return isZero() || (m_digits[0] & 1) == 0; }
    bool isOdd() const { return !isEven(); }

    Comparison compareMagnitude( const BigNum & other ) const;
    Comparison compare( const BigNum & other ) const;

    BigNum & abs();
    BigNum & negate();
    BigNum & multiplyByTwo();
    BigNum & divideByTwo();
    
    BigNum & leftDigitShift( size_t numDigits );
    BigNum & rightDigitShift( size_t numDigits );

    BigNum & mod( const BigNum & modulus );
    BigNum & mod2b( size_t b );

    BigNum & operator=( const BigNum & other );
    BigNum & operator=( digit_t value );

    BigNum & operator+=( const BigNum & rhs );
    friend BigNum operator+( BigNum lhs, const BigNum & rhs )
    {
        lhs += rhs;
        return lhs;
    }

    BigNum & operator-=( const BigNum & rhs );
    friend BigNum operator-( BigNum lhs, const BigNum & rhs )
    {
        lhs -= rhs;
        return lhs;
    }

    BigNum & operator*=( const BigNum & rhs );
    friend BigNum operator*( BigNum lhs, const BigNum & rhs )
    {
        lhs *= rhs;
        return lhs;
    }

    BigNum & operator*=( digit_t rhs );
    friend BigNum operator*( BigNum lhs, digit_t rhs )
    {
        lhs *= rhs;
        return lhs;
    }

    BigNum & operator/=( const BigNum & rhs );
    friend BigNum operator/( BigNum lhs, const BigNum & rhs )
    {
        lhs /= rhs;
        return lhs;
    }

    BigNum & operator<<=( size_t numBits );
    friend BigNum operator<<( BigNum lhs, size_t numBits )
    {
        lhs <<= numBits;
        return lhs;
    }

    BigNum & operator>>=( size_t numBits );
    friend BigNum operator>>( BigNum lhs, size_t numBits )
    {
        lhs >>= numBits;
        return lhs;
    }

private:
    BigNum & unsignedAddEquals( const BigNum & rhs );
    friend BigNum unsignedAdd( BigNum lhs, const BigNum & rhs )
    {
        lhs.unsignedAddEquals( rhs );
        return lhs;
    }

    BigNum & unsignedSubtractEquals( const BigNum & rhs );
    friend BigNum unsignedSubtract( BigNum lhs, const BigNum & rhs )
    {
        lhs.unsignedSubtractEquals( rhs );
        return lhs;
    }

    BigNum & baselineMultiply( const BigNum & rhs, size_t numDigits );

    void divide( const BigNum & rhs, BigNum & q, BigNum & r );

private:
    bool m_negative;
    size_t m_numDigitsUsed;
    std::vector<digit_t> m_digits;
};

constexpr  BigNum::digit_t DigitOne = static_cast<BigNum::digit_t>(1);

// Number of bits in a digit contributing to the value of that digit. If this value is x, then the
// radix of a BigNum is 2^x .
constexpr BigNum::digit_t DigitBits = 31;

// Radix for a digit. This is 2^DigitBits.
constexpr BigNum::digit_t DigitRadix = DigitOne << DigitBits;

// Compute a bit mask that will extract all DigitBits number of bits in a single digit.
constexpr BigNum::digit_t DigitMask = DigitRadix - DigitOne;

// Number of bits in a single precision digit.
constexpr BigNum::digit_t DigitBitSize = CHAR_BIT * sizeof( BigNum::digit_t );

BigNum abs( const BigNum & x );
BigNum negate( const BigNum & x );
BigNum multiplyByTwo( const BigNum & x );
BigNum divideByTwo( const BigNum & x );
BigNum leftDigitShift( const BigNum & x, size_t numDigits );
BigNum rightDigitShift( const BigNum & x, size_t numDigits );
BigNum mod2b( const BigNum & x, size_t b );

#endif
