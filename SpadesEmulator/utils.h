#pragma once
#include "immintrin.h"

#define BYTE_SIZE 8
typedef unsigned char byte;
typedef unsigned int uint;
typedef __m128i u128;
typedef unsigned long u64;
typedef long long tLong;

#define P_NUM 2                                         //for base Spades 2
#define CARD_SIZE_BYTES 1                               //for base Spades 6 bits wich rounds to 1 byte
#define CARD_SIZE_BITS (BYTE_SIZE * CARD_SIZE_BYTES)
#define CARD_NUM 13                                    //for base Spades: 13
#define TURN_NUM 13                                   //for base Sapdes: 13
#define DECK_SIZE ((CARD_NUM < 52) ? 52 : CARD_NUM)	    //Standart deck of 52 for base Spades (or more if we need to draw more than 52 cards)


#define LOAD_BYTES(src) _mm_load_si128((u128 *)(src))
#define STORE_BYTES(dest,src) _mm_storeu_si128((u128 *)(dest),src)

constexpr int ceilNum(float num) {
    int inum = static_cast<int>(num);
    return inum + (num != inum);
}
constexpr int divUp(int a, int b) {
    return ceilNum(static_cast<float>(a) / b);
}
// What is the minimum byte amount to store the necessary bit number
constexpr int bitsToBytes(int bitNum) {
    return divUp(bitNum, BYTE_SIZE);
}
