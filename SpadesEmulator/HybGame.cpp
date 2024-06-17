#include "HybGame.h"


static inline void randU128(u128& randVal) {
    byte randBytes[16];
    for (int i = 0; i < 16; i++) {
        randBytes[i] = static_cast<byte>(rd() & 255);
    }
    randVal = LOAD_BYTES(randBytes);
}

static inline void generateDeck(byte cards[CARD_NUM]) {
    byte tempDeck[DECK_SIZE * 4];
    for (int i = 0; i < DECK_SIZE * 4; i++) {
        //Note that this is not the correct card codification. It is just an example for the CPU usage emulation
        tempDeck[i] = i;
    }
    for (int i = 0; i < CARD_NUM; i++) {
        int randPos = distr(gen);
        tempDeck[i] = tempDeck[i] ^ tempDeck[randPos];
        tempDeck[randPos] = tempDeck[i] ^ tempDeck[randPos];
        tempDeck[i] = tempDeck[i] ^ tempDeck[randPos];
    }
    for (int i = 0; i < CARD_NUM; i++) {
        cards[i] = tempDeck[i];
    }
}

void testHybInit() {
	// generate hash of keys, generate hash of ciphers and generate hash of hash of keys

    u128 seed;
    randU128(seed);

    byte cards[CARD_NUM];
    generateDeck(cards);

    byte keys[CARD_NUM];
    byte ciphers[CARD_NUM];
    for (int i = 0; i < CARD_NUM; i++) {
        keys[i] = static_cast<byte>(rd() & 255);
        ciphers[i] = cards[i] ^ keys[i];
    }
}

void testHybGame() {

}