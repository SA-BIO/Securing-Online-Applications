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
    Player();
    ~Player();
    void generateDeck();
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

    Game();
    ~Game();
    bool playTurn(byte p1Play, byte p2Play);
};


void testBaseInit(Game* game, bool playerOneInit);
void testBaseGame(Game* game, uint* randVals);