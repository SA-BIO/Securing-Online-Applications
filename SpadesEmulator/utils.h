#pragma once
#include "immintrin.h"

#define BYTE_SIZE 8
typedef unsigned char byte;
typedef unsigned int uint;
typedef __m128i u128;
typedef unsigned long u64;
typedef long long tLong;

//DEPENDENT VARS:
#define CARD_SIZE 6      //for base Spades: 6
#define CARD_NUM 13      //for base Spades: 13
#define TURN_NUM 13      //for base Sapdes: 13

#define DECK_SIZE 52	//Standart deck

#define LOAD_BYTES(src) _mm_load_si128((u128 *)(src))
#define STORE_BYTES(dest,src) _mm_storeu_si128((u128 *)(dest),src)