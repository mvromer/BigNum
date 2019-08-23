#include <algorithm>
#include <cstdint>
#include <vector>

namespace
{

constexpr size_t NumberStateColumns = 4;
constexpr size_t NumberStateRows = 4;
constexpr size_t NumberStateBytes = NumberStateColumns * NumberStateRows;

constexpr size_t NumberRounds = 14;
constexpr size_t NumberKeyWords = 8;
constexpr size_t NumberRoundKeysInWords = NumberStateColumns * (NumberRounds + 1);

constexpr uint8_t SBox[]= {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
};

constexpr uint8_t InvSBox[] = {
    0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
    0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
    0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
    0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
    0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
    0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
    0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
    0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
    0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
    0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
    0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
    0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
    0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
    0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
    0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
    0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d
};

// This will be accessed as 7 4-byte words.
// Written this way so that bytes are packed in the correct order.
constexpr uint8_t Rcon[] = {
    0x01, 0x00, 0x00, 0x00,
    0x02, 0x00, 0x00, 0x00,
    0x04, 0x00, 0x00, 0x00,
    0x08, 0x00, 0x00, 0x00,
    0x10, 0x00, 0x00, 0x00,
    0x20, 0x00, 0x00, 0x00,
    0x40, 0x00, 0x00, 0x00
};

inline uint8_t & GetStateByte( uint8_t * state, size_t row, size_t col )
{
    return state[(col * NumberStateRows) + row];
}

inline uint8_t xtime( uint8_t b )
{
    // Bit twiddling fun: Check if high bit on b is set, shift it to least significant bit, and invert.
    // If high bit is set, then this will set all bits to 1 except the least significant bit. Adding 1
    // then sets all bits to one, effectively XORing our shifted value of b with our reducer.
    //
    // If the high bit is not set, then after shifting and inverting, all bits will be set. Adding 1
    // overflows and sets all bits to 0, effectively XORing the shifted value of b with 0, which is
    // the identity transform under XOR.
    //
    constexpr uint8_t reducer = 0x1b;
    return (b << 1) ^ (reducer & (~((b & 0x80) >> 7) + 1));
}

void SubBytes( uint8_t * state )
{
    state[0] = SBox[state[0]];
    state[1] = SBox[state[1]];
    state[2] = SBox[state[2]];
    state[3] = SBox[state[3]];
    state[4] = SBox[state[4]];
    state[5] = SBox[state[5]];
    state[6] = SBox[state[6]];
    state[7] = SBox[state[7]];
    state[8] = SBox[state[8]];
    state[9] = SBox[state[9]];
    state[10] = SBox[state[10]];
    state[11] = SBox[state[11]];
    state[12] = SBox[state[12]];
    state[13] = SBox[state[13]];
    state[14] = SBox[state[14]];
    state[15] = SBox[state[15]];
}

inline void SubWord( uint8_t * word )
{
    word[0] = SBox[word[0]];
    word[1] = SBox[word[1]];
    word[2] = SBox[word[2]];
    word[3] = SBox[word[3]];
}

inline void RotWord( uint8_t * word )
{
    const uint8_t temp = word[0];
    word[0] = word[1];
    word[1] = word[2];
    word[2] = word[3];
    word[3] = temp;
}

void ShiftRows( uint8_t * state )
{
    // Shift second row. First row is skipped.
    uint8_t temp = GetStateByte( state, 1, 0 );
    GetStateByte( state, 1, 0 ) = GetStateByte( state, 1, 1 );
    GetStateByte( state, 1, 1 ) = GetStateByte( state, 1, 2 );
    GetStateByte( state, 1, 2 ) = GetStateByte( state, 1, 3 );
    GetStateByte( state, 1, 3 ) = temp;

    // Shift third row.
    temp = GetStateByte( state, 2, 0 );
    GetStateByte( state, 2, 0 ) = GetStateByte( state, 2, 2 );
    GetStateByte( state, 2, 2 ) = temp;

    temp = GetStateByte( state, 2, 1 );
    GetStateByte( state, 2, 1 ) = GetStateByte( state, 2, 3 );
    GetStateByte( state, 2, 3 ) = temp;

    // Shift fourth row.
    temp = GetStateByte( state, 3, 3 );
    GetStateByte( state, 3, 3 ) = GetStateByte( state, 3, 2 );
    GetStateByte( state, 3, 2 ) = GetStateByte( state, 3, 1 );
    GetStateByte( state, 3, 1 ) = GetStateByte( state, 3, 0 );
    GetStateByte( state, 3, 0 ) = temp;
}

void MixColumns( uint8_t * state )
{
    for( size_t iCol = 0; iCol < NumberStateColumns; ++iCol )
    {
        uint8_t * stateCol = state + iCol * NumberStateRows;
        const uint8_t s0x = xtime( stateCol[0] );
        const uint8_t s1x = xtime( stateCol[1] );
        const uint8_t s2x = xtime( stateCol[2] );
        const uint8_t s3x = xtime( stateCol[3] );

        const uint8_t s0 = s0x ^ stateCol[1] ^ s1x ^ stateCol[2] ^ stateCol[3];
        const uint8_t s1 = stateCol[0] ^ s1x ^ stateCol[2] ^ s2x ^ stateCol[3];
        const uint8_t s2 = stateCol[0] ^ stateCol[1] ^ s2x ^ stateCol[3] ^ s3x;
        const uint8_t s3 = stateCol[0] ^ s0x ^ stateCol[1] ^ stateCol[2] ^ s3x;

        stateCol[0] = s0;
        stateCol[1] = s1;
        stateCol[2] = s2;
        stateCol[3] = s3;
    }
}

inline void AddRoundKey( uint32_t * state, uint32_t * roundKey )
{
    state[0] ^= roundKey[0];
    state[1] ^= roundKey[1];
    state[2] ^= roundKey[2];
    state[3] ^= roundKey[3];
}

void GenerateRoundKeys( const uint8_t * key, uint32_t * roundKeys )
{
    const uint32_t * RconWords = reinterpret_cast<const uint32_t *>(Rcon);

    size_t iWord = 0;
    uint8_t * roundKeyBytes = reinterpret_cast<uint8_t *>(roundKeys);

    for( iWord = 0; iWord < NumberKeyWords; ++iWord )
    {
        const size_t iKeyBase = 4 * iWord;
        roundKeyBytes[iKeyBase] = key[iKeyBase];
        roundKeyBytes[iKeyBase + 1] = key[iKeyBase + 1];
        roundKeyBytes[iKeyBase + 2] = key[iKeyBase + 2];
        roundKeyBytes[iKeyBase + 3] = key[iKeyBase + 3];
    }

    for( ; iWord < NumberRoundKeysInWords; ++iWord )
    {
        uint32_t temp = roundKeys[iWord - 1];

        if( iWord % NumberKeyWords == 0 )
        {
            RotWord( reinterpret_cast<uint8_t *>(&temp) );
            SubWord( reinterpret_cast<uint8_t *>(&temp) );
            temp ^= RconWords[iWord / NumberKeyWords - 1];
        }
        else if( iWord % NumberKeyWords == 4 )
        {
            SubWord( reinterpret_cast<uint8_t *>(&temp) );
        }

        roundKeys[iWord] = roundKeys[iWord - NumberKeyWords] ^ temp;
    }
}

void aesEncryptBlock( const uint8_t * counter, uint8_t * output, uint32_t * roundKeys )
{
    std::copy( counter, counter + NumberStateBytes, output );

    AddRoundKey( reinterpret_cast<uint32_t *>(output), roundKeys );

    size_t iRound;
    for( iRound = 1; iRound < NumberRounds; ++iRound )
    {
        SubBytes( output );
        ShiftRows( output );
        MixColumns( output );
        AddRoundKey( reinterpret_cast<uint32_t *>(output),
            roundKeys + (iRound * NumberStateColumns) );
    }

    SubBytes( output );
    ShiftRows( output );
    AddRoundKey( reinterpret_cast<uint32_t *>(output),
        roundKeys + (iRound * NumberStateColumns) );
}

void incrementCounter( std::vector<uint8_t> & counter )
{
    uint8_t carry = 1;

    for( auto riValue = counter.rbegin(); riValue != counter.rend(); ++riValue )
    {
        uint16_t sum = *riValue + carry;
        *riValue = static_cast<uint8_t>(sum & 0xFF);
        carry = static_cast<uint8_t>((sum >> 8) & 0xFF);
    }
}

}

