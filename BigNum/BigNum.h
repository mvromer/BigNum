#ifndef __BIG_NUM_H__
#define __BIG_NUM_H__

#include <vector>

enum class Comparison
{
    LessThan,
    GreaterThan,
    Equal
};

// Based on multi-precision algorithms described in the book BigNum Math by Tom St. Denis.
//
// Implementation notes:
//   * m_negative corresponds to sign field in mp_int
//   * dp field in mp_int represented as std::vector<digit_t>
//   * vector's size corresponds to alloc field in mp_int
//   * m_numDigitsUsed corresponds to used field in mp_int
//   * mp_init_size performed via user-defined constructor
//   * mp_init_copy performed via default copy constructor
//   * mp_copy performed via overloaded copy-assignment operator
//   * mp_abs is implemented in two different forms:
//     * Calling the abs member function on a BigNum applies the absolute value function to itself
//     * Calling the abs free function on a BigNum creates a copy of the number, applies the
//       absolute value function to the copy, and returns the updated copy
//   * mp_neg is implemented similar to mp_abs:
//     * Calling the negate member function on a BigNum negates the number itself
//     * Calling the negate free function on a BigNum creates a copy of the number, negates it, and
//       returns the updated copy
//   * mp_set performed via overloaded assignment operator that takes a digit_t parameter.
//
class BigNum
{
public:
    typedef uint32_t digit_t;
    typedef uint64_t word_t;

public:
    BigNum();
    BigNum( size_t capacity );

    digit_t & operator[]( std::size_t iDigit ) { return m_digits[iDigit]; }
    const digit_t & operator[]( std::size_t iDigit ) const { return m_digits[iDigit]; }

    size_t numberDigits() const { return m_digits.size();  }
    digit_t getDigit( size_t iDigit ) const { return m_digits[iDigit]; }

    void grow( size_t newCapacity );
    void clamp();
    void zero();

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

private:
    bool m_negative;
    size_t m_numDigitsUsed;
    std::vector<digit_t> m_digits;
};

constexpr  BigNum::digit_t DigitOne = static_cast<BigNum::digit_t>(1);

// Number of bits in a digit contributing to the value of that digit. If this value is x, then the
// radix of a BigNum is 2^x .
constexpr BigNum::digit_t DigitBits = 31;

// Compute a bit mask that will extract all DigitBits number of bits in a single digit.
constexpr BigNum::digit_t DigitMask = (DigitOne << DigitBits) - DigitOne;

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
