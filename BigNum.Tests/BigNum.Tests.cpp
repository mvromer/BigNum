#include "pch.h"
#include "CppUnitTest.h"
#include "../BigNum/BigNum.h"
#include "../BigNum/RsaMath.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace BigNumTests
{
	TEST_CLASS(BigNumTests)
	{
	public:
		
        TEST_METHOD( TestVectorConstruction )
		{
            BigNum a( std::vector<uint8_t>{ 4 } );
            Assert::IsTrue( a.numberDigits() == 1 );
            Assert::IsTrue( a.getDigit( 0 ) == 4 );
            Assert::IsFalse( a.isNegative() );
		}

        TEST_METHOD( TestScalarAssignment )
        {
            BigNum a;
            a = 4;
            Assert::IsTrue( a.numberDigits() == 1 );
            Assert::IsTrue( a.getDigit( 0 ) == 4 );
            Assert::IsFalse( a.isNegative() );
        }

        TEST_METHOD( TestMultiplyByTwo )
		{
            BigNum a( std::vector<uint8_t>{ 4 } );
            a.multiplyByTwo();
            Assert::IsTrue( a.numberDigits() == 1 );
            Assert::IsTrue( a.getDigit( 0 ) == 8 );
            Assert::IsFalse( a.isNegative() );
		}

        TEST_METHOD( TestDivideByTwo )
        {
            BigNum a( std::vector<uint8_t>{ 8 } );
            a = 8;
            a.divideByTwo();
            Assert::IsTrue( a.numberDigits() == 1 );
            Assert::IsTrue( a.getDigit( 0 ) == 4 );
            Assert::IsFalse( a.isNegative() );
        }

        TEST_METHOD( TestLeftDigitShift )
        {
            BigNum a( std::vector<uint8_t>{ 8 } );
            a.leftDigitShift( 1 );
            Assert::IsTrue( a.numberDigits() == 2 );
            Assert::IsTrue( a.getDigit( 0 ) == 0 );
            Assert::IsTrue( a.getDigit( 1 ) == 8 );
            Assert::IsFalse( a.isNegative() );
        }

        TEST_METHOD( TestRightDigitShift )
        {
            BigNum a( std::vector<uint8_t>{ 8 } );
            a.leftDigitShift( 1 );
            Assert::IsTrue( a.numberDigits() == 2 );
            Assert::IsTrue( a.getDigit( 0 ) == 0 );
            Assert::IsTrue( a.getDigit( 1 ) == 8 );
            Assert::IsFalse( a.isNegative() );

            a.rightDigitShift( 1 );
            Assert::IsTrue( a.numberDigits() == 1 );
            Assert::IsTrue( a.getDigit( 0 ) == 8 );
            Assert::IsFalse( a.isNegative() );
        }

        TEST_METHOD( TestLeftShift )
        {
            BigNum a( std::vector<uint8_t>{ 1 } );
            a <<= 34;
            Assert::IsTrue( a.numberDigits() == 2 );
            Assert::IsTrue( a.getDigit( 0 ) == 0 );
            Assert::IsTrue( a.getDigit( 1 ) == 8 );
            Assert::IsFalse( a.isNegative() );
        }

        TEST_METHOD( TestRightShift )
        {
            BigNum a( std::vector<uint8_t>{ 1 } );
            a <<= 34;
            Assert::IsTrue( a.numberDigits() == 2 );
            Assert::IsTrue( a.getDigit( 0 ) == 0 );
            Assert::IsTrue( a.getDigit( 1 ) == 8 );
            Assert::IsFalse( a.isNegative() );

            a >>= 32;
            Assert::IsTrue( a.numberDigits() == 1 );
            Assert::IsTrue( a.getDigit( 0 ) == 4 );
            Assert::IsFalse( a.isNegative() );
        }

        TEST_METHOD( TestMod2B )
        {
            const BigNum a( std::vector<uint8_t>{ 1 } );
            BigNum b( std::vector<uint8_t>{ 1 } );
            b <<= 31;

            BigNum c = a + b;
            Assert::IsTrue( c.numberDigits() == 2 );
            Assert::IsTrue( c.getDigit( 0 ) == 1 );
            Assert::IsTrue( c.getDigit( 1 ) == 1 );
            Assert::IsFalse( c.isNegative() );

            c.mod2b( 31 );
            Assert::IsTrue( c.numberDigits() == 1 );
            Assert::IsTrue( c.getDigit( 0 ) == 1 );
            Assert::IsFalse( c.isNegative() );
        }

        TEST_METHOD( TestSimpleMultiply )
        {
            BigNum a( std::vector<uint8_t>{ 2 } );
            const BigNum b( std::vector<uint8_t>{ 2 } );

            a *= b;
            Assert::IsTrue( a.numberDigits() == 1 );
            Assert::IsTrue( a.getDigit( 0 ) == 4 );
            Assert::IsFalse( a.isNegative() );
        }

        TEST_METHOD( TestMultiDigitMultiply )
        {
            BigNum a( std::vector<uint8_t>{ 2 } );
            BigNum b( std::vector<uint8_t>{ 2 } );
            b <<= 31;

            Assert::IsTrue( b.numberDigits() == 2 );
            Assert::IsTrue( b.getDigit( 0 ) == 0 );
            Assert::IsTrue( b.getDigit( 1 ) == 2 );
            Assert::IsFalse( b.isNegative() );

            a *= b;
            Assert::IsTrue( a.numberDigits() == 2 );
            Assert::IsTrue( a.getDigit( 0 ) == 0 );
            Assert::IsTrue( a.getDigit( 1 ) == 4 );
            Assert::IsFalse( a.isNegative() );
        }

        TEST_METHOD( TestMontgomeryInverse )
        {
            const BigNum::digit_t expected = 1108378657;
            const BigNum a( std::vector<uint8_t>{ 31 } );
            const BigNum::digit_t actual = compute_montgomery_inverse( a );
            Assert::AreEqual( expected, actual );
        }

        TEST_METHOD( TestSingleDigitMultiply )
        {
            const BigNum expected( std::vector<uint8_t>{ 16 } );
            const BigNum a( std::vector<uint8_t>{ 2 } );
            const BigNum actual = a * 8;
            Assert::IsTrue( expected.compare( actual ) == Comparison::Equal );
        }

        TEST_METHOD( TestMontgomeryMultiply )
        {
            const BigNum x( std::vector<uint8_t>{ 7 } );
            const BigNum y( std::vector<uint8_t>{ 15 } );
            const BigNum m( std::vector<uint8_t>{ 17 } );
            const BigNum::digit_t mInv = compute_montgomery_inverse( m );
            const BigNum actual = montgomery_multiply( x, y, m, mInv );

            // xyR^-1 mod m should be 6 in this case. R = b^l, where l is the number of base-b
            // digits in m. In this case, R = b, thus R^-1 mod m = 2.
            const BigNum expected( std::vector<uint8_t> { 6 } );
            Assert::IsTrue( expected.compare( actual ) == Comparison::Equal );
        }

        TEST_METHOD( TestMod )
        {
            const BigNum m( std::vector<uint8_t>{ 17 } );
            const BigNum expected( std::vector<uint8_t>{ 9 } );
            BigNum actual( std::vector<uint8_t>{ 1 } );
            actual.leftDigitShift( 1 ).mod( m );
            Assert::IsTrue( expected.compare( actual ) == Comparison::Equal );
        }

        TEST_METHOD( TestNumberBits )
        {
            BigNum x( std::vector<uint8_t>{ 1 } );
            x.leftDigitShift( 1 );
            const size_t expected = 32;
            Assert::AreEqual( expected, x.numberBits() );
        }

        TEST_METHOD( TestDivide )
        {
            const BigNum x( std::vector<uint8_t>{ 36 } );
            const BigNum y( std::vector<uint8_t>{ 9 } );
            const BigNum actual = x / y;
            const BigNum expected( std::vector<uint8_t>{ 4 } );
            Assert::IsTrue( expected.compare( actual ) == Comparison::Equal );
        }

        TEST_METHOD( TestBiterator )
        {
            const BigNum x( std::vector<uint8_t>{ 36 } );
            auto bi = x.createBiterator();
            Assert::AreNotEqual( bi.nextBit(), 0u );
            Assert::AreEqual( bi.nextBit(), 0u );
            Assert::AreEqual( bi.nextBit(), 0u );
            Assert::AreNotEqual( bi.nextBit(), 0u );
            Assert::AreEqual( bi.nextBit(), 0u );
            Assert::AreEqual( bi.nextBit(), 0u );
            Assert::IsFalse( bi.hasBits() );
        }

        TEST_METHOD( TestModularExponentiation )
        {
            constexpr bool swizzle = true;

            BigNum::digit_t expectedValue = 445;
            const BigNum expected( reinterpret_cast<uint8_t *>(&expectedValue),
                sizeof( expectedValue ),
                swizzle,
                sizeof( BigNum::digit_t ) );

            BigNum::digit_t nValue = 497;
            const BigNum n( reinterpret_cast<uint8_t *>(&nValue),
                sizeof( nValue ),
                swizzle,
                sizeof( BigNum::digit_t ) );

            const BigNum b( std::vector<uint8_t>{ 4 } );
            const BigNum e( std::vector<uint8_t>{ 13 } );
            BigNum r( std::vector<uint8_t>{ 1 } );
            r.leftDigitShift( n.numberDigits() ).mod( n );

            BigNum r2 = r * r;
            r2.mod( n );

            const BigNum::digit_t mInv = compute_montgomery_inverse( n );
            const BigNum actual( montgomery_exponentiation( b, e, n, mInv, r, r2 ) );
            Assert::IsTrue( expected.compare( actual ) == Comparison::Equal );
        }

        TEST_METHOD( TestRsa )
        {
            constexpr bool swizzle = true;

            const BigNum m( std::vector<uint8_t>{ 65 } );
            const BigNum e( std::vector<uint8_t>{ 17 } );

            BigNum::digit_t dValue = 413;
            const BigNum d( reinterpret_cast<uint8_t *>(&dValue),
                sizeof( dValue ),
                swizzle,
                sizeof( BigNum::digit_t ) );

            BigNum::digit_t nValue = 3233;
            const BigNum n( reinterpret_cast<uint8_t *>(&nValue),
                sizeof( nValue ),
                swizzle,
                sizeof( BigNum::digit_t ) );

            BigNum r( std::vector<uint8_t>{ 1 } );
            r.leftDigitShift( n.numberDigits() ).mod( n );

            BigNum r2( r * r );
            r2.mod( n );

            const BigNum::digit_t nInv = compute_montgomery_inverse( n );

            // Encrypt.
            BigNum c = montgomery_exponentiation( m, e, n, nInv, r, r2 );

            // Decrypt.
            BigNum m2 = montgomery_exponentiation( c, d, n, nInv, r, r2 );

            Assert::IsTrue( m.compare( m2 ) == Comparison::Equal );
        }

        TEST_METHOD( TestLoadBytes )
        {
            BigNum a;
            std::vector<uint8_t> bytes{ 8, 0, 0, 0 };
            a.loadBytes( bytes.data(), bytes.size() );

            BigNum b;
            b = 1;
            b <<= 27;

            Assert::IsTrue( a.compare( b ) == Comparison::Equal );
        }

        TEST_METHOD( TestStoreBytes )
        {
            BigNum a;
            std::vector<uint8_t> inputBytes{ 8, 0, 0, 0 };
            a.loadBytes( inputBytes.data(), inputBytes.size() );

            std::vector<uint8_t> outputBytes( inputBytes.size() );
            a.storeBytes( outputBytes.data(), outputBytes.size() );

            Assert::IsTrue( inputBytes == outputBytes );
        }

        TEST_METHOD( TestLoadBytesPreZero )
        {
            BigNum actual;
            std::vector<uint8_t> firstPayload{ 8, 0 };
            std::vector<uint8_t> secondPayload{ 4, 0 };
            constexpr bool preZero = false;
            actual.loadBytes( firstPayload.data(), firstPayload.size() );
            actual.loadBytes( secondPayload.data(), secondPayload.size(), preZero );

            BigNum expected;
            std::vector<uint8_t> totalPayload{ 8, 0, 4, 0 };
            expected.loadBytes( totalPayload.data(), totalPayload.size() );

            Assert::IsTrue( expected.compare( actual ) == Comparison::Equal );
        }

        TEST_METHOD( TestRoundTripSwizzle )
        {
            constexpr bool swizzle = true;

            BigNum::digit_t inputValue = 134217728;
            BigNum input( reinterpret_cast<uint8_t *>(&inputValue),
                sizeof( inputValue ),
                swizzle,
                sizeof( BigNum::digit_t ) );

            Assert::AreEqual( 1u, input.numberDigits() );
            Assert::AreEqual( input.getDigit( 0 ), 134217728u );
            Assert::IsFalse( input.isNegative() );

            BigNum::digit_t outputValue = 0;
            input.storeBytes( reinterpret_cast<uint8_t *>(&outputValue),
                sizeof( outputValue ),
                swizzle,
                sizeof( BigNum::digit_t ) );

            Assert::AreEqual( inputValue, outputValue );
        }

        TEST_METHOD( TestNumberBytes )
        {
            const BigNum a( std::vector<uint8_t>{ 7 } );
            Assert::AreEqual( 3u, a.numberBits() );
            Assert::AreEqual( 1u, a.numberBytes() );

            const BigNum b( std::vector<uint8_t>{ 255 } );
            Assert::AreEqual( 8u, b.numberBits() );
            Assert::AreEqual( 1u, b.numberBytes() );

            const BigNum c( std::vector<uint8_t>{ 1, 3 } );
            Assert::AreEqual( 9u, c.numberBits() );
            Assert::AreEqual( 2u, c.numberBytes() );
        }

        TEST_METHOD( TestRoundTripRsaEncrypt )
        {
            constexpr bool swizzle = true;

            const char plaintext[] = "Hello !";
            const uint64_t modulusValue[] = { 0xe037d35a8b160eb7U,  0xf11919bfef440917U };
            const uint64_t publicExpValue = 65537;
            const uint64_t privateExpValue[] = { 0x00cab10ccaa4437b67U,  0x11c977a277fe00a1U };

            const BigNum modulus( reinterpret_cast<const uint8_t *>(&modulusValue),
                sizeof( modulusValue ),
                swizzle,
                sizeof( uint64_t ) );

            const BigNum publicExp( reinterpret_cast<const uint8_t *>(&publicExpValue),
                sizeof( publicExpValue ),
                swizzle,
                sizeof( uint64_t ) );

            const BigNum privateExp( reinterpret_cast<const uint8_t *>(&privateExpValue),
                sizeof( privateExpValue ),
                swizzle,
                sizeof( uint64_t ) );

            BigNum r( std::vector<uint8_t>{ 1 } );
            r.leftDigitShift( modulus.numberDigits() ).mod( modulus );

            const BigNum r2 = (r * r).mod( modulus );
            const BigNum::digit_t nInv = compute_montgomery_inverse( modulus );

            uint64_t cipher[2];
            rsaEncrypt( reinterpret_cast<const uint8_t *>(plaintext), sizeof( plaintext ),
                reinterpret_cast<uint8_t *>(cipher), sizeof( cipher ),
                modulus, publicExp, nInv, r, r2 );

            char decryptedText[16];
            size_t outputBytesWritten;
            rsaDecrypt( reinterpret_cast<const uint8_t *>(cipher), sizeof( cipher ),
                reinterpret_cast<uint8_t *>(decryptedText), sizeof( decryptedText ), outputBytesWritten,
                modulus, privateExp, nInv, r, r2 );

            Assert::AreEqual( sizeof( plaintext ), outputBytesWritten );
            Assert::AreEqual( std::string( plaintext ), std::string( decryptedText ) );
        }

        TEST_METHOD( TestRightDigitShiftBug )
        {
            const BigNum expected( std::vector<uint8_t>{ 255, 255, 255, 0, 0, 0 } );
            BigNum actual( std::vector<uint8_t>{ 255, 255, 255, 0, 0, 0, 0 } );
            actual >>= 8;
            Assert::IsTrue( expected.compare( actual ) == Comparison::Equal );
        }

        TEST_METHOD( TestRoundTripRsaEncrypt256 )
        {
            const char plaintext[] = ("This a demo of the alternate accelerator interface. "
                "This is a secret message. "
                "This took a long time to get working right.");

            const uint8_t modulusValue[] = {
                0xb8, 0x95, 0x76, 0x2c, 0x77, 0xc2, 0xdb, 0x98, 0x78, 0x46, 0x18, 0x18, 0xed, 0x75, 0x55, 0xfa,
                0xa6, 0xbe, 0x1d, 0xca, 0x8a, 0xe7, 0x5a, 0xb9, 0xf2, 0x13, 0x13, 0xdf, 0x38, 0x69, 0xb7, 0x95
            };
            const uint8_t publicExpValue[] = { 0x01, 0x00, 0x01 };
            const uint8_t privateExpValue[] = {
                0x6d, 0x1f, 0x2e, 0xf5, 0xaa, 0xf7, 0x6f, 0x8a, 0xfb, 0xcf, 0xb4, 0x7f, 0x48, 0x22, 0x8d, 0xe8,
                0xd4, 0x41, 0xce, 0xd1, 0x6d, 0x68, 0x60, 0x19, 0x02, 0x02, 0x5d, 0x69, 0x31, 0xb3, 0x32, 0x01
            };

            const BigNum modulus( modulusValue, sizeof( modulusValue ) );
            const BigNum publicExp( publicExpValue, sizeof( publicExpValue ) );
            const BigNum privateExp( privateExpValue, sizeof( privateExpValue ) );

            BigNum r( std::vector<uint8_t>{ 1 } );
            r.leftDigitShift( modulus.numberDigits() ).mod( modulus );

            const BigNum r2 = (r * r).mod( modulus );
            const BigNum::digit_t nInv = compute_montgomery_inverse( modulus );

            size_t minOutputLength;

            {
                const size_t rsaBitLength = modulus.numberBits();
                const size_t bytesPerInputBlock = (rsaBitLength - 1) / 8;
                const size_t bytesPerOutputBlock = modulus.numberBytes();
                const size_t numInputBlocks = (sizeof(plaintext) / bytesPerInputBlock) + (sizeof(plaintext) % bytesPerInputBlock == 0 ? 0 : 1);
                minOutputLength = numInputBlocks * bytesPerOutputBlock;
            }

            std::vector<uint8_t> cipher( minOutputLength );
            rsaEncrypt( reinterpret_cast<const uint8_t *>(plaintext), sizeof( plaintext ),
                cipher.data(), cipher.size(),
                modulus, publicExp, nInv, r, r2 );

            std::vector<uint8_t> decrypted( cipher.size() );
            size_t outputBytesWritten;
            rsaDecrypt( cipher.data(), cipher.size(),
                decrypted.data(), decrypted.size(), outputBytesWritten,
                modulus, privateExp, nInv, r, r2 );

            Assert::AreEqual( sizeof( plaintext ), outputBytesWritten );
            Assert::AreEqual( std::string( plaintext, sizeof( plaintext ) ),
                std::string( reinterpret_cast<char *>(decrypted.data()), outputBytesWritten ) );
        }
	};
}