void aesEncrypt( const uint8_t * input, size_t inputLength,
    const uint8_t * counter, const uint8_t * key, uint8_t * output )
{
    std::vector<uint8_t> currentCounter( counter, counter + NumberStateBytes );
    std::vector<uint8_t> roundKeys( NumberRoundKeysInWords * sizeof(uint32_t) );
    const auto roundKeyAsWords = reinterpret_cast<uint32_t *>(roundKeys.data());
    GenerateRoundKeys( key, roundKeyAsWords );

    size_t bytesEncrypted;
    for( bytesEncrypted = 0; (bytesEncrypted + NumberStateBytes) <= inputLength; bytesEncrypted += NumberStateBytes )
    {
        aesEncryptBlock( currentCounter.data(), output + bytesEncrypted, roundKeyAsWords );

        for( size_t iByte = 0; iByte < NumberStateBytes; ++iByte )
            output[bytesEncrypted + iByte] ^= input[bytesEncrypted + iByte];

        incrementCounter( currentCounter );
    }

    if( bytesEncrypted < inputLength )
    {
        aesEncryptBlock( currentCounter.data(), output + bytesEncrypted, roundKeyAsWords );

        for( ; bytesEncrypted < inputLength; ++bytesEncrypted )
            output[bytesEncrypted] ^= input[bytesEncrypted];
    }
}