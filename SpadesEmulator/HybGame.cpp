#include "HybGame.h"
#include "aes128.h"
#include "haraka.h"

#include <iomanip>
#include <sstream>
#include <openssl/evp.h>
#include <openssl/err.h>


static u128 one128;
static u128 IV;

static uint combinAux0[4] = {0,0,0,0};
static byte fullDeck[DECK_SIZE];

static byte shaInput_keysAux[64];
static byte shaInput_cipherAux[64] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
static byte shaInptCount_keys = 0;
static int shaInptCount_ciphers = 0;
static byte cardCurrent;
static u128 keyCurrentWhole[2];
static byte keyCurrentLast;
static byte cipherCurrent;
static byte shaOutput[32];

static int p1Suit;
static int p2Suit;
static byte turnAux;
u128 keyCurrentWhole0;


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

void testHybInit(u128& seed) {

    randU128(seed);
    std::mt19937 gen(static_cast<uint>(_mm_cvtsi128_si32(seed)));
    aes128_load_key(seed);

    for (int i = 0; i < DECK_SIZE; i++) {
        //Note that this is not the correct card codification. It is just an example for the CPU usage emulation
        fullDeck[i] = i;
    }

    //sha init
    EVP_MD_CTX* context = EVP_MD_CTX_new();
    EVP_DigestInit_ex(context, EVP_sha256(), nullptr);

    shaInptCount_keys = 0;
    shaInptCount_ciphers = 0;
    for (int i = 0; i < CARD_NUM; i++) {
        int randPos = distr(gen);
        fullDeck[i] = fullDeck[i] ^ fullDeck[randPos];
        fullDeck[randPos] = fullDeck[i] ^ fullDeck[randPos];
        fullDeck[i] = fullDeck[i] ^ fullDeck[randPos];

        cardCurrent = fullDeck[i];

        combinAux0[3] = i << 1;
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

        keyCurrentLast = static_cast<byte>(_mm_cvtsi128_si32(keyCurrentWhole[0]));
        cipherCurrent = cardCurrent ^ keyCurrentLast;
        
        shaInput_cipherAux[shaInptCount_ciphers] = cipherCurrent;
        if (shaInptCount_ciphers == 64 || i == (CARD_NUM - 1)) {
            EVP_DigestUpdate(context, shaInput_cipherAux, 64);
            shaInptCount_ciphers = 0;
        }
        else {
            shaInptCount_ciphers++;
        }
    }

    EVP_DigestFinal_ex(context, shaOutput, 0);
    EVP_MD_CTX_free(context);
}

void testHybGame_Relay(const uint avRelayTurn, byte* randVals0, byte* randVals1, bool* testComp, byte* res) {
    for (int i = 0; i < avRelayTurn; i++) {

        p1Suit = getSuit(randVals0[i]);
        p2Suit = getSuit(randVals1[i]);

        turnAux = randVals0[i] & 128; //the first bit of our message byte is converted to wich player's turn it is
        testComp[i] = (turnAux == (randVals1[i] & 128)); //check if both players agree on wich players turn it is

        if (turnAux) {
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
    }
}

bool testHybGame_Cheat(u128& seed, const uint avRelayTurn, byte ketToCheck) {
    aes128_load_key(seed);

    combinAux0[3] = avRelayTurn << 1;
    keyCurrentWhole0 = _mm_xor_si128(IV, LOAD_BYTES(combinAux0));
    aes128_enc(keyCurrentWhole0);

    keyCurrentLast = static_cast<byte>(_mm_cvtsi128_si32(keyCurrentWhole0));
    return keyCurrentLast == ketToCheck;
}