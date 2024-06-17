//https://gist.github.com/k3it/b740a77db9e823ca7cd11151267d2f76
#include "aes128.h"

#define AES_128_key_exp(k, rcon) aes_128_key_expansion(k, _mm_aeskeygenassist_si128(k, rcon))

static inline u128 aes_128_key_expansion(u128 key, u128 keygened) {
	keygened = _mm_shuffle_epi32(keygened, _MM_SHUFFLE(3, 3, 3, 3));
	key = _mm_xor_si128(key, _mm_slli_si128(key, 4));
	key = _mm_xor_si128(key, _mm_slli_si128(key, 4));
	key = _mm_xor_si128(key, _mm_slli_si128(key, 4));
	return _mm_xor_si128(key, keygened);
}

//public API
void aes128_load_key(u128 enc_key) {
	key_schedule[0] = enc_key;
	key_schedule[1] = AES_128_key_exp(key_schedule[0], 0x01);
	key_schedule[2] = AES_128_key_exp(key_schedule[1], 0x02);
	key_schedule[3] = AES_128_key_exp(key_schedule[2], 0x04);
	key_schedule[4] = AES_128_key_exp(key_schedule[3], 0x08);
	key_schedule[5] = AES_128_key_exp(key_schedule[4], 0x10);
	key_schedule[6] = AES_128_key_exp(key_schedule[5], 0x20);
	key_schedule[7] = AES_128_key_exp(key_schedule[6], 0x40);
	key_schedule[8] = AES_128_key_exp(key_schedule[7], 0x80);
	key_schedule[9] = AES_128_key_exp(key_schedule[8], 0x1B);
	key_schedule[10] = AES_128_key_exp(key_schedule[9], 0x36);

	// generate decryption keys in reverse order.
	// k[10] is shared by last encryption and first decryption rounds
	// k[0] is shared by first encryption round and last decryption round (and is the original user key)
	// For some implementation reasons, decryption key schedule is NOT the encryption key schedule in reverse order
	/*key_schedule[11] = _mm_aesimc_si128(key_schedule[9]);
	key_schedule[12] = _mm_aesimc_si128(key_schedule[8]);
	key_schedule[13] = _mm_aesimc_si128(key_schedule[7]);
	key_schedule[14] = _mm_aesimc_si128(key_schedule[6]);
	key_schedule[15] = _mm_aesimc_si128(key_schedule[5]);
	key_schedule[16] = _mm_aesimc_si128(key_schedule[4]);
	key_schedule[17] = _mm_aesimc_si128(key_schedule[3]);
	key_schedule[18] = _mm_aesimc_si128(key_schedule[2]);
	key_schedule[19] = _mm_aesimc_si128(key_schedule[1]);*/
}