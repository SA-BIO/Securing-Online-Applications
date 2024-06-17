//HARAKA PAPER:   https://eprint.iacr.org/2016/098.pdf
//CODE FROM:      https://github.com/kste/haraka

#pragma once
#include "utils.h"

#define NUMROUNDS 5
u128 rc[40];

#define AES2(s0, s1, rci) \
  s0 = _mm_aesenc_si128(s0, rc[rci]); \
  s1 = _mm_aesenc_si128(s1, rc[rci + 1]); \
  s0 = _mm_aesenc_si128(s0, rc[rci + 2]); \
  s1 = _mm_aesenc_si128(s1, rc[rci + 3]);

#define AES2_4x(s0, s1, s2, s3, rci) \
  AES2(s0[0], s0[1], rci); \
  AES2(s1[0], s1[1], rci); \
  AES2(s2[0], s2[1], rci); \
  AES2(s3[0], s3[1], rci);

#define AES2_8x(s0, s1, s2, s3, s4, s5, s6, s7, rci) \
  AES2_4x(s0, s1, s2, s3, rci); \
  AES2_4x(s4, s5, s6, s7, rci);

#define AES4(s0, s1, s2, s3, rci) \
  s0 = _mm_aesenc_si128(s0, rc[rci]); \
  s1 = _mm_aesenc_si128(s1, rc[rci + 1]); \
  s2 = _mm_aesenc_si128(s2, rc[rci + 2]); \
  s3 = _mm_aesenc_si128(s3, rc[rci + 3]); \
  s0 = _mm_aesenc_si128(s0, rc[rci + 4]); \
  s1 = _mm_aesenc_si128(s1, rc[rci + 5]); \
  s2 = _mm_aesenc_si128(s2, rc[rci + 6]); \
  s3 = _mm_aesenc_si128(s3, rc[rci + 7]); \

#define AES4_4x(s0, s1, s2, s3, rci) \
  AES4(s0[0], s0[1], s0[2], s0[3], rci); \
  AES4(s1[0], s1[1], s1[2], s1[3], rci); \
  AES4(s2[0], s2[1], s2[2], s2[3], rci); \
  AES4(s3[0], s3[1], s3[2], s3[3], rci);

#define AES4_8x(s0, s1, s2, s3, s4, s5, s6, s7, rci) \
  AES4_4x(s0, s1, s2, s3, rci); \
  AES4_4x(s4, s5, s6, s7, rci);

#define MIX2(s0, s1) \
  tmp = _mm_unpacklo_epi32(s0, s1); \
  s1 = _mm_unpackhi_epi32(s0, s1); \
  s0 = tmp;

#define MIX4(s0, s1, s2, s3) \
  tmp  = _mm_unpacklo_epi32(s0, s1); \
  s0 = _mm_unpackhi_epi32(s0, s1); \
  s1 = _mm_unpacklo_epi32(s2, s3); \
  s2 = _mm_unpackhi_epi32(s2, s3); \
  s3 = _mm_unpacklo_epi32(s0, s2); \
  s0 = _mm_unpackhi_epi32(s0, s2); \
  s2 = _mm_unpackhi_epi32(s1, tmp); \
  s1 = _mm_unpacklo_epi32(s1, tmp);

#define TRUNCSTORE(out, s0, s1, s2, s3) \
  *(u64*)(out) = (u64*)(s0)[1]; \
  *(u64*)(out + 8) = (u64*)(s1)[1]; \
  *(u64*)(out + 16) = (u64*)(s2)[0]; \
  *(u64*)(out + 24) = (u64*)(s3)[0];

void load_haraka_constants();

inline void haraka256(byte in[32], byte out[32]) {
	u128 s[2], tmp;

	s[0] = LOAD_BYTES(in);
	s[1] = LOAD_BYTES(in + 16);

	AES2(s[0], s[1], 0);
	MIX2(s[0], s[1]);

	AES2(s[0], s[1], 4);
	MIX2(s[0], s[1]);

	AES2(s[0], s[1], 8);
	MIX2(s[0], s[1]);

	AES2(s[0], s[1], 12);
	MIX2(s[0], s[1]);

	AES2(s[0], s[1], 16);
	MIX2(s[0], s[1]);

	s[0] = _mm_xor_si128(s[0], LOAD_BYTES(in));
	s[1] = _mm_xor_si128(s[1], LOAD_BYTES(in + 16));
	
	STORE_BYTES(out, s[0]);
	STORE_BYTES(out + 16, s[1]);
}

inline void haraka256_4x(byte in[128], byte out[128]) {
	u128 s[4][2], tmp;

	s[0][0] = LOAD_BYTES(in);
	s[0][1] = LOAD_BYTES(in + 16);
	s[1][0] = LOAD_BYTES(in + 32);
	s[1][1] = LOAD_BYTES(in + 48);
	s[2][0] = LOAD_BYTES(in + 64);
	s[2][1] = LOAD_BYTES(in + 80);
	s[3][0] = LOAD_BYTES(in + 96);
	s[3][1] = LOAD_BYTES(in + 112);

	AES2_4x(s[0], s[1], s[2], s[3], 0);

	MIX2(s[0][0], s[0][1]);
	MIX2(s[1][0], s[1][1]);
	MIX2(s[2][0], s[2][1]);
	MIX2(s[3][0], s[3][1]);

	AES2_4x(s[0], s[1], s[2], s[3], 4);

	MIX2(s[0][0], s[0][1]);
	MIX2(s[1][0], s[1][1]);
	MIX2(s[2][0], s[2][1]);
	MIX2(s[3][0], s[3][1]);

	AES2_4x(s[0], s[1], s[2], s[3], 8);

	MIX2(s[0][0], s[0][1]);
	MIX2(s[1][0], s[1][1]);
	MIX2(s[2][0], s[2][1]);
	MIX2(s[3][0], s[3][1]);

	AES2_4x(s[0], s[1], s[2], s[3], 12);

	MIX2(s[0][0], s[0][1]);
	MIX2(s[1][0], s[1][1]);
	MIX2(s[2][0], s[2][1]);
	MIX2(s[3][0], s[3][1]);

	AES2_4x(s[0], s[1], s[2], s[3], 16);

	MIX2(s[0][0], s[0][1]);
	MIX2(s[1][0], s[1][1]);
	MIX2(s[2][0], s[2][1]);
	MIX2(s[3][0], s[3][1]);

	s[0][0] = _mm_xor_si128(s[0][0], LOAD_BYTES(in));
	s[0][1] = _mm_xor_si128(s[0][1], LOAD_BYTES(in + 16));
	s[1][0] = _mm_xor_si128(s[1][0], LOAD_BYTES(in + 32));
	s[1][1] = _mm_xor_si128(s[1][1], LOAD_BYTES(in + 48));
	s[2][0] = _mm_xor_si128(s[2][0], LOAD_BYTES(in + 64));
	s[2][1] = _mm_xor_si128(s[2][1], LOAD_BYTES(in + 80));
	s[3][0] = _mm_xor_si128(s[3][0], LOAD_BYTES(in + 96));
	s[3][1] = _mm_xor_si128(s[3][1], LOAD_BYTES(in + 112));

	STORE_BYTES(out, s[0][0]);
	STORE_BYTES(out + 16, s[0][1]);
	STORE_BYTES(out + 32, s[1][0]);
	STORE_BYTES(out + 48, s[1][1]);
	STORE_BYTES(out + 64, s[2][0]);
	STORE_BYTES(out + 80, s[2][1]);
	STORE_BYTES(out + 96, s[3][0]);
	STORE_BYTES(out + 112, s[3][1]);
}
