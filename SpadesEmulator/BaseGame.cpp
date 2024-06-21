#include "BaseGame.h"


static byte fullDeck[DECK_SIZE];
static int p1Suit;
static int p2Suit;

Player::Player() {
    cards = new byte[CARD_NUM];
}
Player::~Player() {
    delete[] cards;
}
void Player::generateDeck() {
    std::mt19937 gen(seed);

    for (int i = 0; i < DECK_SIZE; i++) {
        //Note that this is not the correct card codification. It is just an example for the CPU usage emulation
        fullDeck[i] = i;
    }
    for (int i = 0; i < CARD_NUM; i++) {
        int randPos = distr(gen);
        fullDeck[i] = fullDeck[i] ^ fullDeck[randPos];
        fullDeck[randPos] = fullDeck[i] ^ fullDeck[randPos];
        fullDeck[i] = fullDeck[i] ^ fullDeck[randPos];
    }
    for (int i = 0; i < CARD_NUM; i++) {
        cards[i] = fullDeck[i];
    }
}

Game::Game() {
    player1 = nullptr;
    player2 = nullptr;
    scoreP1 = 0;
    scoreP2 = 0;
    turnCount = 0;
    playerCardP1 = 0;
    playerCardP2 = 0;
}

Game::~Game() {
    delete player1;
    delete player2;
}

bool Game::playTurn(byte p1Play, byte p2Play) {
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



void testBaseInit(Game* game, bool playerOneInit) {
    if (playerOneInit) {
        Player* p1 = new Player();
        game->player1 = p1;
        p1->currentGame = game;
        p1->seed = static_cast<uint>(rd());
        p1->generateDeck();
    }
    else {
        Player* p2 = new Player();
        game->player2 = p2;
        p2->currentGame = game;
        p2->seed = static_cast<uint>(rd());
        p2->generateDeck();
    }
}


void testBaseGame(Game* game, uint* randVals) {
    for (int i = 0; i < TURN_NUM; i++) {
        game->playerCardP1 = game->player1->cards[randVals[i]];
        game->playerCardP2 = game->player2->cards[randVals[i]];

        p1Suit = getSuit(game->playerCardP1);
        p2Suit = getSuit(game->playerCardP2);

        if (game->turnCount % 2 == 0) {
            if (p1Suit == p2Suit) {
                if (game->playerCardP1 > game->playerCardP2) {
                    game->scoreP1++;
                }
                else {
                    game->scoreP2++;
                }
            }
            else {
                if (p2Suit == 3) {
                    game->scoreP2++;
                }
                else {
                    game->scoreP1++;
                }
            }
        }
        else {
            if (p1Suit == p2Suit) {
                if (game->playerCardP2 > game->playerCardP1) {
                    game->scoreP2++;
                }
                else {
                    game->scoreP1++;
                }
            }
            else {
                if (p1Suit == 3) {
                    game->scoreP1++;
                }
                else {
                    game->scoreP2++;
                }
            }
        }
        game->turnCount++;
    }
}