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

public:
    BigNum( size_t capacity );

    digit_t & operator[]( std::size_t iDigit ) { return m_digits[iDigit]; }
    const digit_t & operator[]( std::size_t iDigit ) const { return m_digits[iDigit]; }

    void grow( size_t newCapacity );
    void clamp();
    void zero();

    bool isZero() const { return m_numDigitsUsed == 0; }
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

private:
    bool m_negative;
    size_t m_numDigitsUsed;
    std::vector<digit_t> m_digits;
};

BigNum abs( const BigNum & x );
BigNum negate( const BigNum & x );
BigNum multiplyByTwo( const BigNum & x );
BigNum divideByTwo( const BigNum & x );
BigNum leftDigitShift( const BigNum & x, size_t numDigits );
BigNum rightDigitShift( const BigNum & x, size_t numDigits );
BigNum mod2b( const BigNum & x, size_t b );

#endif
