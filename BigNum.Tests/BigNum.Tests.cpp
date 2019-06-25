#include "pch.h"
#include "CppUnitTest.h"
#include "../BigNum/BigNum.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace BigNumTests
{
	TEST_CLASS(BigNumTests)
	{
	public:
		
		TEST_METHOD(TestMethod1)
		{
            BigNum a( 4 );
            a = 1;
            Assert::AreEqual( a[0], static_cast<BigNum::digit_t>(1) );
            Assert::AreEqual( a[1], static_cast<BigNum::digit_t>(0) );
            Assert::AreEqual( a[2], static_cast<BigNum::digit_t>(0) );
            Assert::AreEqual( a[3], static_cast<BigNum::digit_t>(0) );
		}
	};
}
