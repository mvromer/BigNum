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
		}

        TEST_METHOD( TestDivideByTwo )
        {
            BigNum a( 4 );
            a = 8;
            a.divideByTwo();
            Assert::AreEqual( a[0], static_cast<BigNum::digit_t>(4) );
        }
	};
}
