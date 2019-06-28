#include "pch.h"
#include "CppUnitTest.h"
#include "../BigNum/BigNum.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace BigNumTests
{
	TEST_CLASS(BigNumTests)
	{
	public:
		
        TEST_METHOD( TestAssignment )
		{
            BigNum a( 4 );
            a = 1;
            Assert::AreEqual( a[0], static_cast<BigNum::digit_t>(1) );
            Assert::AreEqual( a[1], static_cast<BigNum::digit_t>(0) );
            Assert::AreEqual( a[2], static_cast<BigNum::digit_t>(0) );
            Assert::AreEqual( a[3], static_cast<BigNum::digit_t>(0) );
		}

        TEST_METHOD( TestMultiplyByTwo )
		{
            BigNum a( 4 );
            a = 4;
            a.multiplyByTwo();
            Assert::AreEqual( a[0], static_cast<BigNum::digit_t>(8) );
            Assert::AreEqual( a[1], static_cast<BigNum::digit_t>(0) );
            Assert::AreEqual( a[2], static_cast<BigNum::digit_t>(0) );
            Assert::AreEqual( a[3], static_cast<BigNum::digit_t>(0) );
		}

        TEST_METHOD( TestDivideByTwo )
        {
            BigNum a( 4 );
            a = 8;
            a.divideByTwo();
            Assert::AreEqual( a[0], static_cast<BigNum::digit_t>(4) );
            Assert::AreEqual( a[1], static_cast<BigNum::digit_t>(0) );
            Assert::AreEqual( a[2], static_cast<BigNum::digit_t>(0) );
            Assert::AreEqual( a[3], static_cast<BigNum::digit_t>(0) );
        }

        TEST_METHOD( TestLeftDigitShift )
        {
            BigNum a( 4 );
            a = 8;
            a.leftDigitShift( 1 );
            Assert::AreEqual( a[0], static_cast<BigNum::digit_t>(0) );
            Assert::AreEqual( a[1], static_cast<BigNum::digit_t>(8) );
            Assert::AreEqual( a[2], static_cast<BigNum::digit_t>(0) );
            Assert::AreEqual( a[3], static_cast<BigNum::digit_t>(0) );
        }

        TEST_METHOD( TestRightDigitShift )
        {
            BigNum a( 4 );
            a = 8;
            a.leftDigitShift( 1 );
            Assert::AreEqual( a[0], static_cast<BigNum::digit_t>(0) );
            Assert::AreEqual( a[1], static_cast<BigNum::digit_t>(8) );
            Assert::AreEqual( a[2], static_cast<BigNum::digit_t>(0) );
            Assert::AreEqual( a[3], static_cast<BigNum::digit_t>(0) );

            a.rightDigitShift( 1 );
            Assert::AreEqual( a[0], static_cast<BigNum::digit_t>(8) );
            Assert::AreEqual( a[1], static_cast<BigNum::digit_t>(0) );
            Assert::AreEqual( a[2], static_cast<BigNum::digit_t>(0) );
            Assert::AreEqual( a[3], static_cast<BigNum::digit_t>(0) );
        }

        TEST_METHOD( TestLeftShift )
        {
            BigNum a( 4 );
            a = 1;
            a <<= 34;
            Assert::AreEqual( a[0], static_cast<BigNum::digit_t>(0) );
            Assert::AreEqual( a[1], static_cast<BigNum::digit_t>(8) );
            Assert::AreEqual( a[2], static_cast<BigNum::digit_t>(0) );
            Assert::AreEqual( a[3], static_cast<BigNum::digit_t>(0) );
        }

        TEST_METHOD( TestRightShift )
        {
            BigNum a( 4 );
            a = 1;
            a <<= 34;
            Assert::AreEqual( a[0], static_cast<BigNum::digit_t>(0) );
            Assert::AreEqual( a[1], static_cast<BigNum::digit_t>(8) );
            Assert::AreEqual( a[2], static_cast<BigNum::digit_t>(0) );
            Assert::AreEqual( a[3], static_cast<BigNum::digit_t>(0) );

            a >>= 32;
            Assert::AreEqual( a[0], static_cast<BigNum::digit_t>(4) );
            Assert::AreEqual( a[1], static_cast<BigNum::digit_t>(0) );
            Assert::AreEqual( a[2], static_cast<BigNum::digit_t>(0) );
            Assert::AreEqual( a[3], static_cast<BigNum::digit_t>(0) );
        }

        TEST_METHOD( TestMod2B )
        {
            BigNum a( 4 );
            a = 1;

            BigNum b( 4 );
            b = 1;
            b <<= 31;

            BigNum c( 4 );
            c = a + b;
            Assert::AreEqual( c[0], static_cast<BigNum::digit_t>(1) );
            Assert::AreEqual( c[1], static_cast<BigNum::digit_t>(1) );
            Assert::AreEqual( c[2], static_cast<BigNum::digit_t>(0) );
            Assert::AreEqual( c[3], static_cast<BigNum::digit_t>(0) );

            c.mod2b( 31 );
            Assert::AreEqual( c[0], static_cast<BigNum::digit_t>(1) );
            Assert::AreEqual( c[1], static_cast<BigNum::digit_t>(0) );
            Assert::AreEqual( c[2], static_cast<BigNum::digit_t>(0) );
            Assert::AreEqual( c[3], static_cast<BigNum::digit_t>(0) );
        }

        TEST_METHOD( TestSimpleMultiply )
        {
            BigNum a( 4 );
            a = 2;

            BigNum b( 4 );
            b = 2;

            a *= b;
            Assert::AreEqual( a[0], static_cast<BigNum::digit_t>(4) );
            Assert::AreEqual( a[1], static_cast<BigNum::digit_t>(0) );
            Assert::AreEqual( a[2], static_cast<BigNum::digit_t>(0) );
            Assert::AreEqual( a[3], static_cast<BigNum::digit_t>(0) );
        }

        TEST_METHOD( TestMultiDigitMultiply )
        {
            BigNum a( 4 );
            a = 2;

            BigNum b( 4 );
            b = 2;
            b <<= 31;
            Assert::AreEqual( b[0], static_cast<BigNum::digit_t>(0) );
            Assert::AreEqual( b[1], static_cast<BigNum::digit_t>(2) );
            Assert::AreEqual( b[2], static_cast<BigNum::digit_t>(0) );
            Assert::AreEqual( b[3], static_cast<BigNum::digit_t>(0) );

            a *= b;
            Assert::AreEqual( a[0], static_cast<BigNum::digit_t>(0) );
            Assert::AreEqual( a[1], static_cast<BigNum::digit_t>(4) );
            Assert::AreEqual( a[2], static_cast<BigNum::digit_t>(0) );
            Assert::AreEqual( a[3], static_cast<BigNum::digit_t>(0) );
        }
	};
}
