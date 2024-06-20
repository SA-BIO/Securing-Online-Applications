#pragma once
#include <random>
#include "utils.h"

extern std::random_device rd;
extern std::uniform_int_distribution<> distr;

class Game;

class Player {
public:
    byte* cards;
    Game* currentGame;
    uint seed;

    Player() {
        cards = new byte[CARD_NUM];
    }
    ~Player() {
        delete[] cards;
    }
    void generateDeck() {
        std::mt19937 gen(seed);

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
};

class Game {
public:
    Player* player1;
    Player* player2;

    int scoreP1;
    int scoreP2;
    int turnCount;
    byte playerCardP1;
    byte playerCardP2;

    Game() {
        player1 = nullptr;
        player2 = nullptr;
        scoreP1 = 0;
        scoreP2 = 0;
        turnCount = 0;
        playerCardP1 = 0;
        playerCardP2 = 0;
    }

    bool playTurn(byte p1Play, byte p2Play) {
        if (p1Play > CARD_NUM || p2Play > CARD_NUM) { return false; }

        playerCardP1 = player1->cards[p1Play];
        playerCardP2 = player2->cards[p2Play];

        if (turnCount == TURN_NUM) {
            return true;
        }
        else {
            turnCount++;
            return false;
        }
    }

    ~Game() {
        delete player1;
        delete player2;
    }
};


void testBaseInit(Game* game, bool playerOneInit);
void testBaseGame(Game* game, uint* randVals);