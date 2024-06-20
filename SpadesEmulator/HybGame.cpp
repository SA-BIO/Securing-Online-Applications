#include "HybGame.h"
#include "aes128.h"
#include "haraka.h"

#include <iomanip>
#include <sstream>
#include <openssl/evp.h>
#include <openssl/err.h>


u128 one128;
u128 IV;

static inline void randU128(u128& randVal) {
    byte randBytes[16];
    for (int i = 0; i < 16; i++) {
        randBytes[i] = static_cast<byte>(rd() & 255);
    }
    randVal = LOAD_BYTES(randBytes);
}

// This codification doesn't follow the one specified in the paper. This is just an emulation of CPU usage not a real implementation.
inline static int getSuit(byte card) {
    if (card < 13) {
        return 0;
    }
    else if (card < 26) {
        return 1;
    }
    else if (card < 39) {
        return 2;
    }
    else {
        return 3;    //Spades
    }
}

/*static inline void sha256_chunks(const byte* input, int chunkNum, byte hash[32]) {
    EVP_MD_CTX* context = EVP_MD_CTX_new();
    EVP_DigestInit_ex(context, EVP_sha256(), nullptr);
    for (int i = 0; i < chunkNum; i++) {
        EVP_DigestUpdate(context, input + (i << 6), 64);
    }
    EVP_DigestFinal_ex(context, hash, 0);
    EVP_MD_CTX_free(context);
}*/

void initConstans() {
    byte oneBytes[16]{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 };
    one128 = LOAD_BYTES(oneBytes);

    u128 IV;
    randU128(IV);

    load_haraka_constants();
}

void testHybInit(GameHyb* game) {

    randU128(game->seedP1);
    std::mt19937 gen(static_cast<uint>(_mm_cvtsi128_si32(game->seedP1)));
    aes128_load_key(game->seedP1);

    byte tempDeck[DECK_SIZE * 4];
    for (int i = 0; i < DECK_SIZE * 4; i++) {
        //Note that this is not the correct card codification. It is just an example for the CPU usage emulation
        tempDeck[i] = i;
    }

    //sha init
    EVP_MD_CTX* context = EVP_MD_CTX_new();
    EVP_DigestInit_ex(context, EVP_sha256(), nullptr);

    byte shaInput_keysAux[64];
    byte shaInput_cipherAux[64] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
    byte shaInptCount_keys = 0;
    int shaInptCount_ciphers = 0;
    for (int i = 0; i < CARD_NUM; i++) {
        int randPos = distr(gen);
        tempDeck[i] = tempDeck[i] ^ tempDeck[randPos];
        tempDeck[randPos] = tempDeck[i] ^ tempDeck[randPos];
        tempDeck[i] = tempDeck[i] ^ tempDeck[randPos];

        byte cardCurrent = tempDeck[i];

        uint combinAux0[4] = { 0,0,0,i << 1 };
        u128 keyCurrentWhole[2];
        keyCurrentWhole[0] = _mm_xor_si128(IV, LOAD_BYTES(combinAux0));
        keyCurrentWhole[1] = _mm_xor_si128(keyCurrentWhole[0], one128);
        aes128_enc(keyCurrentWhole[0]);
        aes128_enc(keyCurrentWhole[1]);

        if (shaInptCount_keys) {
            haraka256_custom(keyCurrentWhole, shaInput_keysAux + 32);
            EVP_DigestUpdate(context, shaInput_keysAux, 64);
            shaInptCount_keys = 0;
        }
        else {
            haraka256_custom(keyCurrentWhole, shaInput_keysAux);
            shaInptCount_keys = 1;
        }

        byte keyCurrentLast = static_cast<byte>(_mm_cvtsi128_si32(keyCurrentWhole[0]));
        byte cipherCurrent = cardCurrent ^ keyCurrentLast;
        
        shaInput_cipherAux[shaInptCount_ciphers] = cipherCurrent;
        if (shaInptCount_ciphers == 64 || i == (CARD_NUM - 1)) {
            EVP_DigestUpdate(context, shaInput_cipherAux, 64);
            shaInptCount_ciphers = 0;
        }
        else {
            shaInptCount_ciphers++;
        }
    }

    byte shaOutput[32];
    EVP_DigestFinal_ex(context, shaOutput, 0);
    EVP_MD_CTX_free(context);
}

void testHybGame_Relay(GameHyb* game, const uint avRelayTurn, byte* randVals0, byte* randVals1, byte* res) {
    for (int i = 0; i < avRelayTurn; i++) {

        int p1Suit = getSuit(randVals0[i]);
        int p2Suit = getSuit(randVals1[i]);

        if (game->turnCount % 2 == 0) {
            if (p1Suit == p2Suit) {
                if (randVals0[i] > randVals1[i]) {
                    res[i] = 0;
                }
                else {
                    res[i] = 1;
                }
            }
            else {
                if (p2Suit == 3) {
                    res[i] = 1;
                }
                else {
                    res[i] = 0;
                }
            }
        }
        else {
            if (p1Suit == p2Suit) {
                if (randVals1[i] > randVals0[i]) {
                    res[i] = 1;
                }
                else {
                    res[i] = 0;
                }
            }
            else {
                if (p1Suit == 3) {
                    res[i] = 0;
                }
                else {
                    res[i] = 1;
                }
            }
        }
        game->turnCount++;
    }
}

bool testHybGame_Cheat(GameHyb * game, const uint avRelayTurn, byte ketToCheck) {
    aes128_load_key(game->seedP1);

    uint combinAux0[4] = { 0,0,0, avRelayTurn << 1 };
    u128 keyCurrentWhole0 = _mm_xor_si128(IV, LOAD_BYTES(combinAux0));
    aes128_enc(keyCurrentWhole0);

    byte keyCurrentLast = static_cast<byte>(_mm_cvtsi128_si32(keyCurrentWhole0));

    return keyCurrentLast == ketToCheck;
}